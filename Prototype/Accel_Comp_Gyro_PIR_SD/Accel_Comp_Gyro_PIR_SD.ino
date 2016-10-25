/*  1 - Accelerometer, Compass, Gyroscope, and Motion Sensor w/ SD Card  */

#include <Wire.h>
#include <SD.h>
//#include "IntersemaBaro.h"

//Intersema::BaroPressure_MS5607B baro(true);

/* DIP switch registers not pre-defined for accelerometer or gyroscope, 
 so each must be set to "1" position at all times for proper function! */

void setup() {

  Serial.begin(9600); // Start the serial port for output and wait for connection
  
  Serial.println("Warming up motion sensor...");
  delay(1000); // Motion sensor must warm-up for at least 20 sec for proper function (Set low for debugging purposes)
  Serial.println("Motion sensor ready for use.");

  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT); // Pin 10 must be set to OUTPUT for proper SD library function

  // Attempt to initialize SD card on pin 10 (Seeedstudio SD shield chipSelect pin)
  if (!SD.begin(10)) {
    Serial.println("Card failed, or not present.");
    return;
  }

  Serial.println("Card initialized."); // Confirm initialization of SD card

  String headerString = "Accel_X,Accel_Y,Accel_Z,Comp_X,Comp_Y,Comp_Z,Gyro_X,Gyro_Y,Gyro_Z,Motion(0/1)"; // String to identify written values

  File dataFile = SD.open("DATATEST.TXT", FILE_WRITE); // Open or create file to write file header

  // If file available, write to it
  if (dataFile) {
    dataFile.println(headerString); // Write headerString at top of new data write
    dataFile.close();
    Serial.println(headerString); // Print headerString written at top of new data write to serial

  }
  // If file unavailable, display an error message
  else {
    Serial.println("Error opening DATATEST.TXT");
  }

  Wire.begin(); // Join the I2C bus as master
  
  //baro.init(); // Initialize barometer

  // Configure accelerometer settings
  WriteByte(0x1D, 0x2c, 0X08); // Set 25Hz output data rate and 25Hz bandwidth and disable low power mode  
  WriteByte(0x1D, 0x2d, 0x08); // Disable auto sleep
  WriteByte(0x1D, 0x2e, 0x0); // Disable interrupts (the pins are not brought out anyway)

  // Configure compass settings
  WriteByte(0x1E, 0x0, 0x10); // Configure the compass to default values (see datasheet for details)
  WriteByte(0x1E, 0x1, 0x20); // Configure the compass to default values (see datasheet for details)
  WriteByte(0x1E, 0x2, 0x0); // Set compass to continuous-measurement mode (default is single shot)

}

