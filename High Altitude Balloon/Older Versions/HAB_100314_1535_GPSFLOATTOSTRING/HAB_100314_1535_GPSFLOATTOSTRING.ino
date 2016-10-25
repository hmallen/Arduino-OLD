/*
Arduino module to be used on a high-altitude balloon flight as a
 companion to the Raspberry Pi + PiInTheSky telemetry package.
 
 To Do:
 - Create modified driftCompensation() function that is dependent on millis() instead of loopCount
 - Add CFC sensor
 -> Create voltage regulation function for heating element
 - Fix floating point math in altimeter function
 - **** BUY AND ADD CO2 AND METHANE SENSORS ****
 - Add function for landDetect boolean variable
 -> If statement to set final location coordinates for comparison
 - Add landing detect to altimeter read function
 - Add temperature (?and temperature-compensated pressure reading?) to altimeter functions
 - Add SD data logging functionality
 - Add piezo signal after landing detected
 - Add ability to SMS to receive updated location information
 - Find reliable string element for gprsBootLoop() to identify actual data
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
#define gmtOffset -24  // Required for proper formatting of the GPS-->GPRS sync string. Expressed in quarters of an hour. (ex. -2hr offset = -08)

#define sdSlaveSelect 53  // SPI slave select pin (SS) for SD card (Uno: 10 / Mega: 53)

#define compAddress 0x1E

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
String gprsHour12;  // Stores hour after conversion from 24hr - - > 12hr time format
String gprsDateLongString;  // Stores full long form of date after parsing from dateString
String gprsTime12hrString;  // Stores time info in 12hr time format after parsing from timeString

// Altimeter
boolean altZeroedPrev = false;
long altCm, altCalibVal, altCmInitial, altCmLast;
unsigned long altCmMax, altFtMax;  // Can I make some of these variables local instead of global?

// Compass
int compX, compY, compZ;
String compassString;

// GPS
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

boolean syncRequired = true;  // Remains true until GPRS syncs date/time with GPS
String gpsSyncString;  // String formatted for GPRS AT commands to sync date/time
int gpsAltOffsetFt = 0;  // Altitude offset for altimeter based on initial GPS elevation reading
String gpsDataString;  // String containing all data from GPS

String dataString;  // Full data string written to SD card

boolean landDetect = false;  // Triggered when altitude (and possibly other sensor) reads indicate landing

void setup() {
  pinMode(piezoPin, OUTPUT);
  pinMode(readyLED, OUTPUT);

  digitalWrite(piezoPin, LOW);
  digitalWrite(readyLED, LOW);

  Serial.begin(19200);

#ifdef debugMode
  Serial.print(F("Initiating bridged serial for float to string processing..."));
  delay(1000);
#endif

  Serial3.begin(19200);

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing GPS connection..."));
  delay(1000);
#endif

  Serial1.begin(4800);
  if(!Serial1.available()) {
    while(!Serial1.available()) {
      delay(1);
    }
  }

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing GPRS connection..."));
  delay(1000);
#endif

  Serial2.begin(19200);  // Software serial for GPRS communication
  delay(1000);

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Powering up GPRS sheild..."));
  delay(1000);
#endif

  gprsPowerOn();

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing I2C connection..."));
  delay(1000);
#endif

  Wire.begin();

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Configuring I2C sensors..."));
#endif

  configureSensors();

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Loading altimeter library..."));
  delay(1000);
#endif

  baro.init();

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing SD card..."));
  delay(1000);
#endif

  if(!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Zeroing altimeter..."));
  delay(1000);
#endif

  readAlt();
  while(altCm >= 152) {
#ifdef debugMode
    if(altZeroedPrev == true) Serial.print(F("rezeroing..."));
#endif
    zeroAltimeter();
    altZeroedPrev = true;
    delay(2000);
    readAlt();
  }
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Acquiring launch site coordinates..."));
  delay(1000);
#endif

  gpsGetData();
  gpsLatInitial = flat;
  gpsLonInitial = flon;

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.println();
  Serial.print(F("Launch Lat: "));
  Serial.println(gpsLatInitial, 6);
  Serial.print(F("Launch Lon: "));
  Serial.println(gpsLonInitial, 6);
  Serial.println();

  Serial.print(F("Configuring GPRS..."));
  delay(1000);
#endif

  configureGPRS();
  gprsBootLoop();

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.println(F("Syncing GPRS real-time clock with GPS date/time information..."));
  Serial.println();
  delay(1000);
#endif

  gpsGetData();
  syncDateTime();
  syncRequired = false;
  gprsGetDateTime();
  delay(1000);

#ifdef debugMode
  Serial.print(F("AT command issued: AT+CCLK=\""));
  Serial.print(gpsSyncString);
  Serial.println(F("\""));
#endif

#ifdef debugMode
  Serial.println(F("Successful."));
  Serial.println();
  Serial.print(F("Retrieving updated date/time information from GPRS..."));
  delay(1000);
#endif

  gprsGetDateTime();  // Read date and time information from GPRS
  gprsDateLongConst();  // Construct string with long form of date
  gprsTime12hrConst();  // Construct string with 12hr format of time

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.println();
  Serial.print(F("It is currently "));
  Serial.print(gprsTime12hrString);
  Serial.print(F(", "));
  Serial.print(gprsDateLongString);
  Serial.println(F("."));
  Serial.println();
  Serial.print(F("Confirming altimeter calibration..."));
  delay(1000);
#endif

  checkAltCalib();

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.println();
  Serial.println(F("System ready for launch. Beginning data acquisition."));
  Serial.println();
  delay(5000);
#endif

  digitalWrite(readyLED, HIGH);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  gpsGetData();
  //readAlt();
  readCompass();
  constructDataString();
  Serial.println(dataString);
  delay(5000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void constructDataString() {
  dataString = "$";
  dataString += gpsDataString;
  /*dataString += "$";
   long altFt = (altCm * 100) / 3048;  // Avoids floating point math
   dataString += String(altFt);*/
  dataString += "$";
  dataString += compassString;
}

