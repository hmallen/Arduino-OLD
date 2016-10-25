/*
Fil's neat car program
- GPS Tracker
- SMS Menu
- Discreet & Persistent
-- Hidden
-- BaTTERY CHARGES AUTOMATICALLY
*/

#include <SdFat.h>
#include <SoftwareSerial.h>
#include <Time.h>
#include <TinyGPS.h>

const int tripPin = 4;
const int smsPowPin = 9;  // Pin for software power-up of GPRS shield
const int sdSlaveSelect = 10;

const char smsTargetNum[11] = "2145635266";  // Mobile number to send data and receive commands via SMS

const int gpsGmtOffsetHours = -5;  // Offset in hours of current timezone from GMT (I think it only accepts + vals b/c it gets converted to a 'byte')

SdFat sd;
SdFile dataFile;
char logFile[13];

const int x = 3;
const int y = 2;

SoftwareSerial gpsSerial(x, y); /* NNED TOO FIX THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
SoftwareSerial smsSerial(8, 7);
TinyGPS gps;  // GPS

// Landing SMS Functions
String smsMessageRaw;  // Raw received SMS string from GPRS
String smsRecNumber;  // Parsed number of origin of received SMS
String smsMessage;  // Parsed SMS message
String smsCommandOutput;  // Text string written to SD to indicate SMS action taken

// Date & Time
String currentDateTime;  // Stores current date and time information when retrieved via Time.h library

// GPS
const int gpsSatMinimum = 4;  // May want to change to 6 for real launch
const int gpsHDOPMinimum = 250;  // May want to change to 200 for real launch
int satellites, hdop;
float altitude;
float gpsLat, gpsLon;
float gpsLatInitial, gpsLonInitial;
float gpsLatFinal, gpsLonFinal;
float gpsAltitudeFt, gpsAltitudeFtMax;
float gpsCourse;
float gpsSpeedMPH, gpsSpeedMPHMax;
float tripDistanceFt, tripDistanceMi;
int gpsCourseToLand;
String gpsCourseCardDir;

boolean gpsLock = false;  // Becomes true when sufficient satellite lock and HDOP (precision) are achieved
boolean syncRequired = true;  // Remains true until GPRS syncs date/time with GPS
boolean tripMode = false;
boolean tripFinalCoord = false;
boolean tripFinalDataSent = true;

boolean firstLoop = true;
boolean startupComplete = false;  // Triggered when setup() complete
boolean smsCommandExec = false;  // Triggered when an SMS command is received and executed
boolean gpsCoordUpdate = false;  // True if new GPS coordinates acquired after read, false if 0's are read from GPS (Post-landing only)

int loopCount = 0;

void setup() {
  Serial.begin(19200);
  Serial.println(F("Beginning setup..."));

  pinMode(tripPin, INPUT);

  gpsSerial.begin(4800);  // GPS
  if (!gpsSerial.available()) {
    while (!gpsSerial.available()) {
      delay(1);
    }
  }
  smsSerial.begin(19200);  // GPRS

  // Wait for GPS to acquire enough satellites and have sufficient HDOP (precision)
  gpsGetData();
  if (satellites < gpsSatMinimum || hdop > gpsHDOPMinimum) {
    Serial.println();
    while (satellites < gpsSatMinimum || hdop > gpsHDOPMinimum) {
      gpsGetData();
#ifdef debugMode
      Serial.print(F("Sats: "));
      Serial.println(satellites);
      Serial.print(F("HDOP: "));
      Serial.println(hdop);
#endif
      delay(5000);
    }
  }
  gpsLock = true;

  // Software power-up of GPRS shield
  digitalWrite(smsPowPin, LOW);
  delay(100);
  digitalWrite(smsPowPin, HIGH);
  delay(500);
  digitalWrite(smsPowPin, LOW);
  delay(100);

  gpsGetData();
  syncRequired = false;

  if (!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
  time_t t = now();
  int yr = year(t) - 2000;
  sprintf(logFile, "%.2d%.2d%.2d00.csv", month(t), day(t), yr);

  Serial.print(F("Date/time:      "));
  digitalClockDisplay();
  Serial.print(F("Log file:       "));
  Serial.println(logFile);

  for (int i = 0; i < 100; i++) {
    logFile[6] = (i / 10) + '0';
    logFile[7] = (i % 10) + '0';
    if (sd.exists(logFile)) continue;
    if (!dataFile.open(logFile, O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt("Failed to create log file.");
    dataFile.close();
    break;
  }

  startupComplete = true;
}

void loop() {

}

/*===============
 ////  GPS  ////
 ===============*/

