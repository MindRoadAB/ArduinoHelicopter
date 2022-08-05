#include <BMI160Gen.h>
//Project requires Emotibit BMI160 library
//I might have modified the library myself.

//For the IR LED
const int outpin = 3;

//Increase, decrease throttle buttons.
const int incPin = 4;
const int decPin = 5;

byte base[4] = {63, 63, 15, 63};

//TODO: Channel select
int channel;
int count = 0;
unsigned int freq = 38000;

//Start control at this step
byte throttle = 15;
//Each throttle step.
//TODO: Test with smaller intervals, though response can end up sluggish. 
byte step = 5;


//Comment out if the gyro is not connected
//#define GOTGY

const int i2c_addr = 0x68;

void setup() {
  pinMode(outpin, OUTPUT);
  pinMode(decPin, INPUT);
  pinMode(incPin, INPUT);
  int channel = 0;
  Wire.begin();
  Serial.begin(9600);
  while (!Serial);

  //Gyro-setup
  //Sets the "zero" position
  //TODO: Setup some way to calibrate "zero" while running, both buttons together perhaps?
#ifdef GOTGY
  BMI160.begin(BMI160GenClass::I2C_MODE, Wire, i2c_addr);
  BMI160.setGyroRange(250);
  BMI160.setAccelerometerRange(8);
  BMI160.autoCalibrateXAccelOffset(0);
  BMI160.autoCalibrateYAccelOffset(0);
  BMI160.autoCalibrateGyroOffset();
#endif
  

  sync();
}
//To connect with the helicopter
void sync() {
  byte init[] = {63,63,0,63};

  //TODO: Try to remove the '127' step to avoid the litte skip it starts with
  byte change[] = {20, 60, 94,  78, 35, 0, 0};
  //Throttle up then down to sync with the helicopter
  for(int i = 0; i < 6; i++) {
    transmit(init);
    init[2] = change[i];
    delay(120);

  }
  delay(700);
  
}

void increaseThrottle() {
  if(throttle < 127 - step) {
    throttle = throttle + step;
  }
    
}

void decreaseThrottle() {
  if(throttle > step-1) {
    throttle = throttle - step;
  }
}


//Total range of gyro output is  +/- ~5500
//Cap that to 3465 to lower angle needed for reactions to happen
//3465 is 55*127, keep that relationship to make calculations easier. 
//The values can be modified to test different responses
int xLim = 3465;
int xDiv = 55;
int yLim = 3465;
int yDiv = 55;

void loop() {
  if(digitalRead(incPin) == HIGH) {
    increaseThrottle();
  } else if (digitalRead(decPin) == HIGH) {
    decreaseThrottle();
  }
#ifdef GOTGY
  int gxRaw, gyRaw, gzRaw;
  //Appears to output the gyro measurements
  BMI160.readAccelerometer(gxRaw, gyRaw, gzRaw);


   //Assume max value is xLim, yLim, gives about 30deg in each direction
  if(gxRaw > xLim) {
    gxRaw = xLim;
  } else if (gxRaw < -xLim) {
    gxRaw = -xLim;
  }
  gyRaw = -gyRaw;
  if(gyRaw > yLim) {
    gyRaw = yLim;
  } else if(gyRaw < -yLim) {
    gyRaw = -yLim;
  }
  //Convert from scale of xLim, yLim to 127 value scale.
  //Add 63 to center the value according to protocol.
  float pitch = (gxRaw/xDiv)+63;
  float yaw = (gyRaw/xDiv)+63;
  base[0] = byte(yaw);
  base[1] = byte(pitch);
#endif
  base[2] = throttle;

  Serial.print("Commands P:");
  Serial.print(base[1]);
  Serial.print(" Y:");
  Serial.print(base[0]);
  Serial.print(" T:");
  Serial.print(base[2]);
  Serial.println();

  transmit(base);
  delay(120);
  
}
/*
|01234567 01234567 01234567 01234567|
|a-YAW--- b-PITCH- C-THRTTL d-TRIM  |
Last byte, trim, not used, trim value is added/removed from YAW
C is the channel flag
*/
void transmit(byte data[]) {
    sendHead();
    for(int part = 0; part < 4; part++) {
    //Handle flags, first bit in each Byte
    if(part == 2) {
      if(channel==0) {
        sendZero();
      } else {
        sendOne();
      }
    } else {
      sendZero();
    }
    //Then the control data, 7 bits
    for(int i = 0; i < 7; i++) {
      if(bitRead(data[part], 6-i) == 0) {
        sendZero();
      } else {
        sendOne();
      }
    }
  }
  sendFoot();
}

void sendFoot() {
  tone(outpin, freq);
  delayMicroseconds(300);
  noTone(outpin);
  digitalWrite(outpin, LOW);
  delayMicroseconds(2000);
}

void sendHead() {
  tone(outpin, freq);
  delayMicroseconds(2000);
  noTone(outpin);
  digitalWrite(outpin, LOW);
  delayMicroseconds(2000);


}

void sendZero() {
  tone(outpin, freq);
  delayMicroseconds(300);
  noTone(outpin);
  digitalWrite(outpin, LOW);
  delayMicroseconds(300);

  

}
void sendOne() {
  
  tone(outpin, freq);
  delayMicroseconds(300);
  noTone(outpin);
  digitalWrite(outpin, LOW);
  delayMicroseconds(700);

}