void configureSensors() {
  // Configure the compass to default values (see datasheet for details)
  WriteByte(compAddress, 0x0, 0x10);
  WriteByte(compAddress, 0x1, 0x20);
  // Set compass to continuous-measurement mode (default is single shot)
  WriteByte(compAddress, 0x2, 0x0);
}

void readAlt() {
  altCm = baro.getHeightCentiMeters() - altCalibVal;
  if(altCm > altCmMax) {
    altCmMax = altCm;
  }
}

void zeroAltimeter() {
  long altTot = 0;
  for(int y = 0; y < 10; y++) {
    long altPrep = baro.getHeightCentiMeters();
    delay(100);
  }
  for(int k = 0; k < 10; k++) {
    long altCalibRead = baro.getHeightCentiMeters();
    altTot += altCalibRead;
    delay(100);
  }
  altCalibVal = altTot / 10;
  altCmInitial = altCm;
}

void checkAltCalib() {
  for(int x = 0; x < 10; x++) {
    readAlt();
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

void gprsBootLoop() {
  while(gprsDateString == "") {  // Attempt to read date/time from GPRS continuously until data received
    gprsGetDateTime();
    delay(1000);
  }
}

void syncDateTime() {
  Serial2.print("AT+CCLK=\"");
  delay(100);
  Serial2.print(gpsSyncString);
  delay(100);
  Serial2.println("\"");
  delay(100);
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
    compX = x_msb << 8 | x_lsb;
    compY = y_msb << 8 | y_lsb;
    compZ = z_msb << 8 | z_lsb;
  }
  else Serial.println(F("Failed to read from sensor."));

  compassString = String(compX) + "," + String(compY) + "," + String(compZ);
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
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
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
  if(hour < 10) hourFormatted = "0" + String(hour);
  else hourFormatted = String(hour);
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
    gpsSyncString = yearFormatted + "/" + monthFormatted + "/" + dayFormatted + "," + hourFormatted + ":" + minuteFormatted + ":" + secondFormatted + gmtOffset; 
  }
  else {
    gpsTime = hourFormatted + ":" + minuteFormatted + ":" + secondFormatted;
    gpsDate = monthFormatted + "/" + dayFormatted + "/" + year;
    gpsDateTime = gpsTime + ", " + gpsDate;

    gpsDataString = gpsDate + "," + gpsTime;

    gps.f_get_position(&flat, &flon, &age);
    altitude = gps.f_altitude();
    gpsAltitudeFtFloat = altitude / 3.048;
    course = gps.f_course();
    speed = gps.f_speed_mph();

    if(landDetect == true) {
      float fdist = gps.distance_between(gpsLatFinal, gpsLonFinal, gpsLatInitial, gpsLonInitial);
      landDistanceFtFloat = fdist * 3.28;
    }
  }
}

void gpsFloatToString() {
  char c;
#ifdef debugMode
  Serial.print(F("Converting latitude float to string..."));
#endif
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
#ifdef debugMode
  Serial.print(gpsLat);
  Serial.println(F("...complete."));

  Serial.print(F("Converting longitude float to string..."));
#endif
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
      gpsLat += c;
      delay(1);
    }
  }
#ifdef debugMode
  Serial.print(gpsLat);
  Serial.println(F("...complete."));

  Serial.print(F("Converting altitude float to string..."));
#endif
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
#ifdef debugMode
  Serial.print(gpsAltitudeFt);
  Serial.println(F("...complete."));

  Serial.print(F("Converting course float to string..."));
#endif
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
#ifdef debugMode
  Serial.print(gpsCourse);
  Serial.println(F("...complete."));

  Serial.print(F("Converting speed float to string..."));
#endif
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
#ifdef debugMode
  Serial.print(gpsSpeedMPH);
  Serial.println(F("...complete."));
#endif
  if(landDetect == true) {
#ifdef debugMode
    Serial.print(F("Converting land distance float to string..."));
#endif
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
#ifdef debugMode
    Serial.print(landDistanceFt);
    Serial.println(F("...complete."));
#endif
  }
#ifdef debugMode
  Serial.println();
#endif
}

