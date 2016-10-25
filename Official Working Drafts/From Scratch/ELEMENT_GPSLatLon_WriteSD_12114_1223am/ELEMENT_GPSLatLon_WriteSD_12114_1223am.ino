#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SD.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
 It requires the use of SoftwareSerial, and assumes that you have a
 4800-baud serial GPS device hooked up on pins 3(rx) and 4(tx).
 */

TinyGPS gps;
SoftwareSerial ss(6, 255);

void setup()
{
  pinMode(10, OUTPUT);

  Serial.begin(9600);
  ss.begin(4800);

  if(!SD.begin(10)) {
    Serial.println("SD card failed or not present.");
    delay(2000);
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println();
}

void loop()
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    //String dateTimeString = "";
    //int year;
    //byte month, day, hour, minute, second, hundredths;
    float flat, flon;
    unsigned long age;
    //gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    //char sz[32];
    //sprintf(sz, "%02d/%02d/%02d,%02d:%02d:%02d,",
    //month, day, year, hour, minute, second);

    gps.f_get_position(&flat, &flon, &age);

    File dataFile = SD.open("GPSLOG.TXT", FILE_WRITE);

    if(dataFile) {
      dataFile.print(sprintf(sz, "%02d/%02d/%02d,%02d:%02d:%02d,", month, day, year, hour, minute, second));
      dataFile.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
      dataFile.print(",");
      dataFile.println(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
      dataFile.close();
    }
    else {
      Serial.println("Error while attempting to write SD.");
    }
  }
}



