// OSEPP Gyroscope Sensor Example Sketch
// by OSEPP <http://www.osepp.com>
// This sketch demonstrates interactions with the Gyroscope Sensor
#include <Wire.h>
// Possible sensor addresses (suffix correspond to DIP switch positions)
#define SENSOR_ADDR_OFF (0x69)
#define SENSOR_ADDR_ON (0x68)
// Set the sensor address here
const uint8_t sensorAddr = SENSOR_ADDR_OFF;
// One-time setup
void setup()
{
  // Start the serial port for output
  Serial.begin(19200);
  // Join the I2C bus as master
  Wire.begin();
  // Use the default configuration (see datasheet for more details)
}
// Main program loop
void loop()
{
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
  if ((ReadByte(sensorAddr, 0x1d, &x_msb) == 0) &&
    (ReadByte(sensorAddr, 0x1e, &x_lsb) == 0) &&
    (ReadByte(sensorAddr, 0x1f, &y_msb) == 0) &&
    (ReadByte(sensorAddr, 0x20, &y_lsb) == 0) &&
    (ReadByte(sensorAddr, 0x21, &z_msb) == 0) &&
    (ReadByte(sensorAddr, 0x22, &z_lsb) == 0))
  {
    x = (x_msb << 8) | x_lsb;
    y = (y_msb << 8) | y_lsb;
    z = (z_msb << 8) | z_lsb;
    // Perform 2's complement
    int16_t real_x = ~(x - 1);
    int16_t real_y = ~(y - 1);
    int16_t real_z = ~(z - 1);

    Serial.print(real_x);
    Serial.print(", ");
    Serial.print(real_y);
    Serial.print(", ");
    Serial.println(real_z);
  }
  else
  {
    Serial.println("Failed to read from sensor");
  }
  delay(100);  // Run again after delay
}
// Read a byte on the i2c interface
int ReadByte(uint8_t addr, uint8_t reg, uint8_t *data)
{
  // Do an i2c write to set the register that we want to read from
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
// Write a byte on the i2c interface
void WriteByte(uint8_t addr, uint8_t reg, byte data)
{
  // Begin the write sequence
  Wire.beginTransmission(addr);
  // First byte is to set the register pointer
  Wire.write(reg);
  // Write the data byte
  Wire.write(data);
  // End the write sequence; bytes are actually transmitted now
  Wire.endTransmission();
}
