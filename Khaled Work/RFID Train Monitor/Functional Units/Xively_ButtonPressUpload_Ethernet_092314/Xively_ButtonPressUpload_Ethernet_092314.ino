#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Xively.h>

// MAC address for your Ethernet shield
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Your Xively key to let you upload data
char xivelyKey[] = "zuiTAQ42hNVUnHlp9L5w3TxKi2SKHgnvKqzFGd4HxxKi6nuN";

// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 2;

// Define the strings for our datastream IDs
char sensorId[] = "button";
XivelyDatastream datastreams[] = {
  XivelyDatastream(sensorId, strlen(sensorId), DATASTREAM_FLOAT),
};
// Finally, wrap the datastreams into a feed
XivelyFeed feed(1900210572, datastreams, 1 /* number of datastreams */);

EthernetClient client;
XivelyClient xivelyclient(client);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);

  pinMode(sensorPin, INPUT);

  Serial.println("Starting single datastream upload to Xively...");
  Serial.println();

  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(15000);
  }
}

void loop() {
  /*boolean triggerSend = true;
   int sensorValue = 0;
   Serial.println(F("Waiting for button press..."));
   while(digitalRead(sensorPin) == 0) {
   delay(10);
   }
   if(digitalRead(sensorPin) == 1) {
   unsigned long holdTime = millis();
   for(int x = millis() - holdTime; x < 3000; ) {
   if(digitalRead(sensorPin) == 0) {
   triggerSend = false;
   break;
   }
   }
   }*/
  int sensorValue;
  Serial.println(F("Waiting for button press...")); 
  while(digitalRead(sensorPin) == 0) {
    delay(100);
  }
  if(digitalRead(sensorPin) == 1) {
    while(digitalRead(sensorPin) == 1) {
      delay(1);
    }
    delay(100);
  }
  sensorValue = 1;
  datastreams[0].setFloat(sensorValue);

  Serial.print(F("Read sensor value "));
  Serial.println(datastreams[0].getFloat());

  Serial.println(F("Uploading it to Xively"));
  int ret = xivelyclient.put(feed, xivelyKey);
  Serial.print(F("xivelyclient.put returned "));
  Serial.println(ret);
  Serial.println();
  delay(15000);

  sensorValue = 0;
  datastreams[0].setFloat(sensorValue);
  ret = xivelyclient.put(feed, xivelyKey);
  Serial.println(F("Sensor value reset. Uploading to Xively."));
  Serial.print(F("xivelyclient.put returned "));
  Serial.println(ret);
  Serial.println();
  delay(5000);
}
