/*  Framework for debugging individual program elements  */

#include <SoftwareSerial.h>
#include <SD.h>

#define rxPin 7
#define txPin 8
#define powPin 9

const int chipSelect = 10;

SoftwareSerial gprsSerial(rxPin, txPin);

String rawDateTime;
String timeString;
String dateString;
String fullDataString;

int h2oLevel = 0;
const int h2oChangeTrigger = 5;
int h2oLastLevel = 0;
boolean h2oChange = false;
int h2oChangeHigh = 0;
int h2oChangeLow = 0;

unsigned long currTimer = 0;
unsigned long prevTimer = 0;
unsigned long interval = 30000;

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);
  pinMode(10, OUTPUT);  // Required for SD lib fxn

  Serial.begin(19200);
  gprsSerial.begin(19200);

  powerUp();
  gprsBootLoop();

  h2oLevelRead();
  Serial.println(h2oLevel);
  h2oLastLevel = h2oLevel;
  Serial.println(h2oLastLevel);
  h2oCalcChangeInterval();
  Serial.println(h2oChangeLow);
  Serial.println(h2oChangeHigh);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present.");
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println();
}

void loop() {
  currTimer = millis();

  if((currTimer - prevTimer) < interval) {
    h2oLevelRead();
    if(h2oChange == true) {
      Serial.println(h2oChange);
      h2oLastLevel = h2oLevel;
      h2oChange = false;
      h2oWriteSD();
    }
    else {
      Serial.println(h2oChange);
      Serial.print("Next write: ");
      int nextWrite = (interval - (currTimer - prevTimer)) / 1000;
      Serial.print(nextWrite);
      Serial.println(" sec");
      Serial.println();
    }
  }
  else {
    h2oWriteSD();
    h2oLastLevel = h2oLevel;
  }
  delay(2000);
}

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

void gprsBootLoop() {
  while(dateString == 0) {
    getDateTime();
    delay(1000);
  }
  //dateLong();
  //Serial.println(dateLongString);
  clearStrings();
}

// Software start-up function for GPRS shield
void powerUp() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
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

void h2oCalcChangeInterval() {
  h2oChangeHigh = h2oLastLevel + h2oChangeTrigger;
  h2oChangeLow = h2oLastLevel - h2oChangeTrigger;
}

void h2oCheckChange() {
  Serial.println(h2oChangeLow);
  Serial.println(h2oLastLevel);
  Serial.println(h2oLevel);
  Serial.println(h2oChangeHigh);
  if(h2oLevel <= h2oChangeLow || h2oLevel >= h2oChangeHigh) {
    delay(5000);
    h2oLevelRead();
    if(h2oLevel <= h2oChangeLow || h2oLevel >= h2oChangeHigh) {
      h2oCalcChangeInterval();
      h2oChange = true;
    }
}

void h2oWriteSD() {
  getDateTime();

  h2oLevelRead();

  concatStrings();
  //Serial.println(fullDataString);

  sdWriteData();
  Serial.println();

  h2oCalcChangeInterval();

  prevTimer = currTimer;

  clearStrings();
}

void sdWriteData() {
  File dataFile = SD.open("NEWDATA.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(fullDataString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(fullDataString);
  }
  else {
    Serial.println("Error opening file while attempting to write to SD.");  // If data file can't be opened, produce an error message
  }
}


