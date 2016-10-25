/*
Camera rotator for remote monitoring system
 
 Is camServo.write() function active during whole rotation? If so...
 Does calling camServo.read() while still rotating interrupt action?
 --> Important for checkMovement() function!
 
 Calling SoftwareSerial for GPS, but can use Serial1-3 with Mega.
 
 To Do:
 - Create single function for input desired servo position and slow movement to that position
 */

#define DebugMode
//#define Mega

#include <Servo.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "IntersemaBaro.h"
#include <TinyGPS.h>

#define servoPin 5  // PWM-capable pin that will serve as servo input pin
#define gpsPin 6  // Pin for GPS SoftwareSerial connection
#define pirPin 4  // Pin for PIR (motion sensor) input

Intersema::BaroPressure_MS5607B baro(true);

#define altOffset 4515  // Negative offset in feet of altimeter to output actual altitude values

#define compAddr 0x1E  // Hexidecimal address of compass on i2c interface

#define servoMin 5  // Minimum point of servo travel range without motor strain
#define servoMax 179  // Maximum point of servo travel range without motor strain
const int servoMid = (servoMax - servoMid) / 2;  // Middle point of servo travel range without motor strain
#define servoDelay 5000  // Delay to allow servo to move to required position

#define slowServoDelay 20
// slowServoDelay controls rate of rotation when travelling over the full range.
 //Times from min to max/max to min by delay time (ms) listed below:
 //10 - 1.8s
 //20 - 3.6s
 //100 - 18s
 //etc.

TinyGPS gps;
SoftwareSerial gpsSerial(gpsPin, 255);

Servo camServo;

int compX, compY, compZ;
int compXMin, compXMax;

float altFt;

void calibrateServo() {
  int compXMinAvg, compXMaxAvg;
  if(camServo.read() == 179) servoToMinSlow();
  else camServo.write(servoMin);
  delay(servoDelay);
  //checkMovement();
  for(int xMin = 1; xMin++; xMin < 6) {
    getCompData();
    compXMinAvg = compXMinAvg + compX;
  }
  compXMin = compXMinAvg / 5;
  servoToMaxSlow();
  //camServo.write(servoMax);
  //delay(servoDelay);
  //checkMovement();
  for(int xMax = 1; xMax++; xMax < 6) {
    getCompData();
    compXMaxAvg = compXMaxAvg + compX;
  }
  compXMax = compXMaxAvg / 5;
}

void checkCalib() {
  servoToMinSlow();
  //camServo.write(servoMin);
  //delay(servoDelay);
  //checkMovement();
  getCompData();
  if(compX < compXMin - 10 || compX > compXMin + 10) {
#ifdef DebugMode
    Serial.println(F("compXMin incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  servoToMaxSlow();
  //camServo.write(servoMax);
  //delay(servoDelay);
  //checkMovement(); 
  getCompData();
  if(compX < compXMax - 10 || compX > compXMax + 10) {
#ifdef DebugMode
    Serial.println(F("compXMax incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  camServo.write(servoMid);
  delay(servoDelay);
  //checkMovement();
}

void setup() {
  Serial.begin(19200);
#ifdef Mega
  Serial1.begin(4800);
#endif
#ifndef Mega
  gpsSerial.begin(4800);
#endif
  //delay(100);  // Power-up delay
  Wire.begin();
#ifdef debugMode
  Serial.println(F("I2C initiated."));
#endif
  baro.init();
#ifdef debugMode
  Serial.println(F("Altimeter initiated."));
#endif
  camServo.attach(servoPin);
#ifdef debugMode
  Serial.println(F("Servo attached."));
  Serial.print(F("Configuring compass.."));
#endif
  Wire.beginTransmission(compAddr);
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();
#ifdef debugMode
  Serial.println(F("compass configured."));
  Serial.print(F("Calibrating servo..."));
#endif
  calibrateServo();
#ifdef debugMode
  Serial.println(F("servo calibrated."));
  Serial.print(F("Checking calibration..."));
#endif
  checkCalib();
#ifdef debugMode
  Serial.println(F("calibration confirmed."));
#endif

  getCompData();
  getAltFt();

  #ifdef DebugMode
  Serial.print(F("Current heading is: "));
  Serial.println(compX);
  Serial.print(F("Current altitude is: "));
  Serial.println(altFt);
  
  Serial.println(F("calibration complete. Waiting for ArduinoConnectServer command."));
#endif
}

void loop() {
  // Main loop goes here? Heh...
}

// Servo speed control functions
void servoToMinSlow() {
  for(int maxPos = servoMax; maxPos--; maxPos > (servoMin - 1)) {
    camServo.write(maxPos);
    delay(slowServoDelay);
  }
}

void servoToMaxSlow() {
  for(int minPos = servoMin; minPos++; minPos < (servoMax + 1)) {
    camServo.write(minPos);
    delay(slowServoDelay);
  }
}

void servoToMidSlow() {
  int distToMid = 0;
  int servoInitPos = camServo.read();
  if(servoInitPos == servoMid) break;
  else if(servoInitPos < servoMid) {
    for(int minDist = servoInitPos; minDist++; minDist
  }
  else if(camServo.read() > servoMid) {
  }
#ifdef debugMode
  else {
    Serial.println(F("Call to camServo.read() returned unrecognized value."));
  }
#endif
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

void getAltFt() {
#ifdef averageData
  int altFtAvg;
  for(x = 0; x++; x < 3) {
    altFtAvg = altFtAvg + ((baro.getHeightCentiMeters() / 30.48) - 1515);
  }
  altFt = altFtAvg / 3;
#endif
#ifndef averageData
  altFt = (baro.getHeightCentiMeters() / 30.48) - altOffset;
#endif
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

// GPS functions below this line. Need to be configured before use
void getGPSData() {
  if(feedgps()) gpsdump(gps);
  else {
    Serial.println(F("GPS data unavailable."));
  }
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4); 
  Serial.print(", "); 
  Serial.println(flon, 4);
}

// Feed data as it becomes available 
boolean feedgps() {
#ifdef Mega
  while(Serial1.available()) {
    if(gps.encode(Serial1.read())) {
      return true;
    }
  }
#endif
#ifndef Mega
  while(gpsSerial.available()) {
    if(gps.encode(gpsSerial.read())) {
      return true;
    }
  }
#endif
  return false;
}
