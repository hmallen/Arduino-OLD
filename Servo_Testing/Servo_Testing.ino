#include <Servo.h>

Servo myservo;

void setup() {
  Serial.begin(19200);
  myservo.attach(5);
}

void loop() {
Serial.println("5");
myservo.write(5);
delay(2500);
Serial.println("45");
myservo.write(45);
delay(2500);
Serial.println("90");
myservo.write(90);
delay(2500);
Serial.println("135");
myservo.write(135);
delay(2500);
Serial.println("179");
myservo.write(179);
delay(2500);
}
