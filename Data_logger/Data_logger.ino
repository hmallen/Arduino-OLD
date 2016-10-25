#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>
#include "IntersemaBaro.h"
#include <SD.h>

TinyGPS gps; // create gps object
SoftwareSerial gpsSerial(2, 3); // create gps sensor connection

Intersema::BaroPressure_MS5607B baro(true); // barometer startup

int CS = 10; // pin for SD card

float flat, flon;
int year;
byte month, day, hour, minute, second, hundredths;

void getgps(TinyGPS &gps); // declare prototype for functions using TinyGPS library

void setup()
{
  Serial.begin(9600); // connect serial
  gpsSerial.begin(4800); // connect gps sensor
  baro.init();

  //Connect to the SD Card
  if(!SD.begin(CS))
  {
    Serial.println("Failed to connect to SD card!");
    return;
  }
}

void loop()
{
  while(gpsSerial.available())
  { // check for gps data
    if(gps.encode(gpsSerial.read())) // encode gps data
    {
      gps.f_get_position(&flat,&flon); // get latitude and longitude as floating point values
      gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths); // get cracked date and time

      // print date to serial
      Serial.print(month, DEC);
      Serial.print("/");
      Serial.print(day, DEC);
      Serial.print("/");
      Serial.print(year);
      Serial.print(", ");
      // print time to serial
      Serial.print(hour, DEC);
      Serial.print(":");
      Serial.print(minute, DEC);
      Serial.print(":");
      Serial.print(second, DEC);
      Serial.print(".");
      Serial.print(hundredths, DEC);
      Serial.print(" --> ");
      // print coordinates to serial
      Serial.print(flat,5);
      Serial.print(", ");
      Serial.print(flon,5);
      Serial.println();
      //print additional available data to serial
      Serial.print(gps.f_altitude());
      Serial.print(" m");
      Serial.println();
      Serial.print(gps.f_course());
      Serial.print(" deg");
      Serial.println();
      Serial.print(gps.f_speed_mph());
      Serial.print(" mph");
      Serial.println();

      int alt = baro.getHeightCentiMeters(); // define variable for barometer and get altitude

      // print altitude from barometer to serial
      Serial.print((float)(alt) / 30.48);
      Serial.print(",");
      Serial.println();

      // Open and write data to SD card
      File dataFile = SD.open("LOG.CSV", FILE_WRITE);
      if(dataFile) 
      {
        // print date to SD
        dataFile.print(month, DEC);
        dataFile.print("/");
        dataFile.print(day, DEC);
        dataFile.print("/");
        dataFile.print(year);
        dataFile.print(",");
        // print time to SD
        dataFile.print(hour, DEC);
        dataFile.print(":");
        dataFile.print(minute, DEC);
        dataFile.print(":");
        dataFile.print(second, DEC);
        dataFile.print(".");
        dataFile.print(hundredths, DEC);
        dataFile.print(",");
        // print coordinates to SD
        dataFile.print(flat,5);
        dataFile.print(",");
        dataFile.print(flon,5);
        dataFile.print(",");
        // print GPS altitude, course, and speed
        dataFile.print(gps.f_altitude());
        dataFile.print(",");
        dataFile.print(gps.f_course());
        dataFile.print(",");
        dataFile.print(gps.f_speed_mph());
        dataFile.print(",");
        // print altitude from barometer to SD
        dataFile.print((float)(alt) / 30.48);
        dataFile.println();
        dataFile.close();
      }
      else 
      {
        Serial.println("\nCouldn't open the log file!");
      }
      delay(5000); // delay between serial and SD read/writes
    }
  }
}
