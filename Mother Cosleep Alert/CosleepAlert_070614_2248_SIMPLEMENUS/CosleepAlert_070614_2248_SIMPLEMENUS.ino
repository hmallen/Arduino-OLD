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

#define debugMode

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
boolean vibSetLevel = false;
boolean buzzSetLevel = false;
boolean vibSetMode = false;
boolean buzzSetMode = false;

#ifdef debugMode
int buttonReadPrev;
int selectReadPrev;
#endif

void setup() {
  Serial.begin(38400);

  pinMode(buttonPin, INPUT);
  pinMode(selectPin, INPUT);
  pinMode(tiltPin, INPUT);
  pinMode(buzzLED, OUTPUT);
  pinMode(vibLED, OUTPUT);
  pinMode(tiltLED, OUTPUT);

  //digitalWrite(buzzPin, LOW);
  //digitalWrite(vibPin, LOW);

  analogWrite(buzzPin, 0);
  analogWrite(vibPin, 0);

  modeSelect();

#ifdef debugMode
  buttonReadPrev = digitalRead(2);
  selectReadPrev = digitalRead(3);
#endif
}

void loop() {
  // Debugging functions
#ifdef debugMode
  int buttonRead = digitalRead(buttonPin);
  if(buttonRead != buttonReadPrev) {
    Serial.print(F("Button pin: "));
    Serial.println(buttonRead);
    buttonReadPrev = buttonRead;
  }
  int selectRead = digitalRead(selectPin);
  if(selectRead != selectReadPrev) {
    Serial.print(F("Select pin: "));
    Serial.println(selectRead);
    selectReadPrev = selectRead;
  }
#endif
  // Main monitoring loop
  if(digitalRead(tiltPin) == 1) tiltAlert();
  // Button press response
  if(digitalRead(buttonPin) == 0) {
    unsigned long holdTime = millis();
    while(digitalRead(buttonPin) == 0) {
      if(millis() - holdTime > 3000) {
        setLevels();
      }
      delay(1);
    }
    delay(100);
    if(setLevStatus == false) modeSelect();
    else setLevStatus = false;
  }
  delay(100);
}

void modeSelect() {
#ifdef debugMode
  Serial.println(F("modeSelect() triggered."));
#endif
  if(startupSet == true) {
    alertMode = 1;
    startupSet = false;
  }
  else {
    if(alertMode < 3) alertMode++;
    else alertMode = 1;
  }

#ifdef debugMode
  Serial.println(alertMode);
#endif

  ledConfig();
}

void ledConfig() {
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

void setLevels() {
#ifdef debugMode
  Serial.println(F("setLevels() triggered."));
#endif
  int flashInterval = 500;
  boolean vibSet = false;
  boolean buzzSet = false;

  digitalWrite(tiltLED, HIGH);
  digitalWrite(vibLED, LOW);
  digitalWrite(buzzLED, LOW);

  setLevStatus = true;

  while(digitalRead(buttonPin) == 0) {
    delay(100);
  }
  delay(100);

  // Vibration level selection
  unsigned long prevFlash = millis();
  while(vibSet == false) {
    if(millis() - prevFlash > flashInterval) {
      if(digitalRead(vibLED) == LOW) digitalWrite(vibLED, HIGH);
      else digitalWrite(vibLED, LOW);
      prevFlash = millis();
    }
    if(digitalRead(selectPin) == 0) {
      while(digitalRead(selectPin) == 0) {
        delay(100);
      }
      if(vibLevel < 5) vibLevel++;
      else vibLevel = 1;
#ifdef debugMode
      Serial.print(F("Vib Level: "));
      Serial.println(vibLevel);
#endif
      vibSetLevel = true;
      levelTranslate();
      vibLevDemonstrate();
    }
    if(digitalRead(buttonPin) == 0) vibSet = true;
  }
  digitalWrite(vibLED, LOW);
  while(digitalRead(buttonPin) == 0) {
    delay(100);
  }
  delay(100);

  // Buzzer level selection
  prevFlash = millis();
  while(buzzSet == false) {
    if(millis() - prevFlash > flashInterval) {
      if(digitalRead(buzzLED) == LOW) digitalWrite(buzzLED, HIGH);
      else digitalWrite(buzzLED, LOW);
      prevFlash = millis();
    }
    if(digitalRead(selectPin) == 0) {
      while(digitalRead(selectPin) == 0) {
        delay(100);
      }
      if(buzzLevel < 5) buzzLevel++;
      else buzzLevel = 1;
#ifdef debugMode
      Serial.print(F("Buzz Level: "));
      Serial.println(buzzLevel);
#endif
      buzzSetLevel = true;
      levelTranslate();
      buzzLevDemonstrate();
    }
    if(digitalRead(buttonPin) == 0) buzzSet = true;
  }
  while(digitalRead(buttonPin) == 0) {
    delay(100);
  }
  delay(100);

  /*while(buttonPin == 1) {
   if(millis() - prevFlash > flashInterval) {
   if(digitalRead(buzzLED) == LOW) digitalWrite(buzzLED, HIGH);
   else digitalWrite(buzzLED, LOW);
   prevFlash = millis();
   }
   // Buzzer level setting
   if(digitalRead(selectPin) == 0) {
   while(digitalRead(selectPin) == 0) {
   delay(100);
   }
   if(buzzLevel < 5) buzzLevel++;
   else buzzLevel = 1;
   #ifdef debugMode
   Serial.print(F("Buzz Level: "));
   Serial.println(buzzLevel);
   #endif
   buzzSetLevel = true;
   levelTranslate();
   buzzLevDemonstrate();
   }
   }
   while(digitalRead(buttonPin) == 0) {
   delay(100);
   }*/

  digitalWrite(tiltLED, LOW);
  ledConfig();
#ifdef debugMode
  Serial.println(F("setLevels() exited."));
#endif
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
    //analogWrite(vibPin, vibLevel);
    analogWrite(vibPin, 255);
    while(digitalRead(tiltPin) == 1) {
      delay(1);
    }
    analogWrite(vibPin, 0);
  }
  else if(alertMode == 2) {
    //analogWrite(buzzPin, buzzLevel);
    analogWrite(buzzPin, 255);
    while(digitalRead(tiltPin) == 1) {
      delay(1);
    }
    analogWrite(buzzPin, 0);
  }
  else if(alertMode == 3) {
    //analogWrite(vibPin, vibLevel);
    analogWrite(vibPin, 255);
    //analogWrite(buzzPin, buzzLevel);
    analogWrite(buzzPin, 255);
    while(digitalRead(tiltPin) == 1) {
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
  
  delay(3000);  // Delay to allow repositioning before retriggering alert
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
