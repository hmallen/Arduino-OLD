#include <SHT1x.h>

const int shtSDA = 7;
const int shtSCL = 8;

SHT1x sht1x(7, 8);

void setup() {
  Serial.begin(9600);
  Serial.println("Serial initiated");

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void loop() {
  digitalWrite(2, HIGH);
  Serial.println("Relay ON");
  delay(1000);
  digitalWrite(2, LOW);
  Serial.println("Relay OFF");
  delay(1000);
  int light = analogRead(A0);
  Serial.println(light);
  delay(1000);
  int smoke = analogRead(A1);
  Serial.println(smoke);
  delay(1000);
  float tempF = sht1x.readTemperatureF();
  Serial.println(tempF);
  delay(1000);
  float humidity = sht1x.readHumidity();
  Serial.println(humidity);
  delay(1000);
}
