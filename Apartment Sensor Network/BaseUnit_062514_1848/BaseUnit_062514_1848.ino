unsigned long targetNum = 2145635266;

void setup() {
}

void loop() {
}

void smsEvent() {
  gprsSerial.println(F("AT+CMGF=1"));
  delay(100);
  gprsSerial.print(F("AT+CMGS=\"+1"));
  delay(100);
  gprsSerial.print(targetNum);
  delay(100);
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.println(F("Event has ocurred."));
  delay(100);
  gprsSerial.println((char)26);
  delay(1000);
  gprsSerial.flush();
}