void loop() { // Begin main program loop

    String dataString = ""; // Create string for writing data

  // Assign MSB/LSB for accelerometer
  uint8_t x1_msb; // X-axis most significant byte
  uint8_t x1_lsb; // X-axis least significant byte
  uint8_t y1_msb; // Y-axis most significant byte
  uint8_t y1_lsb; // Y-axis least significant byte
  uint8_t z1_msb; // Z-axis most significant byte
  uint8_t z1_lsb; // Z-axis least significant byte
  // Assign 16-bit, unsigned output variables for accelerometer
  uint16_t x1;
  uint16_t y1;
  uint16_t z1;

  // Assign MSB/LSB for compass
  uint8_t x2_msb; // X-axis most significant byte
  uint8_t x2_lsb; // X-axis least significant byte
  uint8_t y2_msb; // Y-axis most significant byte
  uint8_t y2_lsb; // Y-axis least significant byte
  uint8_t z2_msb; // Z-axis most significant byte
  uint8_t z2_lsb; // Z-axis least significant byte
  // Assign integer output variables for compass
  int x2;
  int y2;
  int z2;

  // Assign MSB/LSB for gyroscope
  uint8_t x3_msb; // X-axis most significant byte
  uint8_t x3_lsb; // X-axis least significant byte
  uint8_t y3_msb; // Y-axis most significant byte
  uint8_t y3_lsb; // Y-axis least significant byte
  uint8_t z3_msb; // Z-axis most significant byte
  uint8_t z3_lsb; // Z-axis least significant byte
  // Assign 16-bit, unsigned output variables for gyroscope
  uint16_t x3;
  uint16_t y3;
  uint16_t z3;

  /* Get values from the I2C sensors. Digital & analog reads not assigned variables to save space. */

  // Altimeter (Barometer/Thermometer)
  //int alt = baro.getHeightCentiMeters();

  if ((ReadByte(0x1D, 0x33, &x1_msb) == 0) && // Accelerometer reading confirmation
  (ReadByte(0x1D, 0x32, &x1_lsb) == 0) &&
    (ReadByte(0x1D, 0x35, &y1_msb) == 0) &&
    (ReadByte(0x1D, 0x34, &y1_lsb) == 0) &&
    (ReadByte(0x1D, 0x37, &z1_msb) == 0) &&
    (ReadByte(0x1D, 0x36, &z1_lsb) == 0) &&
    (ReadByte(0x1E, 0x3, &x2_msb) == 0) && // Compass reading confirmation
  (ReadByte(0x1E, 0x4, &x2_lsb) == 0) &&
    (ReadByte(0x1E, 0x5, &y2_msb) == 0) &&
    (ReadByte(0x1E, 0x6, &y2_lsb) == 0) &&
    (ReadByte(0x1E, 0x7, &z2_msb) == 0) &&
    (ReadByte(0x1E, 0x8, &z2_lsb) == 0) &&
    (ReadByte(0x69, 0x1d, &x3_msb) == 0) && // Gyroscope reading confirmation
  (ReadByte(0x69, 0x1e, &x3_lsb) == 0) &&
    (ReadByte(0x69, 0x1f, &y3_msb) == 0) &&
    (ReadByte(0x69, 0x20, &y3_lsb) == 0) &&
    (ReadByte(0x69, 0x21, &z3_msb) == 0) &&
    (ReadByte(0x69, 0x22, &z3_lsb) == 0))
  {
    // Accelerometer (temporary) variable defined
    x1 = (x1_msb << 8) | x1_lsb;
    y1 = (y1_msb << 8) | y1_lsb;
    z1 = (z1_msb << 8) | z1_lsb;

    // Compass variable defined
    x2 = (x2_msb << 8) | x2_lsb;
    y2 = (y2_msb << 8) | y2_lsb;
    z2 = (z2_msb << 8) | z2_lsb;

    // Gyroscope (temporary) variable defined
    x3 = (x3_msb << 8) | x3_lsb;
    y3 = (y3_msb << 8) | y3_lsb;
    z3 = (z3_msb << 8) | z3_lsb;

    // Perform 2's complement for accelerometer and define variable for final value
    int16_t real_x1 = ~(x1 - 1);
    int16_t real_y1 = ~(y1 - 1);
    int16_t real_z1 = ~(z1 - 1);

    // Perform 2's complement for gyroscope and define variable for final value
    int16_t real_x3 = ~(x3 - 1);
    int16_t real_y3 = ~(y3 - 1);
    int16_t real_z3 = ~(z3 - 1);

    /* Append data to string */

    // Accelerometer
    dataString += String(real_x1);
    dataString += ",";
    dataString += String(real_y1);
    dataString += ",";
    dataString += String(real_z1);

    dataString += ",";

    // Compass
    dataString += String(x2);
    dataString += ",";
    dataString += String(y2);
    dataString += ",";
    dataString += String(z2);

    dataString += ",";

    // Gyroscope
    dataString += String(real_x3);
    dataString += ",";
    dataString += String(real_y3);
    dataString += ",";
    dataString += String(real_z3);

    dataString += ",";

    // Motion sensor
    dataString += String(digitalRead(2));
    
    //dataString += ",";
    
    // Altimeter
    //dataString+= String(alt);

    /* Write string to SD card */

    File dataFile = SD.open("DATATEST.TXT", FILE_WRITE); // Open or create file to which dataString will be written

    // If file available, write to it
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println(dataString); // Print string written to SD card to confirm
    }

    // If file unavailable, display an error message
    else {
      Serial.println("Error opening DATATEST.TXT");
    }

    /* Print values to serial monitor */

    // Accelerometer
    Serial.println("Accelerometer");
    Serial.print("X: ");
    Serial.println(real_x1);
    Serial.print("Y: ");
    Serial.println(real_y1);
    Serial.print("Z: ");
    Serial.println(real_z1);

    // Compass
    Serial.println("Compass");
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
/*
    // Altimeter
    Serial.println("Altitimeter");
    Serial.print("Height(cm): ");
    Serial.println((float)(alt));
    Serial.print("Height(ft): ");
    Serial.println((float)(alt)/30.48); 
*/
    // Motion sensor
    Serial.println("Motion");
    Serial.println(digitalRead(2), DEC);
  }

  // If one or more sensors not found, display an error
  else
  {
    Serial.println("Failed to read from one or more sensors.");
  }

  delay(1000); // Run again after delay

} // End main program loop

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
