/*
Arduino module to be used on a high-altitude balloon flight as a
 companion to the Raspberry Pi + PiInTheSky telemetry package.
 
 IntersemaBaro library has been overhauled and currently includes
 two additional functions that return temperature and pressure,
 respectively.
 ******************************************************************************************************
 * MUST CONSIDER THESE THINGS BEFORE COMPILING FOR LIVE LAUNCH:                                       *
 ******************************************************************************************************
 - MS commands may be issued after landing only from those numbers listed in global constants.        *
 -> ** It may be helpful to include a few other numbers in case of phone problems during live launch **
 - Piezo buzzer (AND POSSIBLE OTHER COMMANDS) set to low/mild/quiet values for debugging purposes     *
 ******************************************************************************************************
 
 To Do:
 - Cause GPS coordinate transmission and entry into landWaitForCommand() after long period of time in
 case of failure in triggering landing booleans
 - Make "for()" integers consistent (ie. 1st = x, 2nd = y, 3rd = z)
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

#include "IntersemaBaro.h"
#include <SdFat.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>

#define gprsPowPin 9  // Pin for software power-up of GPRS shield  
#define sdSlaveSelect 53  // SPI slave select pin (SS) for SD card (Uno: 10 / Mega: 53)
#define floatTx 2  // Software serial transmit pin for float to string conversion functions
#define floatRx 3  // Software serial receive pin for float to string conversion functions

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

const int debugPin = 2;  // Pin to trigger "gpsLandDetect & altLandDetect = true" for debugging
const int readyLED = A0;  // Pin for LED to indicate entry of main loop (program start)
const int landLED = A1;  // Pin for LED to indicate detection of landing
const int smsLED = A2;  // Pin for LED to debug SMS command execution
const int piezoPin = A3;  // Pin for executing piezo buzzer on SMS command
const int smokePin = A8;  // Pin for triggering smoke signal on SMS command

const int piezoBuzzTime = 500;  // Duration (ms) of piezo buzzer activation after SMS command
const int smokeIgniteTime = 500;  // Duration (ms) of relay activation to ignite smoke signal (NEEDS FIELD TESTING)

const char smsTargetNum[11] = "2145635266";  // Mobile number to send data and receive commands via SMS

SoftwareSerial floatSerial(floatRx, floatTx);

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
// Landing SMS Functions
String smsMessageRaw;  // Raw received SMS string from GPRS
String smsRecNumber;  // Parsed number of origin of received SMS
String smsMessage;  // Parsed SMS message
String smsCommandOutput;  // Text string written to SD to indicate SMS action taken

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
const int gpsHDOPMinimum = 350;  // May want to change to 200 for real launch
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
boolean finalDataSent = false;  // Triggered after final data string (post-landing) is written to SD
boolean smsCommandExec = false;  // Triggered when an SMS command is received and executed
boolean gpsCoordUpdate = false;  // True if new GPS coordinates acquired after read, false if 0's are read from GPS (Post-landing only)

int loopCount = 0;

void setup() {
  pinMode(debugPin, INPUT);
  pinMode(readyLED, OUTPUT);
  pinMode(landLED, OUTPUT);
  pinMode(smsLED, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(smokePin, OUTPUT);

  digitalWrite(readyLED, LOW);
  digitalWrite(landLED, LOW);
  digitalWrite(piezoPin, LOW);
  digitalWrite(smokePin, LOW);

  Serial.begin(19200);
  Serial.print(F("Beginning setup..."));
  Serial1.begin(4800);  // Hardware serial for GPS communication
  if(!Serial1.available()) {
    while(!Serial1.available()) {
      delay(1);
    }
  }
  Serial2.begin(19200);  // Hardware serial for GPRS communication
  floatSerial.begin(38400);  // Software serial for float to string conversion functions

  gpsGetData();
  if(satellites < gpsSatMinimum || hdop > gpsHDOPMinimum) {
    while(satellites < gpsSatMinimum || hdop > gpsHDOPMinimum) {
      gpsGetData();
#ifdef debugMode
      Serial.println();
      Serial.print(F("Sats: "));
      Serial.println(satellites);
      Serial.print(F("HDOP: "));
      Serial.println(hdop);
#endif
      delay(5000);
    }
  }
  gpsLock = true;

  digitalWrite(gprsPowPin, LOW);
  delay(100);
  digitalWrite(gprsPowPin, HIGH);
  delay(500);
  digitalWrite(gprsPowPin, LOW);
  delay(100);

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
  Serial.println(F("complete."));
  Serial.println();

  gprsSerialFlush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  loopCount++;
  if(gpsLandDetect == false || altLandDetect == false) {
    gpsGetData();
    readAltimeter();
    readAccelerometer();
    readCompass();
    readGyroscope();
    constructDataString();
    if(firstLoop == true) {
      loopCount--;
      firstLoop = !firstLoop;
#ifdef debugMode
      Serial.println(F("NULL"));
#endif
    }
    else {
#ifdef debugMode
      Serial.println(dataString);
#endif
      if(!dataFile.open("HABLOG.txt", O_RDWR | O_CREAT | O_AT_END)) {
        sd.errorHalt("Write of dataString to SD failed.");
      }
      dataFile.println(dataString);
      dataFile.close();
    }
#ifdef debugMode
    if(digitalRead(debugPin) == 0) {
      gpsLandDetect = true;
      altLandDetect = true;
      Serial.println();
      Serial.println(F("Debug landing triggered."));
      Serial.println();
      digitalWrite(landLED, HIGH);
    }
#endif
    delay(5000);
  }

  else {
    if(finalDataSent == false) {
      gpsGetData();
      readAltimeter();
      readAccelerometer();
      readCompass();
      readGyroscope();
      constructDataString();
      if(!dataFile.open("HABLOG.txt", O_RDWR | O_CREAT | O_AT_END)) {
        sd.errorHalt("Write of final dataString to SD failed.");
      }
      dataFile.println(dataString);
      dataFile.println("$$$$FINAL DATA POINT -- LANDING DETECTED -- READY FOR SMS COMMAND$$$$");
      dataFile.close();

      gprsSMSData();
      delay(5000);
      finalDataSent = true;
      //delay(10000);
      //gprsSMSMenu();
    }
    //unsigned long menuSendDelay = 1800000;  // Delay (ms) between each send of the SMS command menu
    if(finalDataSent == true) {
      Serial2.println("AT+CNMI=2,2,0,0,0");  // Make incoming SMS messages go directly to serial
      delay(100);
      Serial2.flush();
      if(finalDataSent == true) {
        Serial2.println(F("AT+CMGD=0,4"));
        delay(100);
        Serial2.flush();

        gprsSMSMenu();
        if(!Serial2.available()) {
          while(!Serial2.available()) {
            delay(10);
          }
        }
        gprsSerialFlush();
        landWaitForCommand();
      }
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void constructDataString() {
  dataString = "$";
  dataString += loopCount;
  dataString += "$";
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

  if(startupComplete == true && gpsLandDetect == false && altLandDetect == false) {
    // CONSIDER MODIFYING THESE FXNS TO USE ONLY ONE INTEGER VARIABLE FOR COUNTING (i.e. "altConsecRead")
    if(altAscentDetect == false) {
      if(altCmDiff > altCmChangeThreshold) altPosRead++;
      else if(altCmDiff < altCmChangeThreshold) altPosRead = 0;  // Latest read doesn't carry same sign as previous, so reset positive read counter
      else;  // No change in altitude detected between reads, so do nothing
      if(altPosRead == 5) altAscentDetect = true;
#ifdef debugMode
      Serial.print(F("+"));
      Serial.println(altPosRead);

      if(altAscentDetect == true) Serial.println(F("Ascent stage detected."));
#endif
    }
    else if(altAscentDetect == true && altDescentDetect == false) {
      if(altCmDiff < -altCmChangeThreshold) altNegRead++;
      else if(altCmDiff > -altCmChangeThreshold) altNegRead = 0;  // Latest read doesn't carry same sign as previous, so reset negative read counter
      else;  // No change in altitude detected between reads, so do nothing
      if(altNegRead == 5) altDescentDetect = true;
#ifdef debugMode
      Serial.print(F("-"));
      Serial.println(altNegRead);

      if(altDescentDetect == true) Serial.println(F("Descent stage detected."));
#endif
    }
    else if(altAscentDetect == true && altDescentDetect == true) {
      // Something to wait for no change in altitude to indicate landing
    }
#ifdef debugMode
    else Serial.println(F("ERROR: Descent detected without first detecting ascent!"));
#endif
  }
  altCmPrev = altCm;
}

void altFloatToString() {
  char c;

  altFt = "";
  floatSerial.print(altFtFloat, 2);
  if(!floatSerial.available()){
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
      altFt += c;
      delay(1);
    }
  }

  altTempC = "";
  floatSerial.print(altTempCFloat, 2);
  if(!floatSerial.available()){
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
      altTempC += c;
      delay(1);
    }
  }

  altPressMillibar = "";
  floatSerial.print(altPressMillibarFloat, 2);
  if(!floatSerial.available()) {
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
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
    if(altLandDetect == true && gpsLandDetect == true && finalDataSent == false) {
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
    byte hourCorrected = hour - byte(gpsGmtOffsetHours);
    if(hourCorrected < 10) hourFormatted = "0" + String(hourCorrected);
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
    else if(gpsLandDetect == false) {  // MAKE SURE THIS IS ACTUALLY THE CONDITION YOU WANT TO TRIGGER THIS OUTPUT BEFORE LIVE LAUNCH!!!!!!!!!!!!!!!!!
      gpsTime = hourFormatted + ":" + minuteFormatted + ":" + secondFormatted;
      gpsDate = monthFormatted + "/" + dayFormatted + "/" + year;
      gpsDateTime = gpsDate + "," + gpsTime;

      gps.f_get_position(&flat, &flon, &age);
      altitude = gps.f_altitude();
      gpsAltitudeFtFloat = altitude / 3.048;
      course = gps.f_course();
      speed = gps.f_speed_mph();
    }
    else {
      if(flat != 0 && flon != 0) {
        gpsLatFinal = flat;
        gpsLonFinal = flon;
        if(smsCommandExec == true) {
          smsCommandOutput = "Updated GPS coordinates sent via SMS.";
          gpsCoordUpdate = true;
        }
      }
      else if(smsCommandExec == true) {
        smsCommandOutput = "GPS read failed. Sent last known coordinates.";
        gpsCoordUpdate = false;
      }
      else {
        // Code placed in here will execute if GPS coordinates are "0" for final read
        // If landing coordinates not updated, try to acquire for some amount of time before aborting and waiting for SMS command
        /*
        for(int x = 0; x < 10; x++) {
         gpsGetData();
         if(gpsCoordUpdate == true) break;
         delay(60000);
         }
         */
      }
      if(finalDataSent == false) {
        float fdist = gps.distance_between(gpsLatFinal, gpsLonFinal, gpsLatInitial, gpsLonInitial);
        landDistanceFtFloat = fdist * 3.28;
      }
    }
  }
}

