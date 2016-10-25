// Libraries
#include <SoftwareSerial.h>
#include <SdFat.h>

#define pirPin ?
#define doorPin ?
#define dbPin ?
#define pirLED(orCOUNTER) ?
#define doorLED ?
#define dbLED ?

SdFat sd;
SdFile dataFile;

void setup() {
  Serial.begin(38400);

  pinMode(pirPin, INPUT);
  pinMode(doorPin, INPUT);
  pinMode(dbPin, INPUT);
  pinMode(pirLED(orCOUNTER), OUTPUT);
  pinMode(doorLED, OUTPUT);
  pinMode(dbLED, OUTPUT);

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();
}

void loop() {
  Serial.println(F(Hello world!));
  delay(5000);
}


