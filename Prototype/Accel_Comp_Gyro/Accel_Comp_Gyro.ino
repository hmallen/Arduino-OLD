#include <Wire.h>

/* DIP switch positions not defined for accelerometer or gyroscope, 
so each must be set to "1" position at all times for proper function! */

// Define accelerometer register addresses (datasheet)
#define REG_DEVID_ADDR (0x00)
#define REG_THRESH_TAP_ADDR (0x1d)
#define REG_TAP_DUR_ADDR (0x21)
#define REG_TAP_LATENCY_ADDR (0x22)
#define REG_TAP_WINDOW_ADDR (0x23)
#define REG_BW_RATE_ADDR (0x2c)
#define REG_PWR_CTL_ADDR (0x2d)
#define REG_INT_ENABLE_ADDR (0x2e)
#define REG_DATA_FORMAT_ADDR (0x31)
#define REG_DATAX0_ADDR (0x32)
#define REG_DATAX1_ADDR (0x33)
#define REG_DATAY0_ADDR (0x34)
#define REG_DATAY1_ADDR (0x35)
#define REG_DATAZ0_ADDR (0x36)
#define REG_DATAZ1_ADDR (0x37)
#define REG_FIFO_CTL_ADDR (0x38)

void setup()
{
  Serial.begin(9600); // Start the serial port for output
  Wire.begin(); // Join the I2C bus as master

  // Configure accelerometer settings
  WriteByte(0x1D, REG_BW_RATE_ADDR, 0X08); // Set 25Hz output data rate and 25Hz bandwidth and disable low power mode  
  WriteByte(0x1D, REG_PWR_CTL_ADDR, 0x08); // Disable auto sleep
  WriteByte(0x1D, REG_INT_ENABLE_ADDR, 0x0); // Disable interrupts (the pins are not brought out anyway)

  // Configure compass settings
  WriteByte(0x1E, 0x0, 0x10); // Configure the compass to default values (see datasheet for details)
  WriteByte(0x1E, 0x1, 0x20); // Configure the compass to default values (see datasheet for details)
  WriteByte(0x1E, 0x2, 0x0); // Set compass to continuous-measurement mode (default is single shot)
}

