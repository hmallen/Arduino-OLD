/*

 WARNING: DO NOT PULL LCD SHIELD PIN 10 HIGH WITHOUT MODIFYING (NPN TRANSISTOR/RESISTOR/MOSFET). NORMALLY FOR 
 BACKLIGHT CONTROL, BUT FACTORY DEFECT CAUSES 100mA OUTPUT.
 
 Prototype v2.0 ALPHA
 
 Board: Arduino Mega 2560 R3+
 - This program is specific to the Arduino Mega. 8K SRAM is sufficient to accomodate the serial prints in the 
 program, so no flash assist macros [F()] are included in the sketch. Given the 3 available hardware serials on the 
 Mega, the GPRS jumpers are set for hardware serial transmission, and the connection is initiated in the sketch 
 through "Serial1" instead of SoftwareSerial. Some pin reassignments, through the use of M-F jumpers, were required 
 for SD shield compatibility, as the SPI interface on the Mega is through pins 50-53 as opposed to 10-13 with the 
 Uno.
 
 Shields:
 - SeeedStudio SD Card
 - SeedStudio GPRS v1.0
 
 Sensors:
 - (A0) 10kOhm potentiometer to simulate h2o level
 - (A1) Magnetic reed switch mounted in hinged box to simulate stall door
 
 Order of shield assembly:
 Arduino Mega --> SD Shield --> GPRS Shield
 - Arduino Mega --> SD Shield: All pins connected by jumpers
 - SD Shield --> GPRS Shield: Shields stacked normally
 - LCD Shield --> SD Shield: Pins 4-9 and all power pins connected by jumpers
 
 Pin reassignments (all other pins from Mega --> SD connected normally):
 SD Shield  --->  Arduino Mega
 0                19 (RX1)  [Hardware "Serial1"]
 1                18 (TX1)  [Hardware "Serial1"]
 10               53 (SS)   [SPI]
 11               51 (MOSI) [SPI]
 12               50 (MISO) [SPI]
 13               52 (SCK)  [SPI]
 
 H2O level is checked and written to SD card at a regular interval. H2O sensor is mimicked by a 10K potentiometer, 
 and values read are normalized to a 0-100 scale using the map() function. AS SUCH, CALIBRATION MUST BE PERFORMED 
 ON INITIAL SETUP OF THE DEVICE TO DEFINE THE UPPER LIMIT OF THE MAP FUNCTION. This value is defined as the
 analogRead value when h2o is at maximum level. On each SD write, current date and time information is read from
 the GPRS shield via AT commands. Date and time are parsed and reconstructed into US date format. During setup(), 
 date and time is parsed further, printing a long form (ex. 11:45am, January 23, 2013) to serial. A "GPRS boot loop"
 function is also included in setup(), forcing the program to continuously read from GPRS until data is received. 
 Without this function call, no date and time information is available to be written to the SD card on the first
 loop.
 
 In addition to the regular h2o level reads and SD writes, a data aggregation function is also included in this 
 sketch. This function calculates any negative changes in h2o level on each read, and adds them to an integer that 
 stores the total amount of h2o consumed over the course of multiple regular h2o level SD writes. This aggregate
 value is stored in its own TXT file, independent of the regularly scheduled h2o level SD writes.
 
 A key feature added in v1.0 and refined in v2.0 of the prototype sketch is the h2oCheckChange() functionality, 
 which performs a series of checks for changes in h2o level, including low level detection, recheck/confirmation 
 of significant level changes, empty bucket detection, and a program hold to wait for bucket refill.
 
 New features to v2.0 are the event and error functions, which log important events and possible errors both as
 integer codes to the principal data string and as human-readable strings to separate TXT files.
 
 Event codes:
 0 = None
 1 = Stall door opened
 2 = H2O bucket empty
 3 = H2O bucket refilled
 
 Error codes:
 0 = None
 1 = Increase in h2oLevel without stall door opening first
 2 = Increase in h2oLevel while bucket empty and without stall door opening first
 
 Changes from previous version:
 - Concatenate string, SD write preparation, and SD write functions merged into single function
 - Fixed abnormally high aggregate consumption values present when multiple intra-aggregation consumption events occur
 - Modified order/structure to also allow regular SD writes to occur on loops where aggregate data writing occurs
 - Normalized all strings written to SD card by making them all the same length to allow for more efficient indexing
 
 Planned changes for next version:
 - Integrate food level sensor using same features as with h2o level sensor
 - Change void functions to define a data type for more efficient return of values
 - Reevaluate global variable and determine if any localizations are possible to reduce sketch size
 - Add more useful errors and events to respective functions
 -- Determine if merging errorCheck() function elements with corresponding code is better option
 - Add start-up calibration function to define maxH2OVal for proper sensor value mapping to 0-100 scale
 - Fix date/time on initial Arduino power-on
 -- Some data not received due to GPRS power on messages (i.e. +CFUN = 1, etc.)
 - Organize SD data logs (ex. by day/week/month)
 
 */

