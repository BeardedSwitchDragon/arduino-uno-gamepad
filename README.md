# arduino-uno-gamepad
Creates a linux Arduino uno gamepad via serial port communication, using Linux's "uinput". (Consequently, can only be run on linux). Made for SIT111
I suggest using this with a GBA emulator (namely mGBA), as I found the most consistent success with this.

(More info coming soon)

## Rumble mode
In `gamecontroller.ino`, there is a function called `select_mode()`, which enables hard-coded rumble for 3 types of games, Mario, Sonic and Tetris (commands `C0`, `C1` and `C2` respectively). The controller won't send inputs without setting a rumble mode first, which can be done by sending input through the serial monitor. **Omit the line `select_mode()` in `setup()` to disable.** 

Whenever I set the rumble mode, I had to restart my machine (I assume due to some conflict with multiple serial connections), this may be because I connected to the serial port and entered commands through the `minicom` terminal command as opposed to the Arduino IDE.

## Circuits
This is what I used for my Arduino circuit, it's a bit of a rat's nest, but you can come up with your own and substitute the pins in `gamecontroller.ino`.
![Swanky Blad-Habbi](https://github.com/BeardedSwitchDragon/arduino-uno-gamepad/assets/26681093/973622e4-573b-466c-9299-2f50568f102c)

## Setting it up on Linux
**Again, this only works on linux**, just making it apparent. 

Find the location of your Arduino on your device, in the terminal/command line type `ls /dev/tty` don't press enter, but **press** `Tab` twice - and the Arduino device should come up under `/dev/ttyACM0` or something similar. If you're unsure, unplug the Arduino and do this step again, and find which device is missing from the list. In serial-to-input.cpp, replace `/dev/ttyACM0` in `main()` with the path of the arduino device.

Compile serial-to-input.cpp, for me, I used `clang++` but I think any compiler should work. `sudo` run the executable (needs super user permissions as it creates a new virtual device), and linux should now be reading the Arduino as a game controller! You can run `evtest` or any gamepad tester website to see if it works. **keep the program running!!** otherwise no controller will be detected.
