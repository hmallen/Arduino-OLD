#define debugMode  // Comment out this line to write to SD via SPI instead of serial monitor
#define verboseSerial  // Comment out this line to eliminate non-essential serial prints
//#define smsEnabled  // Comment out this line to replace sms sending with serial prints

#include <SdFat.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define powPin 9
#define chipSelect 10

#define resetPin A0
#define readyPin A1
#define recPin A2
#define dataPin A3
#define landPin A4
#define recLED 2
#define landLED 3

unsigned long targetNumb = 2145635266;

SdFat sd;
SdFile dataFile;

SoftwareSerial sensSerial(5, 6);
SoftwareSerial gprsSerial(7, 8);

String gpsLat;
String gpsLon;
String gpsLatInitial;
String gpsLonInitial;
String gpsLatFinal;
String gpsLonFinal;
String altCmMax;

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

void setup() {
  pinMode(powPin, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(readyPin, INPUT);
  pinMode(recPin, INPUT);
  pinMode(recLED, OUTPUT);
  pinMode(landPin, INPUT);
  pinMode(landLED, OUTPUT);

  digitalWrite(dataPin, LOW);

  // Pull sensor Arduino reset pin low briefly to reset on processing unit startup
  digitalWrite(resetPin, LOW);
  delay(100);
  digitalWrite(resetPin, HIGH);

  delay(2000);  // Delay to allow sensor unit to pull readyPin LOW

  Serial.begin(38400);
  sensSerial.begin(38400);

  powerUpGPRS();
  Serial.println(F("GPRS powered."));

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();
  Serial.println(F("Calibration and sync in progress..."));
  Serial.println();

  if(digitalRead(readyPin) == LOW) {
#ifdef verboseSerial
    //Serial.print(F("Altimeter calibration & device sync in progress. Please wait..."));
#endif
    while(digitalRead(readyPin) == LOW) {
      delay(1);
    }
  }
  else {
    Serial.println();
    Serial.println(F("Synchronization error. Please perform manual reset."));
    Serial.println();
    resetError();
  }
  getExtraData();
  delay(2000);
  gpsLatInitial = gpsLat;
  gpsLonInitial = gpsLon;
  //Serial.println(F("calibration & sync complete."));
  delay(1000);
#ifdef verboseSerial
  Serial.print(F("Launch site: "));
  Serial.print(gpsLatInitial);
  Serial.print(F(", "));
  Serial.println(gpsLonInitial);
  Serial.println();
#endif
  delay(1000);
#ifdef verboseSerial
  Serial.println(F("Press button to begin data acquisition..."));
  Serial.println();
#endif
}

void loop() {
  recCheck();
  while(recState == true && transmitCheck() == false) {
    if(!sensSerial.available()) {
      while(!sensSerial.available()) {
        delay(1);
      }
    }
#ifdef debugMode
    while(sensSerial.available()) {
      Serial.write(sensSerial.read());
      if(transmitCheck() == true) break;
    }
#endif
#ifndef debugMode
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
  if(recState == true && transmitState == true) {
    if(digitalRead(readyPin) == HIGH) {
      while(digitalRead(readyPin) == HIGH) {
        delay(1);
      }
    }
    getExtraData();
    delay(2000);
    gpsLatFinal = gpsLat;
    gpsLonFinal = gpsLon;
    delay(1000);
    //gprsSerial.begin(19200);
#ifdef debugMode
    printLocation();
    delay(2500);
#endif
#ifdef smsEnabled
    smsLocation();
    delay(2500);
#endif
    endProgram();
  }
  delay(1);
}

void getExtraData() {
  char c;
  String extraDataString = "";
  gpsLat = "";
  gpsLon = "";

  if(!sensSerial.available()) {
    while(!sensSerial.available()) {
      delay(1);
    }
  }
  if(sensSerial.available()) {
    while(sensSerial.available()) {
      c = sensSerial.read();
      if(c == 's') break;
      delay(1);
    }
    extraDataString += c;
    while(sensSerial.available()) {
      c = sensSerial.read();
      if(c == 'e') break;
      extraDataString += c;
      delay(1);
    }
  }
#ifdef debugMode
  Serial.println(extraDataString);
  Serial.println();
#endif
  for(int t = extraDataString.indexOf('s') + 1; t < extraDataString.indexOf('t'); t++) {
    gpsLat += String(extraDataString.charAt(t));
    delay(1);
  }
  for(int n = extraDataString.indexOf('t') + 1; n < extraDataString.indexOf('n'); n++) {
    gpsLon += String(extraDataString.charAt(n));
    delay(1);
  }
  if(transmitState == true) {
    altCmMax = "";
    for(int a = extraDataString.indexOf('n') + 1; a < extraDataString.indexOf('a'); a++) {
      altCmMax += String(extraDataString.charAt(a));
      delay(1);
    }
  }
#ifdef debugMode
  Serial.println(gpsLat);
  Serial.println(gpsLon);
  Serial.println(altCmMax);
#endif
}

boolean transmitCheck() {
  if(digitalRead(landPin) == HIGH) {
    transmitState = true;
  }
  return transmitState;
}

boolean recCheck() {
  if(digitalRead(recPin) == LOW && digitalRead(readyPin) == HIGH) {
    recState = !recState;
    while(digitalRead(recPin) == LOW) {
      delay(1);
    }
    if(recState == true) {
      digitalWrite(recLED, HIGH);
      digitalWrite(dataPin, HIGH);
    }
    else {
      digitalWrite(recLED, LOW);
    }
  }
  return recState;
}

#ifdef smsEnabled
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
  gpsSerial.flush();
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
#endif

#ifndef smsEnabled
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
  Serial.print(gpsLatFinal);
  delay(100);
  Serial.print(F(",+"));
  delay(100);
  Serial.print(gpsLonFinal);
  delay(100);
  Serial.println(F("&t=h&z=18&output=embed"));
  delay(100);
  Serial.println((char)26);
  delay(1000);
  Serial.flush();
#ifdef debugMode
  Serial.print(gpsLatFinal);
  Serial.print(F(",+"));
  Serial.println(gpsLonFinal);
  Serial.println(altCmMax);
#endif
}
#endif

void endProgram() {
  boolean endTrigger = true;
  while(endTrigger == true) {
    digitalWrite(landLED, LOW);
    delay(500);
    digitalWrite(landLED, HIGH);
    delay(50);
  }
}

void resetError() {
  boolean resetTrigger = true;
  while(resetTrigger == true) {
    digitalWrite(landLED, LOW);
    delay(100);
    digitalWrite(landLED, HIGH);
    delay(10);
  }
}

