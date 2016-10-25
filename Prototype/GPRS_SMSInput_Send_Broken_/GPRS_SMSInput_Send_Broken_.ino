#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 8);

void setup()
{
  mySerial.begin(9600);  // GPRS baud rate
  Serial.begin(9600);  // Hardware serial baud rate
  delay(500);

  Serial.print("Type \"t\" and press ENTER to enter SMS send mode.");
  //Serial.println("Type \"s\" and press ENTER to enter Xively transfer mode.");
}

void loop()
{
  if(Serial.available())
    switch(Serial.read())
    {
      // User input to initiate SMS send mode
    case 't':
      SendTextMessage();
      break;
      /*
    // User input to initiate Xively transfer mode
       case 's':
       Send2Xively();
       break;
       */
    }
  if(mySerial.available())
    Serial.write(mySerial.read());
}

// Create SendTextMessage() function
void SendTextMessage()
{
  String SMSdest = "AT+CMGS=\"+1";

  // Ask user to input mobile number where SMS will be sent
  Serial.print("Enter a mobile number:");
  char remoteNum[20];
  readSerial(remoteNum);
  Serial.println(remoteNum);

  SMSdest += String(remoteNum);
  SMSdest += "\"";

  // Ask user to input SMS content
  Serial.print("Enter SMS content:");
  char txtMsg[200];
  Serial.println(txtMsg);

  Serial.print("Sending message, \"");
  Serial.print(txtMsg);
  Serial.print("\" to ");
  Serial.print(remoteNum);
  Serial.println(".");
  delay(500);

  Serial.print("Please wait");
  delay(500);
  Serial.print(".");
  delay(500);
  Serial.print(".");
  delay(500);
  Serial.println(".");
  delay(500);

  Serial.println("Confirming proper string construction:");
  Serial.println(SMSdest);
  delay(2000);

  mySerial.print("AT+CMGF=1\r");  // Sets GPRS to text mode (as opposed to binary mode)
  delay(1000);
  mySerial.println(SMSdest);  // Sets number to send SMS
  delay(1000);
  mySerial.println(txtMsg);  // SMS content
  delay(1000);
  mySerial.println((char)26);  // ASCII code for ctrl+z is 26
  delay(1000);
  mySerial.println();
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

