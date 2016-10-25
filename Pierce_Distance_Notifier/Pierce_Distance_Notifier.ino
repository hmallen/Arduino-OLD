#include <SoftwareSerial.h>
#include <TinyGPS.h> // Special version for 1.0

TinyGPS gps;
SoftwareSerial nss(6, 255); // Yellow wire to pin 6

float homeLat = 0;
float homeLon = 0;
float pieLat = 30.355153;
float pieLon = -97.753806;
float momLat = 0;
float momLon = 0;
float mcbLat = 0;
float mcbLon = 0;

float flat, flon, fdist, distMiles;
int fspd;

void setup() {
  Serial.begin(19200);
  nss.begin(4800);
  Serial.println("Reading GPS");
  Serial.println();
}

void loop() {
  bool newdata = false;
  unsigned long start = millis();
  while(millis() - start < 2500) { // Delay between updates
    if(feedgps())
      newdata = true;
  }
  if(newdata) {
    gpsdump(gps);
  }
}
// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4); 
  Serial.print(", ");
  Serial.println(flon, 4);
  fspd = gps.f_speed_mph();
  Serial.println(fspd);
  fdist = gps.distance_between(flat, flon, destLat, destLon);
  distMiles = fdist * 0.00062137;
  Serial.println(distMiles);
}
// Feed data as it becomes available
bool feedgps() {
  while(nss.available()) {
    if(gps.encode(nss.read()))
      return true;
  }
  return false;
}

void serialprint() {
  Serial.print("Hunter is ");
  Serial.print(distMiles);
  Serial.println(" miles away.");
  Serial.print("He is currently travelling ");
  Serial.print(fspd);
  Serial.println(" mph.");
  Serial.println("To see Hunter on a map, click the link below:");
  Serial.print("htt://maps.google.com/maps?q=");
  Serial.print(fspd);
  Serial.print("+mph+(Current+location)@");
  Serial.print(flat);
  Serial.print(",+");
  Serial.print(flon);
  Serial.println("&t=h&z=18&output=embed");
}
