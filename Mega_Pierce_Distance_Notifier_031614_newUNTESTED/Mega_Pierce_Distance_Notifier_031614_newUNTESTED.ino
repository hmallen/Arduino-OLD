#include <TinyGPS.h>
#include <LiquidCrystal.h>
#define powPin 30

TinyGPS gps;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

/*int lcd_key     = 0;
 int adc_key_in  = 0;
 #define btnRIGHT  0
 #define btnUP     1
 #define btnDOWN   2
 #define btnLEFT   3
 #define btnSELECT 4
 #define btnNONE   5
 
 int read_LCD_buttons()
 {
 adc_key_in = analogRead(0);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 if (adc_key_in < 50)   return btnRIGHT; 
 if (adc_key_in < 195)  return btnUP;
 if (adc_key_in < 380)  return btnDOWN;
 if (adc_key_in < 555)  return btnLEFT;
 if (adc_key_in < 790)  return btnSELECT;  
 return btnNONE;  // when all others fail, return this...
 }*/

// CREATE CASE-SWITCH STATEMENT FOR THESE ITEMS
float destLat;
float destLon;

float flat, flon, fdist, distMiles;
int speedMPH;

String rawDateTime;
String dateString;
String timeString;
String targetName;
String targetNumb;

boolean targetConf = false;

void powergprs() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

void gprsbootloop() {
  Serial.print(F("Initializing GPRS..."));
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
  Serial.println(F("GPRS ready."));
}

void targetSelect() {
  float momLat = 32.878429;
  float momLon = -96.796225;
  float pieLat = 30.355153;
  float pieLon = -97.753806;
  float mcbLat = 32.920863;
  float mcbLon = -96.959084;
  float homeLat = 29.691242;
  float homeLon = -95.437364;

  Serial.println(F("Choose target by typing code listed below and pressing ENTER:"));
  Serial.println();
  Serial.println(F("mom = Mom"));
  Serial.println(F("pie = Pierce"));
  Serial.println(F("mcb = McBee"));
  Serial.println(F("home = Home (Houston)"));
  Serial.println();

  while(targetConf == false) {
    while(!Serial.available()) {
      delay(1);
    }
    if(Serial.available())
      switch(Serial.read())
      {
      case 'mom':
        targetName = "Mom";
        targetNumb = "2145635266";
        //targetNumb = "2146201650";
        destLat = momLat;
        destLon = momLon;
        targetConf = true;
        break;
      case 'pie':
        targetName = "Pierce";
        targetNumb = "2145635266";
        //targetNumb = "2144350118";
        destLat = pieLat;
        destLon = pieLon;
        targetConf = true;
        break;
      case 'mcb':
        targetName = "McBee";
        targetNumb = "2145635266";
        //targetNumb = "4692614651";
        destLat = mcbLat;
        destLon = mcbLon;
        break;
      case 'home':
        targetName = "Home (Houston)";
        targetNumb = "2145635266";
        destLat = homeLat;
        destLon = homeLon;
        targetConf = true;
        break;
      }
  }
}

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  powergprs();
  delay(2000);
  gprsbootloop();
  Serial.begin(19200);
  Serial2.begin(4800);
  Serial1.begin(19200);
  targetSelect();
  Serial.print("Destination: ");
  Serial.print(destLat);
  Serial.print(", ");
  Serial.println(destLon);
  Serial.print("Number: ");
  Serial.println(targetNumb);
}

void loop() {
  bool newdata = false;
  unsigned long start = millis();
  while(millis() - start < 15000) { // Delay between updates
    if(feedgps())
      newdata = true;
  }
  if(newdata) {
    gpsdump(gps);
    //getDateTime();
    delay(1000);
    printlocation();
  }
}
// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4); 
  Serial.print(", ");
  Serial.println(flon, 4);
  speedMPH = gps.f_speed_mph();
  Serial.println(speedMPH);
  fdist = gps.distance_between(flat, flon, destLat, destLon);
  distMiles = fdist * 0.00062137;
  Serial.println(distMiles);
}

// Feed data as it becomes available
bool feedgps() {
  while(Serial2.available()) {
    if(gps.encode(Serial2.read()))
      return true;
  }
  return false;
}

void printlocation() {
  Serial.print(F("Hunter is "));
  Serial.print(distMiles);
  Serial.print(F(" miles away (Straight-line distance), "));
  Serial.print(F("travelling "));
  Serial.print(speedMPH);
  Serial.println(F("mph."));
  Serial.println(F("To see Hunter on a map, click the link below:"));
  Serial.print(F("https://maps.google.com/maps?q="));
  Serial.print(speedMPH);
  Serial.print(F("+mph+(Current+location)@"));
  Serial.print(flat, 8);
  Serial.print(F(",+"));
  Serial.print(flon, 8);
  Serial.println(F("&t=h&z=18&output=embed"));
  /*getDateTime();
   Serial.println(F("Last updated:"));
   Serial.print(timeString);
   Serial.print(F(", "));
   Serial.println(dateString);
   Serial.println();*/
  Serial.println();
}

void smslocation() {
  Serial1.println("AT+CMGF=1");
  delay(100);
  Serial1.print("AT+CMGS=\"+1");
  Serial1.print(targetNumb);
  Serial1.println("\"");
  delay(100);
  Serial1.print(distMiles);
  Serial1.print(" miles away. ");
  Serial1.print(speedMPH);
  Serial1.print(" mph. ");
  Serial1.print("https://maps.google.com/maps?q=");
  Serial1.print(speedMPH);
  Serial1.print("+mph+(Current+location)@");
  Serial1.print(flat, 8);
  Serial1.print(",+");
  Serial1.print(flon, 8);
  Serial1.println("&t=h&z=18&output=embed");
  delay(100);
  /*getDateTime();
   Serial1.print(F("Last updated:"));
   Serial1.print(timeString);
   Serial1.print(F(", "));
   Serial1.print(dateString);
   Serial1.print();*/
  Serial1.println((char)26);
  delay(100);
  Serial1.println();

  Serial.println("Message sent.");
  Serial.println();
}

void getDateTime() {
  dateString = "";
  timeString = "";
  String year = "";  // Clear year string before reading from GPRS
  String month = "";  // Clear month string before reading from GPRS
  String day = "";  // Clear day string before reading from GPRS

  Serial1.println("AT+CCLK?");  // Read date/time from GPRS
  if(Serial1.available()) {  // If data is coming from GPRS
    while(Serial1.available()) {  // Read the data into string from incoming bytes while they're available
      char c = Serial1.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
    for(int y = 8; y < 10; y++) {  // Parse out year characters from rawDateTime
      year += String(rawDateTime.charAt(y));
    }
    for(int mo = 11; mo < 13; mo++) {  // Parse out month characters from rawDateTime
      month += String(rawDateTime.charAt(mo));
    }
    for(int d = 14; d < 16; d++) {  // Parse out day characters from rawDateTime
      day += String(rawDateTime.charAt(d));
    }
    for(int t = 17; t < 25; t++) {  // Parse out time characters from rawDateTime
      timeString += String(rawDateTime.charAt(t));
    }

    // Construct US formatted date string (M/D/Y)
    dateString += String(month);
    dateString += "/";
    dateString += String(day);
    dateString += "/";
    dateString += String(year);
  }
  rawDateTime = "";  // Clear rawDateTime string for use on next read
}

void proximityCheck() {
  smslocation();
}

