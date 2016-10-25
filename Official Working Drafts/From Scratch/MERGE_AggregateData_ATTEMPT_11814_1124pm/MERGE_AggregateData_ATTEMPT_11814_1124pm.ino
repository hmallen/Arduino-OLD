#include <SoftwareSerial.h>
#include <SD.h>

#define rxPin 7
#define txPin 8
#define powPin 9
#define chipSelect 10

SoftwareSerial gprsSerial(rxPin, txPin);

unsigned long currTimer = 0;
unsigned long prevTimer = 0;
unsigned long lapTimer = 0;
const unsigned long interval = 30000;

const int aggIntervalMin = 10;
const unsigned long aggIntervalMillis = aggIntervalMin * 60000;

String rawDateTime;
String year;
String month;
String day;
int yearInt = 0;
int monthInt = 0;
int dayInt = 0;
String dateString;
String hour;
String minute;
String second;
String hour12;
String timeString;
String dateLongString;
String time12hrString;
String fullDataString;

int h2oLevel = 0;
int h2oLastLevel = 0;
const int h2oChangeTrigger = 10;
int h2oAmtCons = 0;

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);
  pinMode(10, OUTPUT);  // Required for SD lib fxn

  Serial.begin(19200);
  gprsSerial.begin(19200);

  powerUp();
  gprsBootLoop();
  clearStrings();
  delay(2000);

  Serial.println("Welcome to MyStall.");
  Serial.println();
  delay(2000);

  getDateTime();
  delay(1000);
  dateLongConst();
  delay(1000);
  time12hrConst();
  delay(1000);

  Serial.println(dateString);
  delay(100);
  Serial.println(timeString);
  delay(100);
  Serial.println(hour);
  delay(100);
  Serial.println(hour12);
  delay(100);
  Serial.println(minute);
  delay(100);
  Serial.println(second);
  delay(100);
  Serial.println();

  Serial.print("It is currently ");
  Serial.print(time12hrString);
  Serial.print(", ");
  Serial.print(dateLongString);
  Serial.println(".");
  Serial.println();

  h2oLevelRead();
  h2oLastLevel = h2oLevel;

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present.");
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println();
}

void loop() {
  currTimer = millis();
  lapTimer = currTimer;
  int elapTimer = currTimer - lapTimer;
  while(elapTimer < aggIntervalMillis) {
    currTimer = millis();
    elapTimer = currTimer - lapTimer;
    int intervalTimer = currTimer - prevTimer;
    if(elapTimer < interval) {
      if(abs(h2oLevel - h2oLastLevel) <= h2oChangeTrigger) {
        int nextWrite = (interval - elapTimer) / 1000;
        Serial.print("Next measurement in ");
        Serial.print(nextWrite);
        Serial.println(" seconds.");
        Serial.println();
      }
      else {
        Serial.println("Water level change >5% detected. Reading again to confirm.");
        delay(5000);
        h2oLevelRead();
        if(abs(h2oLevel - h2oLastLevel) >= h2oChangeTrigger) {
          Serial.println("Change confirmed. Writing to SD card.");
          h2oWriteSD();
          prevTimer = currTimer;
        }
        else {
          Serial.println("Detected change was a false-positive.");
          Serial.println();
        }
      }
    }
    else {
      h2oWriteSD();
      prevTimer = currTimer;
    }
  }
  delay(2000);
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

void gprsBootLoop() {
  Serial.print("Initializing GPRS...");
  while(dateString == 0) {
    getDateTime();
    delay(1000);
  }
  Serial.println("GPRS ready.");
  Serial.println();
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
  year = "";
  month = "";
  day = "";
  gprsSerial.println("AT+CCLK?");  // Request date and time from GPRS
  if(gprsSerial.available()) {  // If data is coming from GPRS
    while(gprsSerial.available()) {  // Read the data into string from incoming bytes while they're available
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
    for(int y = 8; y < 10; y++) {
      year += String(rawDateTime.charAt(y));
      yearInt = year.toInt();
    }
    for(int mo = 11; mo < 13; mo++) {
      month += String(rawDateTime.charAt(mo));
      monthInt = month.toInt();
    }
    for(int d = 14; d < 16; d++) {
      day += String(rawDateTime.charAt(d));
      dayInt = day.toInt();
    }
    for(int t = 17; t < 25; t++)
      timeString += String(rawDateTime.charAt(t));

    dateString += String(month);
    dateString += "/";
    dateString += String(day);
    dateString += "/";
    dateString += String(year);
  }
  rawDateTime = "";
}

void time12hrConst() {
  hour = "";
  minute = "";
  second = "";
  hour12 = "";

  for(int h = 0; h < 2; h++)
    hour += String(timeString.charAt(h));
  for(int mi = 3; mi < 5; mi++)
    minute += String(timeString.charAt(mi));
  for(int s = 6; s < 8; s++)
    second += String(timeString.charAt(s));

  if(hour.toInt() > 12) {
    hour12 += String(hour.toInt() - 12);
    time12hrString += String(hour12);
    time12hrString += ":";
    time12hrString += String(minute);
    time12hrString += "pm";
  }
  else {
    hour12 += String(hour.toInt());
    time12hrString += hour12;
    time12hrString += ":";
    time12hrString += minute;
    time12hrString += "am";
  }
}

void dateLongConst() {
  String yearLong = "";
  String monthLong = "";
  String dayLong = "";
  dateLongString = "";
  // Format year
  yearLong += "20";  // Add leading "20" to form long form of date
  yearLong += String(year);  // Append short date parsed from gprs data

  // Format day
  dayLong += String(dayInt);  // Convert to integer function removes and leading 0's

  // Format month 
  switch(monthInt)  // Convert to integer and use switch case to convert month number to long form
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

void h2oLevelRead() {
  int h2oReadVal = analogRead(A0);
  h2oLevel = h2oReadVal / 9.22;  // Divide by (MaximumPossibleReading / 100) for 0-100 scale
}

void h2oWriteSD() {
  getDateTime();
  concatStrings();
  sdWriteData();
  Serial.println();
  clearStrings();
  h2oLastLevel = h2oLevel;
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
