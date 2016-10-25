#include <SPI.h>
#include <SdFat.h>

const char GPSDATASELECT = 'G';  // Choose data for parsing ('G' = GPGGA / 'R' = GPRMC)

char GPSDATANULL;

SdFat SD;
SdFile logFile;

const int chipSelect = 10;

String serialString;

void setup() {
  Serial.begin(57600);

  if (GPSDATASELECT == 'G') GPSDATANULL = 'R';
  else if (GPSDATASELECT == 'R') GPSDATANULL = 'G';

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(chipSelect, SPI_FULL_SPEED)) {
    SD.initErrorHalt();
  }
  Serial.println(F("initialization done."));

  Serial.println(F("Waiting for start command."));
}

void loop() {
  if (!Serial.available()) {
    while (!Serial.available()) {
      delay(10);
    }
  }
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'p') parseLog();
    else {
      Serial.println(F("Invalid character received. Waiting for valid start command."));
      return;
    }
  }
  Serial.println(F("Log parsing complete."));
  while (true) {
    delay(1000);
  }
}

/*void parseLog(File logFile) {
  if (logFile) {
    unsigned long startTime = millis();
    while (logFile.available()) {
      Serial.write(logFile.read());
    }
    logFile.close();
    unsigned long totalDurationLong = millis() - startTime;
    int totalDurationSec = totalDurationLong / 1000;
    int totalDurationMin = totalDurationSec / 60;
    Serial.println();
    Serial.print(F("Parse duration: "));
    Serial.print(totalDurationSec);
    Serial.print(F(" seconds / "));
    Serial.print(totalDurationMin);
    Serial.println(F(" minutes"));
  }
  else Serial.println(F("Error opening log file for parsing."));
}*/

void parseLog() {
  boolean skipRead = false;
  unsigned long startTime = millis();
  char c;
  if (!logFile.open("COMPILED.txt", O_READ)) SD.errorHalt("Opening of COMPILED.txt for parsing failed.");
  while (logFile.available()) {
    if (skipRead == false) c = logFile.read();
    else skipRead = false;
    if (c == '$') {
      for (int x = 0; x < 3; x++) {
        c = logFile.read();
      }
      if (c == GPSDATASELECT) {
        for (int x = 0; x < 3; x++) {
          c = logFile.read();
        }
        while (true) {
          c = logFile.read();
          if (c == '$') {
            skipRead = true;
            Serial.println(serialString);
            serialString = "";
            break;
          }
          else serialString += c;
          delay(1);
        }
      }
      else if (c == GPSDATANULL) {
        while (true) {
          c = logFile.read();
          if (c == '$') {
            skipRead = true;
            break;
          }
        }
      }
      else {
        Serial.println(F("Unrecognized header. Please reset to attempt parsing again."));
        while (true) {
          delay(1000);
        }
      }
    }
    logFile.close();
    unsigned long totalDurationLong = millis() - startTime;
    int totalDurationSec = totalDurationLong / 1000;
    int totalDurationMin = totalDurationSec / 60;
    Serial.println();
    Serial.print(F("Parse duration: "));
    Serial.print(totalDurationSec);
    Serial.print(F(" seconds / "));
    Serial.print(totalDurationMin);
    Serial.println(F(" minutes"));
  }
}
