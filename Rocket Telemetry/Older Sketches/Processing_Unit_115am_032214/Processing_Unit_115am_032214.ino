/*
MOVE GPRSSERIAL.BEGIN TO AFTER LAND TRIGGERING
 */

#include <SdFat.h>
#include <SoftwareSerial.h>

#define powPin 9
#define chipSelect 10

#define recPin A0
#define recLED 2
#define landPin A1
#define landLED 3
#define debugPin A2

unsigned long targetNumb = 2145635266;

SdFat sd;
SdFile dataFile;

SoftwareSerial sensSerial(5, 6);
SoftwareSerial gprsSerial(7, 8);

String dateString;
String timeString;

String gpsCoord;

boolean recState = false;
boolean transmitState = false;

void powerUp() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

void gprsBootLoop() {
  Serial.print(F("Initializing GPRS..."));
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
  Serial.println(F("GPRS ready."));
}

void setup() {
  Serial.begin(19200);
  sensSerial.begin(115200);
  gprsSerial.begin(19200);

  powerUp();

  pinMode(chipSelect, OUTPUT);
  pinMode(recPin, INPUT);
  pinMode(recLED, OUTPUT);
  pinMode(landPin, INPUT);
  pinMode(landLED, OUTPUT);

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();
}

void loop() {
  transmitCheck();
  recCheck();
  if(recState == true && transmitState == false) {
    if(dataFile.open("SENSORS2.TXT", O_RDWR | O_CREAT | O_AT_END)) {
      while(sensSerial.available()) {
        dataFile.write(sensSerial.read());
        Serial.write(sensSerial.read());
        if(transmitCheck() == true) break;
      }
      dataFile.close();
    }
    else {
      sd.errorHalt("Failed to open file.");
    }
  }
  if(recState == true && transmitState == true) {
    while(transmitState == true) {
      while(sensSerial.available() > 0) {
        char c = sensSerial.read();
        gpsCoord += c;
      }
      Serial.println(gpsCoord);
    }
  }
}

boolean transmitCheck() {
  if(digitalRead(landPin) == HIGH) {
    transmitState = true;
  }
  else {
    return true;
  }
}

boolean recCheck() {
  if(digitalRead(recPin) == 0) {
    recState = !recState;
    while(digitalRead(recPin) == 0) {
      delay(100);
    }
    if(recState == true) {
      digitalWrite(recLED, HIGH);
    }
    else {
      digitalWrite(recLED, LOW);
    }
  }
  return recState;
}

void smsLocation() {
  gprsSerial.println(F("AT+CMGF=1"));
  delay(100);
  gprsSerial.print(F("AT+CMGS=\"+1"));
  gprsSerial.print(targetNumb);
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print(F("https://maps.google.com/maps?q="));
  gprsSerial.print(F("(Landing+location)@"));
  gprsSerial.print(gpsCoord);
  gprsSerial.println(F("&t=h&z=18&output=embed"));
  delay(100);
  gprsSerial.println((char)26);
  delay(100);
  gprsSerial.println();

  Serial.println(F("SMS sent."));
  Serial.println();
}

void getDateTime() {
  String rawDateTime = "";  // Clear rawDateTime string for use on next read
  dateString = "";
  timeString = "";
  String year = "";  // Clear year string before reading from GPRS
  String month = "";  // Clear month string before reading from GPRS
  String day = "";  // Clear day string before reading from GPRS

  gprsSerial.println(F("AT+CCLK?"));  // Read date/time from GPRS
  if(gprsSerial.available()) {  // If data is coming from GPRS
    while(gprsSerial.available()) {  // Read the data into string from incoming bytes while they're available
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
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
}
