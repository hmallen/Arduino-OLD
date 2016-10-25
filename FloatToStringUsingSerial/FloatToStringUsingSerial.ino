float val = 1.2345;
String floatString;

void setup() {
  Serial.begin(19200);
  Serial3.begin(19200);
  delay(2000);
  Serial3.print(val, 4);
  if(!Serial3.available()){
    while(!Serial3.available()) {
      delay(1);
    }
  }
  Serial.println("Data available.");
  if(Serial3.available()) {
    while(Serial3.available()) {
      char c = Serial3.read();
      floatString += c;
      delay(1);
    }
    Serial.println();
  }
  Serial.println(floatString);
}

void loop() {
}
