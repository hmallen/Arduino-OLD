#include <SoftwareSerial.h>

#define gprsRXPin 7
#define gprsTXPin 8
#define gprsPowPin 9

SoftwareSerial gprsSerial(gprsRXPin, gprsTXPin);

char smsTargetNum[11] = "2145635266";

void setup() {
  Serial.begin(19200);
  gprsSerial.begin(19200);

  gprsPowerOn();

  gprsSerial.println(F("AT+CMGF=1"));
  delay(100);
  gprsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  int i;
  for (i = 0; i < 10; i++) {
    gprsSerial.print(smsTargetNum[i]);
    Serial.print(smsTargetNum[i]);
    delay(100);
  }
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print(F("DATA GOES HERE"));
  delay(100);
  gprsSerial.println((char)26);
  delay(1000);
  gprsSerial.flush();
  delay(30000);
}

void gprsPowerOn() {
  digitalWrite(gprsPowPin, LOW);
  delay(100);
  digitalWrite(gprsPowPin, HIGH);
  delay(500);
  digitalWrite(gprsPowPin, LOW);
  delay(100);
}

void loop() {
}
