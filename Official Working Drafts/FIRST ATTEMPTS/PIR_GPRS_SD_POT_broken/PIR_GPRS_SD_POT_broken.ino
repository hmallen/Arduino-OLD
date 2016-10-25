/*

 -- Official working draft of the MyStall prototype --
 
 All called functions are defined at the end of the sketch, immediately following
 the main loop. Pay attention to the clearStrings() function, as it must be updated
 with any new strings added to the sketch for them to be cleared properly after
 each loop.
 
 Current shields:
 - GPRS
 - SD
 
 Current sensors:
 - PIR
 
 Important notes:
 - In the Arduino core libraries, SoftwareSerial.h has been modified to increase max
 buffer size from 64 --> 256 to accomodate longer reads from GPRS over the software
 serial connection.
 - In working prototype, PIR must be warmed up for ~20 seconds on the first boot for
 proper function.
 
 */

#include <SD.h>
#include <SoftwareSerial.h>

// Define constants for GPRS shield function
const int rxPin = 7;  // Receive pin for software serial
const int txPin = 8;  // Transmit pin for software serial
const int powPin = 9;  // Software power-up pin

const int csPin = 10;  // Chip select for SD shield

// Define pin constants for sensor(s)
const int dispPin = 4;  // LED indicating state of motion sensor
const int pirPin = 2;  // Motion sensor pin

// Define strings used globally
String rawDateTime;  // Create string for storing GPRS date and time before parsing
String year;  // Create string for storing raw year
String yearLong;  // Create string for assembling long form of year
String month;  // Create string for storing raw month
String monthLong;  // Create string for assembling long form of month (month name)
String day;  // Create string for storing raw day
String dayLong;   // Create string for assembling long form of day
String dateString;  // Create string for storing date from GPRS when function called
String dateLongString;  // Create string for assembling long form of date
String timeString;  // Create string for storing time from GPRS when function called
String dataString;  // Create string for storing all data to be written to SD

File dataFile;  // Define variable as a file for data logging via SD shield

SoftwareSerial gprsSerial(rxPin, txPin);  // Define software serial connection for GPRS

//int pir = 0;  // Define variable for reading PIR sensor

int potPin = A4;  // Analog pin for potentiometer input
int potValue = 0;  // Raw value read from potentiometer
float potVoltage = 0;  // Calculated potentiometer voltage

void setup()
{ 
  // Define necessary pin modes
  pinMode(rxPin, INPUT);  // Software serial receive input
  pinMode(txPin, OUTPUT);  // Software serial transmiss output
  pinMode(powPin, OUTPUT);  // Software power-up output
  pinMode(dispPin, OUTPUT);  // LED indicator output
  //pinMode(pirPin, INPUT);  // PIR sensor input

  Serial.begin(19200);  // Initiate hardware serial connection for monitor output

  Serial.print("Warming up motion sensor... ");
  //delay(2000);  // Should be ~20 sec on first boot, but set low for debugging
  Serial.println("Motion sensor ready.");
  Serial.println();

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  Serial.println("GPRS powered on.");
  Serial.println();

  gprsSerial.begin(19200);  // Initiate software serial connection for GPRS

  gprsBaselineSettings();  // Call function to set GPRS settings for proper response formatting

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
  delay(1000);  // Delay between each sensor check and read and write loop

  potValue = analogRead(potPin);  // Retrieve measurement from potentiometer
  //potVoltage = potValue*(5.0 / 1023.0);  // Calculate actual voltage across potentiometer
  //int potVol = char(potVoltage);
  
  //pir = digitalRead(pirPin);  // Retrieve value from PIR sensor
  //digitalWrite(dispPin, pir);  // Set LED state to match sensor value

  getDateTime();  // Get GPRS date and time by calling function

  //appendToString(dateString);
  //appendToString(timeString);

  dataString += String(dateString);  // Add date to dataString
  dataString += ",";

  dataString += String(timeString);  // Add time to dataString
  dataString += ",";

  //dataString += String(pir);  // Add PIR sensor value to dataString
  dataString += String(potValue);

  dataStringWriteSD();  // Call function to write dataString to SD card

  clearStrings();  // Call function to clear all strings storing data
}

