/*
Prototype v1.0 ALPHA
 
 Board: Arduino Mega 2560 R3+
 - This program is specific to the Arduino Mega. 8K SRAM is sufficient to accomodate the serial prints in the 
 program, so no flash assist macros [F()] are included in the sketch. Given the 3 available hardware serials on the 
 Mega, the GPRS jumpers are set for hardware serial transmission, and the connection is initiated in the sketch 
 through "Serial1" instead of SoftwareSerial. Some pin reassignments, through the use of M-F jumpers, were required 
 for SD shield compatibility, as the SPI interface on the Mega is through pins 50-53 as opposed to 10-13 with the 
 Uno.
 
 Order of shield assembly:
 Arduino Mega --> SD Shield --> GPRS Shield
 - Arduino Mega --> SD Shield: All pins connected by jumpers
 - SD Shield --> GPRS Shield: Shields stacked normally
 
 Pin reassignments (all other pins from Mega --> SD connected normally):
 SD Shield  --->  Arduino Mega
 0                19 (RX1) [Hardware "Serial1"]
 1                18 (TX1) [Hardware "Serial1"]
 10               53 (SS) [SPI]
 11               51 (MOSI) [SPI]
 12               50 (MISO) [SPI]
 13               52 (SCK) [SPI]
 
 H2O level is checked and written to SD card at a regular interval. H2O sensor is mimicked by a 10K potentiometer, 
 and values read are normalized to a 0-100 scale. On each SD write, current date and time information is read from
 the GPRS shield via AT commands. Date and time are parsed and reconstructed into US date format. During setup(), 
 date and time is parsed further, printing a long form (ex. 11:45am, January 23, 2013) to serial. A "GPRS boot loop"
 function is also included in setup(), forcing the program to continuously read from GPRS until data is received. 
 Without this function call, no date and time information is available to be written to the SD card on the first
 loop.
 
 In addition to the regular h2o level reads and SD writes, a data aggregation function is also included in this 
 sketch. This function calculates any negative changes in h2o level on each read, and adds them to an integer that 
 stores the total amount of h2o consumed over the course of multiple regular h2o level SD writes. This aggregate
 value is stored in its own TXT file, independent of the regularly scheduled h2o level SD writes.
 
 A key feature of this version of the prototype sketch is the h2oCheckChange() functionality, which performs a 
 series of checks for changes in h2o level, including low level detection, recheck/confirmation of significant 
 level changes, empty bucket detection, and a program hold to wait for bucket refill.
 */

////  Libraries  ////

#include <SD.h>

////  Pin definitions  ////

#define powPin 9  // Defines pin for software power-up of GPRS shield
#define chipSelect 53  // Defines SPI chip select pin (SS) for SD card (specific to Arduino board)
#define h2oPin A0  // Defines pin for reading h2o level sensor
#define doorPin A1  // Defines pin for reading stall door open/closed status

////  Timing constants and integers  ////

int currTimer = 0;  // Resets to millis() for current time on each loop
int prevTimer = 0;  // Time of last regular SD write
int lapTimer = 0;  //  Elapsed time since last data aggregation (currTimer - aggTimer)
int aggTimer = 0;  // Time of last data aggregation
int intervalTimer = 0;  // Elapsed time since last regular SD write (currTimer - prevTimer)
int count = 0;
const int writeInterval = 10000;  // Regular SD write interval
const int aggInterval = 30000;  // Data aggregation interval

////  Global strings (and associated integers)  ////

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
String hour12;  // Stores hour after conversion from 24hr --> 12hr time format
String dateLongString;  // Stores full long form of date after parsing from dateString
String time12hrString;  // Stores time info in 12hr time format after parsing from timeString
String fullDataString;  // Full data string written to SD card
String aggString;  // String storing aggregated data written to SD card

