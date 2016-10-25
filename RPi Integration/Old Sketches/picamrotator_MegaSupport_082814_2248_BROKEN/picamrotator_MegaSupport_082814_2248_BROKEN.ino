/*  Camera rotator for remote monitoring system
 
 Is camServo.write() function active during whole rotation? If so...
 Does calling camServo.read() while still rotating interrupt action?
 --> Important for checkMovement() function!
 
 Calling SoftwareSerial for GPS, but can use Serial1-3 with Mega.
 
 */

#define DebugMode
//#define Mega

#include <Servo.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "IntersemaBaro.h"
#include <TinyGPS.h>

Intersema::BaroPressure_MS5607B baro(true);

#define compAddr 0x1E

#define servoMin 5  // Minimum point of servo travel range without motor strain
#define servoMax 179  // Maximum point of servo travel range without motor strain
#define servoPin 9  // PWM-capable pin that will serve as servo input pin
#define servoDelay 5000  // Delay to allow servo to move to required position

TinyGPS gps;
SoftwareSerial gpsSerial(6, 255);

Servo camServo;

int compX, compY, compZ;
int compXMin, compXMax;

float altFt;

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
  delay(servoDelay);
  //checkMovement();
  getCompData();
  if(compX < compXMin - 10 || compX > compXMin + 10) {
#ifdef DebugMode
    Serial.println(F("compXMin incorrect...recalibrating."));
#endif
    calibrateServo();
    checkCalib();
  }
  camServo.write(servoMax);
  delay(servoDelay);
  //checkMovement();
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
  delay(servoDelay);
  //checkMovement();
#ifdef DebugMode
  Serial.println(F("Calibration complete. Waiting for ArduinoConnectServer command."));
#endif
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
  baro.init();
  camServo.attach(servoPin);

  Wire.beginTransmission(compAddr);
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();

  calibrateServo();
  checkCalib();

  getCompData();
  getAltFt();

  Serial.print(F("Current heading is: "));
  Serial.println(compX);
  Serial.print(F("Current altitude is: "));
  Serial.println(altFt);
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

void getAltFt() {
#ifdef averageData
  int altFtAvg;
  for(x = 0; x++; x < 3) {
    altFtAvg = altFtAvg + ((baro.getHeightCentimeters() / 30.48) - 1515);
  }
  altFt = altFtAvg / 3;
#endif
#ifndef averageData
  altFt = (baro.getHeightCentimeters() / 30.48) - 1515;
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
