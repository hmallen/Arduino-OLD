void setup() {
  Serial.begin(19200);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
}

void loop() {
  Serial.println(F("1"));
  digitalWrite(3, HIGH);
  delay(3000);
  Serial.println(F("2"));
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  delay(3000);
  Serial.println(F("1+2"));
  digitalWrite(3, HIGH);
  delay(3000);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
}
