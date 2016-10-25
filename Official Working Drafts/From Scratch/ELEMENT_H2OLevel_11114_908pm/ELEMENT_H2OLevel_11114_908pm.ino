/* -- Official working draft of the MyStall prototype --
 
 All called functions are defined at the end of the sketch, immediately following
 the main loop. Pay attention to the clearStrings() function, as it must be updated
 with any new strings added to the sketch for them to be cleared properly after
 each loop.
 
 Current shields:
 
 
 Current sensors:
 - Water level
 
 Important notes:
 - In the Arduino core libraries, SoftwareSerial.h has been modified to increase max
 buffer size from 64 --> 256 to accomodate longer reads from GPRS over the software
 serial connection.
 - In working prototype, PIR must be warmed up for ~20 seconds on the first boot for
 proper function. */

// Libraries


// Globally available variables
int h2oLevel = 0;  // Water level reading from sensor

void setup() {
  Serial.begin(19200);
}

void loop() {
  h2oLevelRead();
  Serial.println(h2oLevel);
  delay(1000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////<------------  All functions called in main loop defined below  ------------>/////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

// Water level in bucket (mimicked by potentiometer voltage reading for prototyping)
void h2oLevelRead() {
  int h2oReadVal = analogRead(A0);
  h2oLevel = h2oReadVal / 9.22;  // Divide by (MaximumPossibleReading / 100) for 0-100 scale
}
