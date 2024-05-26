#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/uinput.h>
#include <string.h>
#include <errno.h>
#include <iostream>
int open_serial_port(const char* device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_serial_port: Unable to open port ");
        return -1;
    }
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

void read_serial_data(int fd, int *x_axis, int *y_axis, int *joy_click, int *a, int *b, int *start, int *select)
{
  char buffer[256];
  int n;

  n = read(fd, buffer, 255);
  if (n > 0)
  {
    buffer[n] = '\0';

    sscanf(buffer, "X-AXIS:%dY-AXIS:%dJS:%dA%dB%dstart:%dselect:%d", x_axis, y_axis, joy_click, a, b, start, select);
  } else {
    perror("couldn't read serial data");
  }
}
int setup_uinput_device() {
  int fd = open("/dev/uinput", O_RDWR);
  if (fd < 0) {
    perror("Unable to open /dev/uinput");
    return -1;
  }

  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));
  strncpy(uidev.name, "Arduino Joystick", UINPUT_MAX_NAME_SIZE);
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = 0x1234;  // Sample vendor
  uidev.id.product = 0x5678; // Sample product
  uidev.id.version = 1;

  // Set up axes
  ioctl(fd, UI_SET_EVBIT, EV_ABS);
  ioctl(fd, UI_SET_ABSBIT, ABS_X);
  ioctl(fd, UI_SET_ABSBIT, ABS_Y);
  uidev.absmin[ABS_X] = -32768;
  uidev.absmax[ABS_X] = 32767;
  uidev.absmin[ABS_Y] = -32768;
  uidev.absmax[ABS_Y] = 32767;

  // Set up buttons
  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  ioctl(fd, UI_SET_KEYBIT, BTN_A);
  ioctl(fd, UI_SET_KEYBIT, BTN_B);
  ioctl(fd, UI_SET_KEYBIT, BTN_START);
  ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);
  ioctl(fd, UI_SET_KEYBIT, BTN_THUMBL); 


  if (write(fd, &uidev, sizeof(uidev)) < 0) {
    perror("Error writing to uinput device");
    close(fd);
    return -1;
  }

  if (ioctl(fd, UI_DEV_CREATE) < 0) {
    perror("Error creating uinput device");
    close(fd);
    return -1;
  }

  return fd;
}


void send_event(int fd, unsigned int type, unsigned int code, int value)
{
  struct input_event ev;
  memset(&ev, 0, sizeof(struct input_event));
  ev.type = type;
  ev.code = code;
  ev.value = value;
  if (write(fd, &ev, sizeof(struct input_event)) < 0)
  {
    perror("Error sending event");
  }
}

void send_abs_event(int fd, unsigned int code, int value)
{
  send_event(fd, EV_ABS, code, value);
  send_event(fd, EV_SYN, SYN_REPORT, 0);
}


int main()
{
  printf("starting.\n");
  int serial_port = open_serial_port("/dev/ttyACM0");
  printf("port opened.\n");
  int uinput_fd = setup_uinput_device();
  printf("input setup.\n");

  int rumble_strength = 250;
  int x = 0, y = 0, jc = 0, a = 0, b = 0, start = 0, select = 0;
  if (serial_port == -1 || uinput_fd == -1)
  {
    return -1;
  }
  while (true)
  {
    read_serial_data(serial_port, &x, &y, &jc, &a, &b, &start, &select);

    int scaled_x = (int)((x / 1023.0) * 65535 - 32768);
    int scaled_y = (int)((y / 1023.0) * 65535 - 32768);

    std::cout<<scaled_x<<"\n";

    send_abs_event(uinput_fd, ABS_X, scaled_x);
    send_abs_event(uinput_fd, ABS_Y, scaled_y);

    // Send button events
    send_event(uinput_fd, EV_KEY, BTN_A, a);
    send_event(uinput_fd, EV_KEY, BTN_B, b);
    send_event(uinput_fd, EV_KEY, BTN_START, start);
    send_event(uinput_fd, EV_KEY, BTN_SELECT, select);
    send_event(uinput_fd, EV_KEY, BTN_THUMBL, jc);

    send_event(uinput_fd, EV_SYN, SYN_REPORT, 0);

  }
  close(serial_port);
  return 0;
}
