/*  Serial Relay - Arduino will patch a serial 
 link between the computer and the GPRS Shield  */

// Computer is connected to Hardware UART
// GPRS Shield is connected to the Software UART

#include <SoftwareSerial.h>

#define rxPin 7
#define txPin 8
#define powPin 9

#define bufMax 64

SoftwareSerial smsSerial(rxPin, txPin);

unsigned char buffer[bufMax];  // buffer array for data recieve over serial port
int count = 0;  // counter for buffer array

void setup()
{
  // Define necessary pin modes
  //pinMode(rxPin, INPUT);
  //pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);

  Serial.begin(19200); // Initiate hardware serial

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  Serial.println("GPRS powered on.");
  Serial.println();

  smsSerial.begin(19200);

  Serial.println("GPRS connection initialized.");
  Serial.println();
}

void loop()
{
  if(smsSerial.available())  // If date is comming from softwareserial port ==> data is comming from gprs shield
  {
    while(smsSerial.available())  // Read data into char array
    {
      buffer[count++] = smsSerial.read();  // Write data into array
      if(count == bufMax)break;
    }
    Serial.write(buffer, count);  // If no data, transmission ends and writes buffer to hardware serial port
    clearBufferArray();  // Call clearBufferArray function to clear the storaged data from the array
    count = 0;  // Set counter of while loop to zero
  }
  if(Serial.available())  // If data is available on hardwareserial port ==> data is comming from PC or notebook
    smsSerial.write(Serial.read());  // Write it to the GPRS shield
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

void clearBufferArray()  // Function to clear buffer array
{
  for (int i = 0; i < count; i++)
  { 
    buffer[i] = NULL;  // Clear all index of array with command NULL
  }
}


