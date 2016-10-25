#include <SD.h>
#include <SoftwareSerial.h>

// GPRS shield constants for software serial connection and power-on
const int rxPin = 7;
const int txPin = 8;
const int powPin = 9;  // Provides software power-on capability

SoftwareSerial gprsSerial(rxPin, txPin);  // Define software serial connection for GPRS

const int csPin = 10;  // Chip select for SD shield

// Define pin constants for sensor(s)
const int dispPin = 4;  // LED indicating state of motion sensor
const int pirPin = 2;  // Motion sensor pin

// Define storage variables
int pir = 0;  // Variable to store the read value
int pir1 = 0;  // Variable to store the previous value for comparison

File dataFile;  // Define variable as a file for data logging

void setup()
{
  // Define necessary pin modes
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);

  Serial.begin(19200);

  Serial.print("Warming up motion sensor... ");
  delay(5000);  // Should be ~20 sec on first boot, but set low for debugging
  Serial.println("Motion sensor ready.");
  Serial.println();

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  Serial.println("GPRS powered on.");
  Serial.println();

  gprsSerial.begin(19200);  // Initiate software serial connection for GPRS

  Serial.println("GPRS connection initialized.");
  Serial.println();

  Serial.print("Initializing SD card...");

  pinMode(10, OUTPUT);  // Pin 10 (chip select) must be set to OUTPUT even if unused

  if(!SD.begin(csPin))
  {
    Serial.println("card failed or not present.");
    delay(1000);
    return;
  }

  Serial.println("card initialized.");
  Serial.println();

  pinMode(dispPin, OUTPUT);  // Sets digital pin 4 as output for LED indicator
  pinMode(pirPin, INPUT);  // Sets digital pin 2 as input for PIR sensor
}

void loop()
{
  String dataString = "";

  pir = digitalRead(pirPin);

  if(pir != pir1)
  {  
    Serial.print("PIR state via digitalRead(): ");
    Serial.println(pir);

    dataString += String(pir);

    dataFile = SD.open("DATALOG.TXT", FILE_WRITE);

    if(dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
      Serial.print("PIR state written to dataString/SD card: ");
      Serial.println(dataString);
    }
    else
    {
      Serial.println("Error opening DATALOG.TXT!");
    }
    pir1 = pir;
  }
}

// Software start-up function for GPRS shield
void powerUp()
{
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}










