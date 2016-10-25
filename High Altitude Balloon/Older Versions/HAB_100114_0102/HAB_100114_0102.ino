/*
Arduino module to be used on a high-altitude balloon flight as a
 companion to the Raspberry Pi + PiInTheSky telemetry package.
 
 To Do:
 - Add I2C sensor functionality
 - Add SD data logging functionality
 - Find reliable string element for gprsBootLoop() to identify actual data
 - GPS functions
 -> Total distance travelled
 -> Use GPS altitude as initial for altimeter after zeroing
 - I2C sensor functions
 -> If gyroscope detecting rotation ---> Perform necessary camera adjustments
 -> Align compass and camera axes
 -> If possible, add servo function to rotate camera during flight and log direction
 -> Make gyroscope reads at interval, and collect at maximum acquisition rate
 --> Idea: If quick rotation detected, begin fast gyro data acquisition
 */

#define debugMode
#define liveLaunch

#include "IntersemaBaro.h"
#include <SdFat.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>

#define gprsRXPin 7  // Receive pin for GPRS software serial
#define gprsTXPin 8  // Transmit pin for GPRS software serial
#define gprsPowPin 9  // Pin for software power-up of GPRS shield

#define gpsRXPin 2  // Receive pin for GPS software serial
#define gpsTXPin 255  // Transmit pin for GPS software serial (255 when TX unnecessary)

#define sdSlaveSelect 10  // SPI chip select pin (SS) for SD card (specific to Arduino board)

const int piezoPin = 3;  // Pin for piezo buzzer beep on successful RFID read
const int readyLED = A0;  // Pin for LED to indicate entry of main loop

const char smsTargetNum[11] = "2145635266";  // Mobile number to send data via SMS

SdFat sd;
SdFile dataFile;

TinyGPS gps;

Intersema::BaroPressure_MS5607B baro(true);

SoftwareSerial gpsSerial(gpsRXPin, gpsTXPin);
SoftwareSerial gprsSerial(gprsRXPin, gprsTXPin);

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

// GPS
boolean highElevLaunchSite = false;

long altCm;
long altCalibVal;
long altCmInitial;
long altCmLast;
unsigned long altCmMax;
unsigned long landDistanceFt;
unsigned long altFeetMax;

float gpsLat;
float gpsLon;
float gpsLatInitial;
float gpsLonInitial;
float gpsLatFinal;
float gpsLonFinal;
String gpsDateTime = "";

String dataString;  // Full data string written to SD card

