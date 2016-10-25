/*
*/

#define debugMode  // If defined, outputs detailed serial prints to aid in debugging
//#define averageData  // Takes an average of 3 read from sensors before outputting (currently only implemented w/ altimeter)

#include "IntersemaBaro.h"
#include <SdFat.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>

#define gpsPin 6  // Pin for GPS SoftwareSerial connection
#define chipSelect 10  // CS pin for SD shield

Intersema::BaroPressure_MS5607B baro(true);

#define compAddr 0x1E  // Hexidecimal address of compass on i2c interface

SdFat sd;
SdFile dataFile;

TinyGPS gps;
SoftwareSerial gpsSerial(gpsPin, 255);

float gpsLat, gpsLon;

int compX, compY, compZ;
int compXMin, compXMax;

const int altOffset = 4515;  // Negative offset in feet of altimeter to output actual altitude values

float altFt;

void setup() {
  Serial.begin(19200);
#ifdef debugMode
  Serial.println(F("Serial connection initiated."));
#endif
  gpsSerial.begin(4800);
  Wire.begin();
#ifdef debugMode
  Serial.println(F("I2C initiated."));
#endif
  baro.init();
#ifdef debugMode
  Serial.println(F("Altimeter initiated."));
  Serial.print(F("Configuring compass.."));
#endif
  Wire.beginTransmission(compAddr);
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();
#ifdef debugMode
  Serial.println(F("compass configured."));
  Serial.print(F("Initiating SD card..."));
#endif

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

#ifdef debugMode
  Serial.println(F("successful."));
  Serial.println();
  getCompData();
  getGPSData();
  getAltFt();
  Serial.print(F("Current heading is: "));
  Serial.println(compX);
  Serial.print(F("Current altitude is: "));
  Serial.println(altFt);
  Serial.print(F("Current location is: "));
  Serial.print(gpsLat, 4);
  Serial.print(F(", "));
  Serial.println(gpsLon, 4);
  Serial.println();
#endif
}

void loop() {
  String dataString;
  /*
  Delimiters are used to allow parsing of data later.
   a = Altitude
   c = Compass "x" value
   t = Latitude
   n = Longitude
   */
  for(x = 1; x < 11; x++) {
    getCompData();
    dataString = 'c';
    dataString += compX;
    getAltFt();
    dataString += 'a';
    dataString += altFt;
    if(!dataFile.open("datafile.txt", 0_RDWR | 0_CREAT | 0_AT_END)) {
      sd.errorHalt("Failed to open SD data file.");
    }
    dataFile.println(dataString);
    dataFile.close();
#ifdef debugMode
    Serial.println(dataString);
#endif
    delay(100);
  }
  getGPSData();
  dataString = 't';
  dataString += flat;
  dataString += 'n';
  dataString += flon;
  if(!dataFile.open("datafile.txt", 0_RDWR | 0_CREAT | 0_AT_END)){
    sd.errorHalt("Failed to open SD data file.");
  }
  dataFile.println(dataString);                    
  dataFile.close();
#ifdef debugMode
  Serial.println(dataString);
  delay(100);
  /*Serial.print(F("Heading:  "));
   Serial.println(compX);
   Serial.print(F("Altitude: "));
   Serial.println(altFt);
   Serial.print(F("Location: "));
   Serial.print(gpsLat, 4);
   Serial.print(F(", "));
   Serial.println(gpsLon, 4);
   Serial.println();
   delay(1000);*/
}

void zeroAltitude() {
  long altTot = 0;
#ifdef debugMode
  Serial.print(F("Zeroing altimeter. Please wait..."));
#endif
  for(int x = 0; x < 10; x++) {
    long altPrep = baro.getHeightCentiMeters();
    delay(100);
  }
  for(int y = 0; y < 10; y++) {
    long altCalibRead = baro.getHeightCentiMeters();
    altTot += altCalibRead;
    delay(100);
  }
  altCalibVal = altTot / 10;
  delay(1000);
#ifdef verboseSerial
  Serial.println(F("altimeter ready."));
  Serial.println();
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
  for(x = 0; x < 3; x++) {
    altFtAvg = altFtAvg + ((baro.getHeightCentiMeters() / 30.48) - 1515);
  }
  altFt = altFtAvg / 3;
#endif
#ifndef averageData
  altFt = (baro.getHeightCentiMeters() / 30.48) - altOffset;
#endif
}

// GPS functions below this line. Need to be configured before use
void getGPSData() {
  bool newdata = false;
  unsigned long start = millis();
  while(millis() - start < 1000) {  // Delay is required to fully acquire GPS data
    if(feedgps()) newdata = true;
  }
  if(newdata) gpsdump(gps);
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  gpsLat = flat;
  gpsLon = flon;
}

// Feed data as it becomes available
boolean feedgps() {
  while(gpsSerial.available()) {
    if(gps.encode(gpsSerial.read())) return true;
  }
  return false;
}