////  Libraries  ////

#include <SD.h>

////  Pin definitions  ////

// GPRS, SD, and sensors
#define powPin 9  // Pin for software power-up of GPRS shield
#define chipSelect 53  // SPI chip select pin (SS) for SD card (specific to Arduino board)
#define h2oPin A0  // Pin for reading h2o level sensor
#define doorPin 30  // Pin for reading stall door open/closed status
#define maxH2OVal 920  // Maximum possible value of h2o sensor -- used for map() function

////  Timing constants and integers  ////

unsigned long currTimer = 0;  // Reset to millis() for current time on each loop
unsigned long prevTimer = 0;  // Time of last regular SD write
unsigned long lapTimer = 0;  // Time since last data aggregation (currTimer - aggTimer)
unsigned long aggTimer = 0;  // Time of last data aggregation
unsigned long intervalTimer = 0;  // Time since last regular SD write (currTimer - prevTimer)
unsigned long doorTimer = 0;  // Time of last stall door opening
unsigned long doorLastTimer = 0;  // Time since last stall door opening (currTimer - doorTimer)
const unsigned long writeInterval = 10 * 1000;  // Regular SD write interval [1st digit seconds] in milliseconds
const unsigned long aggInterval = 30 * 1000;  // Data aggregation interval [1st digit seconds] in milliseconds
const unsigned long doorRefillInterval = 2 * 60 * 1000;  // Door error timeout [1st digit minutes] in milliseconds

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
String dataString;  // Full data string written to SD card
String aggString;  // String storing aggregated data written to SD card

// H2O sensor
int h2oLevel = 0;  // Stores h2o level from h2oLevelRead() function
int h2oLastLevel = 0;  // Stores h2o level for comparison with levels read in future loops 
const int h2oChangeTrigger = 10;  // % change in h2o level that triggers confirmation read followed by SD write
const int h2oErrorTrigger = -5;  // % change in h2o level without door opening first that triggers error write to SD
int amtCons = 0;  // Stores h2o amount consumed on each read
int amtConsTot = 0;  // Stores h2o amount consumed over time for writing aggregate data to SD
const int fullLevel = 75;  // After bucket empty, program will hold until this level reached again
boolean h2oEmpty = false;  // Will become true when H2O bucket completely empty

// Door sensor
int doorState = 0;  // Will become true when stall door is opened
int openCount = 0;  // Counts each stall door opening and resets when h2o refilled

// SD card
File dataFile;  // Defines file data type for writing to SD card

// Events and errors
int eventCode = 0;  // Integer representing specific event (codes listed above, at bottom of introductory comment)
String eventString;  // Full event string to be written to SD card
int errorCode = 0;  // Integer representing specific error (codes listed above, at bottom of introductory comment)
String errorString;  // Full error string to be written to SD card

/*===========================
 ////  Startup functions  ////
 ===========================*/

// Function to execute software power-on of GPRS shield
void powerUp() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

// Function to loop GPRS read on startup until data received from GPRS (1st loop() receives no data otherwise)
void gprsBootLoop() {
  Serial.print("Initializing GPRS...");
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
  Serial.println("GPRS ready.");
}

