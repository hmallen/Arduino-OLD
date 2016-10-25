/*
 * ReadSHT1xValues
 *
 * Read temperature and humidity values from an SHT1x-series (SHT10,
 * SHT11, SHT15) sensor.
 *
 * Copyright 2009 Jonathan Oxer <jon@oxer.com.au>
 * www.practicalarduino.com
 */

#include <SHT1x.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BACKLIGHT_PIN 13

LiquidCrystal_I2C lcd(0x38);

// Specify data and clock connections and instantiate SHT1x object
#define sdaPin  20
#define sclPin 21
SHT1x sht1x(sdaPin, sclPin);

void setup() {
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  lcd.begin(16, 2);
  Serial.begin(19200); // Open serial connection to report values to host

  sht1x.readTemperatureC();
  sht1x.readTemperatureF();
  sht1x.readHumidity();

  lcd.home();
  lcd.print("Ready.");

  delay(5000);
}

void loop() {
  lcd.clear();
  lcd.home();
  
  float temp_c;
  float temp_f;
  float humidity;

  // Read values from the sensor
  temp_c = sht1x.readTemperatureC();
  temp_f = sht1x.readTemperatureF();
  humidity = sht1x.readHumidity();

  lcd.print(temp_f);
  lcd.print("F");
  lcd.setCursor(0, 1);
  lcd.print(humidity);
  lcd.print("%RH");

  // Print the values to the serial port
  /*Serial.print(temp_c, DEC);
  Serial.print("C / ");
  Serial.print(temp_f, DEC);
  Serial.print("F / ");
  Serial.print(humidity);
  Serial.println("%");*/

  delay(5000);  // Must allow ~5sec b/w reads to prevent self-heating
}
