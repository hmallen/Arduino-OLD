/*
BeachGuard - Locked personal beach container with alarm
 
 Functions to add:
 - Passcode input
 - Multiple wrong entry penalty
 -- Trigger alarm or lockout for defined period
 --- SPST switch to set function
 - Delay b/w code input and arming
 
 Design considerations:
 - Battery compartment should not be accessible (ie. no way to deactivate w/o dearming via code entry)
 - Pro vs con on all-in-one device with attached storage or unit one may add to any container
 - Alarm, vibration, or both
 */

#include <Wire.h>

// Constant definitions
#define powerLED 2
#define armedLED 3
#define codeKey1 4
#define codeKey2 5
#define codeKey3 6
#define penaltySelect 7
#define modeSelect 8

// Debugging functions
#define debugMode

/*  Accelerometer definitions  */
// Possible sensor addresses (DIP switch positions)
#define SENSOR_ADDR_OFF  (0x1D)
#define SENSOR_ADDR_ON  (0x53)

// Set sensor address
const uint8_t sensorAddr = SENSOR_ADDR_OFF;

// Sensor register addresses (datasheet)
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

void setup() {
  Serial.begin(38400);
  Wire.begin();

  // Accelerometer setup operations
  WriteByte(sensorAddr, REG_BW_RATE_ADDR, 0X08);  // Set 25Hz output data rate and 25Hz bandwidth and disable low power mode
  WriteByte(sensorAddr, REG_PWR_CTL_ADDR, 0x08);  // Disable auto sleep
  WriteByte(sensorAddr, REG_INT_ENABLE_ADDR, 0x0);  // Disable interrupts (the pins are not brought out anyway)

  // Read the device ID to verify as correct sensor
  if (ReadByte(sensorAddr, 0x0, &devId) != 0) {
    Serial.println(F("Cannot read device ID from sensor"));
  }
  else if (devId != 0xE5) {
    Serial.print(F("Wrong/invalid device ID "));
    Serial.print(devId);
    Serial.println(F(" expected 0xE5)"));
  }
  else {
    Serial.println(F("Accelerometer setup complete. Device ready."));
  }

  delay(5000);

  pinMode(powerLED, OUTPUT);
  pinMode(armedLED, OUTPUT);
  pinMode(codeKey1, INPUT);
  pinMode(codeKey2, INPUT);
  pinMode(codeKey3, INPUT);

  digitalWrite(powerLED, LOW);
  digitalWrite(armedLED, LOW);
}

void loop() {

}

void calibrateAccl() {

}

void zeroAccl() {

}

void readAccl() {
  if ((ReadByte(sensorAddr, REG_DATAX1_ADDR, &x_msb) == 0) &&
    (ReadByte(sensorAddr, REG_DATAX0_ADDR, &x_lsb) == 0) &&
    (ReadByte(sensorAddr, REG_DATAY1_ADDR, &y_msb) == 0) &&
    (ReadByte(sensorAddr, REG_DATAY0_ADDR, &y_lsb) == 0) &&
    (ReadByte(sensorAddr, REG_DATAZ1_ADDR, &z_msb) == 0) &&
    (ReadByte(sensorAddr, REG_DATAZ0_ADDR, &z_lsb) == 0))
  {
    x = (x_msb << 8) | x_lsb;
    y = (y_msb << 8) | y_lsb;
    z = (z_msb << 8) | z_lsb;

    // Perform 2's complement
    int16_t real_x = ~(x - 1);
    int16_t real_y = ~(y - 1);
    int16_t real_z = ~(z - 1);
  }
  else {
    Serial.println(F("Failed to read from sensor."));
    errorFlash();
  }
}

// Read a byte on the i2c interface
int ReadByte(uint8_t addr, uint8_t reg, uint8_t *data) {
  // Do an i2c write to set the register that will be read
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();

  // Read a byte from the device
  Wire.requestFrom(addr, (uint8_t)1);
  if (Wire.available()) {
    *data = Wire.read();
  }
  else {
    return -1;  // Read nothing back
  }
  return 0;
}

// Write a byte on 12c interface
void WriteByte(uint8_t addr, uint8_t reg, byte data) {
  Wire.beginTransmission(addr);  // Begin the write sequence
  Wire.write(reg);  // First byte set to register pointer
  Wire.write(data);  // Write data byte
  Wire.endTransmission();  // End write sequence and transmit bytes
}

void errorFlash() {
  boolean endProgram = true;
  while(endProgram == true) {
    digitalWrite(powerLED, HIGH);
    delay(100);
    digitalWrite(powerLED, LOW);
    delay(500);
  }
}
