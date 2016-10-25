/*
Camera rotator for remote monitoring system
 
 Is camServo.write() function active during whole rotation? If so...
 Does calling camServo.read() while still rotating interrupt action?
 --> Important for checkMovement() function!
 
 Calling SoftwareSerial for GPS, but can use Serial1-3 with Mega.
 
 To Do:
 - Create single function for input desired servo position and slow movement to that position
 */

#define debugMode
//#define Mega  // Uses Serial1 for GPS connection instead of SoftwareSerial
//#define averageData  // Takes an average of 3 read from sensors before outputting (currently only implemented w/ altimeter)

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

const int servoMin = 5;  // Minimum point of servo travel range without motor strain
const int servoMax = 179;  // Maximum point of servo travel range without motor strain-
const int servoMid = (servoMax - servoMid) / 2;  // Middle point of servo travel range without motor strain
const unsigned int servoDelay = 5000;  // Delay to allow servo to move to required position

const unsigned int slowServoDelay = 20;
/* 
 Controls rate of rotation when travelling over the full range.
 Times from min to max/max to min by delay time (ms) listed below:
 10 - 1.8s
 20 - 3.6s
 100 - 18s
 etc.
 */

TinyGPS gps;
SoftwareSerial gpsSerial(gpsPin, 255);

Servo camServo;

float flat, flon;

int compX, compY, compZ;
int compXMin, compXMax;

float altFt;

void calibrateServo() {
  int compXMinAvg = 0;
  int compXMaxAvg = 0;

  // Compass calibration at minimum of servo travel range
  camServo.write(servoMin);
#ifdef debugMode
  Serial.println(F("Calibrating compass at servo minimum..."));
#endif
  delay(servoDelay);
  for(int xMin = 1; xMin < 6; xMin++) {
    getCompData();
    compXMinAvg = compXMinAvg + compX;
#ifdef debugMode
    Serial.print(xMin);
    Serial.print(F(", "));
    Serial.print(compX);
    Serial.print(F(", "));
    Serial.println(compXMinAvg);
#endif
    delay(1000);
  }
  compXMin = compXMinAvg / 5;
#ifdef debugMode
  Serial.print(F("compXMinAvg = "));
  Serial.print(compXMin);
  Serial.println(F("."));
  Serial.println();
#endif

  // Compass calibration at maximum of servo travel range
  camServo.write(servoMax);
#ifdef debugMode
  Serial.println(F("Calibrating compass at servo maximum..."));
#endif
  delay(servoDelay);
  for(int xMax = 1; xMax < 6; xMax++) {
    getCompData();
    compXMaxAvg = compXMaxAvg + compX;
#ifdef debugMode
    Serial.print(xMax);
    Serial.print(F(", "));
    Serial.print(compX);
    Serial.print(F(", "));
    Serial.println(compXMaxAvg);
#endif
    delay(1000);
  }
  compXMax = compXMaxAvg / 5;
#ifdef debugMode
  Serial.print(F("compXMinAvg = "));
  Serial.print(compXMin);
  Serial.println(F("."));
  Serial.println();
#endif
}

void checkCalib() {
  // Compass calibration check at minimum of servo travel range
  camServo.write(servoMin);
  delay(servoDelay);
  getCompData();
  if(compX < compXMin - 10 || compX > compXMin + 10) {
#ifdef debugMode
    Serial.println(F("compXMin incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }

  // Compass calibration check at maximum of servo travel range
  camServo.write(servoMax);
  delay(servoDelay);
  getCompData();
  if(compX < compXMax - 10 || compX > compXMax + 10) {
#ifdef debugMode
    Serial.println(F("compXMax incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  // Return to middle position of servo travel range
  camServo.write(servoMid);
  delay(servoDelay);
}

void setup() {
  Serial.begin(19200);
#ifdef debugMode
  Serial.println(F("Serial connection initiated."));
#endif
#ifdef Mega
  Serial1.begin(4800);
#endif
#ifndef Mega
  gpsSerial.begin(4800);
#endif
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
  Serial.println();
  Serial.println(F("Beginning servo calibration:"));
#endif
  calibrateServo();
#ifdef debugMode
  Serial.println(F("Calibration complete."));
  Serial.println();
  Serial.print(F("Checking calibration..."));
#endif
  checkCalib();
#ifdef debugMode
  Serial.println(F("calibration confirmed."));
  Serial.println();
#endif

#ifdef debugMode
  getCompData();
  getAltFt();
  Serial.print(F("Current heading is: "));
  Serial.println(compX);
  Serial.print(F("Current altitude is: "));
  Serial.println(altFt);
  Serial.println();
  Serial.println(F("Setup complete. Waiting for ArduinoConnectServer command."));
  Serial.println();
#endif
}

void loop() {
#ifdef debugMode
  getCompData();
  getAltFt();
  getGPSData();
  int pirVal = digitalRead(pirPin);
  int servoReadPos = camServo.read();
  Serial.print(F("Heading:  "));
  Serial.println(compX);
  Serial.print(F("Altitude: "));
  Serial.println(altFt);
  Serial.print(F("PIR:      "));
  Serial.println(pirVal);
  Serial.print(F("Servo:    "));
  Serial.println(servoReadPos);
  Serial.print(F("Location: "));
  Serial.print(flat, 4);
  Serial.print(F(", "));
  Serial.println(flon, 4);
  Serial.println();
  delay(1000);
#endif
}

// Servo speed control functions
/*void servoToMinSlow() {
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
 if(servoInitPos == servoMid) return;
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
 }*/

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
  for(x = 0; x < 3; x++) {
    altFtAvg = altFtAvg + ((baro.getHeightCentiMeters() / 30.48) - 1515);
  }
  altFt = altFtAvg / 3;
#endif
#ifndef averageData
  altFt = (baro.getHeightCentiMeters() / 30.48) - altOffset;
#endif
}

/*void checkMovement() {
 boolean servoStill = false;
 while(servoStill == false) {
 int servoPosLast = camServo.read();
 delay(100);
 int servoPos = camServo.read();
 int servoPosDiff = servoPosLast - servoPos;
 if(-5 > servoPosDiff < 5) servoStill = true;
 }
 }*/

// GPS functions below this line. Need to be configured before use
void getGPSData() {
  if(feedgps()) gpsdump(gps);
#ifdef debugMode
  else {
    Serial.println(F("GPS data unavailable."));
  }
#endif
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4); 
  Serial.print(F(", "));
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
