#define recPin A0
#define recLED 11
#define smsLED 12

boolean state = false;

void setup() {
  Serial.begin(19200);
  pinMode(recPin, INPUT);
  pinMode(recLED, OUTPUT);
  pinMode(smsLED, OUTPUT);
}

void loop() {
  if(digitalRead(recPin) == 0) {
    state = !state;
    while(digitalRead(recPin) == 0) {
      delay(100);
    }
    if(state == true) {
      digitalWrite(recLED, HIGH);
    }
    else {
      digitalWrite(recLED, LOW);
    }
    Serial.println(state);
  }
}
