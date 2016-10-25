/*
Arduino module to be used on a high-altitude balloon flight as a
 companion to the Raspberry Pi + PiInTheSky telemetry package.
 
 IntersemaBaro library has been overhauled and currently includes
 two additional functions that return temperature and pressure,
 respectively.
 
 To Do:
 - Add line(s) in altCheck() to detect landing event and change boolean when appropriate
 - Take initial and final altitude of x consecutive reads and calc ascent rate
 - Add course_to to show direction to balloon position
 - Create modified driftCompensation() function that is dependent on millis() instead of loopCount
 - Add CFC sensor
 -> Create voltage regulation function for heating element
 - **** BUY AND ADD CO2 AND METHANE SENSORS ****
 - Add function for landDetect boolean variable
 -> If statement to set final location coordinates for comparison
 -> Add landing detect to altimeter read function
 - Add piezo signal after landing detected
 - Add ability to send SMS to receive updated location information
 - GPS functions
 -> **** Calculate ascent rate!!!! ****
 -> Total distance travelled
 -> **** Use GPS altitude as initial altitude for altimeter calibration after zeroing ****
 - I2C sensor functions
 -> If gyroscope detecting rotation ---> Perform necessary camera adjustments
 -> Align compass and camera axes
 -> If possible, add servo function to rotate camera during flight and log direction
 -> Make gyroscope reads at interval, and collect at maximum acquisition rate
 --> Idea: If quick rotation detected, begin fast gyro data acquisition
 */

#define debugMode
//#define liveLaunch

#include "IntersemaBaro.h"
#include <SdFat.h>
#include <TinyGPS.h>
#include <Wire.h>

#define gprsPowPin 9  // Pin for software power-up of GPRS shield  

#define sdSlaveSelect 53  // SPI slave select pin (SS) for SD card (Uno: 10 / Mega: 53)

const int gpsGmtOffsetHours = 5;  // Offset in hours of current timezone from GMT
const int gpsGmtOffsetQuarters = gpsGmtOffsetHours * -4;  // Required for proper AT command formatting of the GPS-->GPRS sync string

// Accelerometer
#define accelAddressOff  0x1D
#define accelAddressOn  0x53
// Compass
#define compAddress 0x1E
// Gyroscope
#define gyroAddressOff 0x69
#define gyroAddressOn 0x68

// Define constants from #define's that are based on DIP switch positions
const uint8_t compAddr = compAddress;  // Set I2C address of compass (defined by hardware)
const uint8_t accelAddr = accelAddressOff;  // Set I2C address of accelerometer based on DIP switch position (as defined by hardware)
const uint8_t gyroAddr = gyroAddressOff;  // Set I2C address of gyroscope based on DIP switch position (as defined by hardware)

// Accelerometer register addresses (datasheet)
#define REG_DEVID_ADDR 0x00
#define REG_THRESH_TAP_ADDR 0x1d
#define REG_TAP_DUR_ADDR 0x21
#define REG_TAP_LATENCY_ADDR 0x22
#define REG_TAP_WINDOW_ADDR 0x23
#define REG_BW_RATE_ADDR 0x2c
#define REG_PWR_CTL_ADDR 0x2d
#define REG_INT_ENABLE_ADDR 0x2e
#define REG_DATA_FORMAT_ADDR 0x31
#define REG_DATAX0_ADDR 0x32
#define REG_DATAX1_ADDR 0x33
#define REG_DATAY0_ADDR 0x34
#define REG_DATAY1_ADDR 0x35
#define REG_DATAZ0_ADDR 0x36
#define REG_DATAZ1_ADDR 0x37
#define REG_FIFO_CTL_ADDR 0x38

const int piezoPin = 3;  // Pin for piezo buzzer beep on successful RFID read
const int readyLED = A0;  // Pin for LED to indicate entry of main loop

const char smsTargetNum[11] = "2145635266";  // Mobile number to send data via SMS

SdFat sd;
SdFile dataFile;

TinyGPS gps;

Intersema::BaroPressure_MS5607B baro(true);

String gprsRawDateTime;  // Stores raw date and time info from GPRS before parsing

// Date
String gprsYear;  // Year parsed from gprsRawDateTime
String gprsMonth;  // Month parsed from gprsRawDateTime
String gprsDay;  // Day parsed from gprsRawDateTime
String gprsDateString;  // Parsed/concatenated date info from GPRS that is written to SD
// Time
String gprsHour;  // Hour parsed from gprsRawDateTime for 12hr time construction
String gprsMinute;  // Minute parsed from gprsRawDateTime for 12hr time construction
String gprsSecond;  // Second parsed from gprsRawDateTime for 12hr time construction
String gprsTimeString;  // Parsed/concatenated time info from GPRS that is written to SD

