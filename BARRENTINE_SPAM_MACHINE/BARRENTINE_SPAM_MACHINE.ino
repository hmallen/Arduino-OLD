 #include <SoftwareSerial.h>
 
 #define rxPin 7
 #define txPin 8
 
 SoftwareSerial gprsSerial(rxPin, txPin);
 
 void setup() {
  Serial.begin(19200);
  gprsSerial.begin(19200);
  delay(500)
  
  Serial.println("Type \"t\" and press ENTER to send an SMS.");
}

void loop() {
  if(Serial.available()) {
    switch(Serial.read()) {
      case 't':
        SendTextMessage();
        break;
    }
  }
  if(gprsSerial.available()) {
    Serial.write(gprsSerial.read());
  }
}

void SendTextMessage() {
  String SMSdest = "AT+CMGS=\"+1";
  
  Serial.print("Enter a mobile number:");
  char remoteNum[20];
  readSerial(remoteNum);
  Serial.println(remoteNum)
  
  Serial.println("Sending test SMS in 5 seconds.");
  delay(5000);
  gprsSerial.println("AT+CMGF=1"); //Because we want to send the SMS in text mode
  delay(100);
  gprsSerial.println("AT+CMGS=\"+12142444406\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  gprsSerial.println("Say, fool.");//the content of the message
  delay(100);
  gprsSerial.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  gprsSerial.println();
}

// Define readSerial() function
int readSerial(char result[])
{
  int i = 0;
  while(1)
  {
    while (Serial.available() > 0)
    {
      char inChar = Serial.read();
      if (inChar == '\n')
      {
        result[i] = '\0';
        Serial.flush();
        return 0;
      }
      if(inChar!='\r')
      {
        result[i] = inChar;
        i++;
      }
    }
  }
}
