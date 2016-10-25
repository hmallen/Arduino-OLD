/* 
 *  MS5607 Altimeter/Barometer
 */

#include <Wire.h>
#include "IntersemaBaro_MODIFIED.h"

Intersema::BaroPressure_MS5607B baro(true);

void setup() {
  Serial.begin(57600);
  baro.init();
}

void loop() {
  int altCm = baro.getHeightCentiMeters();
  delay(400);
  unsigned long pressPa = baro.getPressurePascals();
  delay(400);

  Serial.print("Pascals: ");
  Serial.print(pressPa);
  Serial.print(", Centimeters: ");
  Serial.print(altCm);
  Serial.print(", Feet: ");
  Serial.println((float)(altCm) / 30.48);
}