// Altimeter
boolean altZeroedPrev = false;
const float tempCorrection = 4.65;
long altCalibVal, altCm, altCmInitial, altCmPrev;
int altPosRead, altNegRead;
float altFtFloat, altTempCFloat, altPressMillibarFloat;
unsigned long altCmMax, altFtMax;  // Can I make some of these variables local instead of global?
String altFt;
String altTempC;
String altPressMillibar;
String altimeterString;

// Accelerometer, Compass, and Gyroscope
String accelerometerString, compassString, gyroscopeString;

// GPS
const int gpsSatMinimum = 5;  // May want to change to 6 for real launch
const int gpsHDOPMinimum = 250;  // May want to change to 200 for real launch
int satellites, hdop;
float flat, flon, altitude, course, speed;
float gpsLatInitial, gpsLonInitial;
float gpsLatFinal, gpsLonFinal;
float gpsAltitudeFtFloat;
float landDistanceFtFloat;
String gpsDateTime;
String gpsLat, gpsLon;
String gpsAltitudeFt;
String gpsSpeedMPH;
String gpsCourse;
String landDistanceFt;
String gpsDataString;  // String containing all data from GPS

boolean gpsLock = false;  // Becomes true when sufficient satellite lock and HDOP (precision) are achieved
boolean syncRequired = true;  // Remains true until GPRS syncs date/time with GPS
String gpsSyncString;  // String formatted for GPRS AT commands to sync date/time
long gpsAltOffsetCm = 0;  // Altitude offset for altimeter based on initial GPS elevation reading

String dataString;  // Full data string written to SD card

boolean firstLoop = true;
boolean startupComplete = false;  // Triggered when setup() complete
boolean altAscentDetect =  false;  // Triggered when ascent detected for 5 consecutive reads
boolean altDescentDetect = false;  // Triggered when descent detected for 5 consecutive reads
boolean altLandDetect = false;  // Triggered when altitude (and possibly other sensor) reads indicate landing
boolean gpsAscentDetect = false;  // ^^^^
boolean gpsDescentDetect = false;  // ^^^^
boolean gpsLandDetect = false;  // ^^^^

