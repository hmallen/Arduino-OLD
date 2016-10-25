/*
 Example of SSerial2Mobile libary 

Sends a SMS and an email
The SMS phone number and the email address need to be changed to something valid. 

 created Jan 2010
 by Gustav von Roth 
*/

#include <SoftwareSerial.h>
#include <SSerial2Mobile.h>

#define RXpin 10
#define TXpin 11 //blue

int returnVal=10;






void setup() {
  Serial.begin(9600);
  SSerial2Mobile phone = SSerial2Mobile(RXpin,TXpin);
  
  

  //returnVal=phone.isOK();
  //Serial.println(returnVal, DEC);
  //delay(3000);
  
  Serial.print("Batt: ");
  Serial.print(phone.batt());
  Serial.println("%");
  
  Serial.print("RSSI: ");
  Serial.println(phone.rssi());
  // Any RSSI over >=5 should be fine for SMS
  // SMS:  5
  // voice:  10
  // data:  20
  
  phone.sendTxt("+12125550125","Lib SMS Test1");
  delay(3000);
  phone.sendEmail("SSerial2Moble@example.com", "Lib email test1");
  delay(3000);
  
  
  
}
void loop(){}