// H2O sensor
int h2oLevel = 0;  // Stores h2o level from h2oLevelRead() function
int h2oLastLevel = 0;  // Stores h2o level for comparison with levels read in future loops 
const int h2oChangeTrigger = 10;  // % change in H2O level that triggers confirmation read/SD write
int amtCons = 0;  // Stores h2o amount consumed on each read
int amtConsTot = 0;  // Stores h2o amount consumed over time for writing aggregate data to SD
boolean h2oEmpty = false;  // Will become true when H2O bucket completely empty
const int fullLevel = 75;

// Door sensor
boolean doorOpen = false;

// SD card
File dataFile;

void setup() {
  pinMode(powPin, OUTPUT);  // Software power-on pin for GPRS shield
  pinMode(chipSelect, OUTPUT);  // Required for SD lib fxn

  Serial.begin(19200);  // Hardware serial for serial monitor output
  Serial1.begin(19200);  // Hardware serial for GPRS communication

  powerUp();  // Call function to power-up GPRS with software commands
  gprsBootLoop();  // Initiate GPRS read loop until data is received (1st loop() read fails otherwise)
  clearStrings();  // Clear data from gprsBootLoop

  if (!SD.begin(chipSelect)) {  // Attempt connection to SD shield
    Serial.println("Card failed, or not present.");
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println();

  Serial.println("Welcome to MyStall.");
  Serial.println();

  getDateTime();  // Read date and time information from GPRS
  dateLongConst();  // Construct string with long form of date
  time12hrConst();  // Construct string with 12hr format of time

  Serial.print("It is currently ");
  Serial.print(time12hrString);
  Serial.print(", ");
  Serial.print(dateLongString);
  Serial.println(".");
  Serial.println();

  h2oLevelRead();  // Read current h2o level
  h2oLastLevel = h2oLevel;  // Set h2o level storage variable to current H2O level
  currTimer = millis();  // Set to current time
  prevTimer = millis();  // Set to current time
  aggTimer = millis();  // Set to current time

  clearStrings();  // Clear all strings that will be written to SD card
  h2oCheckChange();  // Check for change in H2O level, low level, or empty bucket
}

void loop() {
  currTimer = millis();  // Set to current time
  intervalTimer = currTimer - prevTimer;  // Calculate time since last regular SD write
  lapTimer = currTimer - aggTimer;  // Calculate time since last data aggregation
  doorOpen = digitalRead(doorPin);  // Read state of stall door (open/closed)
  h2oLevelRead();  // Read current h2o level
  if(h2oLevel != h2oLastLevel) {
    h2oCheckChange();
  }
  if(lapTimer >= aggInterval) {  // If aggregation timer has exceeded aggregation interval
    // NOTE: AN ISSUE CURRENTLY EXISTS WITH ABNORMALLY HIGH AGGREGATE VALUES AFTER MULTIPLE INTER-AGGREGATION H2O CHANGES
    aggregateWriteSD();  // Concatenate date/time and data, then write aggregate total to SD card
    Serial.println("Aggregate data written to SD card.");
    aggTimer = currTimer;  // Reset time since last data aggregation to 0
  }
  else if(intervalTimer >= writeInterval) {  // If aggregation interval not exceed, but regular SD write interval has
    h2oLevelRead();  // Read current h2o level
    h2oWriteSD();  // Write date/time and h2oLevel to SD
    prevTimer = currTimer;  // Reset time since last regular SD write to 0
  }
  else {  // If neither aggregation nor regular SD write interval has been exceeded, do nothing (or print timing info to serial)
    //int nextWrite = (writeInterval - intervalTimer) / 1000;
    //Serial.println(nextWrite);
  }
  h2oCheckChange();  // Check for h2o level change, low level, or empty bucket before next loop
  amtCons = 0;  // Clear amount consumed for availability on next loop
  delay(1000);  // Wait before commencing next loop
}

// Execute software power-on of GPRS shield
void powerUp() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

// Loop GPRS read on startup until data received from GPRS. 1st loop() receives no data otherwise.
void gprsBootLoop() {
  Serial.print("Initializing GPRS...");
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
  Serial.println("GPRS ready.");
}

// Clear main data strings that are written to SD regularly
void clearStrings() {
  dateString = "";
  timeString = "";
  fullDataString = "";
}

// Concatenate main data elements into single string for writing to SD
void concatStrings() {
  fullDataString += dateString;
  fullDataString += ",";
  fullDataString += timeString;
  fullDataString += ",";
  fullDataString += h2oLevel;
  fullDataString += ",";
  fullDataString += doorOpen;
}

// Concatenate elements into single string for writing aggregate data to SD
void concatAggStrings() {
  aggString += dateString;
  aggString += ",";
  aggString += timeString;
  aggString += ",";
  aggString += amtConsTot;
}

// Read date/time info from GPRS shield and parse into year, month, and day for reformatting to US date format
void getDateTime() {
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
}

// Changes time read from GPRS into 12hr format
void time12hrConst() {
  hour = "";  // Clear hour string before parsing data into it from timeString
  minute = "";  // Clear minute string before parsing data into it from timeString
  second = "";  // Clear second string before parsing data into it from timeString
  hour12 = "";  // Clear 12hr formatted hour string before parsing data into it from timeString

  for(int h = 0; h < 2; h++) {  // Parse out hour characters from timeString
    hour += String(timeString.charAt(h));
  }
  for(int mi = 3; mi < 5; mi++) {  // Parse out hour characters from timeString
    minute += String(timeString.charAt(mi));
  }
  for(int s = 6; s < 8; s++) {  // Parse out hour characters from timeString
    second += String(timeString.charAt(s));
  }

  if(hour.toInt() > 12) {  // If integer format of 24hr formatted hour is more than 12
    hour12 += String(hour.toInt() - 12);  // Subtract 12 from hour and add to storage string
    time12hrString += String(hour12);  // Add new 12hr formatted hour to 12hr time string
    time12hrString += ":";  // Add colon to create properly formatted time
    time12hrString += String(minute);  // Add minute string after the colon
    time12hrString += "pm";  // Add "pm", since hour was after 12 (>= 13)
  }
  else if (hour.toInt() == 0) {  // If integer format of 24hr formatted hour is not more than 12, but is equal to 0
    hour12 += "12";  // Set hour storage string to 12 (midnight)
    time12hrString += String(hour12);  // Add new 12hr formatted hour to 12hr time string
    time12hrString += ":";  // Add colon to create properly formatted time
    time12hrString += String(minute);  // Add minute string after the colon
    time12hrString += "am";  // Add "am", since time detected was midnight
  }
  else {  // If integer format of 24hr formatted hour is not more than 12 or equal to 0
    hour12 += String(hour.toInt());  // Set hour storage variable to integer form of hour
    time12hrString += hour12;  // Add new 12hr formatted hour to 12hr time string
    time12hrString += ":";  // Add colon to create properly formatted time
    time12hrString += minute;  // Add minute string after the colon
    time12hrString += "am";  // Add "am", since time detected was before noon
  }
}

// Construct long form of date read from GPRS
void dateLongConst() {
  String yearLong = "";  // Clear long-formatted year string before reading from GPRS
  String monthLong = "";  // Clear long-formatted month string before reading from GPRS
  String dayLong = "";  // Clear long-formatted day string before reading from GPRS
  dateLongString = "";  // Clear full long date string before reading from GPRS

  yearLong += "20";  // Lead year with "20" to create long form (ex. 2014)
  yearLong += String(year);  // Store long year in a string

  dayLong += String(day.toInt());  // Convert day to integer to remove leading 0's and store in string

  switch(month.toInt()) {  // Convert to integer and use switch case to convert month number to long form
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

// Read h2o level from sensor (currently mimicked by potentiometer)
void h2oLevelRead() {
  int h2oReadVal = analogRead(h2oPin);  // Read value from sensor
  h2oLevel = h2oReadVal / 9.21;  // Divide by (Maximum-Possible-Value / 100) to make value on 0-100 scale
}

// Check h2o level for significant changes, low level, and empty bucket
void h2oCheckChange() {
  amtCons = h2oLastLevel - h2oLevel;
  amtConsTot += amtCons;
  amtCons = 0;  // Clear amount consumed for availability on next loop
  if(h2oEmpty == false) {  // If bucket isn't empty according to boolean value currently stored,
    if(h2oLevel == 0) {  // but h2o level has just been detected at 0
      Serial.println("Bucket empty or currently being refilled.");
      h2oEmpty = true;  // Change boolean to reflect presence of an empty bucket
    }
    else {  // and bucket is actually empty
      if(abs(h2oLevel - h2oLastLevel) >= h2oChangeTrigger) {  // If h2o level has changed significantly since last read
        Serial.print(">= 10% change detected. Confirming...");
        delay(5000);  // Pause briefly to let conditions settle in case bucket was bumped, etc.
        h2oLevelRead();  // Read h2o level again
        if(abs(h2oLevel - h2oLastLevel) >= h2oChangeTrigger) {  // If h2o change detected was real
          Serial.println("change confirmed.");
          h2oWriteSD();  // Write new h2o level to SD
          prevTimer = currTimer;  // Set regular SD write timer to 0
        }
        else {  // If, after 2nd read, there wasn't actually a change
          Serial.println("detected change was a false-positive.");  // Tell user that the detection was a false-positive
          Serial.println();
        }
      }
    }
    if(h2oLevel <= 10) {  // After other checks, also check if h2o level is < 10% and inform user that refill is required
      Serial.println(h2oLevel);
      Serial.println("Water level low. Refill Bucket.");
    }
  }
  else {  // If bucket is empty according boolean value currently stored
    // NOTE: MAY BREAK WHILE LOOP DOWN INTO BRANCH STRUCTURE W/ IF'S TO PREVENT HANGING ON PARTIAL REFILL
    while(h2oEmpty == true) {  // As long as boolean continues to indicate that bucket is empty
      h2oLevelRead();  // Read current h2o level
      if(h2oLevel >= fullLevel) {  // If h2o level is now above the threshold to be considered "refilled" or "full"
        h2oEmpty = false;  // Change boolean to indicate that bucket is now full
      }
      else {  // If h2o level still below "full" level
        Serial.println("Waiting for bucket to be refilled.");  // Present some messages to the user
        Serial.print("Current level: ");
        Serial.println(h2oLevel);  // Current h2o level
        Serial.print("Full level:    ");
        Serial.println(fullLevel);  // Value that h2o level needs to be raised to before bucket considered full
        Serial.println();
      }
      delay(10000);  // Wait for a few seconds to allow time for bucket refill
    }
  }
  h2oLastLevel = h2oLevel; 
}

void h2oWriteSD() {
  getDateTime();  // Read and parse date/time information from GPRS
  concatStrings();  // Concatenate all relevant h2o level strings for writing to SD card
  sdWriteData();  // Write parsed and concatenated h2o level data to SD card
  Serial.println();
  clearStrings();  // Clear all strings for next SD data write
}

void sdWriteData() {
  dataFile = SD.open("LOGGING.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(fullDataString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println();
    Serial.println(fullDataString);
  }
  else {
    Serial.println("Error opening file while attempting to write to SD.");
  }
}

void aggregateWriteSD() {
  Serial.print("Units consumed since last data aggregation: ");
  Serial.println(amtConsTot);
  getDateTime();  // Read and parse date/time information from GPRS
  concatAggStrings();  // Concatenate all relevant aggregate data into string for writing to SD card
  sdWriteAggregate();  // Write aggregate data to SD
  clearStrings();  // Clear all strings for next SD write
  aggString = "";  // Clear aggregate data string for next period of data aggregation
  amtConsTot = 0;  // Set amount of h2o consumed since last data aggregation back to 0
}

void sdWriteAggregate() {
  // NOTE: ATTEMPTING TO WRITE TO "AGGREGATE.TXT" FAILS...POSSIBLE ISSUE WITH FILE NAME LENGTH
  dataFile = SD.open("AGG.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(aggString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(aggString);
  }
  else {
    Serial.println("Error opening file while attempting to write to SD.");
  }
}