///////////////////////////////////////////////////////////////////////////////////////////////
//\/\/\/\<------------  All functions called in main loop defined below  ------------>/\/\/\/\\
///////////////////////////////////////////////////////////////////////////////////////////////

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

// Retrieve date and time from GPRS and parse into individual strings
void getDateTime()
{
  gprsSerial.println("AT+CCLK?");  // Request date and time from GPRS

  if(gprsSerial.available())  // If data is coming from GPRS
  {
    while(gprsSerial.available())  // Read the data into string from incoming bytes while they're available
    {
      char c = gprsSerial.read();  // Read each byte sent, one at a time, into storage variable

      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
  }
  parseDateTime();
}

/*
void parseDateTime()
 {
 for(int i = 8; i < 16; i++)
 {
 dateString += String(rawDateTime.charAt(i));
 dateString += String("\0");
 }
 for(int j = 17; j < 25; j++)
 {
 timeString += String(rawDateTime.charAt(j));
 timeString += String("\0");
 }
 }
 */

void parseDateTime()
{
  for(int y = 8; y < 10; y++)
  {
    year += String(rawDateTime.charAt(y));
    year += String("\0");
  }
  for(int m = 11; m < 13; m++)
  {
    month += String(rawDateTime.charAt(m));
    month += String("\0");
  }
  for(int d = 14; d < 16; d++)
  {
    day += String(rawDateTime.charAt(d));
    day += String("\0");
  }
  for(int t = 16; t < 26; t++)
  {
    timeString += String(rawDateTime.charAt(t));
    timeString += String("\0");
  }

  //dateString += String(month);
  dateString += "/";
  dateString += String(day);
  dateString += "/";
  dateString += String(year);

  //dateLong();
}

void dateLong()
{
  // Format year
  yearLong += "20";
  yearLong += String(year);
  Serial.println(yearLong);

  // Format day
  dayLong += day.toInt();
  /*if(day.charAt(0) == 0)
   {
   dayLong = String(day.charAt(1));
   }
   else
   {
   dayLong = String(day);
   }*/
  Serial.println(dayLong);

  // Format month
  switch(month.toInt())
  {
  case 1:
    monthLong += "January";
    break;
  case 2:
    monthLong += "February";
    break;
  case 3:
    monthLong += "March";
    break;
  case 4:
    monthLong += "April";
    break;
  case 5:
    monthLong += "May";
    break;
  case 6:
    monthLong += "June";
    break;
  case 7:
    monthLong += "July";
    break;
  case 8:
    monthLong += "August";
    break;
  case 9:
    monthLong += "September";
    break;
  case 0:
    monthLong += "October";
    break;
  case 11:
    monthLong += "November";
    break;
  case 12:
    monthLong += "December";
    break;
  default:
    monthLong = String(month);
    Serial.println("Month formatting failed");
  }
  Serial.println(monthLong);

  dateLongString += String(monthLong);
  dateLongString += " ";
  dateLongString += String(dayLong);
  dateLongString += ", ";
  dateLongString += String(year);
  Serial.println(dateLongString);
}

/*
String appendToString(String appendData)
 {
 if(dataString == "")
 {
 dataString += String(appendData);
 }
 else
 {
 dataString += ",";
 dataString += String(appendData);
 }
 }
 */

// Write string containing all data to SD card (dataString)
void dataStringWriteSD()
{
  dataFile = SD.open("DATALOG.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing

  if(dataFile)  // If file on SD card is opened successfully
  {
    dataFile.println(dataString);  // Print dataString to it
    dataFile.close();  // Close the data file and proceed
    Serial.println(dataString);  // Print dataString to serial monitor to confirm values written
  }
  else
  {
    Serial.println("Error opening DATALOG.TXT!");  // If data file can't be opened, produce an error message
  }
}

// Clear all strings after writing to SD in preparation for next loop
void clearStrings()
{
  rawDateTime = "";  // Unparsed date and time string
  year = "";  // Parsed year
  month = "";  // Parsed month
  day = "";  // Parsed day
  yearLong = "";  // Long form of year
  monthLong = "";  // Long form of month
  dayLong = "";  // Long form of day
  dateString = "";  // Assembled date string
  timeString = "";  // Parsed time string
  dataString = "";  // String storing all data that was written to SD
  dateLongString = "";  // Long form of date
}
