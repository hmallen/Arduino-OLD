#include <Wire.h>
#include "IntersemaBaro.h"

Intersema::BaroPressure_MS5607B baro(true);

#define acclAddr (0x1d)
#define compAddr (0x1e)
//#define gyroAddr (0x69)

#define sensDelay 1000

int16_t acclx;
int16_t accly;
int16_t acclz;
int16_t compx;
int16_t compy;
int16_t compz;
//int16_t gyrox;
//int16_t gyroy;
//int16_t gyroz;

float altFeet;
unsigned long altCalibVal;

void setup() {
  Serial.begin(19200);
  Wire.begin();
  baro.init();
  
  altCalibVal = baro.getHeightCentiMeters() - 762;
  
  configureSensors();
}

void loop() {
  altRead();
  acclRead();
  compRead();
  //gyroRead();
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
  Serial.println(compz);
  // Gyroscope
  /*Serial.print(gyrox);
  Serial.print(",");
  Serial.print(gyroy);
  Serial.print(",");
  Serial.println(gyroz);
  delay(1000);*/
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

void altRead() {
  altFeet = ((baro.getHeightCentiMeters() - altCalibVal) / 30.48);
  delay(sensDelay);
}

void acclRead() {
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
    Serial.println("Cannot read device ID from sensor");
  }
  else if (devId != 0xe5) {
    Serial.print("Wrong/invalid device ID ");
    Serial.print(devId);
    Serial.println(" expected 0xe5)");
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

void compRead() {
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

/*void gyroRead() {
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
}*/
