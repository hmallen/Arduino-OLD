const int accelXPin = A13;
const int accelYPin = A14;
const int accelZPin = A15;

void setup() {
  Serial.begin(19200);
}

void loop() {
  int accelX = analogRead(accelXPin) - 350;
  int accelY = analogRead(accelYPin) - 350;
  int accelZ = analogRead(accelZPin) - 350;

  Serial.print(accelX);
  Serial.print(F(", "));
  Serial.print(accelY);
  Serial.print(F(", "));
  Serial.println(accelZ);

  delay(1000);
}

