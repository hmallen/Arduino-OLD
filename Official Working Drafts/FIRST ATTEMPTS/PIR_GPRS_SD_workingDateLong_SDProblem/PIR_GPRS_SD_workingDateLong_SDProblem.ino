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
String dataString;  // Create string for storing all data to be written to SDString rawDateTime;

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
  delay(2000);  // Delay between each sensor check and read and write loop

  getDateTime();  // Get GPRS date and time by calling function

  dataString += String(dateLongString);
  dataString += ": ";

  dataString += String(dateString);  // Add date to dataString
  dataString += ",";

  dataString += String(timeString);  // Add time to dataString
  dataString += ",";

  pir = digitalRead(pirPin);  // Retrieve value from PIR sensor
  digitalWrite(dispPin, pir);  // Set LED state to match sensor value

  dataString += String(pir);  // Add PIR sensor value to dataString

  //Serial.println(dateLongString);
  Serial.println(dataString);

  delay(2000);

  writeDataSD();

  clearStrings();  // Call function to clear all strings
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

void parseDateTime()
{
  for(int y = 8; y < 10; y++)
  {
    year += String(rawDateTime.charAt(y));
  }
  for(int m = 11; m < 13; m++)
  {
    month += String(rawDateTime.charAt(m));
  }
  for(int d = 14; d < 16; d++)
  {
    day += String(rawDateTime.charAt(d));
  }
  for(int t = 17; t < 25; t++)
  {
    timeString += String(rawDateTime.charAt(t));
  }
  dateString += String(month);
  dateString += "/";
  dateString += String(day);
  dateString += "/";
  dateString += String(year);

  dateLong();
}

void dateLong()
{
  // Format year
  yearLong += "20";  // Add leading "20" to form long form of date
  yearLong += String(year);  // Append short date parsed from gprs data

  // Format day
  dayLong += day.toInt();  // Convert to integer function removes and leading 0's

  //constMonthLong();  // Call function to create long form of month from integer
  monthLong += month.toInt();

  // Concatenate strings to form full long date
  dateLongString += String(monthLong);
  dateLongString += " ";
  dateLongString += String(dayLong);
  dateLongString += ", ";
  dateLongString += String(yearLong);
}

/*void constMonthLong()
 { 
 switch(month.toInt())  // Convert to integer and use switch case to convert month number to long form
 {
 case 0:
 break;
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
 case 10:
 monthLong += "October";
 break;
 case 11:
 monthLong += "November";
 break;
 case 12:
 monthLong += "December";
 break;
 }
 }*/

void writeDataSD()
{
  File dataFile = SD.open("DATATEST.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing

  if(dataFile)  // If file on SD card is opened successfully
  {
    //dataFile.println(dateLongString);

    dataFile.println(dataString);  // Print dataString to it
    delay(1000);

    dataFile.close();  // Close the data file and proceed
  }
  else
  {
    Serial.println("Error opening DATALOG.TXT!");  // If data file can't be opened, produce an error message
    Serial.println();
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