// Feed data as it becomes available 
boolean feedgps() {
  while(Serial1.available()) {
    if(gps.encode(Serial1.read())) return true;
  }
  return false;
}

/*========================
 ////  Date and time  ////
 =======================*/

// Read date/time info from GPRS shield and parse into gprsYear, gprsMonth, and gprsDay for reformatting to US date format
void gprsGetDateTime() {
  gprsRawDateTime = "";
  gprsDateString = "";
  gprsTimeString = "";
  gprsTime12hrString = "";
  gprsDateLongString = "";
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

// Construct long form of date read from GPRS
void gprsDateLongConst() {
  String gprsYearLong = "";  // Clear long - formatted gprsYear string before reading from GPRS
  String gprsMonthLong = "";  // Clear long - formatted gprsMonth string before reading from GPRS
  String gprsDayLong = "";  // Clear long - formatted gprsDay string before reading from GPRS
  gprsDateLongString = "";  // Clear full long date string before reading from GPRS

  gprsYearLong += "20";  // Lead gprsYear with "20" to create long form (ex. 2014)
  gprsYearLong += String(gprsYear);  // Store long gprsYear in a string

  gprsDayLong += String(gprsDay.toInt());  // Convert gprsDay to integer to remove leading 0's and store in string

  switch(gprsMonth.toInt()) {  // Convert to integer and use switch case to convert gprsMonth to long form (ex. 4 = "April");
  case 0:
    return;
  case 1:
    gprsMonthLong += "January";
    break;
  case 2:
    gprsMonthLong += "February";
    break;
  case 3:
    gprsMonthLong += "March";
    break;
  case 4:
    gprsMonthLong += "April";
    break;
  case 5:
    gprsMonthLong += "May";
    break;
  case 6:
    gprsMonthLong += "June";
    break;
  case 7:
    gprsMonthLong += "July";
    break;
  case 8:
    gprsMonthLong += "August";
    break;
  case 9:
    gprsMonthLong += "September";
    break;
  case 10:
    gprsMonthLong += "October";
    break;
  case 11:
    gprsMonthLong += "November";
    break;
  case 12:
    gprsMonthLong += "December";
    break;
  }
  // Concatenate strings to form full long date
  gprsDateLongString += String(gprsMonthLong);
  gprsDateLongString += " ";
  gprsDateLongString += String(gprsDayLong);
  gprsDateLongString += ", ";
  gprsDateLongString += String(gprsYearLong);
}

// Changes time read from GPRS into 12hr format
void gprsTime12hrConst() {
  gprsHour = "";  // Clear gprsHour string before parsing data into it from timeString
  gprsMinute = "";  // Clear gprsMinute string before parsing data into it from timeString
  gprsSecond = "";  // Clear gprsSecond string before parsing data into it from timeString
  gprsHour12 = "";  // Clear 12hr formatted gprsHour string before parsing data into it from timeString

  for(int h = 0; h < 2; h++) {  // Parse out gprsHour characters from timeString
    gprsHour += String(gprsTimeString.charAt(h));
  }
  for(int mi = 3; mi < 5; mi++) {  // Parse out gprsHour characters from timeString
    gprsMinute += String(gprsTimeString.charAt(mi));
  }
  for(int s = 6; s < 8; s++) {  // Parse out gprsHour characters from timeString
    gprsSecond += String(gprsTimeString.charAt(s));
  }
  if(gprsHour.toInt() > 12) {  // If integer format of 24hr formatted gprsHour is more than 12
    gprsHour12 += String(gprsHour.toInt() - 12);  // Subtract 12 from gprsHour and add to storage string
    gprsTime12hrString += String(gprsHour12);  // Add new 12hr formatted gprsHour to 12hr time string
    gprsTime12hrString += ":";  // Add colon to create properly formatted time
    gprsTime12hrString += String(gprsMinute);  // Add gprsMinute string after the colon
    gprsTime12hrString += "pm";  // Add "pm", since gprsHour was after 12 ( >  =  13)
  }
  else if (gprsHour.toInt() == 0) {  // If integer format of 24hr formatted gprsHour is not more than 12, but is equal to 0
    gprsHour12 += "12";  // Set gprsHour storage string to 12 (midnight)
    gprsTime12hrString += String(gprsHour12);  // Add new 12hr formatted gprsHour to 12hr time string
    gprsTime12hrString += ":";  // Add colon to create properly formatted time
    gprsTime12hrString += String(gprsMinute);  // Add gprsMinute string after the colon
    gprsTime12hrString += "am";  // Add "am", since time detected was midnight
  }
  else {  // If integer format of 24hr formatted gprsHour is not more than 12 or equal to 0
    gprsHour12 += String(gprsHour.toInt());  // Set gprsHour storage variable to integer form of gprsHour
    gprsTime12hrString += gprsHour12;  // Add new 12hr formatted gprsHour to 12hr time string
    gprsTime12hrString += ":";  // Add colon to create properly formatted time
    gprsTime12hrString += gprsMinute;  // Add gprsMinute string after the colon
    gprsTime12hrString += "am";  // Add "am", since time detected was before noon
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
