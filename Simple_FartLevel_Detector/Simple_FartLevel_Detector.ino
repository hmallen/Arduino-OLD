int fartMax = 400;
int fartMaxPrev = fartMax;

void setup() {
  Serial.begin(19200);
}

void loop() {
  int fartVal = analogRead(A8);
  if(fartVal > fartMax) fartMax = fartVal;
  Serial.print(F("Fart level: "));
  Serial.print(fartVal);
  if(fartVal < 1000) Serial.print(F("  "));
  else Serial.print(F(" "));
  Serial.print(F("| "));
  Serial.print(F("Fart maximum: "));
  if(fartMax != fartMaxPrev) {
    Serial.print(fartMax);
    Serial.println(F(" NEW FART MAXIMUM!!!!"));
  }
  else Serial.println(fartMax);
  if(fartVal >= 1000) Serial.println(F("1000!!!! THE MOST HEINUS OF ALL ANUSES!"));
  fartMaxPrev = fartMax;
  delay(1000);  // Delay between fart level reads
}
