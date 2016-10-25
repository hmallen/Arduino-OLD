/*
MOVE GPRSSERIAL.BEGIN TO AFTER LAND TRIGGERING
 */

#define debugMode  // Comment out this line to write to SD via SPI instead of serial monitor
#define verboseSerial  // Comment out this line to eliminate extra, non-essential serial prints

#include <SdFat.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define powPin 9
#define chipSelect 10


#define recPin A0
#define landPin A1
#define altReadyPin A2
#define recLED 2
#define landLED 3

unsigned long targetNumb = 2145635266;

SdFat sd;
SdFile dataFile;

SoftwareSerial gpsSerial(4, 255);
SoftwareSerial sensSerial(5, 6);
SoftwareSerial gprsSerial(7, 8);

//String dateString;
//String timeString;

float flat, flon;
String gpsLat;
String gpsLon;
String gpsLatInitial;
String gpsLonInitial;
String gpsLatFinal;
String gpsLonFinal;

boolean recState = false;
boolean transmitState = false;

void powerUpGPRS() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

/*void gprsBootLoop() {
 Serial.print(F("Initializing GPRS..."));
 while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
 getDateTime();
 delay(1000);
 }
 Serial.println(F("GPRS ready."));
 }*/

void setup() {
  Serial.begin(19200);
  sensSerial.begin(19200);

  pinMode(powPin, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  pinMode(altReadyPin, INPUT);
  pinMode(recPin, INPUT);
  pinMode(recLED, OUTPUT);
  pinMode(landPin, INPUT);
  pinMode(landLED, OUTPUT);

  powerUpGPRS();

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();

  if(digitalRead(altReadyPin) == LOW) {
#ifdef verboseSerial
    Serial.println(F("Altimeter calibration in progress. Please wait..."));
    Serial.println();
#endif
    while(digitalRead(altReadyPin) == LOW) {
      delay(1);
    }
    delay(500);
  }
  Serial.println(F("Obtaining launch site coordinates from GPS."));
  getLocation();
  gpsLatInitial = gpsLat;
  gpsLonInitial = gpsLon;
#ifdef verboseSerial
  Serial.print(F("Launch site: "));
  Serial.print(gpsLatInitial);
  Serial.print(F(", "));
  Serial.println(gpsLonInitial);
  Serial.println();
  Serial.println(F("Press button to begin data acquisition..."));
  Serial.println();
#endif
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
    if(dataFile.open("REALTEST.TXT", O_RDWR | O_CREAT | O_AT_END)) {
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
    getLocation();
    gpsLatFinal = gpsLat;
    gpsLonFinal = gpsLon;
    gprsSerial.begin(19200);
    delay(1000);
#ifdef verboseSerial
    Serial.println(F("GPRS connection initiated."));
    delay(500);
#endif
    printLocation();
    //smsLocation();

    endProgram();
  }
  delay(100);
}

void getLocation() {
  if(!sensSerial.available()) {
    while(!sensSerial.available()) {
      delay(100);
    }
  }
  if(sensSerial.available()) {
    while(sensSerial.available()) {
      for(int coord = 0; coord < 13; coord++) {
        char c = sensSerial.read();
        gpsLat += c;
        delay(1);
      }
      for(int coord = 15; coord < 27; coord++) {
        char c = sensSerial.read();
        gpsLon += c;
        delay(1);
      }
    }
  }
}

boolean transmitCheck() {
  if(digitalRead(landPin) == HIGH) {
    transmitState = true;
  }
  return transmitState;
}

boolean recCheck() {
  if(digitalRead(recPin) == 0 && digitalRead(altReadyPin) == HIGH) {
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
  delay(100);
  gprsSerial.print(targetNumb);
  delay(100);
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print(F("https://maps.google.com/maps?q="));
  delay(100);
  gprsSerial.print(F("Landing+Site@"));
  delay(100);
  gprsSerial.print(gpsLatFinal);
  delay(100);
  gprsSerial.print(F(",+"));
  delay(100);
  gprsSerial.print(gpsLonFinal);
  delay(100);
  gprsSerial.println(F("&t=h&z=18"));
  delay(100);
  gprsSerial.println((char)26);
  delay(1000);

#ifdef verboseSerial
  Serial.println(F("Coordinates sent via SMS."));
  delay(1000);
#endif
#ifdef debugMode
  Serial.print(gpsLatFinal);
  Serial.print(F(",+"));
  Serial.println(gpsLonFinal);
#endif
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
  gprsSerial.print(gpsLatFinal);
  delay(100);
  gprsSerial.print(F(",+"));
  delay(100);
  gprsSerial.print(gpsLonFinal);
  delay(100);
  Serial.println(F("&t=h&z=18&output=embed"));
  delay(100);
  Serial.println((char)26);
  delay(1000);

#ifdef verboseSerial
  Serial.println(F("Coordinates sent via SMS."));
  delay(1000);
#endif
#ifdef debugMode
  Serial.print(gpsLatFinal);
  Serial.print(F(",+"));
  Serial.println(gpsLonFinal);
#endif
}

/*void getDateTime() {
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
 }*/

void endProgram() {
  boolean endTrigger = true;
  while(endTrigger == true) {
    digitalWrite(landLED, LOW);
    delay(500);
    digitalWrite(landLED, HIGH);
    delay(50);
  }
}