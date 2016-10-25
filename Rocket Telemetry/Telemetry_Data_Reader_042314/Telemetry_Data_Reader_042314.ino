#include <SdFat.h>

#define chipSelect 10

#define dataPin A0
#define haltPin A1

SdFat sd;

void setup() {
  Serial.begin(38400);
  
  pinMode(dataPin, INPUT);
  
  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  #ifdef debugMode
  Serial.println(F("SD card initialized."));
  Serial.println();
  #endif
}

void loop() {
  if(digitalRead(dataPin) == 1) {
    while(digitalRead(dataPin) == 1) {
      delay(1);
    }
  }
  if(digitalRead(dataPin) == 0) {
    while(digitalRead(dataPin) == 0) {
      delay(1);
    }
  }
  delay(1000);
  readData();
}

void readData() {
  while() {
  if(digitalRead(haltPin) == 0) break;
  }
}