void setup() {
  pinMode(piezoPin, OUTPUT);
  pinMode(readyLED, OUTPUT);

  digitalWrite(piezoPin, LOW);
  digitalWrite(readyLED, LOW);

  Serial.begin(19200);
  Serial1.begin(4800);
  if(!Serial1.available()) {
    while(!Serial1.available()) {
      delay(1);
    }
  }
  Serial2.begin(19200);  // Software serial for GPRS communication
  Serial3.begin(19200);

  gpsGetData();
  if(satellites < gpsSatMinimum || hdop > gpsHDOPMinimum) {
    while(satellites < 4 || hdop > 200) {
      gpsGetData();
      delay(5000);
    }
  }
  gpsLock = true;
  gprsPowerOn();

  Wire.begin();
  configureSensors();
  baro.init();

  if(!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }

  readAltimeter();
  while(altCm >= 152) {
    zeroAltimeter();
    altZeroedPrev = true;
    delay(2000);
    readAltimeter();
  }
  gpsGetData();

  configureGPRS();

  gpsGetData();
  syncDateTime();
  syncRequired = false;

  checkAltCalib();

  gpsGetData();
  gpsAltOffsetCm = altitude * 100;

  startupComplete = true;
  digitalWrite(readyLED, HIGH);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  gpsGetData();
  readAltimeter();
  readAccelerometer();
  readCompass();
  readGyroscope();
  constructDataString();
  if(firstLoop == true) firstLoop != firstLoop;
  else {
#ifdef debugMode
    Serial.println(dataString);
    Serial.println();
#endif
    if(!dataFile.open("HABLOG.txt", O_RDWR | O_CREAT | O_AT_END)) {
      sd.errorHalt("Write of dataString to SD failed.");
    }
    dataFile.println(dataString);
    dataFile.close();
  }

  //if(landDetect == true) ... do something super neat like SMS and other phoentastic stuff

  delay(5000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void constructDataString() {
  dataString = "$";
  dataString += gpsDataString;
  dataString += "$";
  dataString += altimeterString;
  dataString += "$";
  dataString += accelerometerString;
  dataString += "$";
  dataString += compassString;
  dataString += "$";
  dataString += gyroscopeString;
}

/*==================
 ////  Sensors  ////
 =================*/

void readAltimeter() {
  altimeterString = "";

  altCm = baro.altGetAlt() - altCalibVal + gpsAltOffsetCm;
  altFtFloat = (float)altCm / 30.48;
  altTempCFloat = ((float)baro.altGetTemp() / 100.00) + tempCorrection;
  altPressMillibarFloat = (float)baro.altGetPress() / 100.00;
  //unsigned long altPressPascal = baro.altGetPress();
  //altPressMillibarFloat = (float)altPressPascal / 100.00;

  altCheck();
  altFloatToString();

  altimeterString += altFt;
  altimeterString += ",";
  altimeterString += altTempC;
  altimeterString += ",";
  altimeterString += altPressMillibar;
}

void altCheck() {
  if(altCm > altCmMax) altCmMax = altCm;

  int altCmDiff = altCm - altCmPrev;
  const int altCmChangeThreshold = 5;  // Difference in consecutive altimeter reads (cm) to qualify as a real change in altitude

  // CONSIDER MODIFYING THESE FXNS TO USE ONLY ONE INTEGER VARIABLE FOR COUNTING (i.e. "altConsecRead")
  if(altAscentDetect == false) {
    if(altCmDiff > altCmChangeThreshold) altPosRead++;
    else if(altCmDiff < altCmChangeThreshold) altPosRead = 0;  // Latest read doesn't carry same sign as previous, so reset positive read counter
    else;  // No change in altitude detected between reads, so do nothing
    if(altPosRead == 5) altAscentDetect = true;
#ifdef debugMode
    if(startupComplete == true) {
      Serial.print(F("altPosRead: "));
      -
        Serial.println(altPosRead);
    }
    if(altAscentDetect == true) Serial.println(F("Ascent stage detected."));
#endif
  }
  else if(altAscentDetect == true && altDescentDetect == false) {
    if(altCmDiff < -altCmChangeThreshold) altNegRead++;
    else if(altCmDiff > -altCmChangeThreshold) altNegRead = 0;  // Latest read doesn't carry same sign as previous, so reset negative read counter
    else;  // No change in altitude detected between reads, so do nothing
    if(altNegRead == 5) altDescentDetect = true;
#ifdef debugMode
    if(startupComplete == true) {
      Serial.print(F("altNegRead: "));
      Serial.println(altNegRead);
    }
    if(altDescentDetect == true) Serial.println(F("Descent stage detected."));
#endif
  }
  else if(altAscentDetect == true && altDescentDetect == true) {
    // Something to wait for no change in altitude to indicate landing
  }
#ifdef debugMode
  else Serial.println(F("ERROR: Descent detected without first detecting ascent!"));
#endif
  altCmPrev = altCm;
}

void altFloatToString() {
  char c;

  altFt = "";
  Serial3.print(altFtFloat, 2);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      altFt += c;
      delay(1);
    }
  }

  altTempC = "";
  Serial3.print(altTempCFloat, 2);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      altTempC += c;
      delay(1);
    }
  }

  altPressMillibar = "";
  Serial3.print(altPressMillibarFloat, 2);
  if(!Serial3.available()) {
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      altPressMillibar += c;
      delay(1);
    }
  }
}

