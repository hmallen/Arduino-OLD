/*
RFID Train Monitoring with GSM Connectivity:
 - Read RFID tag of passing train
 - Determine direction of travel
 - Package into string for transfer
 - Send data to server via GSM network
 
 Considerations:
 - Include in transmitted string:
 -> Station identifier
 -> Time-stamp
 -> Pass count
 -> RFID tag
 
 To Do:
 - Log all train passes to SD card with time-stamp
 -> If GSM network unavailable immediately after train pass,
 wait for connectivity and send SD archived data when network
 connectivity reestablished
 -> Clean SD after defined time or SD storage limit??
 - Add LCD display
 - Convert "String" types to char arrays
 - Find AT command to detect network connectivity to allow date/time read on cold boot
 To Do (Less critical):
 - If GSM network unavailable, save to SD, then retrieve and send when reconnected
 */

#define debugMode
//#define rawDebug
#define smsEnabled

#include <RFID.h>
#include <SdFat.h>
#include <SoftwareSerial.h>
#include <SPI.h>

#define gprsRXPin 7  // Receive pin for GPRS software serial
#define gprsTXPin 8  // Transmit pin for GPRS software serial
#define gprsPowPin 9  // Pin for software power-up of GPRS shield

#define sdSlaveSelect 10  // SPI chip select pin (SS) for SD card (specific to Arduino board)

#define rfidSlaveSelect 6
#define rfidPowPin 5

#define pirPin 2

const int piezoPin = 3;  // Pin for piezo buzzer beep on successful RFID read
const int readyLED = A0;  // Pin for LED to indicate entry of main loop
const int rfidLED = 4;  // Pin for LED to signal RFID read

const char smsTargetNum[11] = "2145635266";  // Mobile number to send data via SMS

SdFat sd;
SdFile dataFile;

RFID rfid(rfidSlaveSelect, rfidPowPin);

SoftwareSerial gprsSerial(gprsRXPin, gprsTXPin);

////  Global strings (and associated integers)  ////

// RFID variables
int serNum0;
int serNum1;
int serNum2;
int serNum3;
int serNum4;

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
String hour12;  // Stores hour after conversion from 24hr - - > 12hr time format
String dateLongString;  // Stores full long form of date after parsing from dateString
String time12hrString;  // Stores time info in 12hr time format after parsing from timeString

boolean pirDetect = false;  // Provides indicator of a confirmed PIR sensor read
String directionString;  // Direction of RFID travel as read by PIR sensor
int rfidCount = 0;  // Counts number of RFID reads to trigger SMS at certain interval
const int rfidCountTrigger = 3;  // Number of RFID reads before sending SMS

String dataString;  // Full data string written to SD card

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(readyLED, OUTPUT);
  pinMode(rfidLED, OUTPUT);
  pinMode(piezoPin, OUTPUT);

  digitalWrite(readyLED, LOW);
  digitalWrite(rfidLED, LOW);
  digitalWrite(piezoPin, LOW);

  Serial.begin(19200);
  SPI.begin();
  rfid.init();

  gprsPowerOn();
#ifdef debugMode
  Serial.println(F("GPRS powered on."));
#endif
  gprsSerial.begin(19200);  // Software serial for GPRS communication
  delay(5000);
  configureGPRS();
  gprsBootLoop();
  if(!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
#ifdef debugMode
  Serial.println(F("SD card initialized."));
  Serial.println();
#endif
  getDateTime();  // Read date and time information from GPRS
  dateLongConst();  // Construct string with long form of date
  time12hrConst();  // Construct string with 12hr format of time

  Serial.print(F("It is currently "));
  Serial.print(time12hrString);
  Serial.print(F(", "));
  Serial.print(dateLongString);
  Serial.println(F("."));
  Serial.println();

  Serial.println(F("System ready. Scanning for RFID tag..."));
  Serial.println();
  digitalWrite(readyLED, HIGH);
}

void loop() { 
  unsigned long pirTime;
  unsigned long rfidTime;

  if(digitalRead(pirPin) == 1 && pirDetect == false) {
    pirTime = millis();
    pirDetect = true;
#ifdef rawDebug
    Serial.println(pirTime);
#endif
  }
  if(rfid.isCard()) {
    if(rfid.readCardSerial()) {
      if(rfid.serNum[0] != serNum0
        && rfid.serNum[1] != serNum1
        && rfid.serNum[2] != serNum2
        && rfid.serNum[3] != serNum3
        && rfid.serNum[4] != serNum4) {
        /* With a new cardnumber, show it. */
        serNum0 = rfid.serNum[0];
        serNum1 = rfid.serNum[1];
        serNum2 = rfid.serNum[2];
        serNum3 = rfid.serNum[3];
        serNum4 = rfid.serNum[4];

        rfidTime = millis();
#ifdef rawDebug
        Serial.println(rfidTime);
#endif

        digitalWrite(piezoPin, HIGH);
        digitalWrite(rfidLED, HIGH);
        delay(250);
        digitalWrite(piezoPin, LOW);
        digitalWrite(rfidLED, LOW);

        clearDateTimeStrings();
        getDateTime();
        Serial.print(timeString);
        Serial.print(F(", "));
        Serial.println(dateString);

        Serial.print(F("DEC: "));
        Serial.print(rfid.serNum[0]);
        Serial.print(F(", "));
        Serial.print(rfid.serNum[1]);
        Serial.print(F(", "));
        Serial.print(rfid.serNum[2]);
        Serial.print(F(", "));
        Serial.print(rfid.serNum[3]);
        Serial.print(F(", "));
        Serial.println(rfid.serNum[4]);

        int readDifference = rfidTime - pirTime;
#ifdef rawDebug
        Serial.println(readDifference);
#endif

        if(readDifference > 0) directionString = "Right";
        else directionString = "Left";

        Serial.println();

        buildDataString();
        Serial.println(dataString);

        writeDataSD();        

        rfidCount++;
#ifdef debugMode
        int countToSMS = 4 - rfidCount;
        Serial.print(F("Reads until SMS: "));
        Serial.println(countToSMS);
#endif
        Serial.println();
        if(rfidCount > rfidCountTrigger) {
          smsDataString();
          rfidCount = 0;
        }

        pirDetect = false;


      } 
      else {
        /* If we have the same ID, just write a dot. */
        //Serial.print(".");
      }
    }
  }
  rfid.halt();
}

void buildDataString() {
  dataString = "";
  dataString += timeString;
  dataString += ",";
  dataString += dateString;
  for(int z = 0; z < 5; z++) {
    dataString += ",";
    dataString += rfid.serNum[z];
  }
  dataString += ",";
  dataString += directionString;
}

void writeDataSD() {
  // open the file for write at end like the Native SD library
  if (!dataFile.open("datafile.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("Opening datafile.txt for write failed.");
  }
  dataFile.println(dataString);
  dataFile.close();
  Serial.println(F("Data written to SD card."));
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
#ifdef debugMode
  Serial.print(F("Initializing GPRS..."));
#endif
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
#ifdef debugMode
  Serial.println(F("GPRS ready."));
#endif
}

#ifdef smsEnabled
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

// Construct long form of date read from GPRS
void dateLongConst() {
  String yearLong = "";  // Clear long - formatted year string before reading from GPRS
  String monthLong = "";  // Clear long - formatted month string before reading from GPRS
  String dayLong = "";  // Clear long - formatted day string before reading from GPRS

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
    time12hrString += "pm";  // Add "pm", since hour was after 12 ( >  =  13)
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

void clearDateTimeStrings() {
  rawDateTime = "";
  dateString = "";
  timeString = "";
  time12hrString = "";
  dateLongString = "";
}