void loop()
{
  // Assign MSB/LSB for accelerometer
  uint8_t x1_msb; // X-axis most significant byte
  uint8_t x1_lsb; // X-axis least significant byte
  uint8_t y1_msb; // Y-axis most significant byte
  uint8_t y1_lsb; // Y-axis least significant byte
  uint8_t z1_msb; // Z-axis most significant byte
  uint8_t z1_lsb; // Z-axis least significant byte
  // Assign 16-bit, unsigned output variables
  uint16_t x2;
  uint16_t y2;
  uint16_t z2;

  // Assign MSB/LSB for compass
  uint8_t x2_msb; // X-axis most significant byte
  uint8_t x2_lsb; // X-axis least significant byte
  uint8_t y2_msb; // Y-axis most significant byte
  uint8_t y2_lsb; // Y-axis least significant byte
  uint8_t z2_msb; // Z-axis most significant byte
  uint8_t z2_lsb; // Z-axis least significant byte
  // Assign integer output variables
  int x1;
  int y1;
  int z1;

  // Assign MSB/LSB for gyroscope
  uint8_t x3_msb; // X-axis most significant byte
  uint8_t x3_lsb; // X-axis least significant byte
  uint8_t y3_msb; // Y-axis most significant byte
  uint8_t y3_lsb; // Y-axis least significant byte
  uint8_t z3_msb; // Z-axis most significant byte
  uint8_t z3_lsb; // Z-axis least significant byte
  // Assign 16-bit, unsigned output variables
  uint16_t x3;
  uint16_t y3;
  uint16_t z3;

  // Get values from the sensors
  
    // Accelerometer
  if ((ReadByte(0x1D, REG_DATAX1_ADDR, &x1_msb) == 0) && 
    (ReadByte(0x1D, REG_DATAX0_ADDR, &x1_lsb) == 0) &&
    (ReadByte(0x1D, REG_DATAY1_ADDR, &y1_msb) == 0) &&
    (ReadByte(0x1D, REG_DATAY0_ADDR, &y1_lsb) == 0) &&
    (ReadByte(0x1D, REG_DATAZ1_ADDR, &z1_msb) == 0) &&
    (ReadByte(0x1D, REG_DATAZ0_ADDR, &z1_lsb) == 0) &&
    // Compass
    (ReadByte(0x1E, 0x3, &x2_msb) == 0) &&
    (ReadByte(0x1E, 0x4, &x2_lsb) == 0) &&
    (ReadByte(0x1E, 0x5, &y2_msb) == 0) &&
    (ReadByte(0x1E, 0x6, &y2_lsb) == 0) &&
    (ReadByte(0x1E, 0x7, &z2_msb) == 0) &&
    (ReadByte(0x1E, 0x8, &z2_lsb) == 0) &&
    // Gyroscope
    (ReadByte(0x69, 0x1d, &x3_msb) == 0) &&
    (ReadByte(0x69, 0x1e, &x3_lsb) == 0) &&
    (ReadByte(0x69, 0x1f, &y3_msb) == 0) &&
    (ReadByte(0x69, 0x20, &y3_lsb) == 0) &&
    (ReadByte(0x69, 0x21, &z3_msb) == 0) &&
    (ReadByte(0x69, 0x22, &z3_lsb) == 0))
  {
    // Accelerometer
    x1 = (x1_msb << 8) | x1_lsb;
    y1 = (y1_msb << 8) | y1_lsb;
    z1 = (z1_msb << 8) | z1_lsb;
    // Compass
    x2 = (x2_msb << 8) | x2_lsb;
    y2 = (y2_msb << 8) | y2_lsb;
    z2 = (z2_msb << 8) | z2_lsb;
    // Gyroscope
    x3 = (x3_msb << 8) | x3_lsb;
    y3 = (y3_msb << 8) | y3_lsb;
    z3 = (z3_msb << 8) | z3_lsb;

    // Perform 2's complement for accelerometer
    int16_t real_x1 = ~(x1 - 1);
    int16_t real_y1 = ~(y1 - 1);
    int16_t real_z1 = ~(z1 - 1);
    // Perform 2's complement for gyroscope
    int16_t real_x3 = ~(x3 - 1);
    int16_t real_y3 = ~(y3 - 1);
    int16_t real_z3 = ~(z3 - 1);

    // Print values to serial monitor
    
    // Accelerometer
    Serial.println("Compass");
    Serial.print("X: ");
    Serial.println(real_x1);
    Serial.print("Y: ");
    Serial.println(real_y1);
    Serial.print("Z: ");
    Serial.println(real_z1);
    // Accelerometer
    Serial.println("Accelerometer");
    Serial.print("X: ");
    Serial.println(x2);
    Serial.print("Y: ");
    Serial.println(y2);
    Serial.print("Z: ");
    Serial.println(z2);
    // Gyroscope
    Serial.println("Gyroscope");
    Serial.print("X: ");
    Serial.println(real_x3);
    Serial.print("Y: ");
    Serial.println(real_y3);
    Serial.print("Z: ");
    Serial.println(real_z3);
  }
  else
  {
    Serial.println("Failed to read from one or more sensors");
  }
  delay(1000); // Run again after delay
}
int ReadByte(uint8_t addr, uint8_t reg, uint8_t *data) // Read a byte on the i2c interface
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
    return -1; // Read nothing back
  }
  return 0;
}
void WriteByte(uint8_t addr, uint8_t reg, byte data) // Write a byte on the i2c interface
{
  Wire.beginTransmission(addr); // Begin the write sequence
  Wire.write(reg); // First byte is to set the register pointer
  Wire.write(data); // Write the data byte
  Wire.endTransmission(); // End the write sequence and perform actual transmission of bytes
}

