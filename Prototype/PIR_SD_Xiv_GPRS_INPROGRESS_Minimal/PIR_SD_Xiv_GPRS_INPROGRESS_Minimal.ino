#include <SD.h>
#include <SoftwareSerial.h>

// Define constants for GPRS shield function
const int rxPin = 7;  // Receive pin for software serial
const int txPin = 8;  // Transmit pin for software serial
const int powPin = 9;  // Software power-up pin

SoftwareSerial gprsSerial(rxPin, txPin);  // Define software serial connection for GPRS

const int csPin = 10;  // Chip select for SD shield

// Define pin constants for sensor(s)
const int dispPin = 4;  // LED indicating state of motion sensor
const int pirPin = 2;  // Motion sensor pin

// Define storage variables
int pir = 0;  // Variable to store the read value
int pir1 = 0;  // Variable to store the previous value for comparison

File dataFile;  // Define variable as a file for data logging

String DateTime;
String dataString;

void setup()
{ 
  // Define necessary pin modes
  pinMode(rxPin, INPUT);  // Software serial receive input
  pinMode(txPin, OUTPUT);  // Software serial transmiss output
  pinMode(powPin, OUTPUT);  // Software power-up output
  pinMode(dispPin, OUTPUT);  // LED indicator output
  pinMode(pirPin, INPUT);  // PIR sensor input

  Serial.begin(57600);

  Serial.print("Warming up motion sensor... ");
  delay(2000);  // Should be ~20 sec on first boot, but set low for debugging
  Serial.println("Motion sensor ready.");
  Serial.println();

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  Serial.println("GPRS powered on.");
  Serial.println();

  gprsSerial.begin(57600);  // Initiate software serial connection for GPRS

  Serial.println("GPRS connection initialized.");
  Serial.println();

  Serial.print("Initializing SD card...");

  pinMode(10, OUTPUT);  // Pin 10 (chip select) must be set to OUTPUT even if unused

  if(!SD.begin(csPin))
  {
    Serial.println("card failed or not present.");
    delay(2000);
    return;
  }
  Serial.println("card initialized.");
  Serial.println();
}

void loop()
{
  //dataString = "";  // Create string for storing all data
  readAnalogSensors();

  if(pir != pir1)  // If sensor value is different than last time it was read
  { 
    getDateTime();  // Get GPRS date/time by calling function

    dataString += String(DateTime);  // Add DateTime to dataString

    dataString += String(pir);  // Add PIR sensor value to dataString

    dataFile = SD.open("DATALOG.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing

    if(dataFile)  // If file on SD card is opened successfully
    {
      dataFile.println(dataString);  // Print dataString to it
      dataFile.close();  // Close the data file and proceed

      Serial.println(dataString);  // Print dataString to serial monitor to confirm reasonable values
    }
    else
    {
      Serial.println("Error opening DATALOG.TXT!");  // If data file can't be opened, produce an error message
    }
    pir1 = pir;  // Set PIR storage value equal to value just read
    dataString = "";
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

void getDateTime()
{
  DateTime = "";

  gprsSerial.write("\nAT+CCLK?\r");  // Request date and time from GPRS
  // Since using "write", \n and \r included for proper AT formatting

  if(gprsSerial.available())  // If data is coming from GPRS
  {
    while(gprsSerial.available())  // Read the data into string from incoming bytes while they're available
    {
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable

      DateTime += c;  // Add character to the end of the data string to be written to SD later
    }
  }
}

void readAnalogSensors()
{
  pir = digitalRead(pirPin);  // Retrieve value from PIR sensor
  digitalWrite(dispPin, pir);  // Set LED state to match sensor value
}

/*
void processDateTime()
 {
 if(DateTime.indexOf("+CCLK") >= 0)
 {
 int iPos = DateTime.indexOf(":");
 String dtStorePos = DateTime.substring(iPos + 2);
 
 readDateTime();
 }
 }
 */