void setup() {
  pinMode(piezoPin, OUTPUT);
  pinMode(readyLED, OUTPUT);

  digitalWrite(piezoPin, LOW);
  digitalWrite(readyLED, LOW);

  Serial.begin(19200);

#ifdef debugMode
  Serial.print(F("Powering up GPRS sheild..."));
#endif
  gprsPowerOn();
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing I2C connection..."));
#endif
  Wire.begin();
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing altimeter..."));
#endif
  baro.init();
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing GPS connection..."));
#endif
  gpsSerial.begin(4800);
  if(!gpsSerial.available()) {
    while(!gpsSerial.available()) {
      delay(1);
    }
  }
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing GPRS connection..."));
#endif
  gprsSerial.begin(19200);  // Software serial for GPRS communication
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Initializing SD card..."));
#endif
  if(!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.println();
  delay(1000);
  Serial.print(F("Configuring GPRS..."));
  unsigned long bootPrep = millis() + 5000;
  for(unsigned long x = millis(); x < bootPrep; ) {
    Serial.print(F("."));
    delay(500);
  }
#endif
  configureGPRS();
  gprsBootLoop();

#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Zeroing altimeter..."));
#endif
  readAlt();
  while(altCm >= 152) {
    zeroAltimeter();
    delay(2000);
    readAlt();
  }
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.print(F("Acquiring launch site coordinates..."));
#endif
  getGPSData();
  gpsLatInitial = gpsLat;
  gpsLonInitial = gpsLon;
#ifdef debugMode
  Serial.println(F("complete."));
  Serial.println();
  Serial.print(F("Latitude:  "));
  Serial.println(gpsLatInitial, 6);
  Serial.print(F("Longitude: "));
  Serial.println(gpsLonInitial, 6);
  Serial.println();
#endif
  getDateTime();  // Read date and time information from GPRS
  dateLongConst();  // Construct string with long form of date
  time12hrConst();  // Construct string with 12hr format of time
  Serial.print(F("It is currently "));
  Serial.print(gprsTime12hrString);
  Serial.print(F(", "));
  Serial.print(gprsDateLongString);
  Serial.println(F("."));
  Serial.println();
#ifdef debugMode
  Serial.println(F("System ready for launch. Beginning data acquisition."));
  Serial.println();
#endif
  digitalWrite(readyLED, HIGH);
}

void loop() {
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
  gprsSerial.println("ATE0");
  delay(100);
  gprsSerial.println("ATQ1");
  delay(100);
  gprsSerial.println("ATV0");
  delay(100);
}

void gprsBootLoop() {
  while(gprsDateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
}

void zeroAltitude() {
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
}

void checkCalib() {
#ifdef debugMode
  Serial.print(F("Confirming altimeter calibration..."));
#endif
  delay(5000);
  for(int x = 0; x < 10; x++) {
    readAlt();
    if(abs(altCm - altCmInitial) > 61) {
      zeroAltitude();
    }
  }
#ifdef debugMode
  Serial.println(F("altimeter correctly calibrated."));
  Serial.println();
#endif
}

/*========================
 ////  Date and time  ////
 =======================*/

// Read date/time info from GPRS shield and parse into gprsYear, gprsMonth, and gprsDay for reformatting to US date format
void getDateTime() {
  gprsYear = "";  // Clear gprsYear string before reading from GPRS
  gprsMonth = "";  // Clear gprsMonth string before reading from GPRS
  gprsDay = "";  // Clear gprsDay string before reading from GPRS

  gprsSerial.println("AT+CCLK?");  // Read date/time from GPRS
  if(gprsSerial.available()) {  // If data is coming from GPRS
    while(gprsSerial.available()) {  // Read the data into string from incoming bytes while they're available
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
      if(c == '\n') break;
      gprsRawDateTime += c;  // Add character to the end of the data string to be written to SD later
      delay(10);  
    }
#ifdef rawDebug
    Serial.println(gprsRawDateTime);
    Serial.flush();
#endif
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
    gprsDateString += String(gprsMonth);
    gprsDateString += "/";
    gprsDateString += String(gprsDay);
    gprsDateString += "/";
    gprsDateString += String(gprsYear);
  }
}

void clearDateTimeStrings() {
  gprsRawDateTime = "";
  gprsDateString = "";
  gprsTimeString = "";
  gprsTime12hrString = "";
  gprsDateLongString = "";
}

void getGPSData() {
  boolean newdata = false;
  unsigned long start = millis();
  while(millis() - start < 5000) {  // Update every 5 seconds
    if(feedgps()) newdata = true;
    if (newdata) gpsdump(gps);
  }
}

// Get and process GPS data
/*void gpsdump(TinyGPS &gps) {
 float flat, flon;
 int year;
 byte gpsMonth, gpsDay, gpsHour, gpsSecondRaw;
 unsigned long age;
 
 String gpsSecond = "";
 String gpsDate = "";
 String gpsTime = "";
 
 
 gps.f_get_position(&flat, &flon, &age);
 gpsLat = flat;
 gpsLon = flon;
 gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
 }*/
void gpsdump(TinyGPS &gps) {  
  float flat, flon;
  unsigned long age;
  String gpsDate = "";
  String gpsTime = "";

  String monthFormatted = "";
  String dayFormatted = "";
  String hourFormatted = "";
  String minuteFormatted = "";
  String secondFormatted = "";
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4); 
  Serial.print(", "); 
  Serial.println(flon, 4);
  Serial.flush();
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  // Time String
  if(hour < 10) hourFormatted = "0" + String(hour);
  else hourFormatted = String(hour);
  if(minute < 10) minuteFormatted = "0" + String(minute);
  else minuteFormatted = String(minute);
  if(second < 10) secondFormatted = "0" + String(second);
  else secondFormatted = String(second);
  gpsTime = hourFormatted + ":" + minuteFormatted + ":" + secondFormatted;

  // Date String
  if(month < 10) monthFormatted = "0" + String(month);
  else monthFormatted = String(month);
  if(day < 10) dayFormatted = "0" + String(day);
  else dayFormatted = String(day);
  gpsDate = monthFormatted + "/" + dayFormatted + "/" + year;

  gpsDateTime = gpsTime + ", " + gpsDate;

  Serial.println(gpsDate);
  Serial.println(gpsTime);
  Serial.println(gpsDateTime);
  float altitude;
  altitude = gps.f_altitude();
  Serial.println(altitude);
  float course;
  course = gps.f_course();
  Serial.println(course);
  float speed;
  speed = gps.f_speed_mph();
  Serial.println(speed);

  Serial.println();
}

// Feed data as it becomes available 
boolean feedgps() {
  while (gpsSerial.available()) {
    if (gps.encode(gpsSerial.read()))
      return true;
  }
  return false;
}

#ifdef liveLaunch
void smsDataString() {
  gprsSerial.println(F("AT+CMGF=1"));
  delay(100);
  gprsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  gprsSerial.print(targetNumb);
  delay(100);
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print(F("http://maps.google.com/maps?q="));
  delay(100);
  gprsSerial.print(F("Landing+Site@"));
  delay(100);
  gprsSerial.print(gpsLatFinal);
  delay(100);
  gprsSerial.print(F(",+"));
  delay(100);
  gprsSerial.print(gpsLonFinal);
  delay(100);
  gprsSerial.print(F("&t=h&z=19&output=html /"));
  delay(100);
  gprsSerial.print(F(" Max Altitude: "));
  delay(100);
  gprsSerial.print(altFeetMax);
  delay(100);
  gprsSerial.print(F(" feet /"));
  delay(100);
  gprsSerial.print(F(" Landed "));
  delay(100);
  gprsSerial.print(landDistance);
  delay(100);
  gprsSerial.println(F(" feet away from launch site."));
  delay(100);
  gprsSerial.println((char)26);
  delay(1000);
  gprsSerial.flush();
#ifdef debugMode
  Serial.println(F("Coordinates sent via SMS."));
  delay(1000);
  Serial.print(gpsLatFinal);
  Serial.print(F(",+"));
  Serial.println(gpsLonFinal);
  Serial.println(landDistance);
  Serial.println(altFeetMax);
#endif
}
#endif
