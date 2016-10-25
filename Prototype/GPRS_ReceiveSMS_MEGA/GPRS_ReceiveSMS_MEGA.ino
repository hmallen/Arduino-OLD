/*
SMS actions
 
 To Do:
 - Add record of smsIndex to EEPROM for reference after reset
 */

#define gprsPowPin 9  // Pin for software power-up of GPRS shield  

const char smsTargetNum[11] = "2145635266";  // Mobile number to send data via SMS

const int smsLED = A0;
const int piezoPin = A3;
const int smokePin = A8;

String smsMessageRaw;
String smsRecNumber;
String smsMessage;

void setup() {
  pinMode(smsLED, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(smokePin, OUTPUT);
  digitalWrite(smsLED, LOW);
  digitalWrite(piezoPin, LOW);
  digitalWrite(smokePin, LOW);

  Serial.begin(19200);
  Serial2.begin(19200);

  digitalWrite(gprsPowPin, LOW);
  delay(100);
  digitalWrite(gprsPowPin, HIGH);
  delay(500);
  digitalWrite(gprsPowPin, LOW);
  delay(100);

  Serial2.println("ATE0");
  delay(100);
  Serial2.println("ATQ1");
  delay(100);
  Serial2.println("ATV0");
  delay(100);
  Serial2.println("AT+CMGF=1");
  delay(100);
  Serial2.println("AT+CNMI=2,2,0,0,0");
  delay(100);
}

void loop() {
  if(Serial2.available()) {  // If data is coming from GPRS
    smsMessageRaw = "";
    while(Serial2.available()) {  // Read the data into string from incoming bytes while they're available
      char c = Serial2.read();  // Read each byte sent, one at a time, into storage variable
      smsMessageRaw += c;
      delay(10);
    }
    Serial.println(F("----------------------------------------------"));
    Serial.print(F("Raw: "));
    Serial.println(smsMessageRaw);
    parseSMS();
    Serial.print(F("Num: "));
    Serial.println(smsRecNumber);
    Serial.print(F("SMS: "));
    Serial.println(smsMessage);
    Serial.print(F("smsMessageLength: "));
    Serial.println(smsMessage.length());

    if(smsRecNumber == smsTargetNum) executeSMSCommand();
    else Serial.println(F("Sender's number of SMS does not match expected. Aborting."));
  }
  delay(1);
}

void parseSMS() {
  smsRecNumber = "";
  smsMessage = "";
  int numIndex = smsMessageRaw.indexOf('"') + 3;
  Serial.print(F("numIndex: "));
  Serial.println(numIndex);
  int smsIndex = smsMessageRaw.lastIndexOf('"') + 3;
  Serial.print(F("smsIndex: "));
  Serial.println(smsIndex);
  for(numIndex; ; numIndex++) {
    char c = smsMessageRaw.charAt(numIndex);
    if(c == '"') break;
    smsRecNumber += c;
  }
  for(smsIndex; ; smsIndex++) {
    char c = smsMessageRaw.charAt(smsIndex);
    if(c == '\n' || c == '\r') break;
    smsMessage += c;
  }
}

void executeSMSCommand() {
  int smsCommand = 0;
  if(smsMessage.length() == 1) smsCommand = smsMessage.toInt();
  // Wait for SMS
  // If SMS received, parse...start with "$"
  // Convert receive buffer to correct data type
  // Switch case to perform requested action

  // Some sort of "if data available, then proceed to switch case"
  switch(smsCommand) {
  case 1:
    Serial.println(F("LED activated!"));
    // Retrieve updated GPS coordinates and send via SMS
    digitalWrite(smsLED, HIGH);
    delay(1000);
    digitalWrite(smsLED, LOW);
    break;
  case 2:
    Serial.println(F("Piezo buzzer activated!"));
    digitalWrite(piezoPin, HIGH);
    delay(50);
    digitalWrite(piezoPin, LOW);
    break;
  case 3:
    // Trigger smoke signal
    Serial.println(F("Smoke signal activated!"));
    digitalWrite(smokePin, HIGH);
    delay(2000);
    digitalWrite(smokePin, LOW);
    break;
  default:
    Serial.print(F("INVALID COMMAND: "));
    Serial.println(smsMessage);
    break;
  }
}
