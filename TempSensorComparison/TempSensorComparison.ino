/*
Comparison of 3 different temperature sensors:
1) SHT11 (I2C)
2) DHT11 (1-wire)
3) LM35 (Analog)
*/

#include <SHT1x.h>
#include <DHT.h>

#define DHTTYPE DHT11

const int lmPin = A0;
const int dhtPin = 2;
const int sdaPin = A4;
const int sclPin = A5;

DHT dht(dhtPin, DHTTYPE);
SHT1x sht1x(sdaPin, sclPin);

float dhtTempC, dhtTempF, dhtHumidityRH;
float shtTempC, shtTempF, shtHumidityRH;
float lmTempC, lmTempF;

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(lmPin, INPUT);

  Serial.println(F("Line 1 = Temperature (Celsius)          -- DHT11 / SHT11 / LM35"));
  Serial.println(F("Line 2 = Temperature (Fahrenheit)       -- DHT11 / SHT11 / LM35"));
  Serial.println(F("Line 3 = Humidity (Relative Humidity %) -- DHT11 / SHT11"));
}

void loop() {
  // DHT11
  dhtTempC = dht.readTemperature();
  dhtTempF = dht.readTemperature(true);
  dhtHumidityRH = dht.readHumidity();
  /*if (isnan(dhtTempC) || isnan(dhtTempF) || isnan(dhtHumidityRH)) {
    Serial.println(F("Failed to read from DHT11 sensor."));
    return;
  }*/

  // SHT11
  shtTempC = sht1x.readTemperatureC();
  shtTempF = sht1x.readTemperatureF();
  shtHumidityRH = sht1x.readHumidity();

  // LM35
  lmTempC = ((float)analogRead(lmPin) * 5.0) / 10.0;
  lmTempF = (lmTempC * 1.8) + 32.0;

  Serial.print(dhtTempC);
  Serial.print(F(" / "));
  Serial.print(shtTempC);
  Serial.print(F(" / "));
  Serial.println(lmTempC);
  Serial.print(dhtTempF);
  Serial.print(F(" / "));
  Serial.print(shtTempF);
  Serial.print(F(" / "));
  Serial.println(lmTempF);
  Serial.print(dhtHumidityRH);
  Serial.print(F(" / "));
  Serial.println(shtHumidityRH);
  Serial.println();
  delay(2500);
}
