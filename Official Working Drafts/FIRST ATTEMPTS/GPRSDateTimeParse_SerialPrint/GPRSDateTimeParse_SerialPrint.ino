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

#include <SoftwareSerial.h>

// Define pin constants
const int rxPin = 7;  // Receive pin for software serial
const int txPin = 8;  // Transmit pin for software serial
const int powPin = 9;  // Software power-up pin

SoftwareSerial gprsSerial(rxPin, txPin);  // Define software serial connection for GPRS

// Define strings for storing data as it is read/parsed/written
String rawDateTime;  // Create string for storing raw date and time info from GPRS

String day;  // Create string for storing raw day
String month;  // Create string for storing raw month
String year;  // Create string for storing raw year
String dateString;  // Create string for storing date from GPRS when function called

String timeString;  // Create string for storing time from GPRS when function called

String dayLong;   // Create string for assembling long form of day
String monthLong;  // Create string for assembling long form of month (month name)
String yearLong;  // Create string for assembling long form of year
String dateLongString;  // Create string for assembling long form of date

String dataString;  // Create string for storing all data to be written to SDString

void setup()
{ 
  // Define necessary pin modes
  pinMode(rxPin, INPUT);  // Software serial receive input
  pinMode(txPin, OUTPUT);  // Software serial transmiss output
  pinMode(powPin, OUTPUT);  // Software power-up output

  Serial.begin(19200);

  // Power up GPRS using software, if necessary
  powerUp();  // Call function to software power-on GPRS shield
  Serial.println("GPRS powered on.");

  gprsSerial.begin(19200);  // Initiate software serial connection for GPRS

  gprsBaselineSettings();  // Call function to set GPRS settings for proper response formatting
}

void loop()
{ 
  delay(2000);  // Delay between each sensor check and read and write loop

  getDateTime();  // Get GPRS date and time by calling function

  dataString += String(dateString);  // Add date to dataString
  dataString += ",";

  dataString += String(timeString);  // Add time to dataString

    Serial.println(dataString);

  dateLong();

  Serial.println(dateLongString);
  Serial.println();

  clearStrings();  // Call functions to clear all strings
}

///////////////////////////////////////////////////////////////////////////////////////////////
/////////<------------  All functions called in main loop defined below  ------------>/////////
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
  parseDateTime();  // Call function to parse date and time into individual components and form string
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
}

void dateLong()
{
  // Format year
  yearLong += "20";  // Add leading "20" to form long form of date
  yearLong += String(year);  // Append short date parsed from gprs data

  // Format day
  dayLong += day.toInt();  // Convert to integer function removes and leading 0's

  // Format month
  switch(month.toInt())  // Convert to integer and use switch case to convert month number to long form
  {
  case 0:
    monthLong += month.toInt();
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
  // Concatenate strings to form full long date
  dateLongString += String(monthLong);
  dateLongString += " ";
  dateLongString += String(dayLong);
  dateLongString += ", ";
  dateLongString += String(yearLong);
}


// Clear all strings in preparation for read on next loop
void clearStrings()
{
  rawDateTime = "";  // Unparsed date and time string

  day = "";  // Parsed day
  month = "";  // Parsed month
  year = "";  // Parsed year
  dateString = "";  // Assembled date string

  dayLong = "";  // Long form of day
  monthLong = "";  // Long form of month
  yearLong = "";  // Long form of year


  timeString = "";  // Parsed time string
  dateLongString = "";  // Long form of date

  dataString = "";  // String storing all data that was written to SD
}










