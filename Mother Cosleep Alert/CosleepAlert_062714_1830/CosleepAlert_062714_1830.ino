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

#define buzzVolPin A0
#define vibLevPin A1
#define buzzPin A2
#define vibPin A3
#define buttonPin 2
#define tiltPin 3
#define buzzLED 4
#define vibLED 5
#define tiltLED 6

int modeSelect;

void setup() {
  Serial.begin(38400);

  pinMode(buttonPin, INPUT);
  pinMode(tiltPin, INPUT);
  pinMode(buzzLED, OUTPUT);
  pinMode(vibLED, INPUT);

  digitalWrite(buzzLED, LOW);
  digitalWrite(vibLED, LOW);

  analogWrite(buzzPin, 0);
  analogWrite(vibPin, 0);

  modeSelect = 1;
}

void loop() {
  // Main monitoring loop
  while(buttonPin == 1) {
    if(modeSelect == 1) {
    }
    else if(modeSelect == 2) {
    }
    else if(modeSelect == 3) {
    }
    else 
  }
  // Button press response
  if(buttonPin == 0) {
    int holdTime = millis();
    if(modeSelect < 3) modeSelect++;
    else(modeSelect == 3) modeSelect = 1;
  }
  while(buttonPin == 0) {
    delay(100);
    if((millis() - holdTime) > 5000) powerOff();
  }
}

void powerOff() {
  // FUNCTION TO POWER-OFF ARDUINO AFTER BUTTON HOLD
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
