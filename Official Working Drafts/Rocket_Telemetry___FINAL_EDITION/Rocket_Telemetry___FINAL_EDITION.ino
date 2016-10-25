/*
Rocket telemetry
 
 A project long in wait -- only now ready to be pursued competently
 
 TO BYPASS SOFTWARE SERIAL ISSUES, WRITE PROGRAM AND DEBUG THEN SWITCH TO GPRS HARDWARE
 SERIAL FOR DEPLOYMENT...AT THE COST OF NO SERIAL OUTPUT FOR DEBUGGING.
 */

#include <TinyGPS.h>
#include <SdFat.h>
#include <Wire.h>
//#include <SoftwareSerial.h>

#define rxPin 7
#define txPin 8
#define powPin 9
#define chipSelect 10

/*SoftwareSerial gprsSerial(rxPin, txPin);

String dateString;
String timeString;
String year;
String month;
String day;*/

File dataFile;

/*void powergprs() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

void gprsbootloop() {
  Serial.print(F("Initializing GPRS..."));
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
  Serial.println(F("GPRS ready."));
}*/

void setup() {
  Serial.begin(19200);

  //powerUp();  // Call function to power - up GPRS with software commands
  //gprsBootLoop();  // Initiate GPRS read loop until data is received (1st loop() read fails otherwise)

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();
}

/*void getDateTime() {
  dateString = "";
  timeString = "";
  year = "";  // Clear year string before reading from GPRS
  month = "";  // Clear month string before reading from GPRS
  day = "";  // Clear day string before reading from GPRS

  Serial1.println("AT+CCLK?");  // Read date/time from GPRS
  if(Serial1.available()) {  // If data is coming from GPRS
    while(Serial1.available()) {  // Read the data into string from incoming bytes while they're available
      char c = Serial1.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
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
  rawDateTime = "";  // Clear rawDateTime string for use on next read
}*/
