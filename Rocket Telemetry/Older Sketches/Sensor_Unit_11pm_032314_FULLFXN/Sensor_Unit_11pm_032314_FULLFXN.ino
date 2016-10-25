/*
FIX CALIBRATION -- ADDED "+1.5" TO SET TO 0
 
 ADD STATISTICS SMS SEND FUNCTION
 */

#include "IntersemaBaro.h"
#include <SoftwareSerial.h> 
#include <TinyGPS.h>
#include <Wire.h>

TinyGPS gps;
SoftwareSerial gpsSerial(6, 255);

Intersema::BaroPressure_MS5607B baro(true);

#define acclAddr (0x1d)
#define compAddr (0x1e)
#define gyroAddr (0x69)

#define debugPin A0
#define landPin A1
#define landLED A2

#define sensDelay 10
#define gpsDelay 10  // Delay between GPS updates (seconds)

#define altTau 44  // "Decay constant" for altimeter calibration

#define launchTriggerVal 100  // Altitude over which launch state triggered after 10 positive reads
#define landTriggerVal 20  // Altitude under which land state triggered after 10 positive reads

int16_t acclx;
int16_t accly;
int16_t acclz;
int16_t compx;
int16_t compy;
int16_t compz;
int16_t gyrox;
int16_t gyroy;
int16_t gyroz;

float altFeetInitial;
float altFeet;
float altCalibVal;

int altLaunchCount = 0;
int altLandCount = 0;

boolean launchDetect = false;
boolean landDetect = false;

unsigned long landTime;

void zeroAltitude() {
  Serial.print(F("Zeroing altimeter. Please wait..."));
  for(int y = 0; y < 11; y++) {
    unsigned long altPrep = baro.getHeightCentiMeters();
    delay(100);
  }
  unsigned long alt1 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt2 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt3 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt4 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt5 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt6 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt7 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt8 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt9 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long alt10 = baro.getHeightCentiMeters();
  delay(100);
  unsigned long altTot = alt1 + alt2 + alt3 + alt4 + alt5 + alt6 + alt7 + alt8 + alt9 + alt10;
  delay(100);
  altCalibVal = (altTot / 10) - altTau;  // ADDED VALUE TO SET TO ACTUAL ZERO!!!!
  Serial.println(F("altimeter ready."));
  Serial.println();
}

void checkCalib() {
  Serial.print(F("Confirming altimeter calibration..."));
  delay(5000);
  for(int x = 0; x < 11; x++) {
    readAlt();
    if(abs(altFeet - altFeetInitial) > 5) {
    zeroAltitude();
    }
  }
  Serial.println(F("altimeter correctly calibrated."));
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  baro.init();

  pinMode(landPin, OUTPUT);
  pinMode(landLED, OUTPUT);

  zeroAltitude();
  readAlt();

  while(abs(altFeet) >= 5) {
    Serial.println("Re-zeroing.");
    Serial.println();
    zeroAltitude();
    delay(5000);
    readAlt();
  }
  altFeetInitial = altFeet;

  configureSensors();
}

void loop() {
  checkCalib();
  while(landDetect == false) {
    readAlt();
    readAccl();
    readComp();
    gyroRead();
    sendData();
    detectState();
  }
  Serial.println(F("Landing detected. Switching to transmit mode."));
  Serial.println();
  delay(1000);
  transmitMode();
}

void configureSensors() {
  // Accelerometer
  writeByte(acclAddr, 0x2c, 0x08);
  writeByte(acclAddr, 0x2d, 0x08);
  writeByte(acclAddr, 0x2e, 0x0);
  // Compass
  writeByte(compAddr, 0x0, 0x10);
  writeByte(compAddr, 0x1, 0x20);
  writeByte(compAddr, 0x2, 0x0);
}

int readByte(uint8_t addr, uint8_t reg, uint8_t *data) {
  // Do an i2c write to set the register that will be read
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();

  // Read a byte from the device
  Wire.requestFrom(addr, (uint8_t)1);
  if (Wire.available())
  {
    *data = Wire.read();
  }
  else
  {
    // Read nothing back
    return -1;
  }
  return 0;
}

void writeByte(uint8_t addr, uint8_t reg, byte data) {
  // Begin the write sequence
  Wire.beginTransmission(addr);
  // First byte is to set the register pointer
  Wire.write(reg);
  // Write the data byte
  Wire.write(data);
  // End the write sequence; bytes are actually transmitted now
  Wire.endTransmission();
}

void readAlt() {
  altFeet = ((baro.getHeightCentiMeters() - altCalibVal) / 30.48);
  //altFeet = (baro.getHeightCentiMeters() / 30.48);
  delay(sensDelay);
}

void readAccl() {
  uint8_t devId;
  uint8_t x_msb; // X-axis most significant byte
  uint8_t x_lsb; // X-axis least significant byte
  uint8_t y_msb; // Y-axis most significant byte
  uint8_t y_lsb; // Y-axis least significant byte
  uint8_t z_msb; // Z-axis most significant byte
  uint8_t z_lsb; // Z-axis least significant byte
  uint16_t x;
  uint16_t y;
  uint16_t z;

  // Read the device ID to verify as correct sensor
  if (readByte(acclAddr, 0x0, &devId) != 0) {
    Serial.println(F("Cannot read device ID from sensor"));
  }
  else if (devId != 0xe5) {
    Serial.print(F("Wrong/invalid device ID "));
    Serial.print(devId);
    Serial.println(F(" expected 0xe5)"));
  }
  else {
    // Read the output
    if ((readByte(acclAddr, 0x33, &x_msb) == 0) &&
      (readByte(acclAddr, 0x32, &x_lsb) == 0) &&
      (readByte(acclAddr, 0x35, &y_msb) == 0) &&
      (readByte(acclAddr, 0x34, &y_lsb) == 0) &&
      (readByte(acclAddr, 0x37, &z_msb) == 0) &&
      (readByte(acclAddr, 0x36, &z_lsb) == 0)) {
      x = (x_msb << 8) | x_lsb;
      y = (y_msb << 8) | y_lsb;
      z = (z_msb << 8) | z_lsb;

      // Perform 2's complement
      acclx = ~(x - 1);
      accly = ~(y - 1);
      acclz = ~(z - 1);
    }
  }
  delay(sensDelay);
}

