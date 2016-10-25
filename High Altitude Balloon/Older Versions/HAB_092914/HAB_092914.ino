/*
Arduino module to be used on a high-altitude balloon flight as a
 companion to the Raspberry Pi + PiInTheSky telemetry package.
 
 To Do:
 - Find reliable string element for gprsBootLoop() to identify actual data
 - Fill smsDataString with Google maps link from rocket telemetry sketch
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

#define sdSlaveSelect 10  // SPI chip select pin (SS) for SD card (specific to Arduino board)

const int piezoPin = 3;  // Pin for piezo buzzer beep on successful RFID read
const int readyLED = A0;  // Pin for LED to indicate entry of main loop

SdFat sd;
SdFile dataFile;

SoftwareSerial gprsSerial(gprsRXPin, gprsTXPin);

String rawDateTime;  // Stores raw date and time info from GPRS before parsing

// Date
String year;  // Year parsed from rawDateTime
String month;  // Month parsed from rawDateTime
String day;  // Day parsed from rawDateTime
String dateString;  // Parsed/concatenated date info from GPRS that is written to SD

// Time
String hour;  // Hour parsed from rawDateTime for 12hr time construction
String minute;  // Minute parsed from rawDateTime for 12hr time construction
String second;  // Second parsed from rawDateTime for 12hr time construction
String timeString;  // Parsed/concatenated time info from GPRS that is written to SD

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
  Serial.print(F("Initializing SD card..."));
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
  Serial.print(F("Beginning main boot sequence"));
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
  Serial.println(F("Data acquisition initiated."));
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
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
}

/*========================
 ////  Date and time  ////
 =======================*/

// Read date/time info from GPRS shield and parse into year, month, and day for reformatting to US date format
void getDateTime() {
  year = "";  // Clear year string before reading from GPRS
  month = "";  // Clear month string before reading from GPRS
  day = "";  // Clear day string before reading from GPRS

  gprsSerial.println("AT+CCLK?");  // Read date/time from GPRS
  if(gprsSerial.available()) {  // If data is coming from GPRS
    while(gprsSerial.available()) {  // Read the data into string from incoming bytes while they're available
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
      if(c == '\n') break;
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
      delay(10);  
    }
#ifdef rawDebug
    Serial.println(rawDateTime);
    Serial.flush();
#endif
    for(int y = 8; y < 10; y++) {  // Parse out year characters from rawDateTime
      year += String(rawDateTime.charAt(y));
    }
    for(int mo = 11; mo < 13; mo++) {  // Parse out month characters from rawDateTime
      month += String(rawDateTime.charAt(mo));
    }
    for(int d = 14; d < 16; d++) {  // Parse out day characters from rawDateTime
      day += String(rawDateTime.charAt(d));
    }
    for(int t = 17; t < 25; t++) {  // Parse out time characters from rawDateTime
      timeString += String(rawDateTime.charAt(t));
    }

    // Construct US formatted date string (M/D/Y)
    dateString += String(month);
    dateString += "/";
    dateString += String(day);
    dateString += "/";
    dateString += String(year);
  }
}

void clearDateTimeStrings() {
  rawDateTime = "";
  dateString = "";
  timeString = "";
  time12hrString = "";
  dateLongString = "";
}

#ifdef liveLaunch
void smsDataString() {
  gprsSerial.println(F("AT+CMGF=1"));
  delay(100);
  gprsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  for(int i = 0; i < 10; i++) {
    gprsSerial.print(smsTargetNum[i]);
    delay(100);
  }
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print(dataString);
  delay(100);
  gprsSerial.println((char)26);
  delay(1000);
  gprsSerial.flush();
#ifdef debugMode
  Serial.println(F("Train information sent via SMS."));
  delay(1000);
#endif
}
#endif