void setup() {
  pinMode(powPin, OUTPUT);  // Software power-on pin for GPRS shield
  pinMode(chipSelect, OUTPUT);  // Required for SD lib fxn
  pinMode(doorPin, INPUT);  // Set door pin as input for reading status (open/closed)

  Serial.begin(19200);  // Hardware serial for serial monitor output
  Serial1.begin(19200);  // Hardware serial for GPRS communication

  powerUp();  // Call function to power-up GPRS with software commands
  gprsBootLoop();  // Initiate GPRS read loop until data is received (1st loop() read fails otherwise)

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

  h2oCheckChange();  // Check for change in h2o level, low level, or empty bucket
}

void loop() {
  currTimer = millis();  // Set to current time
  intervalTimer = currTimer - prevTimer;  // Calculate time since last regular SD write
  lapTimer = currTimer - aggTimer;  // Calculate time since last data aggregation

  doorState = digitalRead(doorPin);  // Read state of stall door (open/closed)
  //doorCheck();  // Check door (open/closed) status and relay information, if necessary
  h2oLevelRead();  // Read current h2o level and return value from 0-100
  errorCheck();  // Check for abnormal sensor values associated with a hardware or software problem

  amtCons = h2oLastLevel - h2oLevel;
  amtConsTot += amtCons;

  h2oCheckChange();  // Check for h2o level change, low level, or empty bucket before next loop

  if(h2oLevel != h2oLastLevel) {  // If h2o level has changed since last read
    h2oCheckChange();  // Check again for h2o level change, low level, or empty bucket
  }
  if(intervalTimer >= writeInterval) {  // If aggregation interval not exceed, but regular SD write interval has
    h2oLevelRead();  // Read current h2o level
    dataWriteSD();  // Write date/time and h2oLevel to SD
    delay(500);  // A brief delay is necessary to allow dataWriteSD() to finish
    prevTimer = currTimer;  // Reset time since last regular SD write to 0
    if(h2oLevel <= 10) {  // After other checks, also check if h2o level is < 10% and inform user that refill is required
      Serial.println("Water level low. Refill Bucket.");  
    }  
    Serial.println();
  }
  if(lapTimer >= aggInterval) {  // If aggregation timer has exceeded aggregation interval
    Serial.println("Aggregate data written to SD and cleared.");
    amtConsTot = 0;  // Set amount of h2o consumed since last data aggregation back to 0
    aggTimer = currTimer;  // Reset time since last data aggregation to 0
    Serial.println();
  }
  delay(1000);  // Wait before looping again
}

/*=======================
 ////  Date and time  ////
 =======================*/