void gpsGetData() {
  boolean newdata = false;
  unsigned long start = millis();
  while (millis() - start < 5000) { // Update every 5 seconds
    if (feedgps()) newdata = true;
    if (newdata) gpsdump(gps);
  }
  getDateTime();
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  float flat, flon;
  satellites = gps.satellites();
  hdop = gps.hdop();

  if (gpsLock == true) {
    unsigned long age;
    byte month, day, hour, minute, second, hundredths;
    int year;

    if (syncRequired == true) {
      gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
      setTime(hour, minute, second, day, month, year);
      adjustTime(gpsGmtOffsetHours * SECS_PER_HOUR);

      gps.f_get_position(&flat, &flon, &age);
      gpsLatInitial = flat;
      gpsLonInitial = flon;
    }

    else if (tripMode == true) { // MAKE SURE THIS IS ACTUALLY THE CONDITION YOU WANT TO TRIGGER THIS OUTPUT BEFORE LIVE LAUNCH!!!!!!!!!!!!!!!!!
      if (digitalRead(tripPin) == false) {
        tripMode = false;
        tripFinalCoord = true;
        tripFinalDataSent = false;
      }
      gps.f_get_position(&flat, &flon, &age);
      gpsLat = flat;
      gpsLon = flon;
      altitude = gps.f_altitude();
      gpsAltitudeFt = altitude / 3.048;
      gpsCourse = gps.f_course();
      gpsSpeedMPH = gps.f_speed_mph();
    }

    else {
      gps.f_get_position(&flat, &flon, &age);
      gpsLat = flat;
      gpsLon = flon;
      altitude = gps.f_altitude();
      gpsAltitudeFt = altitude / 3.048;
      gpsCourse = gps.f_course();
      gpsSpeedMPH = gps.f_speed_mph();

      if (flat != 0 && flon != 0) {
        if (tripFinalCoord == true) {
          gpsLatFinal = flat;
          gpsLonFinal = flon;

          float fdist = gps.distance_between(gpsLatFinal, gpsLonFinal, gpsLatInitial, gpsLonInitial);
          tripDistanceFt = fdist * 3.28;
          tripDistanceMi = tripDistanceFt / 5280.0;
          gpsCourseToLand = gps.course_to(gpsLatInitial, gpsLonInitial, gpsLatFinal, gpsLonFinal);
          gpsCourseCardDir = gps.cardinal(gpsCourseToLand);

          tripFinalCoord = false;
        }
        if (smsCommandExec == true) {
          smsCommandOutput = "Updated GPS coordinates sent via SMS.";
          gpsCoordUpdate = true;
        }
      }

      else if (smsCommandExec == true) {
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
    }
  }
}

// Feed data as it becomes available
boolean feedgps() {
  while (gpsSerial.available()) {
    if (gps.encode(gpsSerial.read())) return true;
  }
  return false;
}

void getDateTime() {
  currentDateTime = "";

  currentDateTime += month();
  currentDateTime += "/";
  currentDateTime += day();
  currentDateTime += "/";
  currentDateTime += year();
  currentDateTime += ",";
  currentDateTime += formatDigits(hour());
  currentDateTime += ":";
  currentDateTime += formatDigits(minute());
  currentDateTime += ":";
  currentDateTime += formatDigits(second());
}
String formatDigits(int digits) {
  String formattedDigits = "";
  if (digits < 10) {
    formattedDigits += '0';
    formattedDigits += digits;
  }
  else formattedDigits += digits;

  return formattedDigits;
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(month());
  Serial.print(F("/"));
  Serial.print(day());
  Serial.print(F("/"));
  Serial.print(year());
  Serial.print(F(", "));
  printDigits(hour());
  Serial.print(F(":"));
  printDigits(minute());
  Serial.print(F(":"));
  printDigits(second());

  if (startupComplete == false) Serial.println();
  delay(5000);
}
void printDigits(int digits) {
  if (digits < 10) Serial.print('0');
  Serial.print(digits);
}

/*===============
 ////  GPRS  ////
 ==============*/

// GPRS Function for SMS transmission of dataString
void smsSMSData() {
  smsSerial.println(F("AT+CMGF=1"));
  delay(100);
  smsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  for (int i = 0; i < 10; i++) {
    smsSerial.print(smsTargetNum[i]);
    delay(100);
  }
  smsSerial.println(F("\""));
  delay(100);
  smsSerial.print(F("http://maps.google.com/maps?q="));
  delay(100);
  smsSerial.print(F("Landing+Site@"));
  delay(100);
  smsSerial.print(gpsLatFinal, 6);
  delay(100);
  smsSerial.print(F(",+"));
  delay(100);
  smsSerial.print(gpsLonFinal, 6);
  delay(100);
  if (tripFinalDataSent == false) {
    smsSerial.println(F("&t=h&z=19&output=html"));
    delay(100);
    smsSerial.println((char)26);
    delay(100);
    smsSerial.flush();
    if (!smsSerial.available()) {
      while (!smsSerial.available()) {
        delay(10);
      }
    }
    smsSerialFlush();
    if (!smsSerial.available()) {
      while (!smsSerial.available()) {
        delay(10);
      }
    }
    smsSerialFlush();

    smsSerial.print(F("AT+CMGS=\"+1"));
    delay(100);
    for (int i = 0; i < 10; i++) {
      smsSerial.print(smsTargetNum[i]);
      delay(100);
    }
    smsSerial.println(F("\""));
    delay(100);
    smsSerial.print(F(" Max Altitude: "));
    delay(100);
    smsSerial.print(gpsAltitudeFtMax, 2);
    delay(100);
    smsSerial.print(F(" ft / "));
    delay(100);
    smsSerial.print(F("Max Speed: "));
    delay(100);
    smsSerial.print(gpsSpeedMPHMax, 2);
    delay(100);
    smsSerial.print(F(" mph"));
    delay(100);
    smsSerial.print(F(" / Landing site: "));
    delay(100);
    smsSerial.print(tripDistanceFt, 2);
    delay(100);
    smsSerial.print(F(" ft ("));
    delay(100);
    smsSerial.print(tripDistanceMi, 2);
    delay(100);
    smsSerial.print(F(" mi) "));
    delay(100);
    smsSerial.print(gpsCourseCardDir);
    delay(100);
    smsSerial.print(F(" ("));
    delay(100);
    smsSerial.print(gpsCourseToLand);
    delay(100);
    smsSerial.println(F(" deg)"));
    delay(100);
  }
  else {
    smsSerial.print(F("&t=h&z=19&output=html /"));
    delay(100);
    if (gpsCoordUpdate == true) smsSerial.println(F(" Coordinates updated."));
    else smsSerial.println(F(" GPS coordinate update failed."));
    delay(100);
  }
  smsSerial.println((char)26);
  delay(100);
  smsSerial.flush();

  if (!smsSerial.available()) {
    while (!smsSerial.available()) {
      delay(10);
    }
  }
  smsSerialFlush();
  if (!smsSerial.available()) {
    while (!smsSerial.available()) {
      delay(10);
    }
  }
  smsSerialFlush();

  Serial.println(F("Data sent via SMS."));
  Serial.println();
}

void smsSMSConfirmation(int menuVal) {
  switch (menuVal) {
    case 1:
      smsSerial.println(F("AT+CMGF=1"));
      delay(100);
      smsSerial.print(F("AT+CMGS=\"+1"));
      delay(100);
      for (int i = 0; i < 10; i++) {
        smsSerial.print(smsTargetNum[i]);
        delay(100);
      }
      smsSerial.println(F("\""));
      delay(100);
      smsSerial.println(F("Buzzer activated."));
      delay(100);
      smsSerial.println((char)26);
      delay(100);
      smsSerial.flush();

      if (!smsSerial.available()) {
        while (!smsSerial.available()) {
          delay(10);
        }
      }
      smsSerialFlush();
      if (!smsSerial.available()) {
        while (!smsSerial.available()) {
          delay(10);
        }
      }
      smsSerialFlush();
      break;
    case 2:
      smsSerial.println(F("AT+CMGF=1"));
      delay(100);
      smsSerial.print(F("AT+CMGS=\"+1"));
      delay(100);
      for (int i = 0; i < 10; i++) {
        smsSerial.print(smsTargetNum[i]);
        delay(100);
      }
      smsSerial.println(F("\""));
      delay(100);
      smsSerial.println(F("Smoke signal activated."));
      delay(100);
      smsSerial.println((char)26);
      delay(100);
      smsSerial.flush();

      if (!smsSerial.available()) {
        while (!smsSerial.available()) {
          delay(10);
        }
      }
      smsSerialFlush();
      if (!smsSerial.available()) {
        while (!smsSerial.available()) {
          delay(10);
        }
      }
      smsSerialFlush();
      break;
    default:
#ifdef debugMode
      Serial.println(F("Unrecognized menu command."));
#endif
      break;
  }
}

void smsSMSMenu() {
  smsSerial.println(F("AT+CMGF=1"));
  delay(100);
  smsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  for (int i = 0; i < 10; i++) {
    smsSerial.print(smsTargetNum[i]);
    delay(100);
  }
  smsSerial.println(F("\""));
  delay(100);
  smsSerial.println(F("Reply with desired option #: 1 - GPS / 2 - Buzzer / 3 - Smoke / 4 - Reboot"));
  delay(100);
  smsSerial.println((char)26);
  delay(100);
  smsSerial.flush();

  if (!smsSerial.available()) {
    while (!smsSerial.available()) {
      delay(10);
    }
  }
  smsSerialFlush();
  if (!smsSerial.available()) {
    while (!smsSerial.available()) {
      delay(10);
    }
  }
  smsSerialFlush();

#ifdef debugMode
  Serial.println(F("SMS command menu sent via SMS."));
  Serial.println();
#endif
}

void smsSerialFlush() {
#ifdef debugMode
  Serial.println(F("Flushing GPRS serial buffer:"));
#endif
  if (smsSerial.available()) {
    while (smsSerial.available()) {
      Serial.write(smsSerial.read());
      delay(10);
    }
  }
#ifdef debugMode
  Serial.println(F("GPRS serial buffer cleared."));
#endif
}
