/*
CFC sensor for HAB Arduino program
- analogRead value from CFC sensor
- Map value from 1-100 (or 1-1000);
--> UNLESS MORE RELEVANT VALUES DESIRED (i.e. "PPM")

Considerations:
- Cannot use Arduino 5V for heater (NOT TRUE...PERFORMED SUCCESSFULLY)
--> Requires ~850mW to power
- Unknown whether it will need to be powered the whole time
--> If not, use Arduino to trip relay for sending power to heater
--> Determine necessary heating time before ?drift? no longer occurs
---> TEST THIS AT LOW TEMPERATURES
*/

#define debugMode

//#include <SdFat.h>

const int gasPin = A12;
//const int sdSlaveSelect = 53;

//SdFat sd;
//SdFile cfcFile;

void setup() {
  Serial.begin(19200);
  
  /*if(!sd.begin(sdSlaveSelect, SPI_FULL_SPEED)) sd.initErrorHalt("Failed to initiate SD card.");
  
  if(!cfcFile.open("CFC.TXT", O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt("Error writing data to log file.");
  cfcFile.println(F("$$$$$$$$$$$$$$$$"));
  cfcFile.close();*/
  
#ifndef debugMode
  //delay(600000);  // Needed for CFC heater to reach optimal temperature
#endif
}

void loop() {
  int gasVal = analogRead(gasPin);
  Serial.print(millis());
  Serial.print(F(","));
  Serial.println(gasVal);
  
  /*if(!cfcFile.open("CFC.TXT", O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt("Error writing data to log file.");
  cfcFile.print(millis());
  cfcFile.print(F(","));
  cfcFile.println(gasVal);
  cfcFile.close();*/
  
  //int gasValMapped = map(gasVal, 0, 1023, 0, 100);  // Map analogRead to value b/w 1-100
  //Serial.println(gasValMapped);
  
  delay(1000);
  
  /*digitalWrite(cfcPowPin, LOW);
  Serial.println(F("CFC heater disabled."));
  delay(5000);
  unsigned long cfcTimer = millis();
  for(int x = 0; x < 60; x++) {
    int gasVal = analogRead(gasPin);
    int cfcElapsed = (millis() - cfcTimer) / 1000;
    Serial.print(cfcElapsed);
    Serial.print(F(" seconds: "));
    Serial.println(gasVal);
    delay(30000);
  }
  boolean finishProgram = true;
  Serial.println(F("Program complete."));
  while(finishProgram == true) {
    delay(1);
  }*/
}
