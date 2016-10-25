#include <TinyGPS.h> // Special version for 1.0
#define powPin 9

TinyGPS gps;

float homeLat = 0;
float homeLon = 0;
float pieLat = 30.355153;
float pieLon = -97.753806;
float momLat = 0;
float momLon = 0;
float mcbLat = 0;
float mcbLon = 0;

// CREATE CASE-SWITCH STATEMENT FOR THESE ITEMS
float destLat = pieLat;
float destLon = pieLon;

float flat, flon, fdist, distMiles;
int speedMPH;

String rawDateTime;
String dateString;
String timeString;

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

void setup() {
  powergprs();
  Serial.begin(19200);
  Serial1.begin(4800);
  Serial2.begin(19200);
  gprsbootloop();
  Serial.println(F("Reading GPS"));
  Serial.println();
}

void loop() {
  bool newdata = false;
  unsigned long start = millis();
  while(millis() - start < 8000) { // Delay between updates
    if(feedgps())
      newdata = true;
  }
  if(newdata) {
    gpsdump(gps);
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
  while(Serial1.available()) {
    if(gps.encode(Serial1.read()))
      return true;
  }
  return false;
}

void printlocation() {
  Serial.print(F("Hunter is "));
  Serial.print(distMiles);
  Serial.println(F(" miles away. (Straight-line distance)"));
  Serial.print(F("He is currently travelling "));
  Serial.print(speedMPH);
  Serial.println(F(" mph."));
  Serial.println(F("To see Hunter on a map, click the link below:"));
  Serial.print(F("https://maps.google.com/maps?q="));
  Serial.print(speedMPH);
  Serial.print(F("+mph+(Current+location)@"));
  Serial.print(flat, 8);
  Serial.print(F(",+"));
  Serial.print(flon, 8);
  Serial.println(F("&t=h&z=18&output=embed"));
  delay(1000);
  /*Serial.println(F("Last updated:"));
  Serial.print(timeString);
  Serial.print(F(", "));
  Serial.println(dateString);
  Serial.println();
  delay(1000);*/
}

void getDateTime() {
  dateString = "";
  timeString = "";
  String year = "";  // Clear year string before reading from GPRS
  String month = "";  // Clear month string before reading from GPRS
  String day = "";  // Clear day string before reading from GPRS

  Serial2.println("AT+CCLK?");  // Read date/time from GPRS
  if(Serial2.available()) {  // If data is coming from GPRS
    while(Serial2.available()) {  // Read the data into string from incoming bytes while they're available
      char c = Serial2.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
    for(int y = 8; y < 10; y++) {  // Parse out year characters from rawDateTime
      year +=  String(rawDateTime.charAt(y));
    }
    for(int mo = 11; mo < 13; mo++) {  // Parse out month characters from rawDateTime
      month +=  String(rawDateTime.charAt(mo));
    }
    for(int d = 14; d < 16; d++) {  // Parse out day characters from rawDateTime
      day += String(rawDateTime.charAt(d));
    }
    for(int t = 17; t < 25; t++) {  // Parse out time characters from rawDateTime
      timeString +=  String(rawDateTime.charAt(t));
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
