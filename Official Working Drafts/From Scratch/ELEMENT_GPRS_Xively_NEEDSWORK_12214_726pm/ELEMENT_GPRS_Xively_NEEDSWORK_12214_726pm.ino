void setup()
{
  Serial1.begin(19200);               // the GPRS baud rate   
  Serial.begin(19200);    // the GPRS baud rate 
  delay(500);
  Serial.println("Type \"x\" to send data to Xively");
}
 
void loop()
{ 
  if (Serial.available())
    switch(Serial.read())
   {
     case 'x':
       Send2Xively();
       break;
   } 
  if (Serial1.available())
    Serial.write(Serial1.read());
}

void Send2Xively()
{
  Serial1.println("AT+CGATT?");
  delay(1000);
 
  ShowSerialData();
 
  Serial1.println("AT+CSTT=\"CMNET\"");//start task and setting the APN,
  delay(1000);
 
  ShowSerialData();
 
  Serial1.println("AT+CIICR");//bring up wireless connection
  delay(3000);
 
  ShowSerialData();
 
  Serial1.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  ShowSerialData();
 
  Serial1.println("AT+CIPSPRT=0");
  delay(3000);
 
  ShowSerialData();
 
  Serial1.println("AT+CIPSTART=\"tcp\",\"api.cosm.com\",\"8081\"");//start up the connection
  delay(2000);
 
  ShowSerialData();
 
  Serial1.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
  String humidity = "1031";//these 4 line code are imitate the real sensor data, because the demo did't add other sensor, so using 4 string variable to replace.
  String moisture = "1242";//you can replace these four variable to the real sensor data in your project
  String temperature = "30";//
  String barometer = "60.56";//
  Serial1.print("{\"method\": \"put\",\"resource\": \"/feeds/42742/\",\"params\"");//here is the feed you apply from pachube
  delay(500);
  ShowSerialData();
  Serial1.print(": {},\"headers\": {\"X-PachubeApiKey\":");//in here, you should replace your pachubeapikey
  delay(500);
  ShowSerialData();
  Serial1.print(" \"_cXwr5LE8qW4a296O-cDwOUvfddFer5pGmaRigPsiO0");//pachubeapikey
  delay(500);
  ShowSerialData();
  Serial1.print("jEB9OjK-W6vej56j9ItaSlIac-hgbQjxExuveD95yc8BttXc");//pachubeapikey
  delay(500);
  ShowSerialData();
  Serial1.print("Z7_seZqLVjeCOmNbEXUva45t6FL8AxOcuNSsQS\"},\"body\":");
  delay(500);
  ShowSerialData();
  Serial1.print(" {\"version\": \"1.0.0\",\"datastreams\": ");
  delay(500);
  ShowSerialData();
  Serial1.println("[{\"id\": \"01\",\"current_value\": \"" + barometer + "\"},");
  delay(500);
  ShowSerialData();
  Serial1.println("{\"id\": \"02\",\"current_value\": \"" + humidity + "\"},");
  delay(500);
  ShowSerialData();
  Serial1.println("{\"id\": \"03\",\"current_value\": \"" + moisture + "\"},");
  delay(500);
  ShowSerialData();
  Serial1.println("{\"id\": \"04\",\"current_value\": \"" + temperature + "\"}]},\"token\": \"lee\"}");
 
 
  delay(500);
  ShowSerialData();
 
  Serial1.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  Serial1.println();
 
  ShowSerialData();
 
  Serial1.println("AT+CIPCLOSE");//close the connection
  delay(100);
  ShowSerialData();
}
 
void ShowSerialData()
{
  while(Serial1.available()!=0)
    Serial.write(Serial1.read());
}
