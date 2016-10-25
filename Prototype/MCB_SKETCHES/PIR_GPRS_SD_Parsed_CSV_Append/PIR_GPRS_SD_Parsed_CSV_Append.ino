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

String dataString;  // Define String for writing all retrieved data
String dataStringParsed;  //Define String for writing parsed data as CSV (format: pir,date,time)
String dateTime;  // Define String for storing date and time from GPRS
String appendString;  //String to append data to

void setup()
{ 
  // Define necessary pin modes
  pinMode(rxPin, INPUT);  // Software serial receive input
  pinMode(txPin, OUTPUT);  // Software serial transmiss output
  pinMode(powPin, OUTPUT);  // Software power-up output
  pinMode(dispPin, OUTPUT);  // LED indicator output
  pinMode(pirPin, INPUT);  // PIR sensor input

  Serial.begin(19200);

  Serial.print("Warming up motion sensor... ");
  delay(2000);  // Should be ~20 sec on first boot, but set low for debugging
  Serial.println("Motion sensor ready.");
  Serial.println();

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  Serial.println("GPRS powered on.");
  Serial.println();

  gprsSerial.begin(19200);  // Initiate software serial connection for GPRS

  gprsBaselineSettings();  // Call function to set gprs settings for proper response formatting

  Serial.println("GPRS connection initialized.");
  Serial.println();

  /*  Serial.print("Initializing SD card...");
   
   pinMode(10, OUTPUT);  // Pin 10 (chip select) must be set to OUTPUT even if unused
   
   if(!SD.begin(csPin))
   {
   Serial.println("card failed or not present.");
   delay(2000);
   return;
   }
   Serial.println("card initialized.");
   Serial.println();*/
}

void loop()
{
  delay(2000);  // Delay between each sensor check and read and write loop

  getDateTime();  // Get GPRS date and time by calling function
  dataString += String(dateTime);  // Add dateTime to dataString
  
  dataString += ",";

  readAnalogSensors();  // Get analog sensor values
  dataString += String(pir);  // Add PIR sensor value to dataString
  
  Serial.println(dataString);

  /*dataFile = SD.open("DATALOG.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
   
   if(dataFile)  // If file on SD card is opened successfully
   {
   dataFile.println(dataString);  // Print dataString to it
   dataFile.close();  // Close the data file and proceed
   }
   else
   {
   Serial.println("Error opening DATALOG.TXT!");  // If data file can't be opened, produce an error message
   }*/

  //parse pir and timestamp data from dataString
  //dataStringParsed += String(pir);  //add pir to first value in dataStringParsed
  //dataStringParsed += String(",");  //add "," for CSV formatting

    //parse out date and timestamp from dataString
  //for(int k = 8; k < 30; k++) {  //change to k<37 if you want to remove timezone offset.
    //dataStringParsed += String(dataString.charAt(k));  //add each character into dataStringParsed one by one
  //}
  //dataStringParsed += String("\n\r");
  //Serial.print(dataStringParsed);  //debug - print dataStringParsed to Serial as CSV (pir,date,time)

  pir1 = pir;  // Set PIR storage value equal to value just read

  //appendToString(dataStringParsed);

  dataString = "";  //clear dataString for new read
  dataStringParsed = "";  //clear dataStringParsed for new read
}

// Set baseline response parameters for proper string parsing and construction
void gprsBaselineSettings()
{
  delay(100);
  gprsSerial.println("ATE0");
  delay(100);
  gprsSerial.println("ATQ1");
  delay(100);
  gprsSerial.println("ATV0");
  delay(100);
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
  //  Serial.println("start getDateTime()");
  dateTime = "";

  //  Serial.println("AT+CCLK? command issued");
  gprsSerial.write("\nAT+CCLK?\r");  // Request date and time from GPRS
  // Since using "write", \n and \r included for proper AT formatting

  if(gprsSerial.available())  // If data is coming from GPRS
  {
    while(gprsSerial.available())  // Read the data into string from incoming bytes while they're available
    {
      for(int k = 8; k < 30; k++)
      {
        char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable
        dateTime += String(.charAt(k));  // Add character to the end of the data string to be written to SD later
      }
    }
  }
}

void readAnalogSensors()
{
  pir = digitalRead(pirPin);  // Retrieve value from PIR sensor
  digitalWrite(dispPin, pir);  // Set LED state to match sensor value
}
/*
//append value to string
void appendToString(String dateTimeAppend) {
  
  appendString += String(dateTimeAppend);

  //all the below Serial.println commands are for debugging to Serial console
  Serial.println("");
  Serial.println("appendToString START: ");
  Serial.println(appendString);
  Serial.println("appendToString END");
  Serial.println("");
}

//void parseDateTime();
//{
*/



