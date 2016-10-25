#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>
#include <SD.h>

TinyGPS gps; // create gps object
SoftwareSerial gpsSerial(2, 3); // create gps sensor connection

int CS = 10; // pin for SD card

void getgps(TinyGPS &gps); // declare prototype for functions using TinyGPS library

void setup()
{
  Serial.begin(9600); // connect serial
  gpsSerial.begin(4800); // connect gps sensor

  //Connect to the SD Card
  if(!SD.begin(CS))
  {
    Serial.println("Failed to connect to SD card!");
    return;
  }
}

void loop()
{
  bool newdata = false;
  unsigned long start = millis();
  while (millis() - start < 1000) 
  {  // Update every 5 seconds
    if (feedgps())
      newdata = true;
  }
  if (newdata) 
  {
    gpsdump(gps);
  }
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) 
{
  float flat, flon;
  unsigned long age;
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.f_get_position(&flat, &flon, &age);
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  /*Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(day, DEC);
  Serial.print("/");
  Serial.print(year);
  Serial.print(", ");
  Serial.print(hour, DEC);
  Serial.print(":");
  Serial.print(minute, DEC);
  Serial.print(":");
  Serial.print(second, DEC);
  Serial.print(" --> ");
  Serial.print(flat, 4); 
  Serial.print(", "); 
  Serial.print(flon, 4);
  Serial.println();
  Serial.print(gps.f_altitude());
  Serial.print(" meters");
  Serial.println();
  Serial.print(gps.f_course());
  Serial.print(" degrees");
  Serial.println();
  Serial.print(gps.f_speed_mph());
  Serial.print(" mph");
  Serial.println();*/
}

// Feed data as it becomes available 
bool feedgps() 
{
  while (gpsSerial.available()) 
  {
    if (gps.encode(gpsSerial.read()))
      return true;
  }
  return false;
}

