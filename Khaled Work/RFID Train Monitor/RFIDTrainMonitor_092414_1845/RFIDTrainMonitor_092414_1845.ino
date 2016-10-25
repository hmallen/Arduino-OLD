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
 - For debugging purposes:
 -> LED to indicate ready status
 -> Beep (piezo buzzer) to indicate RFID read
 
 To Do:
 - Log all train passes to SD card with time-stamp
 -> If GSM network unavailable immediately after train pass,
 wait for connectivity and send SD archived data when network
 connectivity reestablished
 -> Clean SD after defined time or SD storage limit??
 */

#define debugMode
//#define smsEnabled

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

const int statusLED = 2;  // Pin for LED to indicate ready status

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
String dataString;  // Full data string written to SD card

void setup() {
  pinMode(rfidSlaveSelect, OUTPUT);
  pinMode(sdSlaveSelect, OUTPUT);

  Serial.begin(19200);
  SPI.begin();
  gprsSerial.begin(19200);  // Software serial for GPRS communication
  rfid.init();
  gprsPowerOn();
#ifdef debugMode
  Serial.println(F("GPRS powered on."));
#endif
  if(!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
#ifdef debugMode
  Serial.println(F("SD card initialized."));
  Serial.println();
#endif
  /*while(rawDateTime == "") {
    getDateTime();
    delay(1000);
  }*/
  //delay(1000);
  //getDateTime();  // Read date and time information from GPRS
  //Serial.println(rawDateTime);
  /*dateLongConst();  // Construct string with long form of date
   time12hrConst();  // Construct string with 12hr format of time
   
   Serial.print(F("It is currently "));
   Serial.print(time12hrString);
   Serial.print(", ");
   Serial.print(dateLongString);
   Serial.println(".");
   Serial.println();*/

#ifdef debugMode
  Serial.println(F("End of setup."));
#endif
}

void loop() {
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      if (rfid.serNum[0] != serNum0
        && rfid.serNum[1] != serNum1
        && rfid.serNum[2] != serNum2
        && rfid.serNum[3] != serNum3
        && rfid.serNum[4] != serNum4
        ) {
        /* With a new cardnumber, show it. */
        Serial.println(" ");
        Serial.println("Card found");
        serNum0 = rfid.serNum[0];
        serNum1 = rfid.serNum[1];
        serNum2 = rfid.serNum[2];
        serNum3 = rfid.serNum[3];
        serNum4 = rfid.serNum[4];

        //Serial.println(" ");
        Serial.println("Cardnumber:");
        Serial.print("Dec: ");
        Serial.print(rfid.serNum[0],DEC);
        Serial.print(", ");
        Serial.print(rfid.serNum[1],DEC);
        Serial.print(", ");
        Serial.print(rfid.serNum[2],DEC);
        Serial.print(", ");
        Serial.print(rfid.serNum[3],DEC);
        Serial.print(", ");
        Serial.print(rfid.serNum[4],DEC);
        Serial.println(" ");

        Serial.print("Hex: ");
        Serial.print(rfid.serNum[0],HEX);
        Serial.print(", ");
        Serial.print(rfid.serNum[1],HEX);
        Serial.print(", ");
        Serial.print(rfid.serNum[2],HEX);
        Serial.print(", ");
        Serial.print(rfid.serNum[3],HEX);
        Serial.print(", ");
        Serial.print(rfid.serNum[4],HEX);
        Serial.println(" ");
      } 
      /*else {
        Serial.print(".");
      }*/
    }
  }
  rfid.halt();
}

void gprsPowerOn() {
  digitalWrite(gprsPowPin, LOW);
  delay(100);
  digitalWrite(gprsPowPin, HIGH);
  delay(500);
  digitalWrite(gprsPowPin, LOW);
  delay(100);
}

#ifdef smsEnabled
void smsLocation() {
  gprsSerial.println(F("AT+CMGF=1"));
  delay(100);
  gprsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  int i;
  for (i = 0; i < 10; i++) {
    gprsSerial.print(smsTargetNum[i]);
    delay(100);
  }
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print(F("DATA GOES HERE"));
  delay(100);
  gprsSerial.println((char)26);
  delay(1000);
  gprsSerial.flush();
#ifdef debugMode
  Serial.println(F("Coordinates sent via SMS."));
  delay(1000);
#endif
}
#endif

/*========================
 ////  Date and time  ////
 =======================*/

// Read date/time info from GPRS shield and parse into year, month, and day for reformatting to US date format
void getDateTime() {
  dateString = "";
  timeString = "";
  year = "";  // Clear year string before reading from GPRS
  month = "";  // Clear month string before reading from GPRS
  day = "";  // Clear day string before reading from GPRS

  gprsSerial.println("AT+CCLK?");  // Read date/time from GPRS
  if(gprsSerial.available()) {  // If data is coming from GPRS
    while(gprsSerial.available()) {  // Read the data into string from incoming bytes while they're available
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
      delay(10);  
    }
    for(int y = 10; y < 112; y++) {  // Parse out year characters from rawDateTime
      year += String(rawDateTime.charAt(y));
    }
    for(int mo = 13; mo < 15; mo++) {  // Parse out month characters from rawDateTime
      month += String(rawDateTime.charAt(mo));
    }
    for(int d = 16; d < 18; d++) {  // Parse out day characters from rawDateTime
      day += String(rawDateTime.charAt(d));
    }
    for(int t = 19; t < 27; t++) {  // Parse out time characters from rawDateTime
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
