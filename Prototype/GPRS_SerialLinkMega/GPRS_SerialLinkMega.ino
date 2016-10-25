/*  Serial Relay - Arduino will patch a serial 
 link between the computer and the GPRS Shield  */

// Computer is connected to Hardware UART
// GPRS Shield is connected to the Software UART

#define powPin 9
#define bufMax 64

unsigned char buffer[bufMax];  // buffer array for data recieve over serial port
int count = 0;  // counter for buffer array

void setup() {
  // Define necessary pin modes
  pinMode(powPin, OUTPUT);

  Serial.begin(19200);
  Serial2.begin(19200);

  // Power up GPRS using software, if necessary
  Serial.print("Powering up GPRS...");
  powerUp();
  
  Serial.println("GPRS powered on.");
  Serial.println();

  Serial.println("GPRS connection initialized.");
  Serial.println();
}

void loop() {
  if(Serial2.available()) {  // If data is comming from softwareserial port ==> data is comming from gprs shield
    while(Serial2.available()) {  // Read data into char array
      buffer[count++] = Serial2.read();  // Write data into array
      if(count == bufMax)break;
    }
    Serial.write(buffer, count);  // If no data, transmission ends and writes buffer to hardware serial port
    clearBufferArray();  // Call clearBufferArray function to clear the storaged data from the array
    count = 0;  // Set counter of while loop to zero
  }
  if(Serial.available()) {  // If data is available on hardwareserial port ==> data is comming from PC or notebook
    Serial2.write(Serial.read());  // Write it to the GPRS shield
  }
}

void powerUp() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

void clearBufferArray() {  // Function to clear buffer array
  for (int i = 0; i < count; i++) { 
    buffer[i] = NULL;  // Clear all index of array with command NULL
  }
}
