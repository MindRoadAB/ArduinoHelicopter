# ArduinoHelicopter
## About
A program for controlling a Syma S5 IR Helicopter, by using a gyroscope-equipped board.

The board this program is designed for uses an Arduino Nano card, a BMI160 combined gyro/accelerometer, a pair of buttons and an IR diode with some resistors.  The image below shows an example card.

<img src="https://user-images.githubusercontent.com/38464472/183080042-ac7336fb-93f9-4750-8bad-834aea5968e7.jpg" height="350">

## Using the program

To use and modify the program, the [Arduino editor](https://www.arduino.cc/en/software) needs to be installed.

To interface with the BMI160 chip, a library is required. A basic library is slightly modified for this project, but fits seamlessly into the regular Arduino editor. Just drop the "EmotiBit_BMI160" folder into the Arduino library folder (.../Documents/Arduino/libraries/ by default on Windows).

To then use the program, open the project file, *ArduinoHelicopter.ino* , found in the *GyroControl* folder. When this is open, connect your board to your computer using a serial cable, make sure that the Arduino editor knows which serial port to connect to and which type of board is there. COM1 can appear when using certain computers and is usually an internal connection and is not the one to use. At this point, you can upload the program to your board and try it out.

The board will start transmitting as soon as it is started, as well as setting its current attitude as the *Zero* position. So when starting or hitting *Reset*, make sure the board is level and the IR diode points towards the helicopter. This is to make sure the synchronization happens, which is noticed by the helicopter spooling up, then back down again. At this point, the buttons can be used to control the throttle and angling the board controls the pitch and yaw. 

## Modifying the program

The default pins for the program might not fit how you have installed all the components on your board. To change these, all the variables are present near the top of the project file. 

To edit how the angle of the board translates into commands for the helicopter, there are a few variables that can be changed. These are the *\*Lim* and *\*Div* variables. These variables control the maximum angle to be translated and adhere to a strict relationship, where *\*Lim* = *\*Div* / 63, which simplifies calculations immensely.

The throttle response the can be modified as well. A larger step means every 120 milliseconds, the throttle is increased by that amount up to 127. The base throttle setting is a multiple of the step to make calculations easy as well. Small steps are recommended if you want it easy to acheive a steady hover.


## TODO
- Setup channel handling. Currently, the program only uses a single channel.
- Test different throttle intervals.
- Create way to recalibrate the zero position


## Extras
The entire protocol for the transmitter/receiver communication can be found at http://www.jimhung.co.uk/wp-content/uploads/2013/01/Syma107_ProtocolSpec_v1.txt

