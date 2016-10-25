#define tiltPin A5

void setup() {
  Serial.begin(38400);
  pinMode(tiltPin, INPUT);
}

void loop() {
  int val = digitalRead(tiltPin);
  Serial.println(val);
  delay(1000);
}