void readAccelerometer() {
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
  if(ReadByte(accelAddr, 0x0, &devId) != 0) Serial.println("Cannot read device ID from sensor");
  else if(devId != 0xE5) {
    Serial.print("Wrong/invalid device ID ");
    Serial.print(devId);
    Serial.println(" expected 0xE5)");
  }
  else {
    // Read the output
    if((ReadByte(accelAddr, REG_DATAX1_ADDR, &x_msb) == 0) &&
      (ReadByte(accelAddr, REG_DATAX0_ADDR, &x_lsb) == 0) &&
      (ReadByte(accelAddr, REG_DATAY1_ADDR, &y_msb) == 0) &&
      (ReadByte(accelAddr, REG_DATAY0_ADDR, &y_lsb) == 0) &&
      (ReadByte(accelAddr, REG_DATAZ1_ADDR, &z_msb) == 0) &&
      (ReadByte(accelAddr, REG_DATAZ0_ADDR, &z_lsb) == 0)) {
      x = (x_msb << 8) | x_lsb;
      y = (y_msb << 8) | y_lsb;
      z = (z_msb << 8) | z_lsb;

      // Perform 2's complement
      int16_t accelX = ~(x - 1);
      int16_t accelY = ~(y - 1);
      int16_t accelZ = ~(z - 1);

      accelerometerString = String(accelX) + "," + String(accelY) + "," + String(accelZ);
    }
    else Serial.println("Failed to read from sensor.");
  }
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
  if((ReadByte(compAddress, 0x3, &x_msb) == 0) &&
    (ReadByte(compAddress, 0x4, &x_lsb) == 0) &&
    (ReadByte(compAddress, 0x5, &y_msb) == 0) &&
    (ReadByte(compAddress, 0x6, &y_lsb) == 0) &&
    (ReadByte(compAddress, 0x7, &z_msb) == 0) &&
    (ReadByte(compAddress, 0x8, &z_lsb) == 0)) {
    uint8_t compX = x_msb << 8 | x_lsb;
    uint8_t compY = y_msb << 8 | y_lsb;
    uint8_t compZ = z_msb << 8 | z_lsb;

    compassString = String(compX) + "," + String(compY) + "," + String(compZ);
  }
  else Serial.println(F("Failed to read from sensor."));
}

void readGyroscope() {
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
  if((ReadByte(gyroAddr, 0x1d, &x_msb) == 0) &&
    (ReadByte(gyroAddr, 0x1e, &x_lsb) == 0) &&
    (ReadByte(gyroAddr, 0x1f, &y_msb) == 0) &&
    (ReadByte(gyroAddr, 0x20, &y_lsb) == 0) &&
    (ReadByte(gyroAddr, 0x21, &z_msb) == 0) &&
    (ReadByte(gyroAddr, 0x22, &z_lsb) == 0)) {
    x = (x_msb << 8) | x_lsb;
    y = (y_msb << 8) | y_lsb;
    z = (z_msb << 8) | z_lsb;
    // Perform 2's complement
    int16_t gyroX = ~(x - 1);
    int16_t gyroY = ~(y - 1);
    int16_t gyroZ = ~(z - 1);

    gyroscopeString = String(gyroX) + "," + String(gyroY) + "," + String(gyroZ);
  }
  else Serial.println("Failed to read from sensor.");
}