// Read date/time info from GPRS shield and parse into year, month, and day for reformatting to US date format
void getDateTime() {
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

/*==================================================
 ////  Sensor reads, calculations, and reponses  ////
 ==================================================*/

// Read h2o level from sensor (currently simulated by 10kOhm potentiometer)
void h2oLevelRead() {
  int h2oReadVal = analogRead(h2oPin);  // Read value from sensor
  h2oLevel = map(h2oReadVal, 0, maxH2OVal, 0, 100);  // Set h2oLevel to 0-100 scale by mapping analogRead value
}

void doorCheck() {
  if(doorState == 1) {
    openCount++;
    eventCode = 1;
    eventWriteSD();
    delay(500);  // A brief delay is necessary to allow eventWriteSD() to finish
    if(doorTimer == 0) {
      doorTimer = currTimer;
    }
    else {
      doorLastTimer = currTimer - doorTimer;
    }
    if(doorLastTimer > doorRefillInterval) {
      doorTimer = currTimer;
    }
  }
}

// Check h2o level for significant changes, low level, and empty bucket
void h2oCheckChange() {
  if(h2oEmpty == false) {  // If bucket isn't empty according to boolean value currently stored,**
    if(h2oLevel == 0) {  // **but h2o level has just been detected at 0
      Serial.println("Bucket empty or currently being refilled.");
      h2oEmpty = true;  // Change boolean to reflect presence of an empty bucket
    }
    else {  // **and bucket is actually not empty
      if(amtCons >= h2oChangeTrigger) {  // If h2o level has changed significantly since last read
        Serial.print(">= 10% change detected. Confirming...");
        delay(5000);  // Pause briefly to let conditions settle in case bucket was bumped, etc.
        h2oLevelRead();  // Read h2o level again
        if(amtCons >= h2oChangeTrigger) {  // If h2o change detected was real
          Serial.println("change confirmed and logged.");
          dataWriteSD();  // Write new h2o level to SD
          delay(500);  // A brief delay is necessary to allow dataWriteSD() to finish
          prevTimer = currTimer;  // Set regular SD write timer to 0
        }
        else {  // If, after 2nd read, there wasn't actually a change
          Serial.println("detected change was a false-positive.");  // Tell user that the detected change was a false-positive
        }
        Serial.println();
      }
    }
  }
  else {  // If bucket is empty according boolean value currently stored
    waitH2ORefill();  // Call function that checks h2o level regularly and waits for refill to proceed
  }
  h2oLastLevel = h2oLevel; 
}

void waitH2ORefill() {
  // NOTE: MAY BREAK CALLED FUNCTION DOWN INTO "IF/ELSE" BRANCH STRUCTURE TO PREVENT HANGING ON PARTIAL REFILL
  while(h2oEmpty == true) {  // As long as boolean continues to indicate that bucket is empty
    h2oLevelRead();  // Read current h2o level
    if(h2oLevel >= fullLevel) {  // If h2o level is now above the threshold to be considered "refilled" or "full"
      h2oEmpty = false;  // Change boolean to indicate that bucket is now full
      openCount = 0;  // Resets stall door opening counter
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

/*==============================
 ////   SD and data logging  ////
 ==============================*/

void dataWriteSD() {
  getDateTime();  // Read and parse date/time information from GPRS

  dataString = dateString;
  dataString += ",";
  dataString += timeString;
  dataString += ",";
  dataString += h2oLevel;
  dataString += ",";
  dataString += doorState;
  dataString += ",";
  dataString += amtConsTot;
  dataString += ",";
  dataString += eventCode;
  dataString += ",";
  dataString += errorCode;
  delay(200);

  dataFile = SD.open("H2OLOG.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(dataString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(dataString);
  }
  else {
    Serial.println("Error opening file for regular data write.");
  }
}

void eventWriteSD() {
  defineEvent();  // Translate integer into human-readable error string
  getDateTime();  // Read and parse date/time information from GPRS

  eventString = dateString;
  eventString += ",";
  eventString += timeString;
  eventString += " - ";
  eventString += eventString;


  dataFile = SD.open("EVENTS.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(eventString);  // Print errorString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(eventString);
  }
  else {
    Serial.println("Error opening file for writing event information.");
  }
  eventCode = 0;  // Reset error code after writing to SD
}

void errorWriteSD() {
  defineError();  // Translate integer into human-readable error string
  getDateTime();  // Read and parse date/time information from GPRS

  errorString = dateString;
  errorString += ", ";
  errorString += timeString;
  errorString += " - ";
  errorString += errorString;

  dataFile = SD.open("ERRORS.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
  if(dataFile) {  // If file on SD card is opened successfully
    dataFile.println(errorString);  // Print errorString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(errorString);
  }
  else {
    Serial.println("Error opening file for writing error information.");
  }
  errorCode = 0;  // Reset error code after writing to SD
}

/*===========================
 ////  Errors and events  ////
 ===========================*/

// Delared as independent function for future use when additional error checks may be required
void errorCheck() {
  if(openCount == 0 && amtCons < h2oErrorTrigger) {
    if(h2oEmpty == true) {
      errorCode = 2;
    }
    else if(h2oEmpty == false) {
      errorCode = 1;
    }
  }
  else {
    errorCode = 0;
  }
  if(errorCode > 0) {
    errorWriteSD();
    delay(500);  // A brief delay is necessary to allow errorWriteSD() to finish
  }
}

void defineEvent() {
  eventString = "Event: ";
  switch(eventCode) {  // Translate event code integer into human-readable string
  case 0:
    break;
  case 1:
    eventString += "Stall door opened.";
    break;
  case 2:
    eventString += "H2O bucket empty.";
    break;
  case 3:
    eventString += "H2O bucket refilled.";
    break;
  }
}

void defineError() {
  errorString = "Error: ";
  switch(errorCode) {  // Translate error code integer into human-readable string
  case 0:
    break;
  case 1:
    errorString += "H2O level increase detected while stall door closed.";
    break;
  case 2:
    errorStringString += "Bucket empty and H2O level increase detected while stall door closed.";
    break;
  }
}

