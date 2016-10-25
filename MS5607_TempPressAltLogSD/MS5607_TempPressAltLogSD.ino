#define debugMode
#define sdActive

#include "IntersemaBaro.h"
#include <SdFat.h>
#include <Wire.h>

uint8_t address = 0x76;

const int chipSelect = 53;

Intersema::BaroPressure_MS5607B baro(true);

uint32_t D1 = 0;
uint32_t D2 = 0;
int64_t dT = 0;
int32_t TEMP = 0;
int64_t OFF = 0; 
int64_t SENS = 0; 
int32_t P = 0;
uint16_t C[7];

float altTempC;
int tempCorrection = 0;
float altPressMillibar;
int pressCorrection = 0;
const float sea_press = 1013.25;
float altMeters;

SdFat sd;
SdFile myFile;

unsigned long timer = 0;

void setup() {
  // Disable internal pullups, 10Kohms are on the breakout
  PORTC |= (1 << 4);
  PORTC |= (1 << 5);

  Wire.begin();
  baro.init();
  Serial.begin(9600); //9600 changed 'cos of timing?
  delay(100);
  initial(address);
  delay(1000);

#ifdef debugMode
  Serial.println("Initializing SD card.");
#endif
  if(!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();
  delay(1000);
#ifdef sdActive
#ifdef debugMode
  Serial.println("Writing to SD card in setup function.");
#endif
  if (!myFile.open("TEMPLOG.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening test.txt for write failed");
  }
  myFile.println("$$$$");
  myFile.close();
#endif
}

void loop() {
  timer = millis();
  if(timer <= 3600000) {
    while(timer <= 3600000) {
      timer = millis();
      altGetData();
      long altCm = baro.getHeightCentiMeters();
#ifndef sdActive
      Serial.print(timer);
      Serial.print(",");
      Serial.print(altTempC);
      Serial.print(",");
      Serial.print(altPressMillibar);
      Serial.print(",");
      Serial.print(altMeters);
      Serial.print(",");
      Serial.println(altCm);
#endif
#ifdef sdActive
      // open the file for write at end like the Native SD library
#ifdef debugMode
      Serial.println("Writing to SD card in loop function.");
#endif
      if (!myFile.open("TEMPLOG.txt", O_RDWR | O_CREAT | O_AT_END)) {
        sd.errorHalt("opening test.txt for write failed");
      }
      myFile.print(timer);
      myFile.print(",");
      myFile.print(altTempC);
      myFile.print(",");
      myFile.print(altPressMillibar);
      myFile.print(",");
      myFile.print(altMeters);
      myFile.print(",");
      myFile.println(altCm);
      myFile.close();
#endif
      delay(1000);
    }
  }
}

void altGetData() {
  D1 = getVal(address, 0x48); // altPressMillibar raw
  D2 = getVal(address, 0x58);// altTempC raw

  dT   = D2 - ((uint32_t)C[5] << 8);
  OFF  = ((int64_t)C[2] << 16) + ((dT * C[4]) >> 7);
  SENS = ((int32_t)C[1] << 15) + ((dT * C[3]) >> 8);

  TEMP = (int64_t)dT * (int64_t)C[6] / 8388608 + 2000;

  // if temperature lower than 20 Celsius 
  if(TEMP < 2000) {
    int32_t T1    = 0;
    int64_t OFF1  = 0;
    int64_t SENS1 = 0;

    T1    = pow(dT, 2) / 2147483648;
    OFF1  = 5 * pow((TEMP - 2000), 2) / 2;
    SENS1 = 5 * pow((TEMP - 2000), 2) / 4;

    // if temperature lower than -15 Celsius 
    if(TEMP < -1500) {
      OFF1  = OFF1 + 7 * pow((TEMP + 1500), 2); 
      SENS1 = SENS1 + 11 * pow((TEMP + 1500), 2) / 2;
    }
    TEMP -= T1;
    OFF -= OFF1; 
    SENS -= SENS1;
  }
  altTempC = ((float)TEMP / 100) + tempCorrection; 

  P  = ((int64_t)D1 * SENS / 2097152 - OFF) / 32768;

  altPressMillibar = ((float)P / 100) + pressCorrection;

  altMeters = getAltitude(altPressMillibar, altTempC);
}
float getAltitude(float altPressMillibar, float altTempC) {
  return ((pow((sea_press / altPressMillibar), 1/5.257) - 1.0) * (altTempC + 273.15)) / 0.0065;
}
long getVal(int address, byte code) {
  unsigned long ret = 0;
  Wire.beginTransmission(address);
  Wire.write(code);
  Wire.endTransmission();
  delay(10);
  // start read sequence
  Wire.beginTransmission(address);
  Wire.write((byte) 0x00);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.requestFrom(address, (int)3);
  if (Wire.available() >= 3) {
    ret = Wire.read() * (unsigned long)65536 + Wire.read() * (unsigned long)256 + Wire.read();
  }
  else ret = -1;
  Wire.endTransmission();
  return ret;
}

void initial(uint8_t address) {
  Wire.beginTransmission(address);
  Wire.write(0x1E); // reset
  Wire.endTransmission();
  delay(10);

  for (int i = 0; i < 6; i++) {
    Wire.beginTransmission(address);
    Wire.write(0xA2 + (i * 2));
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.requestFrom(address, (uint8_t) 6);
    delay(1);
    if(Wire.available()) C[i+1] = Wire.read() << 8 | Wire.read();
    else Serial.println("Error reading PROM 1"); // error reading the PROM or communicating with the device

    Serial.println(C[i+1]);
  }
}