// Read a byte on the i2c interface
int ReadByte(uint8_t addr, uint8_t reg, uint8_t *data) {
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
void WriteByte(uint8_t addr, uint8_t reg, byte data) {
  // Begin the write sequence
  Wire.beginTransmission(addr);
  // First byte is to set the register pointer
  Wire.write(reg);
  // Write the data byte
  Wire.write(data);
  // End the write sequence; bytes are actually transmitted now
  Wire.endTransmission();
}

/*===============
 ////  GPS  ////
 ===============*/

void gpsGetData() {
  boolean newdata = false;
  unsigned long start = millis();
  while(millis() - start < 5000) {  // Update every 5 seconds
    if(feedgps()) newdata = true;
    if(newdata) gpsdump(gps);
  }
  if(startupComplete == true) {
    gpsFloatToString();
    gpsDataString += gpsDateTime;
    gpsDataString += "$";
    gpsDataString += String(satellites);
    gpsDataString += ",";
    gpsDataString += String(hdop);
    gpsDataString += "$";
    gpsDataString += gpsLat;
    gpsDataString += ",";
    gpsDataString += gpsLon;
    gpsDataString += "$";
    gpsDataString += gpsAltitudeFt;
    gpsDataString += "$";
    gpsDataString += gpsCourse;
    gpsDataString += "$";
    gpsDataString += gpsSpeedMPH;
    if(altLandDetect == true) {
      gpsDataString += "$";
      gpsDataString += landDistanceFt;
    }
  }
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  satellites = gps.satellites();
  hdop = gps.hdop();

  if(gpsLock == true) {
    gpsDataString = "";
    gpsDateTime = "";
    String gpsDate = "";
    String gpsTime = "";

    String monthFormatted = "";
    String dayFormatted = "";
    String hourFormatted = "";
    String minuteFormatted = "";
    String secondFormatted = "";
    unsigned long age;
    byte month, day, hour, minute, second, hundredths;
    int year;

    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    // Time String
    byte hourCorrected = hour - gpsGmtOffsetHours;
    if(hour < 10) hourFormatted = "0" + String(hourCorrected);
    else hourFormatted = String(hourCorrected);
    if(minute < 10) minuteFormatted = "0" + String(minute);
    else minuteFormatted = String(minute);
    if(second < 10) secondFormatted = "0" + String(second);
    else secondFormatted = String(second);
    // Date String
    if(month < 10) monthFormatted = "0" + String(month);
    else monthFormatted = String(month);
    if(day < 10) dayFormatted = "0" + String(day);
    else dayFormatted = String(day);

    if(syncRequired == true) {
      String yearFormatted = "";
      String yearString = String(year);
      for(int z = 2; z < 4; z++) {
        yearFormatted += yearString.charAt(z);
      }
      gpsSyncString = yearFormatted + "/" + monthFormatted + "/" + dayFormatted + "," + hourFormatted + ":" + minuteFormatted + ":" + secondFormatted + gpsGmtOffsetQuarters; 
      gps.f_get_position(&flat, &flon, &age);
      gpsLatInitial = flat;
      gpsLonInitial = flon;
    }
    else {
      gpsTime = hourFormatted + ":" + minuteFormatted + ":" + secondFormatted;
      gpsDate = monthFormatted + "/" + dayFormatted + "/" + year;
      gpsDateTime = gpsDate + "," + gpsTime;

      gps.f_get_position(&flat, &flon, &age);
      altitude = gps.f_altitude();
      gpsAltitudeFtFloat = altitude / 3.048;
      course = gps.f_course();
      speed = gps.f_speed_mph();

      if(gpsLandDetect == true) {
        float fdist = gps.distance_between(gpsLatFinal, gpsLonFinal, gpsLatInitial, gpsLonInitial);
        landDistanceFtFloat = fdist * 3.28;
      }
    }
  }
}

void gpsFloatToString() {
  char c;

  gpsLat = "";
  Serial3.print(flat, 6);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      gpsLat += c;
      delay(1);
    }
  }

  gpsLon = "";
  Serial3.print(flon, 6);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      gpsLon += c;
      delay(1);
    }
  }

  gpsAltitudeFt = "";
  Serial3.print(gpsAltitudeFtFloat, 2);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      gpsAltitudeFt += c;
      delay(1);
    }
  }

  gpsCourse = "";
  Serial3.print(course, 2);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      gpsCourse += c;
      delay(1);
    }
  }

  gpsSpeedMPH = "";
  Serial3.print(speed, 2);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  if(Serial3.available()) {
    while(Serial3.available()) {
      c = Serial3.read();
      gpsSpeedMPH += c;
      delay(1);
    }
  }

  if(gpsLandDetect == true) {
    landDistanceFt = "";
    Serial3.print(landDistanceFtFloat, 2);
    if(!Serial3.available()){
      while(!Serial3.available()) {
        delay(1);
      }
    }
    if(Serial3.available()) {
      while(Serial3.available()) {
        c = Serial3.read();
        landDistanceFt += c;
        delay(1);
      }
    }
  }
}

// Feed data as it becomes available 
boolean feedgps() {
  while(Serial1.available()) {
    if(gps.encode(Serial1.read())) return true;
  }
  return false;
}

/*===============
 ////  GPRS  ////
 ==============*/

// Read date/time info from GPRS shield and parse into gprsYear, gprsMonth, and gprsDay for reformatting to US date format
void gprsGetDateTime() {
  gprsRawDateTime = "";
  gprsDateString = "";
  gprsTimeString = "";
  gprsYear = "";  // Clear gprsYear string before reading from GPRS
  gprsMonth = "";  // Clear gprsMonth string before reading from GPRS
  gprsDay = "";  // Clear gprsDay string before reading from GPRS

  Serial2.println("AT+CCLK?");  // Read date/time from GPRS
  if(Serial2.available()) {  // If data is coming from GPRS
    while(Serial2.available()) {  // Read the data into string from incoming bytes while they're available
      char c = Serial2.read();  // Read each byte sent, one at a time, into storage variable
      if(c == '\n') break;
      gprsRawDateTime += c;  // Add character to the end of the data string to be written to SD later
      delay(10);  
    }
    for(int y = 8; y < 10; y++) {  // Parse out gprsYear characters from gprsRawDateTime
      gprsYear += String(gprsRawDateTime.charAt(y));
    }
    for(int mo = 11; mo < 13; mo++) {  // Parse out gprsMonth characters from gprsRawDateTime
      gprsMonth += String(gprsRawDateTime.charAt(mo));
    }
    for(int d = 14; d < 16; d++) {  // Parse out gprsDay characters from gprsRawDateTime
      gprsDay += String(gprsRawDateTime.charAt(d));
    }
    for(int t = 17; t < 25; t++) {  // Parse out time characters from gprsRawDateTime
      gprsTimeString += String(gprsRawDateTime.charAt(t));
    }

    // Construct US formatted date string (M/D/Y)
    gprsDateString += gprsMonth;
    gprsDateString += "/";
    gprsDateString += gprsDay;
    gprsDateString += "/";
    gprsDateString += gprsYear;
  }
}

