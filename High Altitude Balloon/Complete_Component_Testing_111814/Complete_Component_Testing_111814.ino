/* Testing framework for all components in HAB telemetry package */
// CFC sensor --> A11
// Methane sensor --> A12
// PIR sensor --> 6
// Smoke signal relay --> 8
// Siren relay --> 7
// Altimeter --> I2C
// Sensiron (Parallax) temp/humidity sensor --> I2C
// Accelerometer --> A13/A14/A15
// Compass --> I2C
// Gyroscope --> I2C
// GPS --> Serial1
// SD shield --> "SdInfo" sketch in SdFat examples
// GPRS shield --> Call of "AT+CCLK?" AT command to check time

#include <IntersemaBaro.h>
#include <SdFat.h>
#include <SHT1x.h>
#include <TinyGPS.h>
#include <Wire.h>

#define sensironSDA 25
#define sensironSCL 27
#define sdSlaveSelect 53  // SPI slave select pin (SS) for SD card (Uno: 10 / Mega: 53)

#define compAddr 0x1E  // Compass
#define gyroAddr 0x69  // Gyroscope

// Digital pins
const int debugPin = 2;  // Pin to trigger "altLandDetect = true" for debugging
const int smsLED = 3;  // Pin for LED to debug SMS command execution
const int readyLED = 4;  // Pin for LED to indicate entry of main loop (program start)
const int landLED = 5;  // Pin for LED to indicate detection of landing
const int pirPin = 6;	// Pin for detecting motion with PIR sensor during post-landing phase
const int piezoPin = 7;  // Pin for executing piezo buzzer on SMS command
const int smokePin = 8;  // Pin for triggering smoke signal on SMS command
const int gprsPowPin = 9;  // Pin for software power-up of GPRS shield
// Analog pins
const int cfcPin = A11;  // Pin for reading CFC levels from sensor
const int metPin = A12;  // Pin for reading methane levels from sensor
const int accelXPin = A15;  // Accelerometer X axis
const int accelYPin = A14;  // Accelerometer Y axis
const int accelZPin = A13;  // Accelerometer Z axis

unsigned long age;
float flat, flon, altitude; 
int satellites, hdop;

// Initialize sensor libraries
SHT1x sht1x(sensironSDA, sensironSCL);  // Temperature & humidity (Sensiron)
TinyGPS gps;  // GPS
Intersema::BaroPressure_MS5607B baro(true);  // Altimeter

SdFat sd;
SdFile myFile;

