//  Baby & Mother Co-sleeping Roll-over Alert  //

#define buttonPin 2
#define tiltPin 3
#define buzzPin 4
#define vibPin 5
#define buzzLED 6
#define vibLED 7
#define tiltLED 8
#define buzzVolPin A0
#define vibLevPin A1

int modeSelect = 0;

void setup() {
  Serial.begin(38400);
  
  pinMode(buttonPin, INPUT);
  pinMode(tiltPin, INPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(vibPin, OUTPUT);
  
  digitalWrite(buzzPin, LOW);
  digitalWrite(vibPin, LOW);
}

void loop() {
  
}
