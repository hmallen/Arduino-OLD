#include <SD.h>
#include <SoftwareSerial.h>

SoftwareSerial gprsSerial(7,8);

const int csPin = 10;
const int pirPin = 2;

// Define pin constants for sensor(s)
int dispPin = 4;  // LED indicating state of motion sensor
int inPin = 2;  // Motion sensor pin

// Define storage variables
int val = 0;  // Variable to store the read value
int val1 = 0;  // Variable to store the previous value for comparison

File dataFile;

void setup()
{
  Serial.begin(19200);
  
    Serial.print("Warming up motion sensor... ");
  delay(5000);  // Should be ~20 sec on first boot, but set low for debugging
  Serial.println("Motion sensor ready.");
  Serial.println();

  Serial.print("Initializing SD card...");

  pinMode(10, OUTPUT);

  if(!SD.begin(csPin))
  {
    Serial.println("card failed or not present.");
    delay(1000);
    return;
  }

  Serial.println("card initialized.");

gprsSerial.begin(19200);

Serial.println("GPRS connection initialized.");

  pinMode(dispPin, OUTPUT);  // Sets digital pin 4 as output
  pinMode(inPin, INPUT);  // Sets digital pin 2 as input
}

void loop()
{
  String dataString = "";

  int pir = digitalRead(pirPin);
  
  Serial.print("PIR state via digitalRead(): ");
  Serial.println(pir);

if(pir != pir1)
{
  dataString += String(pir);

  dataFile = SD.open("DATALOG.TXT", FILE_WRITE);

  if(dataFile)
  {
    dataFile.println(dataString);
    dataFile.close();
    Serial.print("PIR state in dataString/written to SD card: ");
    Serial.println(dataString);
  }
  else
  {
    Serial.println("Error opening DATALOG.TXT!");
  }
  
  pir1 = pir;
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
