#include <BMI160Gen.h>
//Project requires Emotibit BMI160 library.
//Modified library is included in Git repository.

//For the IR LED
const int outpin = 2;
//Increase, decrease throttle buttons.
const int incPin = 4;
const int decPin = 5;

//Default, select by using ground as signal
const int i2c_addr = 0x68;

//Channel A has 120ms pause, B has 180ms
const int chanAtime = 120;
const int chanBtime = 180;
int channel = 0; //Start on channel A
int delayms = 120;


//Frequency that the helicopter communicates over (KHz).
const unsigned int freq = 38000;

byte base[4] = {63, 63, 15, 63};

//Each throttle step.
byte step = 5;
//Start control at this step.
byte throttle = step*3;



void setup() {

  bool channelChosen = false;

  pinMode(outpin, OUTPUT);
  pinMode(decPin, INPUT);
  pinMode(incPin, INPUT);
  channelSelect();
  Wire.begin();
  Serial.begin(9600);
  while (!Serial);

  //Gyro-setup
  //Sets the "zero" position
  //TODO: Setup some way to calibrate "zero" while running, both buttons together perhaps?
  digitalWrite(7, HIGH);
  BMI160.begin(BMI160GenClass::I2C_MODE, Wire, i2c_addr);
  BMI160.setGyroRange(250);
  BMI160.setAccelerometerRange(8);
  BMI160.autoCalibrateXAccelOffset(0);
  BMI160.autoCalibrateYAccelOffset(0);
  BMI160.autoCalibrateGyroOffset();
  
  sync();
}
//To connect with the helicopter
void sync() {
  byte init[] = {63,63,0,63};
  
  byte change[] = {20, 60, 94,  78, 35, 0, 0};
  //Throttle up then down to sync with the helicopter
  for(int i = 0; i < 6; i++) {
    transmit(init);
    init[2] = change[i];
    delay(120);

  }
  delay(700);
  
}

void channelSelect() {
  bool ledLit = false;
  bool channelChosen = false;
  while(!channelChosen) {
    //Blink built in LED while choosing
    if(ledLit) {
      digitalWrite(LED_BUILTIN, LOW);
      ledLit = false;      
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
      ledLit = true;
    }
    if(digitalRead(incPin) == HIGH) {
      delayms = chanAtime;
      channel = 0;    
      channelChosen = true;
    } else if(digitalRead(decPin) == HIGH) {
      delayms = chanBtime;  
      channelChosen = true;
      channel = 1;
    }
    delay(400);
  }
  //Blink twice for A, blink thrice for B.
  digitalWrite(LED_BUILTIN, LOW);
  if(delayms == chanAtime) {
    blink();
    blink();
  } else {
    blink();
    blink();
    blink();
  }
}

void blink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
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


//Total range of gyro output is  +/- ~5500.
//The values can be modified to test different responses.
//Cap that value to lower angle needed for reactions to happen since ~5500 equals 90deg card angle.
//3465 is 55*63, keep that relationship to make calculations easier. 
// So xLim = xDiv*63

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
  int gxRaw, gyRaw, gzRaw;
  //Appears to output the gyro measurements
  BMI160.readAccelerometer(gxRaw, gyRaw, gzRaw);


   //Assume max value is xLim, yLim, gives about 30deg control deflection in each direction
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
  base[2] = throttle;

  transmit(base);
  Serial.print("Commands P:");
  Serial.print(base[1]);
  Serial.print(" Y:");
  Serial.print(base[0]);
  Serial.print(" T:");
  Serial.print(base[2]);
  Serial.println();
  
  delay(delayms);
  
}
/*
|01234567 01234567 01234567 01234567|
|a-YAW--- b-PITCH- C-THRTTL d-TRIM  |
Last byte, trim, not used, trim value is added/removed from YAW
C is the channel flag
other flags are not used
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
    //Then the control data, 7 bits, reverse order
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


