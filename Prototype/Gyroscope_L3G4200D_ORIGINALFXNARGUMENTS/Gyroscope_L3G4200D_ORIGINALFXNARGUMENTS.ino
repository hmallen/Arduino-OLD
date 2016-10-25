#include <Wire.h>

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23

#define gyroAddr 0x69                 // I2C address of gyro

const uint8_t gyroAddress = gyroAddr;
const int gyroOffset = 114;
String gyroscopeString;

void setup() {
  Wire.begin();
  Serial.begin(19200);
  i2cWriteByte(gyroAddress, CTRL_REG1, 0x1F);    // Turn on all axes, disable power down
  i2cWriteByte(gyroAddress, CTRL_REG3, 0x08);    // Enable control ready signal
  i2cWriteByte(gyroAddress, CTRL_REG4, 0x80);    // Set scale (500 deg/sec)
  delay(100);                   // Wait to synchronize 
}

void loop() {
  getGyroValues();  // Get new values
  Serial.println(gyroscopeString);
  delay(500);                   // Short delay between reads
}

void getGyroValues() {
  int gyroX, gyroY, gyroZ;

  uint8_t x_msb; // X-axis most significant byte
  uint8_t x_lsb; // X-axis least significant byte
  uint8_t y_msb; // Y-axis most significant byte
  uint8_t y_lsb; // Y-axis least significant byte
  uint8_t z_msb; // Z-axis most significant byte
  uint8_t z_lsb; // Z-axis least significant byte

  if((i2cReadByte(gyroAddress, 0x29, &x_msb) == 0) &&
    (i2cReadByte(gyroAddress, 0x28, &x_lsb) == 0) &&
    (i2cReadByte(gyroAddress, 0x2B, &y_msb) == 0) &&
    (i2cReadByte(gyroAddress, 0x2A, &y_lsb) == 0) &&
    (i2cReadByte(gyroAddress, 0x2D, &z_msb) == 0) &&
    (i2cReadByte(gyroAddress, 0x2C, &z_lsb) == 0)) {
    uint16_t x = x_msb << 8 | x_lsb;
    uint16_t y = y_msb << 8 | y_lsb;
    uint16_t z = z_msb << 8 | z_lsb;

    /*gyroX = (int)x / gyroOffset;
     gyroY = (int)y / gyroOffset;
     gyroZ = (int)z / gyroOffset;*/
    gyroX = (int)x;
    gyroY = (int)y;
    gyroZ = (int)z;

    gyroscopeString = String(gyroX) + "," + String(gyroY) + "," + String(gyroZ);
  }
}

int i2cReadByte(uint8_t addr, uint8_t reg, uint8_t *data) {
  Wire.beginTransmission(addr);
  Wire.write(reg);                // Register address to read
  Wire.endTransmission();             // Terminate request
  Wire.requestFrom(addr, (uint8_t)1);          // Read a byte
  if(Wire.available()) *data = Wire.read();
  else return -1;  // Read nothing back
  return 0;
  /*while(!Wire.available()) { 
   };       // Wait for receipt
   return(Wire.read());*/  // Get result
}

void i2cWriteByte(uint8_t addr, uint8_t reg, byte data) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}
