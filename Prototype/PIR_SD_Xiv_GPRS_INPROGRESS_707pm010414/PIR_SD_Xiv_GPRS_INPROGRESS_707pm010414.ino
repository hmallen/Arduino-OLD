#include <SD.h>
#include <SoftwareSerial.h>

// GPRS shield constants for software serial connection, power-on, and baud rate
const int rxPin = 7;
const int txPin = 8;
const int powPin = 9;  // Provides software power-on capability
const int gprsBaud = 19200;  // Baud rate of GPRS serial (default = 19200 / set via AT)

// Buffer and count to write GPRS data into array
const int bufMax = 256;
unsigned char buffer[bufMax];  // buffer array for data recieve over serial port
int count = 0;  // counter for buffer array

SoftwareSerial gprsSerial(rxPin, txPin);  // Define software serial connection for GPRS

const int csPin = 10;  // Chip select for SD shield

// Define pin constants for sensor(s)
const int dispPin = 4;  // LED indicating state of motion sensor
const int pirPin = 2;  // Motion sensor pin

// Define storage variables
int pir = 0;  // Variable to store the read value
int pir1 = 0;  // Variable to store the previous value for comparison

File dataFile;  // Define variable as a file for data logging

//String DateTime = "";  // String to store date/time info retrieved from GPRS

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

  gprsSerial.begin(gprsBaud);  // Initiate software serial connection for GPRS

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

  pinMode(dispPin, OUTPUT);  // Sets digital pin 4 as output for LED indicator
  pinMode(pirPin, INPUT);  // Sets digital pin 2 as input for PIR sensor
}

void loop()
{
  String dataString = "";

  pir = digitalRead(pirPin);

  if(pir != pir1)
  {  
    Serial.print("GPRS date/time stored in buffer: ");
    gprsDateTime();
    delay(50);
    Serial.println();

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

void gprsDateTime()
{
  gprsSerial.write("AT+CCLK?");

  if(gprsSerial.available())  // If date is comming from softwareserial port ==> data is comming from gprs shield
  {
    while(gprsSerial.available())  // Read data into char array
    {
      buffer[count++] = gprsSerial.read();  // Write data into array
      if(count == bufMax)break;
    }
    Serial.write(buffer, count);  // If no data, transmission ends and writes buffer to hardware serial port
    clearBufferArray();  // Call clearBufferArray function to clear the storaged data from the array
    count = 0;  // Set counter of while loop to zero
  }
}
/*
// Function to retrieve GPRS date and time info
 void gprsDateTime()
 {
 String DateTime = "";  // Create empty string for storing date and time info received from GPRS
 
 gprsSerial.println("AT+CCLK?");  // AT: Ask for date and time info
 
 if(gprsSerial.available())  // If data is coming from software serial port, data is coming from GPRS shield
 {
 while(gprsSerial.available())  // Read data into char array
 {
 buffer[count++] = gprsSerial.read();  // Write data into array
 if(count == bufMax)break;
 }
 Serial.write(buffer, count);  // If no data, transmission ends and writes buffer to hardware serial port
 
 //DateTime = buffer[count];
 
 clearBufferArray();  // Call clearBufferArray function to clear the storaged data from the array
 count = 0;  // Set counter of while loop to zero
 }
 //if(Serial.available())  // If data is available on hardware serial port, data is coming from PC or notebook
 //gprsSerial.write(Serial.read());  // Write it to the GPRS shield
 }
 */
// Function to clear GPRS buffer array after writing
void clearBufferArray()  // Function to clear buffer array
{
  for (int i = 0; i < count; i++)
  { 
    buffer[i] = NULL;  // Clear all index of array with command NULL
  }
}

