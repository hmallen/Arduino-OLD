/* MyStall: Equine Asset Management Platform 

Features:
-

To Do:
-

*/

#include <SHT1x.h>

// Pin Definitions
const int sdaPin = 8;	// SHT1X
const int sclPin = 9;	// SHT1X
const int lightPin = A2;	// Photoresistor
const int tempPin = A1;	// LM35 Thermometer

// Parallax SHT1X Temperature & Humidity Sensor
SHT1x sht1x(sdaPin, sclPin);
float tempC, tempF, humidity;

// Analog sensors
int light = 0;	// Light level
int temp = 0;	// Temp level

void setup() {
	Serial.begin(19200);
	Serial.println(F("Setup complete."));
}

void loop() {
	readSHT1X();
	readLight();
	readTemp();

	Serial.print(F("SHT1X Temp: "));
	Serial.print(tempC);
	Serial.print(F("C, "));
	Serial.print(tempF);
	Serial.println(F("F"));
	Serial.print(F("SHT1X Humidity: "));
	Serial.println(humidity);
	Serial.print(F("Light: "));
	Serial.println(light);
	Serial.print(F("Temp: "));
	Serial.println(temp);

	delay(3000);
}

// Sensor read functions

void readSHT1X() {
	tempC = sht1x.readTemperatureC();
	tempF = sht1x.readTemperatureF();
	humidity = sht1x.readHumidity();
}

void readLight() {
	light = analogRead(lightPin);
}

void readTemp() {
	temp = analogRead(tempPin);
}

// Communication