void setup() {
  pinMode(piezoPin, OUTPUT);
  pinMode(smokePin, OUTPUT);
  pinMode(pirPin, INPUT);

  digitalWrite(piezoPin, LOW);
  digitalWrite(smokePin, LOW);

  Serial.begin(19200);

  Serial1.begin(4800);  // GPS
  if(!Serial1.available()) {
    while(!Serial1.available()) {
      delay(1);
    }
  }
  Serial2.begin(19200);  // GPRS

  // Software power-up of GPRS shield
  digitalWrite(gprsPowPin, LOW);
  delay(100);
  digitalWrite(gprsPowPin, HIGH);
  delay(500);
  digitalWrite(gprsPowPin, LOW);
  delay(100);

  Wire.begin();
  baro.init();

  if(!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

  Serial.println(F("Initiating component testing."));
  Serial.println();
}

void loop() {
  // CFC sensor
  Serial.println(F("Testing CFC sensor."));
  for(int p = 0; p < 5; p++) {
    int cfcVal = analogRead(cfcPin);
    Serial.println(cfcVal);
    delay(500);
  }
  // Methane sensor
  Serial.println(F("Testing methane sensor."));
  for(int q = 0; q < 5; q++) {
    int metVal = analogRead(metPin);
    Serial.println(metVal);
    delay(500);
  }
  // PIR sensor
  Serial.println(F("Testing PIR sensor."));
  long startTime = millis();
  int pirCount = 1;
  while(millis() - startTime < 5000) {
    if(pirCount = 10) Serial.println();
    int pirVal = digitalRead(pirPin);
    Serial.println(pirVal);
    pirCount++;
    delay(100);
  }
  // Smoke signal relay
  Serial.println(F("Testing smoke signal relay."));
  for(int s = 0; s < 5; s++) {
    digitalWrite(smokePin, HIGH);
    Serial.print(F("ON "));
    delay(500);
    digitalWrite(smokePin, LOW);
    Serial.print(F("OFF "));
    delay(500);
  }
  Serial.println();
  // Siren relay
  Serial.println(F("Testing siren relay."));
  for(int t = 0; t < 5; t++) {
    digitalWrite(piezoPin, HIGH);
    Serial.print(F("ON "));
    delay(500);
    digitalWrite(piezoPin, LOW);
    Serial.print(F("OFF "));
    delay(500);
  }
  Serial.println();
  // Altimeter
  Serial.println(F("Testing altimeter."));
  for(int u = 0; u < 5; u++) {
    readAltimeter();
    delay(500);
  }
  // Sensiron (Parallax) temp/humidity sensor
  Serial.println(F("Testing temperature/humidity sensor."));
  for(int v = 0; v < 5; v++) {
    getTempHumid();
    delay(500);
  }
  // Accelerometer
  Serial.println(F("Testing accelerometer."));
  for(int w = 0; w < 5; w++) {
    readAccelerometer();
    delay(500);
  }
  // Compass
  Serial.println(F("Testing compass."));
  for(int x = 0; x < 5; x++) {
    readCompass();
    delay(500);
  }
  // Gyroscope
  Serial.println(F("Testing gyroscope."));
  for(int y = 0; y < 5; y++) {
    readGyroscope();
    delay(500);
  }
  // GPS
  Serial.println(F("Testing GPS."));
  for(int z = 0; z < 5; z++) {
    gpsGetData();
    delay(500);
  }
  // SD shield
  Serial.println(F("Testing SD shield."));
  sdReadWrite();
  // GPRS shield
  Serial.println(F("Testing GPRS shield."));
  gprsDateTime();

  endProgram();
}

void readAltimeter() {
  long altCm = baro.getAltitude();
  float altFt = (float)altCm / 30.48;
  float altPressMillibar = (float)baro.getPressure() / 100.0;

  Serial.print(F("Alt. cm: "));
  Serial.print(altCm);
  Serial.print(F(" cm / Alt. ft: "));
  Serial.print(altFt, 2);
  Serial.print(F(" ft / Press. millibar: "));
  Serial.print(altPressMillibar, 2);
  Serial.println(F(" mbar"));
}

// Pololu Accelerometer
void readAccelerometer() {
  int accelX = analogRead(accelXPin);
  int accelY = analogRead(accelYPin);
  int accelZ = analogRead(accelZPin);

  String accelerometerString = String(accelX) + "," + String(accelY) + "," + String(accelZ);

  Serial.print(F("Accel. values: "));
  Serial.println(accelerometerString);
}

// Compass
void readCompass() {
  uint8_t x_msb; // X-axis most significant byte
  uint8_t x_lsb; // X-axis least significant byte
  uint8_t y_msb; // Y-axis most significant byte
  uint8_t y_lsb; // Y-axis least significant byte
  uint8_t z_msb; // Z-axis most significant byte
  uint8_t z_lsb; // Z-axis least significant byte
  // Get the value from the sensor
  if((i2cReadByte(compAddr, 0x3, &x_msb) == 0) &&
    (i2cReadByte(compAddr, 0x4, &x_lsb) == 0) &&
    (i2cReadByte(compAddr, 0x5, &y_msb) == 0) &&
    (i2cReadByte(compAddr, 0x6, &y_lsb) == 0) &&
    (i2cReadByte(compAddr, 0x7, &z_msb) == 0) &&
    (i2cReadByte(compAddr, 0x8, &z_lsb) == 0)) {
    uint8_t compX = x_msb << 8 | x_lsb;
    uint8_t compY = y_msb << 8 | y_lsb;
    uint8_t compZ = z_msb << 8 | z_lsb;

    String compassString = String(compX) + "," + String(compY) + "," + String(compZ);

    Serial.print(F("Comp. values: "));
    Serial.println(compassString);
  }
  else Serial.println(F("Failed to read from sensor."));
}

void readGyroscope() {
  int gyroX, gyroY, gyroZ;
  uint8_t x_msb; // X-axis most significant byte
  uint8_t x_lsb; // X-axis least significant byte
  uint8_t y_msb; // Y-axis most significant byte
  uint8_t y_lsb; // Y-axis least significant byte
  uint8_t z_msb; // Z-axis most significant byte
  uint8_t z_lsb; // Z-axis least significant byte

  if((i2cReadByte(gyroAddr, 0x29, &x_msb) == 0) &&
    (i2cReadByte(gyroAddr, 0x28, &x_lsb) == 0) &&
    (i2cReadByte(gyroAddr, 0x2B, &y_msb) == 0) &&
    (i2cReadByte(gyroAddr, 0x2A, &y_lsb) == 0) &&
    (i2cReadByte(gyroAddr, 0x2D, &z_msb) == 0) &&
    (i2cReadByte(gyroAddr, 0x2C, &z_lsb) == 0)) {
    uint16_t x = x_msb << 8 | x_lsb;
    uint16_t y = y_msb << 8 | y_lsb;
    uint16_t z = z_msb << 8 | z_lsb;

    gyroX = (int)x;
    gyroY = (int)y;
    gyroZ = (int)z;

    String gyroString = String(gyroX) + String(gyroY) + String(gyroZ);

    Serial.print(F("Gyro. values: "));
    Serial.println(gyroString);
  }
  else Serial.println(F("Failed to read from sensor."));
}

void getTempHumid() {
  float temp_c;
  float temp_f;
  float humidity;

  // Read values from the sensor
  temp_c = sht1x.readTemperatureC();
  temp_f = sht1x.readTemperatureF();
  humidity = sht1x.readHumidity();

  // Print the values to the serial port
  Serial.print(F("Temperature: "));
  Serial.print(temp_c, 2);
  Serial.print(F(" C / "));
  Serial.print(temp_f, 2);
  Serial.print(F(" F / Humidity: "));
  Serial.print(humidity);
  Serial.println(F("%"));
}

// Read a byte on the i2c interface
int i2cReadByte(uint8_t addr, uint8_t reg, uint8_t *data) {
  // Do an i2c write to set the register that we want to read from
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  // Read a byte from the device
  Wire.requestFrom(addr, (uint8_t)1);
  if(Wire.available()) *data = Wire.read();
  else return -1;  // Read nothing back
  return 0;
}

// Write a byte on the i2c interface
void i2cWriteByte(uint8_t addr, uint8_t reg, byte data) {
  Wire.beginTransmission(addr);  // Begin the write sequence
  Wire.write(reg);  // First byte is to set the register pointer
  Wire.write(data);  // Write the data byte
  Wire.endTransmission();  // End the write sequence; bytes are actually transmitted now
}

void sdReadWrite() {
  // open the file for write at end like the Native SD library
  if(!myFile.open("test.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening test.txt for write failed");
  }
  Serial.println(F("Writing to SD file."));
  // if the file opened okay, write to it:
  myFile.println("testing 1, 2, 3.");
  // close the file:
  myFile.close();
  Serial.println(F("Reading SD file."));
  // re-open the file for reading:
  if (!myFile.open("test.txt", O_READ)) {
    sd.errorHalt("opening test.txt for read failed");
  }
  // read from the file until there's nothing else in it:
  int data;
  while((data = myFile.read()) > 0) Serial.write(data);
  // close the file:
  myFile.close();
}

void gprsDateTime() {
  Serial2.println("AT+CCLK?");
  if(!Serial2.available()) {
    while(!Serial2.available()) {
      delay(1);
    }
  }
  while(Serial2.available()) {
    char c = Serial2.read();
    Serial.print(c);
    delay(10);
  }
}

void gpsGetData() {
  boolean newdata = false;
  unsigned long start = millis();
  while(millis() - start < 1000) {  // Update every 5 seconds
    if(feedgps()) newdata = true;
    if(newdata) gpsdump(gps);
  }
  Serial.print(flat, 4);
  Serial.print(F(", "));
  Serial.print(flon, 4);
  Serial.print(F(" - Elev: "));
  Serial.print(altitude, 2);
  Serial.print(F(" meters / Satellites: "));
  Serial.print(satellites);
  Serial.print(F(" / HDOP: "));
  Serial.println(hdop);
}

void gpsdump(TinyGPS &gps) {
  satellites = gps.satellites();
  hdop = gps.hdop();
  gps.f_get_position(&flat, &flon, &age);
  altitude = gps.f_altitude();
}

// Feed
boolean feedgps() {
  while(Serial1.available()) {
    if(gps.encode(Serial1.read())) return true;
  }
  return false;
}

void endProgram() {
  boolean programComplete = true;
  Serial.println(F("Component test complete."));
  while(programComplete == true) {
    delay(1000);
  }
}
