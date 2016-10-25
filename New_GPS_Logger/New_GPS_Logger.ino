#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SD.h>
#include <stdlib.h>

long lat,lon; // create variable for latitude and longitude object

SoftwareSerial gpsSerial(2, 3); // create gps sensor connection
TinyGPS gps; // create gps object
int CS = 10;
int LED = 13;

String dataString = "";

void setup(){
  pinMode(CS, OUTPUT); //Chip Select Pin for SD Card
  pinMode(LED, OUTPUT); //LED Indicator
  Serial.begin(9600); // connect serial
  gpsSerial.begin(4800); // connect gps sensor

  //Connect to the SD Card
  if(!SD.begin(CS))
  {
    Serial.println("Failed to connect to SD card!");
    return;
  }
}

void loop(){
  while(gpsSerial.available()){ // check for gps data
    if(gps.encode(gpsSerial.read())){ // encode gps data

      float flat, flon;  // make lat & lon floating point values

      gps.f_get_position(&flat, &flon, &age); // get latitude and longitude
      float falt = gps.f_altitude(); // +/- altitude in meters
      float fc = gps.f_course(); // course in degrees
      float fk = gps.f_speed_knots(); // speed in knots
      float fmph = gps.f_speed_mph(); // speed in miles/hr
      float fmps = gps.f_speed_mps(); // speed in m/sec
      float fkmph = gps.f_speed_kmph(); // speed in km/hr

      dataString = flat + "," + flon;
      
      if(dataString != "")
        digitalWrite(LED, HIGH);
      else
        digitalWrite(LED, LOW);

      //Open the Data CSV Files
      File dataFile = SD.open("LOG.CSV", FILE_WRITE);
      if (dataFile)
      {
        dataFile.println(dataString);
        Serial.println(dataString);
        dataFile.close();
      }
      else
      {
        Serial.println("\nCouldn't open the log file!");
      }
    }
  }
}



