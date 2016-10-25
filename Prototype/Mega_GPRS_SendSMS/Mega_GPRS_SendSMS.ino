 void setup() {
  Serial.begin(19200);
  Serial1.begin(19200);
}

void loop() {
  Serial.println("Sending test SMS in 5 seconds.");
  delay(5000);
  Serial1.println("AT+CMGF=1"); //Because we want to send the SMS in text mode
  delay(100);
  Serial1.println("AT+CMGS=\"+12145635266\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  Serial1.println("A test message!");//the content of the message
  delay(100);
  Serial1.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  Serial1.println();
}

