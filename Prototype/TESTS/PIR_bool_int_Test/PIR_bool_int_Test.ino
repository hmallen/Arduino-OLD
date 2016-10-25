void setup() {
  Serial.begin(9600);
  Serial.println("Warming up");
  delay(1000);
  Serial.println("Warmed up");
}

void loop() {
  
  boolean mot = digitalRead(2);
  
  Serial.println(mot);
  
  delay(2000);
  
}
