#include <SdFat.h>
#include <SoftwareSerial.h>

#define chipSelect 10
#define recPin A0
#define recLED A1
#define landPin A2
#define landLED A3
#defin targetNumb "2145635266"

SdFat sd;
SdFile dataFile;

SoftwareSerial sensSerial(5, 6);
SoftwareSerial gprsSerial(7, 8);

String gpsLat;
String gpsLon;

boolean recState = false;
boolean landState = false;

/*void powerUp() {
 digitalWrite(powPin, LOW);
 delay(100);
 digitalWrite(powPin, HIGH);
 delay(500);
 digitalWrite(powPin, LOW);
 delay(100);
 }*/

void setup() {
  Serial.begin(19200);
  sensSerial.begin(19200);
  gprsSerial.begin(19200);

  pinMode(chipSelect, OUTPUT);
  pinMode(recPin, INPUT);
  pinMode(recLED, OUTPUT);
  pinMode(landPin, INPUT);
  pinMode(landLED, OUTPUT);

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();
}

void loop() {
  stateCheck();
  if(recState == true && landState == false) {
    if(dataFile.open("SENSORS1.TXT", O_RDWR | O_CREAT | O_AT_END)) {
      while(sensSerial.available()) {
        dataFile.write(sensSerial.read());
        if(stateCheck() == false) break;
      }
      dataFile.close();
    }
    else {
      sd.errorHalt("Failed to open file.");
    }
  }
  else if(landState == true) {
    while(!sensSerial.available()) {
      delay(1);
    }
    gpsReceiveData();
  }
}

void stateCheck() {
  if(digitalRead(recPin) == 0) {
    recState = !recState;
    while(digitalRead(recPin) == 0) {
      delay(100);
    }
    if(recState == true) {
      digitalWrite(recLED, HIGH);
    }
    else {
      digitalWrite(recLED, LOW);
    }
  }
  if(digitalRead(landPin) == HIGH) {
    landState = true;
    digitalWrite(landLED, HIGH);
  }
}

void gpsReceiveData() {
  while(sensSerial.available()) {
    char c = sensSerial.read();
    String gpsCoord += c;
  }
  for(int lat = 0; lat < VALUE; lat++) {
    gpsLat += String(gpsCoord.charAt(lat));
  }
  for(int lon = VALUE; lon < VALUE-2; lon++) {
    gpsLon += String(gpsCoord.charAt(lon));
  }
}

void smsLocation() {
  gprsSerial.println("AT+CMGF=1");
  delay(100);
  gprsSerial.print("AT+CMGS=\"+1");
  gprsSerial.print(targetNumb);
  gprsSerial.println("\"");
  delay(100);
  gprsSerial.print("https://maps.google.com/maps?q=");
  gprsSerial.print("(Landing+location)@");
  gprsSerial.print(gpsLat, 8);
  gprsSerial.print(",+");
  gprsSerial.print(gpsLon, 8);
  gprsSerial.println("&t=h&z=18&output=embed");
  delay(100);
  gprsSerial.println((char)26);
  delay(100);
  gprsSerial.println();

  Serial.println("Message sent.");
  Serial.println();
}
