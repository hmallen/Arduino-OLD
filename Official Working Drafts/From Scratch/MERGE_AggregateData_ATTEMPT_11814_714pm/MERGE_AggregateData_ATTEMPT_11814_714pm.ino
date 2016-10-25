#include <SD.h>
#include <SoftwareSerial.h>

#define rxPin 7
#define txPin 8
#define chipSelect 10

SoftwareSerial gprsSerial(rxPin, txPin);

unsigned long currTimer = 0;
unsigned long prevTimer = 0;
const unsigned long interval = 30000;

const int aggIntervalMin = 10;
unsigned long aggIntervalMillis = aggIntervalMin * 60000;

String rawDateTime;
String yearInt;
String monthInt;
String dayInt;
String dateString;
String hourInt;
String minuteInt;
String secondInt;
String timeString;
String dateLongString;
String 12hrTimeString;
String fullDataString;

int h2oLevel = 0;
int h2oAmtCons = 0;

unsigned long currTimer = 0;
unsigned long prevTimer = 0;
unsigned long interval = 10000;


void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);
  pinMode(10, OUTPUT);  // Required for SD lib fxn

  Serial.begin(19200);
  gprsSerial.begin(19200);

  gprsBootLoop();

  Serial.println("Welcome to MyStall.")
    Serial.println();

  getDateTime();


  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present.");
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println();
}

void loop() {
  millis();
  currTimer = millis();
  while(currTimer < aggIntervalMillis) {
    currTimer = millis();
    int elapTimer = currTimer - prevTimer;

    if(elapTimer < interval) {
      Serial.println(interval - elapTimer);
    }
    else {
      h2oWriteSD();
    }
  }
  h2oAmtConsSince();
  h2oWrite
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

void getDateTime() {
  gprsSerial.println("AT+CCLK?");  // Request date and time from GPRS
  if(gprsSerial.available()) {  // If data is coming from GPRS
    while(gprsSerial.available()) {  // Read the data into string from incoming bytes while they're available
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
    String year = "";
    String month = "";
    String day = "";
    String hour = "";
    String minute = "";
    String second = "";

    for(int y = 8; y < 10; y++) {
      year += String(rawDateTime.charAt(y));
      yearInt = year.toInt();
    }
    for(int mo = 11; mo < 13; mo++) {
      month += String(rawDateTime.charAt(m));
      monthInt = month.toInt();
    }
    for(int d = 14; d < 16; d++) {
      day += String(rawDateTime.charAt(d));
      dayInt = day.toInt();
    }
    for(int t = 17; t < 25; t++)
      timeString += String(rawDateTime.charAt(t));
    for(int h = 1; h < 3; h++)
      hour += String(timeString.charAt(h));
    for(int mi = 4; mi < 6; mi++)
      minute += String(timeString.charAt(mi));
    for(int s = 7; s < 9; s++)
      second += String(timeString.charAt(s));

    hourInt = hour.toInt();
    minuteInt = minute.toInt();
    secondInt = second.toInt();

    dateString += String(month);
    dateString += "/";
    dateString += String(day);
    dateString += "/";
    dateString += String(year);
  }
  rawDateTime = "";
}

void 12hrTimeStringConst() {
  int hour12 = 0;
  12hrTimeString = "";
  if(hour.toInt > 12) {
    hour12 = hour.toInt() - 12;
    12hrTimeString += String(hour12);
    12hrTimeString += ":";
    12hrTimeString += minute;
    12hrTimeString += "pm";
  }
  else {
    hour12 = hour.toInt();
    12hrTimeString += String(hour12);
    12hrTimeString += ":";
    12hrTimeString += minute;
    12hrTimeString += "am";
  }
}

void dateLong()
{
  String yearLong = "";
  String monthLong = "";
  String dayLong = "";
  dateLongString = "";
  // Format year
  yearLong += "20";  // Add leading "20" to form long form of date
  yearLong += String(year);  // Append short date parsed from gprs data

  // Format day
  dayLong += day.toInt();  // Convert to integer function removes and leading 0's

  // Format month 
  switch(month.toInt())  // Convert to integer and use switch case to convert month number to long form
  {
  case 0:
    return;
  case 1:
    monthLong += "January";
    break;
  case 2:
    monthLong += "February";
    break;
  case 3:
    monthLong += "March";
    break;
  case 4:
    monthLong += "April";
    break;
  case 5:
    monthLong += "May";
    break;
  case 6:
    monthLong += "June";
    break;
  case 7:
    monthLong += "July";
    break;
  case 8:
    monthLong += "August";
    break;
  case 9:
    monthLong += "September";
    break;
  case 10:
    monthLong += "October";
    break;
  case 11:
    monthLong += "November";
    break;
  case 12:
    monthLong += "December";
    break;
  }
  // Concatenate strings to form full long date
  dateLongString += String(monthLong);
  dateLongString += " ";
  dateLongString += String(dayLong);
  dateLongString += ", ";
  dateLongString += String(yearLong);
}

void h2oWriteSD() {
  getDateTime();
  concatStrings();
  sdWriteData();
  Serial.println();
  clearStrings();
}

void sdWriteData() {
  File dataFile = SD.open("NEWESTDATA.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(fullDataString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(fullDataString);
  }
  else {
    Serial.println("Error opening file while attempting to write to SD.");  // If data file can't be opened, produce an error message
  }
}

void h2oAmtConsSince() {

}



