/*
Temperature & Humidity Sensors with LCD Display
- SHT11 Temp/Hum Sensor

TO DO:
- Add push button
- 1 Press --> Record high/low
- 2 Press --> Sleep display
 */

#include <LiquidCrystal_I2C.h>
#include <SHT1x.h>
#include <Wire.h>

#define lcdAddress 0x27
#define BACKLIGHT_PIN 13

LiquidCrystal_I2C lcd(lcdAddress, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Specify data and clock connections and instantiate SHT1x object
#define sdaPin 2
#define sclPin 3
SHT1x sht1x(sdaPin, sclPin);

void setup() {
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  lcd.begin(16, 2);
  Serial.begin(19200); // Open serial connection to report values to host

  //sht1x.readTemperatureC();
  sht1x.readTemperatureF();
  sht1x.readHumidity();

  lcd.print("Ready.");

  delay(5000);
}

void loop() {
  //float temp_c;
  float temp_f;
  float humidity;

  // Read values from the sensor
  //temp_c = sht1x.readTemperatureC();
  temp_f = sht1x.readTemperatureF();
  humidity = sht1x.readHumidity();

  lcd.clear();
  lcd.print(temp_f, 2);
  lcd.print(" F");
  lcd.setCursor(0, 1);
  lcd.print(humidity, 2);
  lcd.print(" %RH");

  // Print the values to the serial port
  //Serial.print(temp_c, DEC);
  //Serial.println4(" C");
  Serial.print(temp_f, 2);
  Serial.println(" F");
  Serial.print(humidity, 2);
  Serial.println(" %RH");
  Serial.println();

  delay(5000);  // Must allow ~5sec b/w reads to prevent self-heating
}
