/*  Camera rotator for remote monitoring system
 
 Is camServo.write() function active during whole rotation? If so...
 Does calling camServo.read() while still rotating interrupt action?
 --> Important for checkMovement() function!
 
 */

#define DebugMode

#include <Servo.h>
#include <Wire.h>

#define compAddr 0x1E

#define servoMin 5  // Minimum point of servo travel range without motor strain
#define servoMax 179  // Maximum point of servo travel range without motor strain
#define servoPin 9  // PWM-capable pin that will serve as servo input pin
#define servoDelay 5000  // Delay to allow servo to move to required position

Servo camServo;

int compX, compY, compZ;
int compXMin, compXMax;

void calibrateServo() {
  int compXMinAvg, compXMaxAvg;
  camServo.write(servoMin);
  //delay(servoDelay);
  checkMovement();
  for(int xMin = 1; xMin++; xMin < 6) {
    getCompData();
    compXMinAvg = compXMinAvg + compX;
  }
  compXMin = compXMinAvg / 5;
  camServo.write(servoMax);
  //delay(servoDelay);
  checkMovement();
  for(int xMax = 1; xMax++; xMax < 6) {
    getCompData();
    compXMaxAvg = compXMaxAvg + compX;
  }
  compXMax = compXMaxAvg / 5;
}

void checkCalib() {
  camServo.write(servoMin);
  //delay(servoDelay);
  checkMovement();
  getCompData();
  if(compX < compXMin - 10 || compX > compXMin + 10) {
#ifdef DebugMode
    Serial.println(F("compXMin incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  camServo.write(servoMax);
  //delay(servoDelay);
  checkMovement();
  getCompData();
  if(compX < compXMax - 10 || compX > compXMax + 10) {
#ifdef DebugMode
    Serial.println(F("compXMax incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  int servoMid = (servoMax - servoMin) / 2;
  camServo.write(servoMid);
  //delay(servoDelay);
  checkMovement();
#ifdef DebugMode
  Serial.println(F("Calibration complete. Waiting for ArduinoConnectServer command."));
#endif
}

void setup() {
  Serial.begin(19200);
  delay(100);  // Power-up delay
  Wire.begin();
  camServo.attach(servoPin);

  Wire.beginTransmission(compAddr);
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();

  calibrateServo();
  checkCalib();
}

void loop() {
  // Main loop goes here? Heh...
}

void getCompData() {
  // Initiate communications with compass
  Wire.beginTransmission(compAddr);
  Wire.write(byte(0x03));       // Send request to X MSB register
  Wire.endTransmission();

  Wire.requestFrom(compAddr, 6);    // Request 6 bytes; 2 bytes per axis
  if(Wire.available() <=6) {    // If 6 bytes available
    compX = Wire.read() << 8 | Wire.read();
    compZ = Wire.read() << 8 | Wire.read();
    compY = Wire.read() << 8 | Wire.read();
  }
}

void checkMovement() {
  boolean servoStill = false;
  while(servoStill == false) {
    int servoPosLast = camServo.read();
    delay(100);
    int servoPos = camServo.read();
    int servoPosDiff = servoPosLast - servoPos;
    if(-5 > servoPosDiff < 5) servoStill = true;
  }
}
