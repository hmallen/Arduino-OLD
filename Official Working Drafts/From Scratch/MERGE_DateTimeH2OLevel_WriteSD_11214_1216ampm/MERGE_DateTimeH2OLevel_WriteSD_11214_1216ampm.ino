/*  Framework for debugging individual program elements  */

#include <SoftwareSerial.h>
#include <SD.h>

#define rxPin 7
#define txPin 8

const int chipSelect = 10;

SoftwareSerial gprsSerial(rxPin, txPin);

String rawDateTime;
String timeString;
String dateString;
String fullDataString;

int h2oLevel = 0;

void clearStrings() {
  dateString = "";
  timeString = "";
  fullDataString = "";
}

void concatStrings() {
  fullDataString += dateString;
  fullDataString += ",";
  fullDataString += timeString;
  fullDataString += ",";
  fullDataString += h2oLevel;
}

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(10, OUTPUT);  // Required for SD lib fxn

  Serial.begin(19200);
  gprsSerial.begin(19200);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present.");
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println();
}

void loop() {
  getDateTime();
  h2oLevelRead();

  Serial.println(dateString);
  Serial.println(timeString);
  Serial.println(h2oLevel);

  concatStrings();
  Serial.println(fullDataString);
  Serial.println();
  
  sdWriteData();

  clearStrings();
  delay(2500);
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

void h2oLevelRead() {
  int h2oReadVal = analogRead(A0);
  h2oLevel = h2oReadVal / 9.22;  // Divide by (MaximumPossibleReading / 100) for 0-100 scale
}

void sdWriteDate() {
  dataFile = SD.open("NEWDATA.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(dataString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(dataString);  // Print dataString to serial monitor to confirm reasonable values
  }
  else {
    Serial.println("Error opening file while attempting to write to SD.");  // If data file can't be opened, produce an error message
  }
}
