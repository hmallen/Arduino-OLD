/*
MOVE GPRSSERIAL.BEGIN TO AFTER LAND TRIGGERING
 */

#define debugMode  // Comment out this line to write to SD via SPI instead of serial monitor

#include <SdFat.h>
#include <SoftwareSerial.h>

#define powPin 9
#define chipSelect 10

#define readyPin A2
#define recPin A0
#define recLED 2
#define landPin A1
#define landLED 3

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
  sensSerial.begin(19200);

  pinMode(powPin, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  pinMode(readyPin, INPUT);
  pinMode(recPin, INPUT);
  pinMode(recLED, OUTPUT);
  pinMode(landPin, INPUT);
  pinMode(landLED, OUTPUT);

  powerUp();

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();
}

void loop() {
  recCheck();
  while(recState == true && transmitCheck() == false) {
#ifdef debugMode
    while(sensSerial.available()) {
      Serial.write(sensSerial.read());
      if(transmitCheck() == true) break;
    }
#else
    if(dataFile.open("SENSORS2.TXT", O_RDWR | O_CREAT | O_AT_END)) {
      while(sensSerial.available()) {
        dataFile.write(sensSerial.read());
        if(transmitCheck() == true) break;
      }
      dataFile.close();
    }
    else {
      sd.errorHalt("Failed to open file.");
    }
#endif
  }
  while(recState == true && transmitState == true) {
    if(!sensSerial.available()) {
      while(!sensSerial.available()) {
        delay(1);
      }
    }
    if(sensSerial.available()) {
      while(sensSerial.available()) {
        char c = sensSerial.read();
        gpsCoord += c;
      }
    }
    gprsSerial.begin(19200);
    delay(1000);
    Serial.println(F("GPRS connection initiated."));
#ifdef debugMode
    printLocation();
#else
    smsLocation();
#endif
    endProgram();
  }
  delay(100);
}

boolean transmitCheck() {
  if(digitalRead(landPin) == HIGH) {
    transmitState = true;
  }
  return transmitState;
}

boolean recCheck() {
  if(digitalRead(recPin) == 0 && digitalRead(readyPin) == HIGH) {
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
  else if(digitalRead(readyPin) == LOW) {
    Serial.println(F("Altimeter calibration in progress. Please wait and try again."));
    Serial.println();
  }
  return recState;
}

void smsLocation() {
  gprsSerial.println(F("AT+CMGF=1"));
  delay(100);
  gprsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  gprsSerial.print(targetNumb);
  delay(100);
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print(F("https://maps.google.com/maps?q="));
  delay(100);
  gprsSerial.print(F("(Landing+location)@"));
  delay(100);
  gprsSerial.print(gpsCoord);
  delay(100);
  gprsSerial.println(F("&t=h&z=18&output=embed"));
  delay(100);
  gprsSerial.println((char)26);
  delay(100);
  gprsSerial.println();
  delay(1000);

  Serial.println(F("SMS sent."));
  Serial.println();
}

void printLocation() {
  Serial.println(F("AT+CMGF=1"));
  delay(100);
  Serial.print(F("AT+CMGS=\"+1"));
  delay(100);
  Serial.print(targetNumb);
  delay(100);
  Serial.println("\"");
  delay(100);
  Serial.print(F("https://maps.google.com/maps?q="));
  delay(100);
  Serial.print(F("(Landing+location)@"));
  delay(100);
  Serial.print(gpsCoord);
  delay(100);
  Serial.println(F("&t=h&z=18&output=embed"));
  delay(100);
  Serial.println((char)26);
  delay(100);
  Serial.println();
  delay(1000);

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

void endProgram() {
  boolean endTrigger = true;
  while(endTrigger == true) {
    digitalWrite(landLED, LOW);
    delay(500);
    digitalWrite(landLED, HIGH);
    delay(50);
  }
}
