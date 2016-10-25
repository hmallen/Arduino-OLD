#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 8); // RX, TX pins

void setup() {
  Serial.begin(19200);
  mySerial.begin(19200); // Open serial connection at baud rate of 4800
}

void loop() {
  Serial.println("Sending test SMS in 5 seconds.");
  delay(5000);
  mySerial.println("AT+CMGF=1"); //Because we want to send the SMS in text mode
  delay(100);
  mySerial.println("AT+CMGS=\"+12145635266\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  mySerial.println("A test message!");//the content of the message
  delay(100);
  mySerial.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  mySerial.println();
}

