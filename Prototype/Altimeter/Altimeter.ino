#include <Wire.h>
#include "IntersemaBaro.h"

Intersema::BaroPressure_MS5607B baro(true);

void setup() { 
  Serial.begin(19200);
  Wire.begin();
  baro.init();
}

void loop() {
  long alt = baro.getHeightCentiMeters();
  //long altFt = (alt * 100) / 3048;  // Avoids floating point math
  Serial.println(alt);
  //Serial.println(altFt);
  //Serial.print("Centimeters: ");
  //Serial.print((float)(alt));
  //Serial.print(", Feet: ");
  //Serial.println((float)(alt) / 30.48);
  delay(1000);
}