void gpsFloatToString() {
  char c;

  gpsLat = "";
  floatSerial.print(flat, 6);
  if(!floatSerial.available()){
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
      gpsLat += c;
      delay(1);
    }
  }

  gpsLon = "";
  floatSerial.print(flon, 6);
  if(!floatSerial.available()){
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
      gpsLon += c;
      delay(1);
    }
  }

  gpsAltitudeFt = "";
  floatSerial.print(gpsAltitudeFtFloat, 2);
  if(!floatSerial.available()){
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
      gpsAltitudeFt += c;
      delay(1);
    }
  }

  gpsCourse = "";
  floatSerial.print(course, 2);
  if(!floatSerial.available()){
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
      gpsCourse += c;
      delay(1);
    }
  }

  gpsSpeedMPH = "";
  floatSerial.print(speed, 2);
  if(!floatSerial.available()){
    while(!floatSerial.available()) {
      delay(1);
    }
  }
  if(floatSerial.available()) {
    while(floatSerial.available()) {
      c = floatSerial.read();
      gpsSpeedMPH += c;
      delay(1);
    }
  }

  if(gpsLandDetect == true) {
    landDistanceFt = "";
    floatSerial.print(landDistanceFtFloat, 2);
    if(!floatSerial.available()){
      while(!floatSerial.available()) {
        delay(1);
      }
    }
    if(floatSerial.available()) {
      while(floatSerial.available()) {
        c = floatSerial.read();
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
void gprsSMSData() {
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

  if(finalDataSent == false) {
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
  }
  else {
    if(gpsCoordUpdate == true) Serial2.println(F(" Coordinates updated."));
    else Serial2.println(F(" GPS coordinate update failed."));
  }
  Serial2.println((char)26);
  delay(100);
  Serial2.flush();

  if(!Serial2.available()) {
    while(!Serial2.available()) {
      delay(10);
    }
  }
  gprsSerialFlush();
#ifdef debugMode
  Serial.println(F("Data sent via SMS."));
  Serial.println();
#endif
}

void gprsSMSBuzzConf() {
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
  Serial2.println(F("Buzzer activated."));
  delay(100);
  Serial2.println((char)26);
  delay(100);
  Serial2.flush();

  if(!Serial2.available()) {
    while(!Serial2.available()) {
      delay(10);
    }
  }
  gprsSerialFlush();
}

void gprsSMSSmokeConf() {
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
  Serial2.println(F("Smoke signal activated."));
  delay(100);
  Serial2.println((char)26);
  delay(100);
  Serial2.flush();

  if(!Serial2.available()) {
    while(!Serial2.available()) {
      delay(10);
    }
  }
  gprsSerialFlush();
}

void gprsSerialFlush() {
#ifdef debugMode
  Serial.println(F("Flushing GPRS serial buffer:"));
#endif
  if(Serial2.available()) {
    while(Serial2.available()) {
      Serial.write(Serial2.read());
      delay(10);
    }
  }
#ifdef debugMode
  Serial.println(F("GPRS serial buffer cleared."));
  Serial.println();
#endif
}

/*=================================
 ////  Post-landing Functions  ////
 ================================*/

void landWaitForCommand() {
  if(finalDataSent == true) {
    smsMessageRaw = "";
    smsRecNumber = "";
    smsMessage = "";
    String smsCommandString = "";

    if(!Serial2.available()) {
      while(!Serial2.available()) {
        delay(10);
      }
    }
    gprsSerialFlush();
#ifdef debugMode
    Serial.println(F("Waiting for SMS command."));
#endif
    while(finalDataSent == true) {
      if(Serial2.available()) {
        while(Serial2.available()) {
          char c = Serial2.read();  // Read each byte sent, one at a time, into storage variable
          smsMessageRaw += c;
          delay(10);
        }
#ifdef debugMode
        Serial.print(F("smsMessageRaw: "));
        Serial.println(smsMessageRaw);
#endif
        int numIndex = smsMessageRaw.indexOf('"') + 3;
        int smsIndex = smsMessageRaw.lastIndexOf('"') + 3;
        for(numIndex; ; numIndex++) {
          char c = smsMessageRaw.charAt(numIndex);
          if(c == '"') break;
          smsRecNumber += c;
        }
        for(smsIndex; ; smsIndex++) {
          char c = smsMessageRaw.charAt(smsIndex);
          if(c == '\n' || c == '\r') break;
          smsMessage += c;
        }
#ifdef debugMode
        Serial.print(F("Num: "));
        Serial.println(smsRecNumber);
        Serial.print(F("SMS: "));
        Serial.println(smsMessage);
        Serial.print(F("smsMessageLength: "));
        Serial.println(smsMessage.length());
#endif
        if(smsRecNumber == smsTargetNum) {
#ifdef debugMode
          Serial.println(F("SMS sender's number matches expected. Executing SMS command."));
#endif
          executeSMSCommand();
          gprsSerialFlush();
        }
        else {
#ifdef debugMode
          Serial.println(F("SMS sender's number does not match expected. Aborting."));
#endif  
          smsCommandOutput = "Invalid SMS sender number.";
        }
        gprsGetDateTime();

        smsCommandString = "SMS command issued: ";
        smsCommandString += gprsDateString;
        smsCommandString += ", ";
        smsCommandString += gprsTimeString;
        smsCommandString += " $ ";
        smsCommandString += smsRecNumber;
        smsCommandString += " $ ";
        smsCommandString += smsMessage;
        smsCommandString += " $ ";
        smsCommandString += smsCommandOutput;

        if(!dataFile.open("HABLOG.txt", O_RDWR | O_CREAT | O_AT_END)) {
          sd.errorHalt("Write of smsCommandString to SD failed.");
        }
        dataFile.println(smsCommandString);
        dataFile.close();
#ifdef debugMode
        Serial.println();
#endif
        Serial2.flush();
        landWaitForCommand();
      }
      delay(10);
    }
  }
}

void executeSMSCommand() {
  smsCommandExec = true;
  smsCommandOutput = "";
  int smsCommand = 0;
  if(smsMessage.length() == 1) smsCommand = smsMessage.toInt();
  switch(smsCommand) {
  case 1:
    // Retrieve updated GPS coordinates and send via SMS
    gpsGetData();
    gprsSMSData();
    break;
  case 2:
    // Trigger piezo buzzer
    digitalWrite(piezoPin, HIGH);
    delay(piezoBuzzTime);
    digitalWrite(piezoPin, LOW);
    smsCommandOutput = "Piezo buzzer triggered.";
    gprsSMSBuzzConf();
    break;
  case 3:
    // Trigger smoke signal
    digitalWrite(smokePin, HIGH);
    delay(smokeIgniteTime);
    digitalWrite(smokePin, LOW);
    smsCommandOutput = "Smoke signal triggered.";
    gprsSMSSmokeConf();
    break;
  case 4:
    // Reboot Arduino
  default:
    // Display invalid command message
    smsCommandOutput = "Invalid command issued.";
    break;
  }
}

void gprsSMSMenu() {
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
  Serial2.println(F("Reply with desired option #: 1 - GPS / 2 - Buzzer / 3 - Smoke / 4 - Reboot"));
  delay(100);
  Serial2.println((char)26);
  delay(100);
  Serial2.flush();

#ifdef debugMode
  Serial.println(F("SMS command menu sent via SMS."));
  Serial.println();
#endif
}

/*void gprsSMSReceive() {
 if(Serial2.available()) {  // If data is coming from GPRS
 smsMessageRaw = "";
 String smsCommandString = "";
 while(Serial2.available()) {  // Read the data into string from incoming bytes while they're available
 char c = Serial2.read();  // Read each byte sent, one at a time, into storage variable
 smsMessageRaw += c;
 delay(10);
 }
 gprsParseSMS();
 #ifdef debugMode
 Serial.print(F("Num: "));
 Serial.println(smsRecNumber);
 Serial.print(F("SMS: "));
 Serial.println(smsMessage);
 Serial.print(F("smsMessageLength: "));
 Serial.println(smsMessage.length());
 #endif
 if(smsRecNumber == smsTargetNum) {
 #ifdef debugMode
 Serial.println(F("SMS sender's number matches expected. Executing SMS command."));
 #endif
 executeSMSCommand();
 }
 else {
 smsCommandOutput = "Invalid SMS sender number.";
 #ifdef debugMode
 Serial.println(F("SMS sender's number does not match expected. Aborting."));
 #endif  
 }
 gprsGetDateTime();
 
 smsCommandString = "SMS command issued: ";
 smsCommandString += gprsDateString;
 smsCommandString += ", ";
 smsCommandString += gprsTimeString;
 smsCommandString += " $ ";
 smsCommandString += smsRecNumber;
 smsCommandString += " $ ";
 smsCommandString += smsMessage;
 smsCommandString += " $ ";
 smsCommandString += smsCommandOutput;
 
 if(!dataFile.open("HABLOG.txt", O_RDWR | O_CREAT | O_AT_END)) {
 sd.errorHalt("Write of smsCommandString to SD failed.");
 }
 dataFile.println(smsCommandString);
 dataFile.close();
 #ifdef debugMode
 Serial.println();
 #endif
 }
 }*/

/*void gprsParseSMS() {
 smsRecNumber = "";
 smsMessage = "";
 int numIndex = smsMessageRaw.indexOf('"') + 3;
 int smsIndex = smsMessageRaw.lastIndexOf('"') + 3;
 for(numIndex; ; numIndex++) {
 char c = smsMessageRaw.charAt(numIndex);
 if(c == '"') break;
 smsRecNumber += c;
 }
 for(smsIndex; ; smsIndex++) {
 char c = smsMessageRaw.charAt(smsIndex);
 if(c == '\n' || c == '\r') break;
 smsMessage += c;
 }
 }*/

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

void configureGPRS() {
  Serial2.println("ATE0");
  delay(100);
  Serial2.println("ATQ1");
  delay(100);
  Serial2.println("ATV0");
  delay(100);
  Serial2.println("AT+CMGF=1");
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
  // Accelerometer
  WriteByte(accelAddr, REG_BW_RATE_ADDR, 0X08);  // Set 25Hz output data rate and 25Hz bandwidth and disable low power mode
  WriteByte(accelAddr, REG_PWR_CTL_ADDR, 0x08);  // Disable auto sleep
  WriteByte(accelAddr, REG_INT_ENABLE_ADDR, 0x0);  // Disable interrupts (the pins are not brought out anyway)
  // Compass
  WriteByte(compAddress, 0x0, 0x10);  // Configure the compass to default values (see datasheet for details)
  WriteByte(compAddress, 0x1, 0x20);  // Configure the compass to default values (see datasheet for details)
  WriteByte(compAddress, 0x2, 0x0);  // Set compass to continuous-measurement mode (default is single shot)
}
