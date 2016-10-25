const int ledPin = 7;

void setup() {
  Serial.begin(38400);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  Serial.println("LED High");
  digitalWrite(ledPin, HIGH);
  int ledPinState = digitalRead(ledPin);
  Serial.println(ledPinState);
  if(ledPin == HIGH) {
    Serial.println("ON");
  }
  else {
    Serial.println("NULL");
  }
  delay(3000);
  Serial.println("LED Low");
  digitalWrite(ledPin, LOW);
  ledPinState = digitalRead(ledPin);
  Serial.println(ledPinState);
  if(ledPin == LOW) {
    Serial.println("OFF");
  }
  else {
    Serial.println("NULL");
  }
  delay(3000);
}
