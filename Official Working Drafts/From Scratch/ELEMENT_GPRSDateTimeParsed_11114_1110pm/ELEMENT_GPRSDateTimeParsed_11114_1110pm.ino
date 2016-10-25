/*  Framework for debugging individual program elements  */

#include <SoftwareSerial.h>

#define rxPin 7
#define txPin 8

SoftwareSerial gprsSerial(rxPin, txPin);

String rawDateTime;
String timeString;
String dateString;

void clearStrings() {
  timeString = "";
  dateString = "";
}

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  Serial.begin(19200);
  gprsSerial.begin(19200);
}

void loop() {
  delay(2000);
  getDateTime();
  Serial.println(dateString);
  Serial.println(timeString);
  Serial.println();
  clearStrings();
}

void getDateTime() {
  gprsSerial.println("AT+CCLK?");  // Request date and time from GPRS
  if(gprsSerial.available()) {  // If data is coming from GPRS
    while(gprsSerial.available()) {  // Read the data into string from incoming bytes while they're available
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
    parseDateTime();
  }
}

void parseDateTime() {
  String year = "";
  String month = "";
  String day = "";

  for(int y = 8; y < 10; y++)
    year += String(rawDateTime.charAt(y));
  for(int m = 11; m < 13; m++)
    month += String(rawDateTime.charAt(m));
  for(int d = 14; d < 16; d++)
    day += String(rawDateTime.charAt(d));
  for(int t = 17; t < 25; t++)
    timeString += String(rawDateTime.charAt(t));

  rawDateTime = "";

  dateString += String(month);
  dateString += "/";
  dateString += String(day);
  dateString += "/";
  dateString += String(year);
}


