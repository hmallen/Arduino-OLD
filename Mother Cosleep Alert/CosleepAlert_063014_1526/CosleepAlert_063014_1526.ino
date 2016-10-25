/*
//  Baby & Mother Co-sleeping Roll-over Alert  //
 Button functions:
 - Unit defaults to vibrate only on power-up
 - 1 button press changes to buzzer only
 - Next button press changes to buzzer & vibrate
 - Next button press cycles back to vibrate only
 - Holding button for 5 seconds powers-off unit
 
 Digital pins:
 - Vibrate mode LED indicator
 - Buzzer mode LED indicator
 - Tilt LED indicator
 
 Analog pins:
 - Vibrate output
 - Buzzer output
 - Vibration intensity potentiometer
 - Buzzer volume potentiometer
 
 Button mode selection program variables:
 0 - Startup (pre-loop)
 1 - Vib only
 2 - Buzz only
 3 - Vib & Buzz
 
 Feature immplementation ideas:
 - Low battery periodic vibration pulse & flashing LED
 - Variable vibration patterns
 - Variable buzzer patterns
 - Remote baby sleep position sensor/monitor
 */
 
#define buzzPin A0
#define vibPin A1

#define buttonPin 2
#define tiltPin 3
#define tiltLED 4
//#define buzzPin 5
#define buzzLED 6
//#define vibPin 7
#define vibLED 8

int alertMode;
int vibLevel = 1;
int buzzLevel = 1;
byte buzzVal = 0;
byte vibVal = 0;

boolean setLevStatus = false;

void setup() {
  Serial.begin(38400);

  pinMode(buttonPin, INPUT);
  pinMode(tiltPin, INPUT);
  pinMode(buzzLED, OUTPUT);
  pinMode(vibLED, INPUT);

  //digitalWrite(buzzPin, LOW);
  //digitalWrite(vibPin, LOW);
  
  analogWrite(buzzPin, 0);
  analogWrite(vibPin, 0);

  alertMode = 1;
  digitalWrite(vibLED, HIGH);
}

void loop() {
  // Main monitoring loop
  if(tiltPin == 0) tiltAlert();
  // Button press response
  if(buttonPin == 0) {
    setLevStatus = false;
    unsigned long holdTime = millis();
    while(buttonPin == 0) {
      if(millis() - holdTime > 3000) {
        setLevels();
      }
      delay(1);
    }
    delay(100);
    if(setLevStatus == false) modeSelect();
  }
}

void modeSelect() {
  if(alertMode < 3) alertMode++;
  else alertMode = 1;

  if(alertMode == 1) {
    digitalWrite(buzzLED, LOW);
    digitalWrite(vibLED, HIGH);
  }
  else if(alertMode == 2) {
    digitalWrite(vibLED, LOW);
    digitalWrite(buzzLED, HIGH);
  }
  else if(alertMode == 3) {
    digitalWrite(vibLED, HIGH);
    digitalWrite(buzzLED, HIGH);
  }
}

//  MCBEE FUCKED ME UP BADLY SO THIS SECTION NEEDS SERIOUS WORK 
void setLevels() {
  unsigned long prevFlash = millis();
  int flashInterval = 500;
  setLevStatus = true;
  while(buttonPin == 1) {
    if(millis() - prevFlash > flashInterval) {
      if(vibLED == LOW) vibLED = HIGH;
      else vibLED = LOW;
      prevFlash = millis();
    }
    if(selectPin == 0) {
      while(selectPin == 0) {
        delay(100);
      }
      if(vibLevel < 5) vibLevel++;
      else vibLevel = 1;
    }
  }
  while(buttonPin == 0) {
    delay(100);
  }
  while(buttonPin == 1) {
    if(millis() - prevFlash > flashInterval) {
      if(buzzLED == LOW) buzzLED = HIGH;
      else buzzLED = LOW;
      prevFlash = millis();
    }
    if(selectPin == 0) {
      while(selectPin == 0) {
        delay(100);
      }
      if(buzzLevel < 5) buzzLevel++;
      else buzzLevel = 1;
    }
  }
  while(buttonPin == 0) {
    delay(100);
  }
}

void tiltAlert() {
  digitalWrite(tiltLED, HIGH);
  if(alertMode == 1) {
    digitalWrite(vibPin, HIGH);
    while(tiltPin == 0) {
      delay(1);
    }
    digitalWrite(vibPin, LOW);
  }
  else if(alertMode == 2) {
    digitalWrite(buzzPin, HIGH);
    while(tiltPin == 0) {
      delay(1);
    }
    digitalWrite(buzzPin, LOW);
  }
  else if(alertMode == 3) {
    digitalWrite(vibPin, HIGH);
    digitalWrite(buzzPin, HIGH);
    while(tiltPin == 0) {
      delay(1);
    }
    digitalWrite(vibPin, LOW);
    digitalWrite(buzzPin, LOW);
    digitalWrite(tiltLED, LOW);
  }
  else errorFlash();
}

void errorFlash() {
  boolean errorState = true;
  while(errorState == true) {
    digitalWrite(buzzLED, HIGH);
    digitalWrite(vibLED, HIGH);
    digitalWrite(tiltLED, HIGH);
    delay(500);
    digitalWrite(buzzLED, LOW);
    digitalWrite(vibLED, LOW);
    digitalWrite(tiltLED, LOW);
    delay(500);
  }
}