void readComp() {
  uint8_t x_msb; // X-axis most significant byte
  uint8_t x_lsb; // X-axis least significant byte
  uint8_t y_msb; // Y-axis most significant byte
  uint8_t y_lsb; // Y-axis least significant byte
  uint8_t z_msb; // Z-axis most significant byte
  uint8_t z_lsb; // Z-axis least significant byte
  int x;
  int y;
  int z;
  // Get the value from the sensor
  if ((readByte(compAddr, 0x3, &x_msb) == 0) &&
    (readByte(compAddr, 0x4, &x_lsb) == 0) &&
    (readByte(compAddr, 0x5, &y_msb) == 0) &&
    (readByte(compAddr, 0x6, &y_lsb) == 0) &&
    (readByte(compAddr, 0x7, &z_msb) == 0) &&
    (readByte(compAddr, 0x8, &z_lsb) == 0)) {
    compx = x_msb << 8 | x_lsb;
    compy = y_msb << 8 | y_lsb;
    compz = z_msb << 8 | z_lsb;
  }
  delay(sensDelay);
}

void gyroRead() {
  uint8_t x_msb; // X-axis most significant byte
  uint8_t x_lsb; // X-axis least significant byte
  uint8_t y_msb; // Y-axis most significant byte
  uint8_t y_lsb; // Y-axis least significant byte
  uint8_t z_msb; // Z-axis most significant byte
  uint8_t z_lsb; // Z-axis least significant byte
  uint16_t x;
  uint16_t y;
  uint16_t z;
  // Get the value from the sensor
  if ((readByte(gyroAddr, 0x1d, &x_msb) == 0) &&
    (readByte(gyroAddr, 0x1e, &x_lsb) == 0) &&
    (readByte(gyroAddr, 0x1f, &y_msb) == 0) &&
    (readByte(gyroAddr, 0x20, &y_lsb) == 0) &&
    (readByte(gyroAddr, 0x21, &z_msb) == 0) &&
    (readByte(gyroAddr, 0x22, &z_lsb) == 0)) {
    x = (x_msb << 8) | x_lsb;
    y = (y_msb << 8) | y_lsb;
    z = (z_msb << 8) | z_lsb;
    // Perform 2's complement
    gyrox = ~(x - 1);
    gyroy = ~(y - 1);
    gyroz = ~(z - 1);
  }
  // Run again in 1 s (1000 ms)
  delay(sensDelay);
}

void sendData() {
  // Altimeter
  Serial.print(altFeet);
  Serial.print("/");
  // Accelerometer
  Serial.print(acclx);
  Serial.print(",");
  Serial.print(accly);
  Serial.print(",");
  Serial.print(acclz);
  Serial.print("/");
  // Compass
  Serial.print(compx);
  Serial.print(",");
  Serial.print(compy);
  Serial.print(",");
  Serial.print(compz);
  Serial.print("/");
  // Gyroscope
  Serial.print(gyrox);
  Serial.print(",");
  Serial.print(gyroy);
  Serial.print(",");
  Serial.println(gyroz);
}

void detectState() {
  int altChange = altFeet - altFeetInitial;
  if(digitalRead(debugPin) == 0) {
    landDetect = true;
    Serial.println(F("Debug landing triggered."));
    while(digitalRead(debugPin) == 0) {
      delay(100);
    }
  }
  if(launchDetect == false && altChange > launchTriggerVal && altLaunchCount < 10) {
    altLaunchCount++;
  }
  else if(launchDetect == false && altChange > launchTriggerVal) {
    launchDetect == true;
  }
  if(launchDetect == true && landDetect == false && altChange < landTriggerVal && altLandCount < 10) {
    altLandCount++;
  }
  else if(launchDetect == true && landDetect == false && altChange < landTriggerVal) {
    landTime = millis();
    landDetect = true;
  }
  if(landDetect == true) {
    digitalWrite(landPin, HIGH);
  }
}

void sendGPSData() {
  boolean newdata = false;
  for(unsigned long start = millis(); millis() - start < 5000;) {
    while(gpsSerial.available()) {
      if(feedGPS()) {
        newdata = true;
      }
    }
  }
  if(newdata) {
    gpsDump(gps);
  }
  Serial.println();
  Serial.println(F("GPS coordinates sent."));
  Serial.println();
}

// Get and process GPS data
void gpsDump(TinyGPS &gps) {
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 8); 
  Serial.print(",+");
  Serial.println(flon, 8);
}

// Feed data as it becomes available
boolean feedGPS() {
  while(gpsSerial.available()) {
    if(gps.encode(gpsSerial.read()))
      return true;
  }
  return false;
}

void transmitMode() {
  gpsSerial.begin(4800);
  digitalWrite(landPin, HIGH);
  while(landDetect == true) {
    unsigned long landElapTime = millis() - landTime;
    if(landElapTime > (gpsDelay * 1000)) {
      sendGPSData();
      //Serial.println(F("Program complete."));
      while(landDetect == true) {
        digitalWrite(landLED, LOW);
        delay(500);
        digitalWrite(landLED, HIGH);
        delay(50);
      }
    }
  }
}
