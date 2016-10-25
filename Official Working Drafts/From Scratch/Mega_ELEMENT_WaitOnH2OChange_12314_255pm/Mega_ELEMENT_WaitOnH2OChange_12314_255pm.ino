#include <SD.h>

#define powPin 9
#define chipSelect 53

int currTimer = 0;
int prevTimer = 0;
int lapTimer = 0;
int aggTimer = 0;
int intervalTimer = 0;
int count = 0;
int amtCons = 0;
const int writeInterval = 10000;
const int aggInterval = 30000;
boolean h2oEmpty = false;

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
String aggString;

int h2oLevel = 0;
int h2oLastLevel = 0;
const int h2oChangeTrigger = 10;
int amtCons = 0;

void setup() {
  pinMode(powPin, OUTPUT);
  pinMode(chipSelect, OUTPUT);  // Required for SD lib fxn

  Serial.begin(19200);
  Serial1.begin(19200);

  powerUp();
  gprsBootLoop();
  clearStrings();
  delay(1000);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present.");
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println();

  Serial.println("Welcome to MyStall.");
  Serial.println();
  delay(1000);

  getDateTime();
  dateLongConst();
  time12hrConst();

  Serial.print("It is currently ");
  Serial.print(time12hrString);
  Serial.print(", ");
  Serial.print(dateLongString);
  Serial.println(".");
  Serial.println();

  h2oLevelRead();
  h2oLastLevel = h2oLevel;

  currTimer = millis();
  prevTimer = millis();
  aggTimer = millis();

  clearStrings();

  h2oCheckChange();
}

void loop() {
  currTimer = millis();
  intervalTimer = currTimer - prevTimer;
  lapTimer = currTimer - aggTimer;
  h2oLevelRead();
  if(h2oLevel < h2oLastLevel) {
    amtCons = amtCons + (h2oLastLevel - h2oLevel);
  }
  if(lapTimer >= aggInterval) {
    h2oLevelRead();
    Serial.println("Data aggregation occurs here.");
    //h2oAmtConsSince();
    aggregateWriteSD();
    prevTimer = currTimer;
    aggTimer = currTimer;
  }
  else if(intervalTimer >= writeInterval) {
    h2oLevelRead();
    h2oWriteSD();
    prevTimer = currTimer;
  }
  else {
    //int nextWrite = (writeInterval - intervalTimer) / 1000;
    //Serial.println(nextWrite);
  }
  h2oCheckChange();
  delay(1000);
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

void concatAggStrings() {
  aggString += dateString;
  aggString += ",";
  aggString += timeString;
  aggString += ",";
  aggString += amtCons;
}

void getDateTime() {
  year = "";
  month = "";
  day = "";
  Serial1.println("AT+CCLK?");  // Request date and time from GPRS
  if(Serial1.available()) {  // If data is coming from GPRS
    while(Serial1.available()) {  // Read the data into string from incoming bytes while they're available
      char c = Serial1.read();  // Read each byte sent, one at a time, into storage variable
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
  else if (hour.toInt() == 0) {
    hour12 += "12";
    time12hrString += String(hour12);
    time12hrString += ":";
    time12hrString += String(minute);
    time12hrString += "am";
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

void h2oCheckChange() {
  h2oLevelRead();
  if(h2oLevel <= 10) {
    Serial.println(h2oLevel);
    Serial.println("Water level low. Refill Bucket.");
  }
  if(h2oEmpty == false) {
    if(h2oLevel == 0) {
      Serial.println("Bucket empty or currently being refilled.");
      h2oEmpty = true;
    }
    else {
      if(abs(h2oLevel - h2oLastLevel) >= h2oChangeTrigger) {
        Serial.print(">= 10% change detected. Confirming...");
        delay(5000);
        h2oLevelRead();
        if(abs(h2oLevel - h2oLastLevel) >= h2oChangeTrigger) {
          Serial.println("change confirmed.");
          h2oWriteSD();
          prevTimer = currTimer;
          h2oLastLevel = h2oLevel;
        }
        else {
          Serial.println("detected change was a false-positive.");
          Serial.println();
        }
      }
    }
  }
  else {
    while(h2oEmpty == true) {
      h2oLevelRead();
      if(h2oLevel >= 75) {
        h2oEmpty = false;
      }
      else {
        Serial.println(h2oLevel);
        Serial.println("Waiting for bucket refill.");
        Serial.println();
      }
      delay(10000);
    }
  }
}

void h2oWriteSD() {
  getDateTime();
  concatStrings();
  sdWriteData();
  delay(1000);
  Serial.println();
  clearStrings();
}

void sdWriteData() {
  File dataFile = SD.open("LOGGING.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(fullDataString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println();
    Serial.println(fullDataString);
  }
  else {
    Serial.println("Error opening file while attempting to write to SD.");  // If data file can't be opened, produce an error message
  }
}

void aggregateWriteSD() {
  Serial.print(amtCons);
  Serial.println(" units consumed since last data aggregation.");
  getDateTime();
  concatAggStrings();
  sdWriteAggregate();
  delay(1000);
  Serial.println();
  clearStrings();
  aggString = "";
  amtCons = 0;
}

void sdWriteAggregate() {
  File aggFile = SD.open("AGGREGATE.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(aggFile) {  // If file on SD card is opened successfully
    aggFile.println(aggString);  // Print dataString to it
    aggFile.close();  // Close the data file and proceed
    Serial.println();
    Serial.println(aggString);
  }
  else {
    Serial.println("Error opening file while attempting to write to SD.");  // If data file can't be opened, produce an error message
  }
}
