#include <SD.h>

void setup() {
  Serial.begin(19200);
  if(!SD.begin(10, 11, 12, 13))
}

void loop() {
  // put your main code here, to run repeatedly:

}
