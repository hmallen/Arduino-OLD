/*  First test of Xively connectivity using a motion sensor  */

// GPRS shield
#include <SoftwareSerial.h>

// Xively
#include <Xively.h>

// HttpClient
#include <HttpClient.h>

// Define necessary constants for GPRS shield
int rxPin = 7;
int txPin = 8;
int powPin = 9;

// Define software serial connection for GPRS shield
SoftwareSerial gprsSerial(rxPin, txPin);

// Define pin constants for sensor(s)
int dispPin = 4;  // LED indicating state of motion sensor
int inPin = 2;  // Motion sensor input

// Define storage variables
int val = 0;  // Variable to store the read value
int val1 = 0;  // Variable to store the previous value for comparison

// Define Xively API key
//char xivelyKey[] = "";

// Define strings for datastream IDs
char sensorId[] =  "sensor_reading";
XivelyDatastream datastreams[] = 
{
  XivelyDatastream(sensorId, strlen(sensorId), DATASTREAM_FLOAT),
};

// Wrap datastreams into a feed
XivelyFeed feed(15552, datastreams, 1 /* (# of datastreams) */);

//NEED CLIENT STUFF HERE!134l@%$#$dfs0


void setup()
{
  // Define necessary pin modes
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);

  Serial.begin(19200);  // Initiate serial output for monitoring

  Serial.print("Warming up motion sensor... ");
  delay(5000);  // Should be ~20 sec on first boot, but set low for debugging
  Serial.println("Motion sensor ready.");
  Serial.println();

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  Serial.println("GPRS powered on.");
  Serial.println();

  gprsSerial.begin(19200);  // Initiate software serial for GPRS

  Serial.println("GPRS connection initialized.");

  pinMode(dispPin, OUTPUT);  // Sets digital pin 4 as output
  pinMode(inPin, INPUT);  // Sets digital pin 2 as input
}

void loop()
{
  val = digitalRead(inPin);  // Read state of motion sensor

  if(val != val1)
  {
    datastreams[0].setFloat(val);

    Serial.print("Sensor value: ");
    Serial.println(datastreams[0].getFloat());

    Serial.println("Uploading it to Xively...");
    //int ret = xivelyclient.put(feed, xivelyKey);
    //Serial.print("xivelyclient.put returned: ");
    //Serial.println(ret);

    digitalWrite(dispPin, val);  // Sets LED state based on value of motion sensor
  }

  val1 = val;  // Stores read value for comparison with read on next loop
}

void powerUp()
{
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}







