/* Read data file written to SD card */

#include <SD.h>

File myFile;

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
   pinMode(10, OUTPUT);
   
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  // re-open the file for reading:
  myFile = SD.open("NEWDATA.TXT");
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  }
   else {
    // if the file didn't open, print an error:
    Serial.println("Error opening file.");
  }
}

void loop()
{
	// nothing happens after setup
}


