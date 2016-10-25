#include <SdFat.h>
#include <Wire.h>
#include "IntersemaBaro.h"

Intersema::BaroPressure_MS5607B baro(true);

const int chipSelect = 53;

const float tempCorrection = 4.65;

float altTempC;
unsigned long altPressPascal;

SdFat sd;
SdFile myFile;

void setup() { 
  Serial.begin(19200);
  Wire.begin();
  baro.init();

  if(!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  if(!myFile.open("DRFT500.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening DRFT500.txt for write failed");
  }
  myFile.println("$$$$");
  myFile.close();
}

void loop() {
  int loopCount = 1;
  unsigned long timer = millis();
  unsigned long timerStart = timer;
  if(timer >= 600000) {
    Serial.print("Entering SD log loop at ");
    Serial.print(timer);
    Serial.println("ms.");
    while(timer >= 600000 && timer <= 3200000) {
      Serial.println(loopCount);
      timer = millis();
      long alt = baro.altGetAlt();
      altPressPascal = baro.altGetPress();
      altTempC = ((float)baro.altGetTemp() / 100) + tempCorrection;
      //Serial.println("Printing from main loop:");
      //Serial.println(altTempC);
      //Serial.println(altPressMillibar);
      //long altFt = (alt * 100) / 3048;  // Avoids floating point math
      if(!myFile.open("DRFT500.txt", O_RDWR | O_CREAT | O_AT_END)) {
        sd.errorHalt("opening DRFT500.txt for write failed");
      }
      myFile.print("$");
      myFile.print(loopCount);
      myFile.print(",");
      myFile.print(millis());
      myFile.print(",");
      myFile.print(alt);
      myFile.print(",");
      myFile.print(altPressPascal);
      myFile.print(",");
      myFile.println(altTempC);
      myFile.close();
      /*Serial.println(alt);
       Serial.println(altTempC);
       Serial.println(altPressMillibar);*/
      //Serial.println(altFt);
      //Serial.print("Centimeters: ");
      //Serial.print((float)(alt));
      //Serial.print(", Feet: ");
      //Serial.println((float)(alt) / 30.48);
      delay(500);
      loopCount++;
    }
    unsigned long timerStop = millis();
    unsigned long logTimer = timerStop - timerStart;
    int logTotal = logTimer / 60000;
    Serial.print("Exiting SD log loop at ");
    Serial.print(timerStop);
    Serial.println(" milliseconds.");
    Serial.print("Total log time: ");
    Serial.print(logTotal);
    Serial.println(" minutes.");

    delay(1000);
  }
}
