#include <SdFat.h>
#include <SoftwareSerial.h>

#define chipSelect 10
#define recPin A0
#define recLED A1
#define smsLED A2

SdFat sd;
SdFile dataFile;

SoftwareSerial sensSerial(5, 6);
//SoftwareSerial gprsSerial(7, 8);

boolean recState = false;

/*void powerUp() {
 digitalWrite(powPin, LOW);
 delay(100);
 digitalWrite(powPin, HIGH);
 delay(500);
 digitalWrite(powPin, LOW);
 delay(100);
 }*/

void setup() {
  Serial.begin(19200);
  sensSerial.begin(19200);

  pinMode(chipSelect, OUTPUT);
  pinMode(recPin, INPUT);
  pinMode(recLED, OUTPUT);
  pinMode(smsLED, OUTPUT);

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();
}

void loop() {
  stateCheck();
  if(recState == true) {
    if(dataFile.open("SENSORS1.TXT", O_RDWR | O_CREAT | O_AT_END)) {
      while(sensSerial.available() > 0) {
        dataFile.write(sensSerial.read());
        if(stateCheck() == false) break;
      }
      dataFile.close();
    }
    else {
      sd.errorHalt("Failed to open file.");
    }
  }
}

boolean stateCheck() {
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
