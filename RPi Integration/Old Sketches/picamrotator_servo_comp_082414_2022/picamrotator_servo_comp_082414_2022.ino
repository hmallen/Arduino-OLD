#define DebugMode

#include <Servo.h>
#include <Wire.h>

#define compAddr 0x1E

#define servoDelay 5000  // Delay to allow servo to move to required position

Servo camServo;

int compX, compY, compZ;
int compMin, compMax;

void calibrateServo {
  int compXMinAvg, compXMaxAvg;
  camServo.write(0);
  delay(servoDelay);
  for(int xMin = 1; xMin++; xMin < 6) {
    getCompData();
    compXMinAvg = compXMinAvg + compX;
  }
  compXMin = compXMinAvg / 5;
  camServo.write(179);
  delay(servoDelay);
  for(int xMax = 1; xMax++; xMax < 6) {
    getCompData();
    compXMaxAvg = compXMaxAvg + compX;
  }
  compXMax = compXMaxAvg / 5;
}

void checkCalib() {
  camServo.write(0);
  delay(servoDelay);
  getCompData();
  if(compX < compXMin - 10 || compX > compXMin + 10) {
#ifdef DebugMode
    Serial.println(F("compXMin incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  camServo.write(180);
  delay(servoDelay);
  getCompData();
  if(compX < compXMax - 10 || compX > compXMax + 10) {
#ifdef DebugMode
    Serial.println(F("compXMax incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  camServo.write(89);
  delay(servoDelay);
  #ifdef DebugMode
  Serial.println(F("Calibration complete. Waiting for ArduinoConnectServer command."));
  #endif
}

void setup() {
  Serial.begin(19200);
  delay(100);  // Power-up delay
  Wire.begin();

  Wire.beginTransmission(compAddr);
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();

  calibrateServo();
  checkCalib();
}

void loop() {
}

void getCompData() {
  // Initiate communications with compass
  Wire.beginTransmission(Addr);
  Wire.write(byte(0x03));       // Send request to X MSB register
  Wire.endTransmission();

  Wire.requestFrom(Addr, 6);    // Request 6 bytes; 2 bytes per axis
  if(Wire.available() <=6) {    // If 6 bytes available
    compX = Wire.read() << 8 | Wire.read();
    compZ = Wire.read() << 8 | Wire.read();
    compY = Wire.read() << 8 | Wire.read();
  }
}

