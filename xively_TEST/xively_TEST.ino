void setup()
{
  Serial1.begin(19200);               // the GPRS baud rate   
  Serial.begin(19200);    // the GPRS baud rate 
  delay(500);
}
 
void loop()
{
  //after start up the program, you can using terminal to connect the serial of gprs shield,
  //if you input 't' in the terminal, the program will execute SendTextMessage(), it will show how to send a sms message,
  //if input 'd' in the terminal, it will execute DialVoiceCall(), etc.
 
  if (Serial.available())
    switch(Serial.read())
   {
     case 't':
       SendTextMessage();
       break;
     case 'd':
       DialVoiceCall();
       break;
     case 'h':
       SubmitHttpRequest();
       break;
     case 's':
       Send2Pachube();
       break;
   } 
  if (Serial1.available())
    Serial.write(Serial1.read());
}
 
///SendTextMessage()
///this function is to send a sms message
void SendTextMessage()
{
  Serial1.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  Serial1.println("AT + CMGS = \"+86138xxxxx615\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  Serial1.println("A test message!");//the content of the message
  delay(100);
  Serial1.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  Serial1.println();
}
 
///DialVoiceCall
///this function is to dial a voice call
void DialVoiceCall()
{
  Serial1.println("ATD + +86138xxxxx615;");//dial the number
  delay(100);
  Serial1.println();
}
 
///SubmitHttpRequest()
///this function is submit a http request
///attention:the time of delay is very important, it must be set enough 
void SubmitHttpRequest()
{
  Serial1.println("AT+CSQ");
  delay(100);
 
  ShowSerialData();// this code is to show the data from gprs shield, in order to easily see the process of how the gprs shield submit a http request, and the following is for this purpose too.
 
  Serial1.println("AT+CGATT?");
  delay(100);
 
  ShowSerialData();
 
  Serial1.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR, the connection type is using gprs
  delay(1000);
 
  ShowSerialData();
 
  Serial1.println("AT+SAPBR=3,1,\"APN\",\"CMNET\"");//setting the APN, the second need you fill in your local apn server
  delay(4000);
 
  ShowSerialData();
 
  Serial1.println("AT+SAPBR=1,1");//setting the SAPBR, for detail you can refer to the AT command mamual
  delay(2000);
 
  ShowSerialData();
 
  Serial1.println("AT+HTTPINIT"); //init the HTTP request
 
  delay(2000); 
  ShowSerialData();
 
  Serial1.println("AT+HTTPPARA=\"URL\",\"www.google.com.hk\"");// setting the httppara, the second parameter is the website you want to access
  delay(1000);
 
  ShowSerialData();
 
  Serial1.println("AT+HTTPACTION=0");//submit the request 
  delay(10000);//the delay is very important, the delay time is base on the return from the website, if the return datas are very large, the time required longer.
  //while(!Serial1.available());
 
  ShowSerialData();
 
  Serial1.println("AT+HTTPREAD");// read the data from the website you access
  delay(300);
 
  ShowSerialData();
 
  Serial1.println("");
  delay(100);
}
 
///send2Pachube()///
///this function is to send the sensor data to the pachube, you can see the new value in the pachube after execute this function///
void Send2Pachube()
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
 
  Serial1.println("AT+CIPSTART=\"tcp\",\"api.xively.com\",\"8081\"");//start up the connection
  delay(2000);
 
  ShowSerialData();
 
  Serial1.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
  String humidity = "1031";//these 4 line code are imitate the real sensor data, because the demo did't add other sensor, so using 4 string variable to replace.
  String moisture = "1242";//you can replace these four variable to the real sensor data in your project
  String temperature = "30";//
  String barometer = "60.56";//
  Serial1.print("{\"method\": \"put\",");
  Serial1.print("\"resource\": \"/feeds/42742\",");//here is the feed you apply from pachube
  Serial1.print(": {},\"headers\": {\"X-ApiKey\":");//in here, you should replace your pachubeapikey
  Serial1.print("\"dOtspYFKWYLMXn");//pachubeapikey
  Serial1.print("JePtaeYbdWmbq1P31DQE");//pachubeapikey
  Serial1.print("\"d6FU2e33YE99UI},");
  Serial1.print("\"body\":");
  Serial1.print(" {\"version\": \"1.0.0\",\"datastreams\": ");
  Serial1.println("[{\"id\": \"Barometer\",\"current_value\": \"" + barometer + "\"},");
  Serial1.println("{\"id\": \"Humidity\",\"current_value\": \"" + humidity + "\"},");
  Serial1.println("{\"id\": \"Moisture\",\"current_value\": \"" + moisture + "\"},");
  Serial1.print("{\"id\": \"Temperature\",\"current_value\": \"" + temperature + "\"}]},");
  Serial1.println("\"token\": \"9V4JFNG9HCDR\"}");
  Serial1.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet
  ShowSerialData();
  Serial1.println();
  Serial1.println("AT+CIPCLOSE");//close the connection
}
 
void ShowSerialData()
{
  while(Serial1.available()!=0)
    Serial.write(Serial1.read());
}
