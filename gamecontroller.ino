
#include <SoftwareSerial.h>
#include <arduino-timer.h>

auto timer = timer_create_default();

const int a_button = 7;
const int b_button = 4;
const int motor = 11;
const int SW_pin = 2; // digital pin connected to switch output
const int X = 0; // analog pin connected to X output
const int Y = 1; // analog pin connected to Y output
const int select = 8;
const int start = 12;

const int deadzone = 200; //200 + neutral and neutral - 200 are the deadzones
const int neutral = 500;
int x_axis;
int y_axis;

const int min_rumble = 30;

bool enable_rumble;
String a = "";

struct rumble_opt  {
  bool isSpindashing;
  bool mario() {

    return (digitalRead(b_button) == HIGH) && ((x_axis > 500) || (x_axis < 500));
  } //Mario rumble mode. vibrates when mario runs (when b and direction is held)
  bool sonic() {
    if (((digitalRead(a_button) == HIGH) || (digitalRead(b_button) == HIGH)) && (y_axis < 500)) {
      isSpindashing = true;
    } else if (y_axis >= 500) {
      isSpindashing = false;
    }
    return isSpindashing;
  } //Sonic rumble mode. Vibrates when spindash is charging.
  bool tetris(){
    return (y_axis > 500);
  } //Tetris rumble mode. Vibrates when using the hard drop.
};
rumble_opt rumble_mode;
int mode;
//SoftwareSerial Serial(0, 1);

double vibes;

void setup() {
  pinMode(SW_pin, INPUT);
  pinMode(X, INPUT);
  pinMode(Y, INPUT);

  pinMode(a_button, INPUT_PULLUP);
  pinMode(b_button, INPUT_PULLUP);
  pinMode(select, INPUT_PULLUP);
  pinMode(start, INPUT_PULLUP);

  pinMode(motor, OUTPUT);
  Serial.begin(9600);
  Serial.begin(9600);
  select_mode();
  
}

void select_mode() {
  while (true) {
  if (Serial.available() > 0) {
    char startChar = Serial.read();
    if (startChar == 'C') {  // 'C' indicates a command
      mode = Serial.parseInt();
      if (mode < 0 && mode > 2) {
        mode = 0; //choose mario rumble by default
        // Control vibrator strength
      }
      return;
    }
  }
  }
}
bool increment_vibes(void *) {
  vibes += 2;
  return true;
}
bool increase_vibes_b_4(void *) {
  vibes += 4;
  return true;
}
void mario_rumble() {
  if (vibes < 200) {
    timer.in(10, increment_vibes);
  }
}
void sonic_rumble() {
  if (vibes < 200) {
    timer.in(10, increase_vibes_b_4);
  }
}

void tetris_rumble() {
  vibes = 200;
}

void rumble() {
  switch (mode) {
    case 0:

      if (rumble_mode.mario()) {
        mario_rumble();
      } else if (vibes > min_rumble) {
        vibes-= 5;
      }
      break;
    case 1:
      if (rumble_mode.sonic()) {
        sonic_rumble();
      } else if (vibes > min_rumble) {
        vibes-=20;
      }
      break;
    case 2:
      if (rumble_mode.tetris()) {
        tetris_rumble();
      } else if (vibes > min_rumble) {
        vibes = 20;
      }
      break;
    default:
      vibes = min_rumble;
      break;
  }
}


void clean_stick_input(int &x_axis, int &y_axis) {
  if (analogRead(X) <= deadzone + neutral && analogRead(X) >= neutral - deadzone) {
    x_axis = neutral;
  } else {
    x_axis = analogRead(X);
  }
  if (analogRead(Y) <= deadzone + neutral && analogRead(Y) >= neutral - deadzone) {
    y_axis = neutral;
  } else {
    y_axis = analogRead(Y);
  }
}

void output_controller_data() 
{
  //digitalWrite(motor, HIGH);
  // if (digitalRead(a_button) == HIGH) {
  //   Serial.println("A");
  // }
  // if (digitalRead(b_button) == HIGH) {
  //   Serial.println("B");
  // }
  // if (digitalRead(SW_pin) == HIGH) {
  //   Serial.println("JOYCLICK");
  // }

  clean_stick_input(x_axis, y_axis);
 
  Serial.print("X-AXIS:");
  Serial.print(x_axis);
  Serial.print("Y-AXIS:");
  Serial.print(y_axis);
  Serial.print("JS:");
  Serial.print(analogRead(SW_pin));
  Serial.print("A");
  Serial.print(digitalRead(a_button));
  Serial.print("B");
  Serial.print(digitalRead(b_button));
  Serial.print("start:");
  Serial.print(digitalRead(start));
  Serial.print("select:");
  Serial.println(digitalRead(select));



  
}
void loop() {
  
  //if (await_input()) {
    
    timer.tick();
    rumble();
    output_controller_data();
    analogWrite(motor, (int)vibes);
    if (vibes < min_rumble) {
      vibes = min_rumble;
    }
    //Serial.println(vibes);
    //Serial.println(a == "er\n");
    //if (enable_rumble){
    
    //}
    
  //}
}