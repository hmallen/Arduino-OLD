#include <SoftwareSerial.h>

#define rxPin 7
#define txPin 8
#define powPin 9

SoftwareSerial gprsSerial(rxPin, txPin);

const int bufMax = 64;

unsigned char buffer[bufMax];  // buffer array for data recieve over serial port
int count = 0;  // counter for buffer array

String DateTime;

void setup()
{
  // Define necessary pin modes
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);

  Serial.begin(19200); // Initiate hardware serial

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  Serial.println("GPRS powered on.");
  Serial.println();

  gprsSerial.begin(19200);

  Serial.println("GPRS connection initialized.");
  Serial.println();
}

void loop()
{
  delay(1000);

  String DateTime = "";

  gprsSerial.println("AT+CCLK?");

  if(gprsSerial.available())  // If date is comming from softwareserial port ==> data is comming from gprs shield
  {
    while(gprsSerial.available())  // Read data into char array
    {
      //buffer[count++] = gprsSerial.read();  // Write data into array
      //if(count == bufMax)break;

      char c = gprsSerial.read();

      DateTime += c;
    }
    Serial.print(DateTime);
    //Serial.write(buffer, count);  // If no data, transmission ends and writes buffer to hardware serial port
    //clearBufferArray();  // Call clearBufferArray function to clear the storaged data from the array
    //count = 0;  // Set counter of while loop to zero

    delay(3000);
  }
  //delay (5000);
  //if(Serial.available())  // If data is available on hardwareserial port ==> data is comming from PC or notebook
  //gprsSerial.write(Serial.read());  // Write it to the GPRS shield
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
