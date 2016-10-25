/*
 SMS sender
 
 This sketch, for the Arduino GSM shield,sends an SMS message 
 you enter in the serial monitor. Connect your Arduino with the 
 GSM shield and SIM card, open the serial monitor, and wait for 
 the "READY" message to appear in the monitor. Next, type a 
 message to send and press "return". Make sure the serial 
 monitor is set to send a newline when you press return.
 
 Circuit:
 * GSM shield 
 * SIM card that can send SMS
 
 created 25 Feb 2012
 by Tom Igoe
 
 This example is in the public domain.
 
 http://arduino.cc/en/Tutorial/GSMExamplesSendSMS
 
 */

#include <SoftwareSerial.h>

#define PINNUMBER ""

SoftwareSerial GPRS(7, 8);

void setup() {

  GPRS.begin(9600);
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);

  Serial.println("SMS Messages Sender");
}

void loop()
{

  if(Serial.available())
    switch(Serial.read()) {
    case 't':
      SendTextMessage();
      break;
    }
  if(GPRS.available())
    Serial.write(GPRS.read());
}

void SendTextMessage() {

  GPRS.print("AT+CMGF=1\r"); // Set to SMS text mode (as opposed to binary mode)
  delay(100);
  
  String(num) = "AT + CMGS = ";

  Serial.print("Enter a 10-digit US mobile number (ex. +12145635266): ");
  char remoteNum[20];  // telephone number to send sms
  Serial.println(remoteNum);
  
  String(num) += String(remoteNum);

  // sms text
  Serial.print("Now, enter SMS content: ");
  char txtMsg[200];
  readSerial(txtMsg);
  Serial.println("SENDING");
  Serial.println();
  Serial.println("Message:");
  Serial.println(txtMsg);
  
  Serial.println("Please wait...");

  // send the message
  GPRS.println(num);
  delay(1000);
  GPRS.println(txtMsg);
  delay(1000);
  GPRS.println((char)26);
  delay(1000);
  GPRS.println();
  Serial.println("\nYour message has been sent.\n");

}

/*
  Read input serial
 */
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