// GPRS Function for SMS transmission of dataString
#ifdef liveLaunch
void smsDataString() {
  Serial2.println(F("AT+CMGF=1"));
  delay(100);
  Serial2.print(F("AT+CMGS=\"+1"));
  delay(100);
  for(int i = 0; i < 10; i++) {
    Serial2.print(smsTargetNum[i]);
    delay(100);
  }
  Serial2.println(F("\""));
  delay(100);
  Serial2.print(F("http://maps.google.com/maps?q="));
  delay(100);
  Serial2.print(F("Landing+Site@"));
  delay(100);
  Serial2.print(gpsLatFinal, 6);
  delay(100);
  Serial2.print(F(",+"));
  delay(100);
  Serial2.print(gpsLonFinal, 6);
  delay(100);
  Serial2.print(F("&t=h&z=19&output=html /"));
  delay(100);
  Serial2.print(F(" Max Altitude: "));
  delay(100);
  Serial2.print(altFtMax);
  delay(100);
  Serial2.print(F(" feet /"));
  delay(100);
  Serial2.print(F(" Landed "));
  delay(100);
  Serial2.print(landDistanceFtFloat);
  delay(100);
  Serial2.println(F(" feet away from launch site."));
  delay(100);
  Serial2.println((char)26);
  delay(1000);
  Serial2.flush();

#ifdef debugMode
  Serial.println(F("Coordinates sent via SMS."));
  delay(1000);
  Serial.print(gpsLatFinal, 6);
  Serial.print(F(",+"));
  Serial.println(gpsLonFinal, 6);
  Serial.println(landDistanceFtFloat, 2);
  Serial.println(altFtMax);
#endif
}
#endif

/*================================
 ////  Setup & Configuration  ////
 ===============================*/

void zeroAltimeter() {
  long altTot = 0;
  for(int y = 0; y < 10; y++) {
    long altPrep = baro.altGetAlt();
    delay(100);
  }
  for(int k = 0; k < 10; k++) {
    long altCalibRead = baro.altGetAlt();
    altTot += altCalibRead;
    delay(100);
  }
  altCalibVal = altTot / 10;
  altCmInitial = altCm;
}

void checkAltCalib() {
  for(int x = 0; x < 10; x++) {
    readAltimeter();
    if(abs(altCm - altCmInitial) > 61) zeroAltimeter();
    delay(500);
  }
}

void gprsPowerOn() {
  digitalWrite(gprsPowPin, LOW);
  delay(100);
  digitalWrite(gprsPowPin, HIGH);
  delay(500);
  digitalWrite(gprsPowPin, LOW);
  delay(100);
}

void configureGPRS() {
  Serial2.println("ATE0");
  delay(100);
  Serial2.println("ATQ1");
  delay(100);
  Serial2.println("ATV0");
  delay(100);
}

void syncDateTime() {
  Serial2.print("AT+CCLK=\"");
  delay(100);
  Serial2.print(gpsSyncString);
  delay(100);
  Serial2.println("\"");
  delay(100);
}

void configureSensors() {
  WriteByte(compAddress, 0x0, 0x10);  // Configure the compass to default values (see datasheet for details)
  WriteByte(compAddress, 0x1, 0x20);  // Configure the compass to default values (see datasheet for details)
  WriteByte(compAddress, 0x2, 0x0);  // Set compass to continuous-measurement mode (default is single shot)
  WriteByte(accelAddr, REG_BW_RATE_ADDR, 0X08);  // Set 25Hz output data rate and 25Hz bandwidth and disable low power mode
  WriteByte(accelAddr, REG_PWR_CTL_ADDR, 0x08);  // Disable auto sleep
  WriteByte(accelAddr, REG_INT_ENABLE_ADDR, 0x0);  // Disable interrupts (the pins are not brought out anyway)
}
