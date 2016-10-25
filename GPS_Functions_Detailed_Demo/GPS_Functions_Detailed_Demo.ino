/* Important: This version is intended for Arduino 1.0 IDE. It will
 not compile in earlier versions. Be sure the following files are
 present in the folder with this sketch:
 
 TinyGPS.h
 TinyGPS.cpp
 keywords.txt
 
 A revised version of the TinyGPS object library is included in the sketch folder
 to avoid conflict with any earlier version you may have in the Arduino libraries 
 location. 
 */

#include <SoftwareSerial.h>
#include <TinyGPS.h>                 // Special version for 1.0

TinyGPS gps;
SoftwareSerial nss(7, 8);            // Yellow wire to pin 6

void setup() {
  Serial.begin(19200);
  nss.begin(9600);
  Serial.println("Reading GPS");
  Serial.println();
}

void loop() {
  boolean newdata = false;
  unsigned long start = millis();
  while(millis() - start < 5000) {  // Update every 5 seconds
    if(feedgps()) newdata = true;
  }
  if(newdata) gpsdump(gps);
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  float flat, flon;
  unsigned long age;
  String gpsDate = "";
  String gpsTime = "";
  String gpsDateTime = "";

  String monthFormatted = "";
  String dayFormatted = "";
  String hourFormatted = "";
  String minuteFormatted = "";
  String secondFormatted = "";
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4); 
  Serial.print(", "); 
  Serial.println(flon, 4);
  Serial.flush();
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  // Time String
  if(hour < 10) hourFormatted = "0" + String(hour);
  else hourFormatted = String(hour);
  if(minute < 10) minuteFormatted = "0" + String(minute);
  else minuteFormatted = String(minute);
  if(second < 10) secondFormatted = "0" + String(second);
  else secondFormatted = String(second);
  gpsTime = hourFormatted + ":" + minuteFormatted + ":" + secondFormatted;

  // Date String
  if(month < 10) monthFormatted = "0" + String(month);
  else monthFormatted = String(month);
  if(day < 10) dayFormatted = "0" + String(day);
  else dayFormatted = String(day);
  gpsDate = monthFormatted + "/" + dayFormatted + "/" + year;

  gpsDateTime = gpsTime + ", " + gpsDate;

  Serial.println(gpsDate);
  Serial.println(gpsTime);
  Serial.println(gpsDateTime);
  float altitude;
  altitude = gps.f_altitude();
  Serial.println(altitude);
  float course;
  course = gps.f_course();
  Serial.println(course);
  float speed;
  speed = gps.f_speed_mph();
  Serial.println(speed);

  Serial.println();
}

// Feed data as it becomes available 
boolean feedgps() {
  while(nss.available()) {
    if(gps.encode(nss.read()))
      return true;
  }
  return false;
}
