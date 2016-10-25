/*
//  Baby & Mother Co-sleeping Roll-over Alert  //
 Button functions:
 - Unit defaults to vibrate only on power-up
 - 1 button press changes to buzzer only
 - Next button press changes to buzzer & vibrate
 - Next button press cycles back to vibrate only
 - Holding button for 5 seconds powers-off unit
 
 Digital pins:
 - Mode select (tactile push-button)
 - Level select (tactile push-button)
 - Vibrate mode LED indicator
 - Buzzer mode LED indicator
 - Tilt LED indicator
 
 Analog pins:
 - Vibrate output
 - Buzzer output
 
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
#define selectPin 3
#define tiltPin 4
#define tiltLED 5
//#define buzzPin 5
//#define buzzLED 6
//#define vibPin 7
//#define vibLED 8
#define buzzLED 6
#define vibLED 7

int alertMode = 1;
int vibLevel = 1;
int buzzLevel = 1;
byte buzzVal = 0;
byte vibVal = 0;

boolean startupSet = true;
boolean setLevStatus = false;
boolean vibSetMode = false;
boolean buzzSetMode = false;

void setup() {
  Serial.begin(38400);

  pinMode(buttonPin, INPUT);
  pinMode(tiltPin, INPUT);
  pinMode(buzzLED, OUTPUT);
  pinMode(vibLED, OUTPUT);

  //digitalWrite(buzzPin, LOW);
  //digitalWrite(vibPin, LOW);

  analogWrite(buzzPin, 0);
  analogWrite(vibPin, 0);

  modeSelect();
  startupSet = false;
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
  if(startupSet == false) {
    if(alertMode < 3) alertMode++;
    else alertMode = 1;
  }

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
    // Vibration level setting
    if(selectPin == 0) {
      while(selectPin == 0) {
        delay(100);
      }
      if(vibLevel < 5) vibLevel++;
      else vibLevel = 1;
      vibSetLevel = true;
      levelTranslate();
      vibLevDemonstrate();
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
    // Buzzer level setting
    if(selectPin == 0) {
      while(selectPin == 0) {
        delay(100);
      }
      if(buzzLevel < 5) buzzLevel++;
      else buzzLevel = 1;
      buzzSetLevel = true;
      levelTranslate();
      buzzLevDemonstrate();
    }
  }
  while(buttonPin == 0) {
    delay(100);
  }
}

void levelTranslate() {
  if(vibSetLevel == true) {
    if(vibVal == 1) vibLevel = 51;
    else if(vibVal == 2) vibLevel = 102;
    else if(vibVal == 3) vibLevel = 153;
    else if(vibVal == 4) vibLevel = 204;
    else if(vibVal == 5) vibLevel = 255;
    vibSetLevel = false;
  }
  else if(buzzSetLevel == true) {
    if(buzzVal == 1) buzzLevel = 51;
    else if(buzzVal == 2) buzzLevel = 102;
    else if(buzzVal == 3) buzzLevel = 153;
    else if(buzzVal == 4) buzzLevel = 204;
    else if(buzzVal == 5) buzzLevel = 255;
    buzzSetLevel = false;
  }
}

void vibLevDemonstrate() {
  analogWrite(vibPin, vibLevel);
  delay(500);
  analogWrite(vibPin, 0);
}

void buzzLevDemonstrate() {
  analogWrite(buzzPin, buzzLevel);
  delay(500);
  analogWrite(buzzPin, 0);
}

void tiltAlert() {
  digitalWrite(tiltLED, HIGH);
  if(alertMode == 1) {
    analogWrite(vibPin, vibLevel);
    while(tiltPin == 0) {
      delay(1);
    }
    analogWrite(vibPin, 0);
  }
  else if(alertMode == 2) {
    analogWrite(buzzPin, buzzLevel);
    while(tiltPin == 0) {
      delay(1);
    }
    analogWrite(buzzPin, 0);
  }
  else if(alertMode == 3) {
    analogWrite(vibPin, vibLevel);
    analogWrite(buzzPin, buzzLevel);
    while(tiltPin == 0) {
      delay(1);
    }
    analogWrite(vibPin, 0);
    analogWrite(buzzPin, 0);
  }
  /*if(alertMode == 1) {
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
   }*/
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
