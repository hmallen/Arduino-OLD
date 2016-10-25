//#include <SoftwareSerial.h>

#define debugMode

#define rxPin 7
#define txPin 8
#define powPin 22

#define bufMax 256

unsigned char buffer[bufMax];
int count = 0;

String smsNumber, smsMessage;

//SoftwareSerial Serial2 (rxPin, txPin);

unsigned long smsDelayStart = millis();

boolean newData = false;

void setup() {
  // Define necessary pin modes
  //pinMode(rxPin, INPUT);
  //pinMode(txPin, OUTPUT);
  pinMode(powPin, OUTPUT);

  Serial.begin(38400);

  Serial2.begin(19200);
  delay(100);
  powerUp();
  const int smsStartupDelay = 6000;
  smsDelayStart = millis();
  boolean firstLoop = true;
  for ( ; (millis() - smsDelayStart) < smsStartupDelay; ) {
    String startupBuffer = "";
    if (Serial2.available()) {
      while (Serial2.available()) {
        char c = Serial2.read();
        startupBuffer += c;
        delay(10);
      }
#ifdef debugMode
      if (firstLoop == true) {
        Serial.println(F("SMS Buffer Flush:"));
        firstLoop = false;
      }
      Serial.println(startupBuffer);
#endif
      smsDelayStart = millis();
    }
    delay(10);
  }
  configureSMS();
  Serial.println(F("SMS ready."));
}

void loop() {
  const int smsCheckDelay = 10000;
  smsDelayStart = millis();
  while ((millis() - smsDelayStart) < smsCheckDelay) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '1') checkSMS();
    }
    delay(10);
  }
  checkSMS();
  if (newData == true) {
#ifdef debugMode
    Serial.print(F("smsNumber:  "));
    Serial.println(smsNumber);
    Serial.print(F("smsMessage: "));
    Serial.println(smsMessage);
#endif
    newData = false;
  }
}

void checkSMS() {
  /*String smsIndex;
  if (Serial2.available()) {
    String smsRecString = "";
    smsNumber = "";
    smsMessage = "";
    while (Serial2.available()) {
      char c = Serial2.read();
      if (c == ',') {
        smsIndex = "";
        for (int x = 0; Serial2.available(); x++) {
          char c = Serial2.read();
          if (c == '\r' || c == '\n') break;
          smsIndex += c;
          delay(10);
        }
      }
      delay(10);
    }
    Serial2.print(F("AT+CMGR="));
    Serial2.println(smsIndex);
    delay(1000);
    if (Serial2.available()) {
      int quoteCount = 0;
      while (Serial2.available()) {
        char c = Serial2.read();
        if (c == '"') quoteCount++;
        if (quoteCount == 3) {
          for (int x = 0; x < 13; x++) {
            c = Serial2.read();
            if (c == '"') {
              quoteCount++;
              break;
            }
            smsNumber += c;
            delay(10);
          }
          Serial.println(smsNumber);
        }
        if (quoteCount == 8) {
          for (int x = 0; x < 2; x++) {
            c = Serial.read();
            delay(10);
          }
          while (Serial2.available()) {
            c = Serial2.read();
            if (c == '\n') Serial.println(F("NL"));
            else if (c == '\r') Serial.println(F("CR"));
            else Serial.write(c);
            smsMessage += c;
            delay(100);
          }
          Serial.print(F("smsMessage: "));
          Serial.println(smsMessage);
        }
        delay(10);
      }
    }
  }*/
}

void parseSMS(String smsRaw, int smsLength) {
  // Numbers permitted for SMS menu use
  String smsTargetNum0 = "+12145635266";  // HA
  String smsTargetNum1 = "+12146738002";  // PK
  String smsTargetNum2 = "+12143155885";  // GK
  String smsTargetNum3 = "+12147070200";  // TK
  String smsTargetNum4 = "+12146738003";  // MK
  String smsTargetNum5 = "+18327790024";  // Google Voice (HA)

  smsNumber = "";
  smsMessage = "";
  for (int x = (smsRaw.indexOf('+', 2)); ; x++) {
    char c = smsRaw.charAt(x);
    if (c == '\"') break;
    smsNumber += c;
  }
  for (int x = (smsRaw.lastIndexOf('\r') + 1); x < (smsLength + 1); x++) {
    char c = smsRaw.charAt(x);
    smsMessage += c;
  }
  if (smsNumber == smsTargetNum0 ||
      smsNumber == smsTargetNum1 ||
      smsNumber == smsTargetNum2 ||
      smsNumber == smsTargetNum3 ||
      smsNumber == smsTargetNum4 ||
      smsNumber == smsTargetNum5) smsMenu();
  else {
#ifdef debugMode
    Serial.println(F("SMS number not recognized."));
#endif
  }
}

void smsMenu() {
  int smsCommandPos = smsMessage.indexOf('@') + 1;
  if (smsCommandPos == -1) {
#ifdef debugMode
    Serial.println(F("Invalid command."));
#endif
  }
  else {
    int smsCommand = int(smsMessage.charAt(smsCommandPos)) - 48;
    switch (smsCommand) {
      case 0:
        Serial.println(F("0"));
        break;
      case 1:
        Serial.println(F("1"));
        break;
      case 2:
        Serial.println(F("2"));
        break;
      default:
        break;
    }
  }
}

void smsFlush() {
  if (Serial2.available()) {
    while (Serial2.available()) {
      char c = Serial2.read();
      delay(100);
    }
  }
}

//// Configuration functions ////

void powerUp() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

void configureSMS() {
  // Configure GPRS output for proper parsing
  Serial2.println(F("ATE0"));
  delay(1000);
  Serial2.println(F("ATQ1"));
  delay(1000);
  Serial2.println(F("ATV0"));
  delay(1000);
  Serial2.println(F("AT+CMGF=1"));
  delay(1000);
  Serial2.flush();
  smsFlush();
}

void clearBufferArray() {  // Function to clear buffer array
  for (int i = 0; i < count; i++) {
    buffer[i] = NULL;  // Clear all index of array with command NULL
  }
}
