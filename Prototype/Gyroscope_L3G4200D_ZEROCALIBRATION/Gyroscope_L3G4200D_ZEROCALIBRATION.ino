#include <Wire.h>

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23

#define gyroAddr 0x69                 // I2C address of gyro

const uint8_t gyroAddress = gyroAddr;

String gyroscopeString;

int gyroX, gyroY, gyroZ;

//int accelXOffset, accelYOffset, accelZOffset;
int gyroXOffset, gyroYOffset, gyroZOffset;

int gyroOffset = 0;

boolean sensorZeroMode = false;

void setup() {
  Wire.begin();
  Serial.begin(19200);
  i2cWriteByte(gyroAddress, CTRL_REG1, 0x1F);    // Turn on all axes, disable power down
  i2cWriteByte(gyroAddress, CTRL_REG3, 0x08);    // Enable control ready signal
  i2cWriteByte(gyroAddress, CTRL_REG4, 0x80);    // Set scale (500 deg/sec)
  delay(100);                   // Wait to synchronize
  zeroSensors();
}

void loop() {
  getGyroValues();  // Get new values
  Serial.println(F("Uncompensated:"));
  Serial.print(gyroX);
  Serial.print(F(", "));
  Serial.print(gyroY);
  Serial.print(F(", "));
  Serial.println(gyroZ);

  Serial.println(F("Compensated:"));
  Serial.print(gyroX / gyroOffset);
  Serial.print(F(", "));
  Serial.print(gyroY / gyroOffset);
  Serial.print(F(", "));
  Serial.println(gyroZ / gyroOffset);

  Serial.println(F("Added Compensation:"));
  Serial.print((gyroX / gyroOffset) - gyroXOffset);
  Serial.print(F(", "));
  Serial.print((gyroY / gyroOffset) - gyroYOffset);
  Serial.print(F(", "));
  Serial.println((gyroZ / gyroOffset) - gyroZOffset);

  /*Serial.print(gyroX + gyroXOffset);
   Serial.print(F(", "));
   Serial.print(gyroY + gyroYOffset);
   Serial.print(F(", "));
   Serial.println(gyroZ + gyroZOffset);*/

  delay(500);                   // Short delay between reads
}

void getGyroValues() {
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
}

void i2cWriteByte(uint8_t addr, uint8_t reg, byte data) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void zeroSensors() {
  sensorZeroMode = true;

  const int sampleNumber = 10;

  //int accelXTot, accelYTot, accelZTot;
  int gyroXTot = 0;
  int gyroYTot = 0;
  int gyroZTot = 0;
  // Accelerometer
  /*for(int x = 0; x < sampleNumber; x++) {
   readAccelerometer();
   accelXTot += accelX;
   accelYTot += accelY;
   accelZTot += accelZ;
   delay(100);
   }
   accelXOffset = accelXTot / sampleNumber;
   accelYOffset = accelYTot / sampleNumber;
   accelZOffset = accelZTot / sampleNumber;*/

  // Gyroscope
  for(int y = 0; y < sampleNumber; y++) {
    getGyroValues();
    Serial.print(gyroX);
    Serial.print(F(", "));
    Serial.print(gyroY);
    Serial.print(F(", "));
    Serial.println(gyroZ);
    if(abs(gyroX) > gyroOffset) gyroOffset = gyroX;
    if(abs(gyroY) > gyroOffset) gyroOffset = gyroY;
    if(abs(gyroZ) > gyroOffset) gyroOffset = gyroZ;
    gyroXTot += gyroX;
    gyroYTot += gyroY;
    gyroZTot += gyroZ;
    Serial.println(gyroOffset);
    delay(100);
  }
  gyroOffset = sqrt(gyroOffset);
  gyroXOffset = gyroXTot / (sampleNumber * 10);
  gyroYOffset = gyroYTot / (sampleNumber * 10);
  gyroZOffset = gyroZTot / (sampleNumber * 10);
  Serial.print(gyroXOffset);
  Serial.print(F(", "));
  Serial.print(gyroYOffset);
  Serial.print(F(", "));
  Serial.println(gyroZOffset);
  Serial.println();

  // Gyroscope
  /*for(int y = 0; y < sampleNumber; y++) {
   getGyroValues();
   gyroXTot += gyroX;
   gyroYTot += gyroY;
   gyroZTot += gyroZ;
   delay(100);
   }
   gyroXOffset = gyroXTot / sampleNumber;
   gyroYOffset = gyroYTot / sampleNumber;
   gyroZOffset = gyroZTot / sampleNumber;*/

  Serial.print(F("gyroOffset: "));
  Serial.println(gyroOffset);

  /*Serial.println(F("Sensor offsets:"));
   Serial.print(gyroXOffset);
   Serial.print(F(", "));
   Serial.print(gyroYOffset);
   Serial.print(F(", "));
   Serial.println(gyroZOffset);
   Serial.println();*/

  //sensorZeroMode = false;
}
