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

#define buttonPin 2
#define tiltPin 3
#define tiltLED 4
#define buzzPin 5
#define buzzLED 6
#define vibPin 7
#define vibLED 8

int alertMode;

void setup() {
  Serial.begin(38400);

  pinMode(buttonPin, INPUT);
  pinMode(tiltPin, INPUT);
  pinMode(buzzLED, OUTPUT);
  pinMode(vibLED, INPUT);

  digitalWrite(buzzPin, LOW);
  digitalWrite(vibPin, LOW);

  alertMode = 1;
  digitalWrite(vibLED, HIGH);
}

void loop() {
  // Main monitoring loop
  if(tiltPin == 0) tiltAlert();
  // Button press response
  if(buttonPin == 0) modeSelect();
}

void modeSelect() {
      if(alertMode < 3) alertMode++;
    else alertMode = 1;
    
    
    
  while(buttonPin == 0) {
    delay(100);
  }
}

void tiltAlert() {
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
