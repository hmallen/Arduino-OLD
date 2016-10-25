/*

 WARNING: DO NOT PULL LCD SHIELD PIN 10 HIGH WITHOUT MODIFYING (NPN TRANSISTOR/RESISTOR/MOSFET). NORMALLY FOR 
 BACKLIGHT CONTROL, BUT FACTORY DEFECT CAUSES 100mA OUTPUT.
 
 Prototype v5.0 ALPHA
 
 Board: Arduino Mega 2560 R3 + 
 - This program is specific to the Arduino Mega. 8K SRAM is sufficient to accomodate the serial prints in the 
 program, so no flash assist macros [F()] are technically necessary. However, flash assists are included for all
 static serial prints, nonetheless, to free up as much SRAM as possible for later use, if necessary. Given the 3 
 available hardware serials on the Mega, the GPRS jumpers are set for hardware serial transmission, and the 
 connection is initiated in the sketch through "Serial1" instead of SoftwareSerial. Some pin reassignments, 
 through the use of M - F jumpers, were required for SD shield compatibility, as the SPI interface on the Mega is 
 through pins 50 - 53 as opposed to 10 - 13 with the Uno.
 
 Shields:
 - SeeedStudio SD Card
 - SeedStudio GPRS v1.0
 
 Sensors:
 - (A0) 10kOhm potentiometer to simulate h2o level
 - (A1) 1kOhm potentiometer to simulate food level
 - (30) Magnetic reed switch mounted in hinged box to simulate stall door
 - (31) PIR sensor to detect motion
 - (32) Tactile push - button to initiate analog sensor calibration
 - (33) SPST PC board toggle switch to enter debug mode
 
 Other:
 - RadioShack camera board (RX1/TX1) [Hardware "Serial2"]
 
 Order of shield assembly:
 Arduino Mega - - > SD Shield - - > GPRS Shield
 - Arduino Mega - - > SD Shield: All pins connected by jumpers
 - SD Shield - - > GPRS Shield: Shields stacked normally
 
 Pin reassignments (all other pins from Mega - - > SD connected normally):
 SD Shield  - -  - >  Arduino Mega
 0                19 (RX1)  [Hardware "Serial1"]
 1                18 (TX1)  [Hardware "Serial1"]
 10               53 (SS)   [SPI]
 11               51 (MOSI) [SPI]
 12               50 (MISO) [SPI]
 13               52 (SCK)  [SPI]
 
 H2O level is checked and written to SD card at a regular interval. H2O sensor is mimicked by a 10K potentiometer, 
 and values read are normalized to a 0 - 100 scale using the map() function. AS SUCH, CALIBRATION MUST BE PERFORMED 
 ON INITIAL SETUP OF THE DEVICE TO DEFINE THE UPPER LIMIT OF THE MAP FUNCTION. This value is defined as the
 analogRead value when h2o is at maximum level. On each SD write, current date and time information is read from
 the GPRS shield via AT commands. Date and time are parsed and reconstructed into US date format. During setup(), 
 date and time is parsed further, printing a long form (ex. 11:45am, January 23, 2013) to serial. A "GPRS boot loop"
 function is also included in setup(), forcing the program to continuously read from GPRS until data is received. 
 Without this function call, no date and time information is available to be written to the SD card on the first
 loop.
 
 In addition to the regular h2o level reads and SD writes, a data aggregation function is also included in this 
 sketch. This function calculates any negative changes in h2o level on each read, and adds them to an integer that 
 stores the total amount of h2o consumed over the course of multiple regular h2o level SD writes. This aggregate
 value is stored in its own TXT file, independent of the regularly scheduled h2o level SD writes.
 
 A key feature added in v1.0 and refined in v2.0 of the prototype sketch is the h2oCheckChange() functionality, 
 which performs a series of checks for changes in h2o level, including low level detection, recheck/confirmation 
 of significant level changes, empty bucket detection, and a program hold to wait for bucket refill.
 
 New features to v2.0 are the event and error functions, which log important events and possible errors both as
 integer codes to the principal data string. Attempts were made to also write human - readable event and error strings 
 to separate TXT files, but those functions are still non - functional currently.
 
 In v3.0, a PIR sensor was added for motion detection functionality, and an additional potentiometer was added to
 simulate food level. Output data strings were also normalized in length for increased ease of indexing when
 archived data needs to be referenced later.
 
 With v4.0 came the integration of an analog sensor calibration function with associated EEPROM storage and retrieval
 capabilities. Due to potential variations in analog read measurements, based on sensor, measurement conditions, and 
 resistors used with the sensor output, it is necessary to define the minimum and maximum possible values if mapping 
 to a 0 - 100 scale is desired. On first run of the program, the calibration function is automatically initiated, and 
 values are subsequently stored to EEPROM. Due to the bitwise storage mechanism of EEPROM function, each EEPROM slot 
 is limited to values between 0 - 255. Thus, 5 EEPROM slots are allocated for each required value: min h2o level, max 
 h2o level, min food level, and max food level. This is sufficient to cover all potential values across the range of 
 possible analogRead values, 0 - 1023. As such, when values are retrieve from EEPROM, the 5 values for each group are 
 totalled to yield the actual analog value read from the sensor. At the end of the initial calibration, a value of 1 
 is written to EEPROM slot #255, which indicates that sensors have been previously calibrated on all future reboots. 
 However, if recalibration is desired at a later time, it can be triggered by holding a tactile push - button during 
 setup(). While not in this version, future versions will also include an error check condition that resets EEPROM 
 slot #255 to 0 to force recalibration if the error check deems that this is required.
 
 A toggle switch was included with v4.1 that, if turned on, enables debug functions that may be included in the 
 debugMode() function at the bottom of the sketch. It currently only serves to write a 0 to EEPROM slot #255, forcing
 analog sensor calibration upon startup. Debugging is broken out into its own isolated function, such that additional
 code/parameters may be included in the future, as needed.
 
 Two new features were added in v5.0. First, the native "SD.h" library was replaced with the superior library, 
 "SdFat.h". This library is much more efficient, encounters less serial overflow issues, has more built in funtions, 
 and allows the use of SPI at full speed, instead of half speed. Next, a camera board was added to include jpg capture
 functionality. Currently, jpg's are captured by pressing a tactile push-button switch, however, this capture feature
 will be triggered via software, at regular intervals, upon motion detection, etc in the future. Furthermore, only one
 jpg may be saved at a time in the current sketch, with the jpg file being overwritten upon each new capture.
 
 Event codes:
 0 = None
 1 = Stall door opened
 2 = H2O bucket empty
 3 = Food bucket empty
 4 = H2O bucket and food bucket empty
 5 = H2O bucket refilled
 6 = Food bucket refilled
 7 = No motion detected for significant interval
 
 Error codes:
 0 = None
 1 = Increase in h2oLevel without stall door opening first
 
 Changes from previous version:
 - Native "SD.h" library replaced with "SdFat.h" library
 - Camera added to capture jpg upon button press
 
 Planned changes for next version:
 - Add analog sensor error check to force sensor min/max recalibration
 - Change void functions to define a data type for more efficient return of values
 - Reevaluate global variables and determine if any localizations are possible to reduce sketch size
 --  Determine if merging errorCheck() function elements with corresponding code is better option
 - Fix date/time on initial Arduino power-on (low priority)
 --  Some data not received due to GPRS power on messages (i.e.  + CFUN = 1, etc.)
 - Organize SD data logs (ex. by day/week/month)
 - Setup SD indexing functions for later use in calculating daily/hourly/etc. totals or analyzing trends
 - Add consequences for h2oLevel > h2oLastLevel under various conditions (ESPECIALLY WITH AGGREGATE CONSUMPTION)
 - Prioritize event/error codes and/or address situation in which multiple are valid on single read
 - Add h2o and food low level event codes
 - Add LCD screen for data display, calibration triggering, and customizing initial setup parameters
 - Add Xively API connectivity via GPRS to allow data display on website or mobile app
 
 */

////  Libraries  ////

#include  <SdFat.h> 
#include  <EEPROM.h> 
//#include  <LiquidCrystal.h> 

////  Library start - ups for connected hardware  ////

//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  // Define LCD display pins for LiquidCrystal library

////  Pin definitions  ////

// GPRS, SD, and sensors
#define debugPin 33  // Pin for triggering debug mode
#define powPin 9  // Pin for software power - up of GPRS shield
#define chipSelect 53  // SPI chip select pin (SS) for SD card (specific to Arduino board)
#define calibPin 32  // Pin for tactile button triggering of analog sensor calibration in setup
#define h2oPin A0  // Pin for reading h2o level sensor
#define foodPin A1  // Pin for reading food level sensor
#define doorPin 30  // Pin for reading stall door open/closed status with magnetic switch
#define pirPin 31  // Pin for detecting motion with PIR sensor
#define photoPin 34  // Pin for triggering photo capture
/*#define maxH2OVal 919  // Maximum possible value of h2o sensor - -  used for map() function
 #define minH2OVal 0  // Minimum possible value of h2o sensor - -  used for map() function
 // NOTE: max(and min)H2OVal IS MAX POSSIBLE analogRead VALUE FROM SENSOR AND NEEDS TO BE CALIBRATED BEFORE FIRST USE
 #define maxFoodVal 918  // Maximum possible value of food sensor - -  used for map() function
 #define minFoodVal 0  // Minimum possible value of h2o sensor - -  used for map() function
 // NOTE: max(and min)FoodVal IS MAX POSSIBLE analogRead VALUE FROM SENSOR AND NEEDS TO BE CALIBRATED BEFORE FIRST USE*/

// Camera
#define VC0706_PROTOCOL_SIGN 0x56
#define VC0706_SERIAL_NUMBER 0x00
#define VC0706_COMMAND_RESET 0x26
#define VC0706_COMMAND_GEN_VERSION 0x11
#define VC0706_COMMAND_TV_OUT_CTRL 0x44
#define VC0706_COMMAND_OSD_ADD_CHAR 0x45
#define VC0706_COMMAND_DOWNSIZE_SIZE 0x53
#define VC0706_COMMAND_READ_FBUF 0x32
#define FBUF_CURRENT_FRAME 0
#define FBUF_NEXT_FRAME	0
#define VC0706_COMMAND_FBUF_CTRL 0x36
#define VC0706_COMMAND_COMM_MOTION_CTRL	0x37
#define VC0706_COMMAND_COMM_MOTION_DETECTED 0x39
#define VC0706_COMMAND_POWER_SAVE_CTRL 0x3E
#define VC0706_COMMAND_COLOR_CTRL 0x3C
#define VC0706_COMMAND_MOTION_CTRL 0x42
#define VC0706_COMMAND_WRITE_DATA 0x31
#define VC0706_COMMAND_GET_FBUF_LEN 0x34
#define READ_DATA_BLOCK_NO 56

boolean debugState = false;  // Boolean to trigger execution of debug mode actions

////  Calibration variables  ////

uint16_t h2oMinRemain = 0;
uint16_t h2oMaxRemain = 0;
uint16_t foodMinRemain = 0;
uint16_t foodMaxRemain = 0;
uint16_t h2oMinA = 0;
uint16_t h2oMinB = 0;
uint16_t h2oMinC = 0;
uint16_t h2oMinD = 0;
uint16_t h2oMinE = 0;
uint16_t h2oMaxA = 0;
uint16_t h2oMaxB = 0;
uint16_t h2oMaxC = 0;
uint16_t h2oMaxD = 0;
uint16_t h2oMaxE = 0;
uint16_t foodMinA = 0;
uint16_t foodMinB = 0;
uint16_t foodMinC = 0;
uint16_t foodMinD = 0;
uint16_t foodMinE = 0;
uint16_t foodMaxA = 0;
uint16_t foodMaxB = 0;
uint16_t foodMaxC = 0;
uint16_t foodMaxD = 0;
uint16_t foodMaxE = 0;

int minH2OVal = 0;
int maxH2OVal = 0;
int minFoodVal = 0;
int maxFoodVal = 0;

////  Timing constants and integers  ////

unsigned long currTimer = 0;  // Reset to millis() for current time on each loop
unsigned long prevTimer = 0;  // Time of last regular SD write
unsigned long aggTimer = 0;  // Time of last data aggregation
unsigned long doorTimer = 0;  // Time of last stall door opening
unsigned long doorLastTimer = 0;  // Time since last stall door opening (currTimer - doorTimer)
unsigned long pirTimer = 0;  // Time of last detected motion
unsigned long pirLastTimer = 0;  // Time since last detected motion (currTimer - doorTimer)
const unsigned long writeInterval = 10 * 1000;  // Regular SD write interval [1st digit seconds]
const unsigned long aggInterval = 30 * 1000;  // Data aggregation interval [1st digit seconds]
const unsigned long doorResetInterval = 2 * 60 * 1000;  // Door timer reset interval [1st digit minutes]
const unsigned long pirStillInterval = 5 * 60 * 1000;  // Elapsed time without motion triggering event code [1st digit minutes]

////  Global strings (and associated integers)  ////

String rawDateTime;  // Stores raw date and time info from GPRS before parsing

// Date
String year;  // Year parsed from rawDateTime
String month;  // Month parsed from rawDateTime
String day;  // Day parsed from rawDateTime
String dateString;  // Parsed/concatenated date info from GPRS that is written to SD

// Time
String hour;  // Hour parsed from rawDateTime for 12hr time construction
String minute;  // Minute parsed from rawDateTime for 12hr time construction
String second;  // Second parsed from rawDateTime for 12hr time construction
String timeString;  // Parsed/concatenated time info from GPRS that is written to SD
String hour12;  // Stores hour after conversion from 24hr - - > 12hr time format
String dateLongString;  // Stores full long form of date after parsing from dateString
String time12hrString;  // Stores time info in 12hr time format after parsing from timeString
String dataString;  // Full data string written to SD card
String aggString;  // String storing aggregated data written to SD card

// H2O sensor
int h2oLevel = 0;  // Stores h2o level from h2oLevelRead() function
int h2oLastLevel = 0;  // Stores h2o level for comparison with levels from future reads 
const int h2oChangeTrigger = 10;  // % change in h2o level that triggers confirmation read followed by SD write
const int h2oErrorTrigger =  - 5;  // % change in h2o level without door opening first that triggers error write to SD
int h2oAmtCons = 0;  // Stores h2o amount consumed on each read
int h2oAmtConsTot = 0;  // Stores h2o amount consumed over time for writing aggregate data to SD
const int h2oFullLevel = 75;  // After bucket empty, program will hold until this level reached again
boolean h2oEmpty = false;  // Will become true when H2O bucket completely empty

// Food sensor
int foodLevel = 0;  // Stores food level read from sensor
int foodLastLevel = 0;  // Stores food level for comparison with levels from future reads
const int foodChangeTrigger = 10;  // % change in h2o level that triggers confirmation read followed by SD write
const int foodErrorTrigger =  - 5;  // % change in h2o level without door opening first that triggers error write to SD
int foodAmtCons = 0;  // Stores h2o amount consumed on each read
int foodAmtConsTot = 0;  // Stores h2o amount consumed over time for writing aggregate data to SD
const int foodFullLevel = 75;  // After bucket empty, program will hold until this level reached again
boolean foodEmpty = false;  // Will become true when H2O bucket completely empty

// Door sensor
int doorState = 0;  // Equals 1 when stall door is open
int doorLastState = 0;  // Stores door state on last read so openCount only incremented when door closed first
int openCount = 0;  // Counts each stall door opening and resets when h2o refilled

// PIR sensor
int pirState = 0;  // Equals 1 when motion is detected in the stall
int pirLastState = 0;  // Stores pir state for comparison on future read

// SD card
SdFat sd;
SdFile dataFile;

// Events and errors
int eventCode = 0;  // Integer representing specific event (codes listed above, at bottom of introductory comment)
String eventString;  // Full event string to be written to SD card
int errorCode = 0;  // Integer representing specific error (codes listed above, at bottom of introductory comment)
String errorString;  // Full error string to be written to SD card

// Camera
int loopCount = 0;
unsigned char tx_counter;
unsigned char tx_vcbuffer[20];
boolean tx_ready;
boolean rx_ready;
unsigned char rx_counter;
unsigned char VC0706_rx_buffer[80];
uint32_t frame_length = 0;
uint32_t vc_frame_address = 0;
uint32_t last_data_length = 0;

/* = = = = = = = = = = = = = = = = = = = = = = = = = == = 
 ////  Startup functions  ////
 = = = = = = = = = = = = = = = = = = = = = = = = == = */

void calibAnalogSensors() {
  for(int i = 0; i < 20; i++) {
    EEPROM.write(i, 0);
    delay(5);
  }
  Serial.println(F("Set sensors to minimum value, then hold down button."));
  while(digitalRead(calibPin) == 1) {
    delay(100);
  }
  int h2oCalibMin = analogRead(h2oPin);
  int foodCalibMin = analogRead(foodPin);
  Serial.println(F("Release the button to continue."));
  while(digitalRead(calibPin) == 0) {
    delay(100);
  }
  Serial.println(F("Set sensors to maximum value, then hold down button."));
  while(digitalRead(calibPin) == 1) {
    delay(100);
  }
  int h2oCalibMax = analogRead(h2oPin);
  int foodCalibMax = analogRead(foodPin);
  Serial.println(F("Release the button to continue."));

  while(digitalRead(calibPin) == 0) {
    delay(100);
  }
  Serial.println(F("Analog sensor calibration complete."));
  Serial.println();
  Serial.println(F("analogRead values:"));
  Serial.print(F("H2O:  min = "));
  Serial.print(h2oCalibMin);
  Serial.print(F(" / max = "));
  Serial.println(h2oCalibMax);
  Serial.print(F("Food: min = "));
  Serial.print(foodCalibMin);
  Serial.print(F(" / max = "));
  Serial.println(foodCalibMax);
  Serial.println();

  if(h2oCalibMin > 255) {
    h2oMinRemain = h2oCalibMin % 255;
  }
  if(h2oCalibMax > 255) {
    h2oMaxRemain = h2oCalibMax % 255;
  }
  if(foodCalibMin > 255) {
    foodMinRemain = foodCalibMin % 255;
  }
  if(foodCalibMax > 255) {
    foodMaxRemain = foodCalibMax % 255;
  }

  // Store minimum h2o analog read value
  if(h2oCalibMin < 256) {
    EEPROM.write(0, h2oMinRemain);
    delay(5);
  }
  else if(h2oCalibMin < 511) {
    EEPROM.write(0, 255);
    delay(5);
    EEPROM.write(1, h2oMinRemain);
    delay(5);
  }
  else if(h2oCalibMin < 766) {
    EEPROM.write(0, 255);
    delay(5);
    EEPROM.write(1, 255);
    delay(5);
    EEPROM.write(2, h2oMinRemain);
    delay(5);
  }
  else if(h2oCalibMin < 1021) {
    EEPROM.write(0, 255);
    delay(5);
    EEPROM.write(1, 255);
    delay(5);
    EEPROM.write(2, 255);
    delay(5);
    EEPROM.write(3, h2oMinRemain);
    delay(5);
  }
  else if(h2oCalibMin < 1275) {
    EEPROM.write(0, 255);
    delay(5);
    EEPROM.write(1, 255);
    delay(5);
    EEPROM.write(2, 255);
    delay(5);
    EEPROM.write(3, 255);
    delay(5);
    EEPROM.write(4, h2oMinRemain);
    delay(5);
  }
  // Store maximum h2o analog read value
  if(h2oCalibMax < 256) {
    EEPROM.write(5, h2oMaxRemain);
    delay(5);
  }
  else if(h2oCalibMax < 511) {
    EEPROM.write(5, 255);
    delay(5);
    EEPROM.write(6, h2oMaxRemain);
    delay(5);
  }
  else if(h2oCalibMax < 766) {
    EEPROM.write(5, 255);
    delay(5);
    EEPROM.write(6, 255);
    delay(5);
    EEPROM.write(7, h2oMaxRemain);
    delay(5);
  }
  else if(h2oCalibMax < 1021) {
    EEPROM.write(5, 255);
    delay(5);
    EEPROM.write(6, 255);
    delay(5);
    EEPROM.write(7, 255);
    delay(5);
    EEPROM.write(8, h2oMaxRemain);
    delay(5);
  }
  else if(h2oCalibMax < 1275) {
    EEPROM.write(5, 255);
    delay(5);
    EEPROM.write(6, 255);
    delay(5);
    EEPROM.write(7, 255);
    delay(5);
    EEPROM.write(8, 255);
    delay(5);
    EEPROM.write(9, h2oMaxRemain);
    delay(5);
  }
  // Store minimum food analog read value
  if(foodCalibMin < 256) {
    EEPROM.write(10, foodMinRemain);
    delay(5);
  }
  else if(foodCalibMin < 511) {
    EEPROM.write(10, 255);
    delay(5);
    EEPROM.write(11, foodMinRemain);
    delay(5);
  }
  else if(foodCalibMin < 766) {
    EEPROM.write(10, 255);
    delay(5);
    EEPROM.write(11, 255);
    delay(5);
    EEPROM.write(12, foodMinRemain);
    delay(5);
  }
  else if(foodCalibMin < 1021) {
    EEPROM.write(10, 255);
    delay(5);
    EEPROM.write(11, 255);
    delay(5);
    EEPROM.write(12, 255);
    delay(5);
    EEPROM.write(13, foodMinRemain);
    delay(5);
  }
  else if(foodCalibMin < 1275) {
    EEPROM.write(10, 255);
    delay(5);
    EEPROM.write(11, 255);
    delay(5);
    EEPROM.write(12, 255);
    delay(5);
    EEPROM.write(13, 255);
    delay(5);
    EEPROM.write(14, foodMinRemain);
    delay(5);
  }
  // Store maximum food analog read value
  if(foodCalibMax < 256) {
    EEPROM.write(15, foodMaxRemain);
    delay(5);
  }
  else if(foodCalibMax < 511) {
    EEPROM.write(15, 255);
    delay(5);
    EEPROM.write(16, foodMaxRemain);
    delay(5);
  }
  else if(foodCalibMax < 766) {
    EEPROM.write(15, 255);
    delay(5);
    EEPROM.write(16, 255);
    delay(5);
    EEPROM.write(17, foodMaxRemain);
    delay(5);
  }
  else if(foodCalibMax < 1021) {
    EEPROM.write(15, 255);
    delay(5);
    EEPROM.write(16, 255);
    delay(5);
    EEPROM.write(17, 255);
    delay(5);
    EEPROM.write(18, foodMaxRemain);
    delay(5);
  }
  else if(foodCalibMax < 1275) {
    EEPROM.write(15, 255);
    delay(5);
    EEPROM.write(16, 255);
    delay(5);
    EEPROM.write(17, 255);
    delay(5);
    EEPROM.write(18, 255);
    delay(5);
    EEPROM.write(19, foodMaxRemain);
    delay(5);
  }
  EEPROM.write(255, 1);
  Serial.println(F("Analog sensor min/max values written to EEPROM."));
}

void retrieveAnalogMinMax() {
  h2oMinA = EEPROM.read(0);
  delay(5);
  h2oMinB = EEPROM.read(1);
  delay(5);
  h2oMinC = EEPROM.read(2);
  delay(5);
  h2oMinD = EEPROM.read(3);
  delay(5);
  h2oMinE = EEPROM.read(4);
  delay(5);
  h2oMaxA = EEPROM.read(5);
  delay(5);
  h2oMaxB = EEPROM.read(6);
  delay(5);
  h2oMaxC = EEPROM.read(7);
  delay(5);
  h2oMaxD = EEPROM.read(8);
  delay(5);
  h2oMaxE = EEPROM.read(9);
  delay(5);
  foodMinA = EEPROM.read(10);
  delay(5);
  foodMinB = EEPROM.read(11);
  delay(5);
  foodMinC = EEPROM.read(12);
  delay(5);
  foodMinD = EEPROM.read(13);
  delay(5);
  foodMinE = EEPROM.read(14);
  delay(5);
  foodMaxA = EEPROM.read(15);
  delay(5);
  foodMaxB = EEPROM.read(16);
  delay(5);
  foodMaxC = EEPROM.read(17);
  delay(5);
  foodMaxD = EEPROM.read(18);
  delay(5);
  foodMaxE = EEPROM.read(19);
  delay(5);
  minH2OVal = h2oMinA + h2oMinB + h2oMinC + h2oMinD + h2oMinE;
  maxH2OVal = h2oMaxA + h2oMaxB + h2oMaxC + h2oMaxD + h2oMaxE;
  minFoodVal = foodMinA + foodMinB + foodMinC + foodMinD + foodMinE;
  maxFoodVal = foodMaxA + foodMaxB + foodMaxC + foodMaxD + foodMaxE;
}

// Function to execute software power - on of GPRS shield
void powerUp() {
  digitalWrite(powPin, LOW);
  delay(100);
  digitalWrite(powPin, HIGH);
  delay(500);
  digitalWrite(powPin, LOW);
  delay(100);
}

// Function to loop GPRS read on startup until data received from GPRS (1st loop() receives no data otherwise)
void gprsBootLoop() {
  Serial.print(F("Initializing GPRS..."));
  while(dateString == 0) {  // Attempt to read date/time from GPRS continuously until data received
    getDateTime();
    delay(1000);
  }
  Serial.println(F("GPRS ready."));
}

void setup() {
  pinMode(debugPin, INPUT);  // Set toggle switch pin as input for debug functions
  pinMode(powPin, OUTPUT);  // Software power - on pin for GPRS shield
  pinMode(chipSelect, OUTPUT);  // Required for SD lib fxn
  pinMode(calibPin, INPUT);  // Set tactile button pin as input for calibration function
  pinMode(doorPin, INPUT);  // Set door pin as input for reading status (open/closed)
  pinMode(pirPin, INPUT);  // Set pir pin as input for detecting motion

  //lcd.begin(16, 2);  // Initiate connection to LCD display
  Serial.begin(19200);  // Hardware serial for serial monitor output
  Serial1.begin(19200);  // Hardware serial for GPRS communication
  Serial2.begin(115200);  // Hardware serial for camera

  powerUp();  // Call function to power - up GPRS with software commands
  gprsBootLoop();  // Initiate GPRS read loop until data is received (1st loop() read fails otherwise)
  debugMode();

  if(!sd.begin(chipSelect, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  Serial.println(F("SD card initialized."));
  Serial.println();

  if(EEPROM.read(255) == 0) {
    Serial.println(F("Analog sensor calibration required."));
    calibAnalogSensors();
  }
  else {
    Serial.print(F("Hold button to calibrate analog sensors..."));
    delay(3000);
    if(digitalRead(calibPin) == 0) {
      Serial.println();
      Serial.println(F("Release button to continue."));
      while(digitalRead(calibPin) == 0) {
        delay(100);
      }
      calibAnalogSensors();
    }
    else {
      Serial.println(F("calibration bypassed. Using EEPROM values."));
    }
    Serial.println();
  }
  retrieveAnalogMinMax();
  Serial.println(F("EEPROM values:"));
  Serial.print(F("H2O:  min = "));
  Serial.print(minH2OVal);
  Serial.print(F(" / max = "));
  Serial.println(maxH2OVal);
  Serial.print(F("Food: min = "));
  Serial.print(minFoodVal);
  Serial.print(F(" / max = "));
  Serial.println(maxFoodVal);
  Serial.println();

  /*lcd.print("Welcome to");
   lcd.setCursor(0, 1);
   lcd.print("MyStall");
   delay(2000);
   lcd.clear();*/

  Serial.println(F("Welcome to MyStall."));
  Serial.println();

  getDateTime();  // Read date and time information from GPRS
  dateLongConst();  // Construct string with long form of date
  time12hrConst();  // Construct string with 12hr format of time

  /*lcd.print(dateLongString);
   lcd.setCursor(0, 1);
   lcd.print(time12hrString);
   delay(2000);
   lcd.clear();*/

  Serial.print(F("It is currently "));
  Serial.print(time12hrString);
  Serial.print(", ");
  Serial.print(dateLongString);
  Serial.println(".");
  Serial.println();

  Serial.println(F("Data string format:"));
  Serial.print(F("Date,Time,"));
  Serial.print(F("H2O Level,Agg H2O,"));
  Serial.print(F("Food Level,Agg H2O,"));
  Serial.print(F("Door,Open Counter,"));
  Serial.print(F("Motion,"));
  Serial.println(F("Event Code,Error Code"));
  Serial.println();

  h2oLevelRead();  // Read current h2o level
  foodLevelRead();  // Read current food level
  h2oLastLevel = h2oLevel;  // Set h2o level storage variable to current h2o level
  foodLastLevel = foodLevel;  // Set food level storage variable to current food level
  currTimer = millis();  // Set to current time
  prevTimer = millis();  // Set to current time
  aggTimer = millis();  // Set to current time

  h2oCheckChange();  // Check for change in h2o level, low level, or empty bucket
  foodCheckChange();  // Check for change in food level, low level, or empty bucket
}

void loop() {
  if(digitalRead(photoPin) == 0) {
    capture_photo();
    delay(100);
    Serial.println("Photo captured. Release button to continue...");
    while(digitalRead(photoPin) == 0) {
      delay(100);
      }
      Serial.println();
  }
  currTimer = millis();  // Set to current time
  unsigned long intervalTimer = currTimer - prevTimer;  // Calculate time since last regular SD write
  unsigned long lapTimer = currTimer - aggTimer;  // Calculate time since last data aggregation
  loopCount++;

  h2oLevelRead();  // Read current h2o level and return value from 0 - 100
  foodLevelRead();  // Read current h2o level and return value from 0 - 100
  doorStateRead();  // Read door (open/closed) status and relay information, if necessary
  pirStateRead();  // Read PIR sensor for motion detection
  eventCheck();  // Check for conditions warranting a posting of an event code
  errorCheck();  // Check for abnormal sensor values associated with a hardware or software problem

  h2oCheckChange();  // Check again for h2o level change, low level, or empty bucket
  foodCheckChange();  // Check again for food level change, low level, or empty bucket

  if(h2oLevel !=  h2oLastLevel) {  // If h2o level has changed since last read
    h2oAmtConsTot +=  h2oAmtCons;  // Add consumed amount to aggregate total
    h2oLastLevel = h2oLevel;  // Store h2o level just read to allow detection of change on next read
  }
  if(foodLevel !=  foodLastLevel) {  // If food level has changed since last read
    foodAmtConsTot +=  foodAmtCons;  // Add consumed amount to aggregate total
    foodLastLevel = foodLevel;  // Store food level just read to allow detection of change on next read
  }
  if(intervalTimer >=  writeInterval) {  // If aggregation interval not exceed, but regular SD write interval has
    h2oLevelRead();  // Read current h2o level
    dataWriteSD();  // Write date/time and h2oLevel to SD
    delay(500);  // A brief delay is necessary to allow dataWriteSD() to finish
    prevTimer = currTimer;  // Reset time since last regular SD write to 0
    if(h2oLevel <=  10) {  // After other checks, also check if h2o level is < 10% and inform user that refill is required
      Serial.println(F("Water level low. Refill Bucket."));  
    }
    eventCode = 0;
    errorCode = 0;  
    Serial.println();
  }
  if(lapTimer >= aggInterval) {  // If aggregation timer has exceeded aggregation interval
    h2oAmtConsTot = 0;  // Set amount of h2o consumed since last data aggregation back to 0
    foodAmtConsTot = 0;  // Set amount of food consumed since last data aggregation back to 0
    aggTimer = currTimer;  // Reset time since last data aggregation to 0
    openCount = 0;  // Resets stall door opening counter
    // NOTE: STILL NEED TO DETERMINE PROPER LOCATION FOR CLEARING openCount
    Serial.println(F("Aggregate data written to SD and cleared."));
    Serial.println();
  }
  /*if(eventCode > 0 || errorCode > 0) {
   defineEvent();  // Translate integer into human - readable error string
   codesWriteSD();  // Write event and error codes to SD then reset to 0
   Serial.println();
   delay(500);
   }*/

  delay(1000);  // Wait before looping again
}

/* = = = = = = = = = = = = = = = = = = = = = == = 
 ////  Date and time  ////
 = = = = = = = = = = = = = = = = = = = = == = */

// Read date/time info from GPRS shield and parse into year, month, and day for reformatting to US date format
void getDateTime() {
  dateString = "";
  timeString = "";
  year = "";  // Clear year string before reading from GPRS
  month = "";  // Clear month string before reading from GPRS
  day = "";  // Clear day string before reading from GPRS

  Serial1.println("AT + CCLK?");  // Read date/time from GPRS
  if(Serial1.available()) {  // If data is coming from GPRS
    while(Serial1.available()) {  // Read the data into string from incoming bytes while they're available
      char c = Serial1.read();  // Read each byte sent, one at a time, into storage variable
      rawDateTime += c;  // Add character to the end of the data string to be written to SD later
    }
    for(int y = 8; y < 10; y ++) {  // Parse out year characters from rawDateTime
      year +=  String(rawDateTime.charAt(y));
    }
    for(int mo = 11; mo < 13; mo ++) {  // Parse out month characters from rawDateTime
      month +=  String(rawDateTime.charAt(mo));
    }
    for(int d = 14; d < 16; d ++) {  // Parse out day characters from rawDateTime
      day += String(rawDateTime.charAt(d));
    }
    for(int t = 17; t < 25; t ++) {  // Parse out time characters from rawDateTime
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

// Construct long form of date read from GPRS
void dateLongConst() {
  String yearLong = "";  // Clear long - formatted year string before reading from GPRS
  String monthLong = "";  // Clear long - formatted month string before reading from GPRS
  String dayLong = "";  // Clear long - formatted day string before reading from GPRS
  dateLongString = "";  // Clear full long date string before reading from GPRS

  yearLong += "20";  // Lead year with "20" to create long form (ex. 2014)
  yearLong += String(year);  // Store long year in a string

  dayLong += String(day.toInt());  // Convert day to integer to remove leading 0's and store in string

  switch(month.toInt()) {  // Convert to integer and use switch case to convert month number to long form
  case 0:
    return;
  case 1:
    monthLong += "January";
    break;
  case 2:
    monthLong += "February";
    break;
  case 3:
    monthLong += "March";
    break;
  case 4:
    monthLong += "April";
    break;
  case 5:
    monthLong += "May";
    break;
  case 6:
    monthLong += "June";
    break;
  case 7:
    monthLong += "July";
    break;
  case 8:
    monthLong += "August";
    break;
  case 9:
    monthLong += "September";
    break;
  case 10:
    monthLong += "October";
    break;
  case 11:
    monthLong += "November";
    break;
  case 12:
    monthLong += "December";
    break;
  }
  // Concatenate strings to form full long date
  dateLongString += String(monthLong);
  dateLongString += " ";
  dateLongString += String(dayLong);
  dateLongString += ", ";
  dateLongString += String(yearLong);
}

// Changes time read from GPRS into 12hr format
void time12hrConst() {
  hour = "";  // Clear hour string before parsing data into it from timeString
  minute = "";  // Clear minute string before parsing data into it from timeString
  second = "";  // Clear second string before parsing data into it from timeString
  hour12 = "";  // Clear 12hr formatted hour string before parsing data into it from timeString

  for(int h = 0; h < 2; h++) {  // Parse out hour characters from timeString
    hour += String(timeString.charAt(h));
  }
  for(int mi = 3; mi < 5; mi++) {  // Parse out hour characters from timeString
    minute += String(timeString.charAt(mi));
  }
  for(int s = 6; s < 8; s++) {  // Parse out hour characters from timeString
    second += String(timeString.charAt(s));
  }
  if(hour.toInt() > 12) {  // If integer format of 24hr formatted hour is more than 12
    hour12 += String(hour.toInt() - 12);  // Subtract 12 from hour and add to storage string
    time12hrString += String(hour12);  // Add new 12hr formatted hour to 12hr time string
    time12hrString += ":";  // Add colon to create properly formatted time
    time12hrString += String(minute);  // Add minute string after the colon
    time12hrString += "pm";  // Add "pm", since hour was after 12 ( >  =  13)
  }
  else if (hour.toInt() == 0) {  // If integer format of 24hr formatted hour is not more than 12, but is equal to 0
    hour12 += "12";  // Set hour storage string to 12 (midnight)
    time12hrString += String(hour12);  // Add new 12hr formatted hour to 12hr time string
    time12hrString += ":";  // Add colon to create properly formatted time
    time12hrString += String(minute);  // Add minute string after the colon
    time12hrString += "am";  // Add "am", since time detected was midnight
  }
  else {  // If integer format of 24hr formatted hour is not more than 12 or equal to 0
    hour12 += String(hour.toInt());  // Set hour storage variable to integer form of hour
    time12hrString += hour12;  // Add new 12hr formatted hour to 12hr time string
    time12hrString += ":";  // Add colon to create properly formatted time
    time12hrString += minute;  // Add minute string after the colon
    time12hrString += "am";  // Add "am", since time detected was before noon
  }
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 ////  Sensor reads, calculations, and reponses  ////
 = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

// Read h2o level from sensor (currently simulated by 10kOhm potentiometer)
void h2oLevelRead() {
  int h2oReadVal = analogRead(h2oPin);  // Read value from sensor
  h2oLevel = map(h2oReadVal, 0, maxH2OVal, 0, 100);  // Set h2oLevel to 0 - 100 scale by mapping analogRead value
  h2oAmtCons = h2oLastLevel - h2oLevel;  // Calculate h2o amount consumed since last read
}

// Read food level from sensor (currently simulated by 1kOhm potentiometer)
void foodLevelRead() {
  int foodReadVal = analogRead(foodPin);  // Read value from sensor
  foodLevel = map(foodReadVal, 0, maxFoodVal, 0, 100);  // Set h2oLevel to 0 - 100 scale by mapping analogRead value
  foodAmtCons = foodLastLevel - foodLevel;  // Calculate food amount consumed since last read
}

void doorStateRead() {
  doorLastTimer = currTimer - doorTimer;  // Calculate elapsed time since last door opening
  doorState = digitalRead(doorPin);  // Read state of stall door (open/closed)
  if(doorState == 1 && doorLastState == 0) {  // If door is detected as open and was closed on last read
    openCount++;  // Increment door opening counter
    eventCode = 1;  // Set event code to indicate door was just opened
    if(doorTimer == 0 || doorLastTimer > doorResetInterval) {  // If door timer never started or refill interval exceeded
      doorTimer = currTimer;  // Set door timer to current time
    }
  }
  doorLastState = doorState;  // New status of door (open/closed) placed in memory for comparison on next read
}

void pirStateRead() {
  pirLastTimer = currTimer - pirTimer;  // Calculate elapsed time since last door opening
  pirState = digitalRead(pirPin);  // Read state of pir sensor
  if(pirState == 1) {  // If motion was detected
    pirTimer = currTimer;  // Set pir timer to current time
  }
  if(pirLastTimer > pirStillInterval) {  // If no motion has been detected for the previously defined still interval
    eventCode = 7;  // Set corresponding event code
  }
}

// Check h2o level for significant changes, low level, and empty bucket
void h2oCheckChange() {
  if(h2oLevel == 0) {  // If h2o level has just been detected at 0
    h2oEmpty = true;  // Change boolean to reflect presence of an empty bucket
    waitH2ORefill();  // Call function to wait until h2o bucket is refilled
  }
  else {  // **and bucket is actually not empty
    if(h2oAmtCons >= h2oChangeTrigger) {  // If h2o level has changed significantly since last read
      Serial.print(F(" >  =  10% change in h2o detected. Confirming..."));
      delay(5000);  // Pause briefly to let conditions settle in case bucket was bumped, etc.
      h2oLevelRead();  // Read h2o level again
      if(h2oAmtCons >= h2oChangeTrigger) {  // If h2o change detected was real
        Serial.println(F("change confirmed and logged."));
        Serial.println();
        dataWriteSD();  // Write new h2o level to SD
        delay(500);  // A brief delay is necessary to allow dataWriteSD() to finish
        prevTimer = currTimer;  // Set regular SD write timer to 0
      }
      else {  // If, after 2nd read, there wasn't actually a significant change
        Serial.println(F("detected change was a false - positive."));  // Tell user that the detected change was a false - positive
      }
      Serial.println();
    }
  }
}

void waitH2ORefill() {
  // NOTE: MAY BREAK CALLED FUNCTION DOWN INTO "IF/ELSE" BRANCH STRUCTURE TO PREVENT HANGING ON PARTIAL REFILL
  while(h2oEmpty == true) {  // As long as boolean continues to indicate that bucket is empty
    h2oLevelRead();  // Read current h2o level
    Serial.println(F("H2O bucket empty...waiting for refill."));  // Present some messages to the user
    Serial.print(F("Current level: "));
    Serial.println(h2oLevel);  // Current h2o level
    Serial.print(F("Full level:    "));
    Serial.println(h2oFullLevel);  // Value that h2o level needs to be raised to before bucket considered full
    if(h2oLevel >= h2oFullLevel) {  // If h2o level is now above the threshold to be considered "refilled" or "full"
      h2oEmpty = false;  // Change boolean to indicate that bucket is now full
      Serial.println(F("H2O bucket has been refilled."));
      Serial.println();
    }
    else {  // If h2o level still below "full" level
      Serial.println();
      delay(10000);  // Wait for a few seconds to allow time for bucket refill
    }
  }
}

// Check food level for significant changes, low level, and empty bucket
void foodCheckChange() {
  if(foodEmpty == false) {  // If bucket isn't empty according to boolean value currently stored,**
    if(foodLevel == 0) {  // **but food level has just been detected at 0
      Serial.println(F("Food bucket empty or currently being refilled."));
      foodEmpty = true;  // Change boolean to reflect presence of an empty bucket
    }
    else {  // **and bucket is actually not empty
      if(foodAmtCons >= foodChangeTrigger) {  // If food level has changed significantly since last read
        Serial.print(F(" >  =  10% change in food detected. Confirming..."));
        delay(5000);  // Pause briefly to let conditions settle in case bucket was bumped, etc.
        foodLevelRead();  // Read food level again
        if(foodAmtCons >= foodChangeTrigger) {  // If food change detected was real
          Serial.println(F("change confirmed and logged."));
          Serial.println();
          dataWriteSD();  // Write new food level to SD
          delay(500);  // A brief delay is necessary to allow dataWriteSD() to finish
          prevTimer = currTimer;  // Set regular SD write timer to 0
        }
        else {  // If, after 2nd read, there wasn't actually a change
          Serial.println(F("detected change was a false - positive."));  // Tell user that the detected change was a false - positive
        }
        Serial.println();
      }
    }
  }
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 ////   SD and data logging  ////
 = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void dataWriteSD() {
  getDateTime();  // Read and parse date/time information from GPRS

  dataString = dateString;
  dataString += ",";
  dataString += timeString;
  dataString += ",";
  dataString += h2oLevel;
  dataString += ",";
  dataString += h2oAmtConsTot;
  dataString += ",";
  dataString += foodLevel;
  dataString += ",";
  dataString += foodAmtConsTot;
  dataString += ",";
  dataString += doorState;
  dataString += ",";
  dataString += openCount;
  dataString += ",";
  dataString += pirState;
  dataString += ",";
  dataString += eventCode;
  dataString += ",";
  dataString += errorCode;
  delay(200);

  if(dataFile.open("LOGX.TXT", O_RDWR | O_CREAT | O_AT_END)) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  }
  else {
    sd.errorHalt("Failed to open file for writing");
  }
}

/*void codesWriteSD() {
 if(eventCode > 0) {
 eventString = "";
 getDateTime();  // Read and parse date/time information from GPRS
 
 eventString = dateString;
 eventString += ",";
 eventString += timeString;
 eventString += " - ";
 eventString += eventString;
 
 dataFile = SD.open("EVENTS.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
 if(dataFile) {  // If file on SD card is opened successfully
 dataFile.println(eventString);  // Print errorString to it
 dataFile.close();  // Close the data file and proceed
 Serial.println(eventString);
 
 }
 else {
 Serial.println("Error opening file for writing event information.");
 }
 eventCode = 0;  // Reset error code after writing to SD
 delay(500);  // A brief delay is required to allow SD write to finish
 }
 if(errorCode > 0) {
 defineError();  // Translate integer into human - readable error string
 getDateTime();  // Read and parse date/time information from GPRS
 
 errorString = dateString;
 errorString += ", ";
 errorString += timeString;
 errorString += " - ";
 errorString += errorString;
 
 dataFile = SD.open("ERRORS.TXT", FILE_WRITE);  // Open data file, with full privileges, for writing
 if(dataFile) {  // If file on SD card is opened successfully
 dataFile.println(errorString);  // Print errorString to it
 dataFile.close();  // Close the data file and proceed
 Serial.println(errorString);
 }
 else {
 Serial.println("Error opening file for writing error information.");
 }
 errorCode = 0;  // Reset error code after writing to SD
 delay(500);  // A brief delay is required to allow SD write to finish
 }
 }*/

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 ////  SMS data summary  ////
 = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*void createSMSSummary() {
 }
 
 void sendSMSSummary() {
 }*/

/* = = = = = = = = = = = = = = = = = = = = = = = = = == = 
 ////  Errors and events  ////
 = = = = = = = = = = = = = = = = = = = = = = = = == = */

// Check for conditions defined as an "event" and set corresponding code
void eventCheck() {
  if(h2oEmpty == true && foodEmpty == true) {  // If h2o bucket and food bucket are empty
    eventCode = 4;  // Set corresponding event code
  }
  else if(h2oEmpty == true) {  // If h2o bucket is empty but food bucket is not empty
    eventCode = 2;  // Set corresponding event code
  }
  else if(foodEmpty == true) {  // If food bucket is empty but h2o bucket is not empty
    eventCode = 3;  // Set corresponding event code
  }
}

// Delared as independent function for future use when additional error checks may be required
void errorCheck() {
  if(openCount == 0 && h2oAmtCons < h2oErrorTrigger) {  // If door hasn't been opened, an increase in h2o level detected
    errorCode = 1;  // Set corresponding error code
  }
}

/*void defineEvent() {
 eventString = "Event: ";
 switch(eventCode) {  // Translate event code integer into human - readable string
 case 0:
 break;
 case 1:
 eventString = "Stall door opened.";
 break;
 case 2:
 eventString = "H2O bucket empty.";
 break;
 case 3:
 eventString = "Food bucket empty.";
 break;
 case 4:
 eventString = "H2O bucket and food bucket empty.";
 break;
 case 5:
 eventString = "H2O bucket refilled.";
 break;
 case 6:
 eventString = "Food bucket refilled.";
 break;
 }
 }
 
 void defineError() {
 errorString = "Error: ";
 switch(errorCode) {  // Translate error code integer into human - readable string
 case 0:
 break;
 case 1:
 errorString += "H2O level increase detected while stall door closed.";
 break;
 }
 }*/

void debugMode() {
  if(digitalRead(debugPin) == 1) {
    debugState = true;
  }
  if(debugState == true) {
    EEPROM.write(255, 0);  // Force calibration upon program startup
  }
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 ////  Camera functions  ////
 = = = = = = = = = = = = = = = = = = = = = = = = = = */

// Reset VC0706
void VC0706_reset()
{
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_RESET;
  tx_vcbuffer[3] = 0x00;

  tx_counter = 4;

  buffer_send();
}

// Request version string from VC0706
void VC0706_get_version()
{
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_GEN_VERSION;
  tx_vcbuffer[3] = 0x00;

  tx_counter = 4;

  buffer_send();
}

// Start or stop TV output from VC0706
/*
Input
 on = 0 - - > Stop TV output
 on = 1 - - > Start TV output
 */
void VC0706_tv_out_control(int on)
{
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_TV_OUT_CTRL;
  tx_vcbuffer[3] = 0x01;
  tx_vcbuffer[4] = on;
  tx_counter = 5;

  buffer_send();
}

//Add OSD characters to channels (channel 1)
/*
Input
 col: Display column
 row: Display row
 osd_string: Display string (max 14 char)
 */
void VC0706_osd_add_char(int col, int row, String osd_string)
{
  unsigned char col_row;
  int string_length;
  int i;

  col &= 0x0f;
  row &= 0x0f;
  col_row = (unsigned char)(col << 4 | row);

  string_length = osd_string.length();
  if (string_length > 14)
    string_length = 14;		// max 14 osd characters

  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_OSD_ADD_CHAR;
  tx_vcbuffer[3] = string_length + 2;		
  tx_vcbuffer[4] = string_length;		// character number
  tx_vcbuffer[5] = col_row;					

  for (i = 0; i < string_length; i++)
  {
    tx_vcbuffer[i + 6] = osd_string.charAt(i);
  }

  tx_counter = string_length + 6;

  buffer_send();
}

// Control width and height downsize attribute
/*
Input
 scale_width (and scale_height) = 0 - - > 1:1
 1 - - > 1:2
 2 - - > 1:4
 */
void VC0706_w_h_downsize(int scale_width, int scale_height)
{
  int scale;

  if (scale_width >= 2)	scale_width = 2;
  if (scale_height > scale_width)	scale_height = scale_width;
  scale = (unsigned char)(scale_height << 2 | scale_width);


  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_DOWNSIZE_SIZE;
  tx_vcbuffer[3] = 0x01;

  tx_vcbuffer[4] = scale;		//bit[1:0] width zooming proportion
  //bit[3:2] height zooming proportion

  tx_counter = 5;

  buffer_send();
}

// Read image data from FBUF
/*
Input
 buffer_address(4 bytes)
 buffer_length(4 bytes)
 */
void VC0706_read_frame_buffer(unsigned long buffer_address, unsigned long buffer_length)
{

  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_READ_FBUF;
  tx_vcbuffer[3] = 0x0c;
  tx_vcbuffer[4] = FBUF_CURRENT_FRAME;
  tx_vcbuffer[5] = 0x0a;		// 0x0a = data transfer by MCU mode; 0x0f = data transfer by SPI interface
  tx_vcbuffer[6] = buffer_address >> 24;			//starting address
  tx_vcbuffer[7] = buffer_address >> 16;			
  tx_vcbuffer[8] = buffer_address >> 8;			
  tx_vcbuffer[9] = buffer_address&0x0ff;			

  tx_vcbuffer[10] = buffer_length >> 24;		// data length
  tx_vcbuffer[11] = buffer_length >> 16;
  tx_vcbuffer[12] = buffer_length >> 8;		
  tx_vcbuffer[13] = buffer_length&0x0ff;
  tx_vcbuffer[14] = 0x00;		// delay time
  tx_vcbuffer[15] = 0x0a;


  tx_counter = 16;

  buffer_send();
}

// Control frame buffer register
/*
Input
 frame_control = 0 - - > stop current frame
 1 - - > stop next frame
 2 - - > step frame
 3 - - > resume frame
 */
void VC0706_frame_control(byte frame_control)
{
  if(frame_control > 3)frame_control = 3;
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_FBUF_CTRL;
  tx_vcbuffer[3] = 0x01;
  tx_vcbuffer[4] = frame_control;
  tx_counter = 5;

  buffer_send();
}

// Get motion monitoring status in communication interface
/*
Input
 control_flag = 0 - - > stop motion moitoring
 1 - - > start motion monitoring
 */
void VC0706_motion_detection(int control_flag)
{
  if(control_flag > 1)control_flag = 1;
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_COMM_MOTION_CTRL;
  tx_vcbuffer[3] = 0x01;
  tx_vcbuffer[4] = control_flag;
  tx_counter = 5;

  buffer_send();
}

// Motion control
/*
Input
 control_flag = 0 - - > forbid motion monitoring
 1 - - > enable motion monitoring
 */
void VC0706_motion_control(int control_flag)
{
  if(control_flag > 1)control_flag = 1;
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_MOTION_CTRL;
  tx_vcbuffer[3] = 0x03;
  tx_vcbuffer[4] = 0x00;			//motion control attribute
  tx_vcbuffer[5] = 0x01;			//mcu uart control
  tx_vcbuffer[6] = control_flag;	
  tx_counter = 7;

  buffer_send();
}

// Get byte - lengths in FBUF
/*
Input
 fbuf_type = 0 - - > current frame
 1 - - > next frame
 */
void VC0706_get_framebuffer_length(byte fbuf_type)
{
  if(fbuf_type > 1)fbuf_type = 1;
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_GET_FBUF_LEN;
  tx_vcbuffer[3] = 0x01;
  tx_vcbuffer[4] = fbuf_type;
  tx_counter = 5;

  buffer_send();
}

// Stop current frame for reading
/*
Input
 power_on = 1 - - > start power - save
 0 - - > stop power - save
 */
void VC0706_uart_power_save(byte power_save_on)
{
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_POWER_SAVE_CTRL;
  tx_vcbuffer[3] = 0x03;
  tx_vcbuffer[4] = 00;			//power save control mode
  tx_vcbuffer[5] = 01;			// control by UART
  tx_vcbuffer[6] = power_save_on;		//start power save
  tx_counter = 7;

  buffer_send();
}

// Stop current frame for reading
/*
Input
 show_mode = 0 - - > automatically step black - white and color
 1 - - > manually step color, select color
 2 - - > manually step color, select black - white
 */
void VC0706_uart_color_control(byte show_mode)
{
  if(show_mode > 2) show_mode = 2;
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_COLOR_CTRL;
  tx_vcbuffer[3] = 0x02;
  tx_vcbuffer[4] = 01;		//control by UART
  tx_vcbuffer[5] = show_mode;	// automatically step black - white and colour
  tx_counter = 6;

  buffer_send();
}

// Stop current frame for reading
/*
Input
 (minimum) 13 < ration < 63 (maximum)
 */
void VC0706_compression_ratio(int ratio)
{
  if(ratio > 63)ratio = 63;
  if(ratio < 13)ratio = 13;
  int vc_comp_ratio = (ratio - 13)*4 + 53;
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_WRITE_DATA;
  tx_vcbuffer[3] = 0x05;
  tx_vcbuffer[4] = 01;		//chip register
  tx_vcbuffer[5] = 0x01;	        //data num ready to write
  tx_vcbuffer[6] = 0x12;	        //register address
  tx_vcbuffer[7] = 0x04;
  tx_vcbuffer[8] = vc_comp_ratio;   //data

  tx_counter = 9;

  buffer_send();
}

// Motion windows setting
/*
Input
 register_address (2 bytes)
 data (4 bytes) = data ready to write
 */
void VC0706_motion_windows_setting(unsigned int register_address, unsigned long data)
{
  tx_vcbuffer[0] = VC0706_PROTOCOL_SIGN;
  tx_vcbuffer[1] = VC0706_SERIAL_NUMBER;
  tx_vcbuffer[2] = VC0706_COMMAND_WRITE_DATA;
  tx_vcbuffer[3] = 0x08;
  tx_vcbuffer[4] = 01;		//chip register
  tx_vcbuffer[5] = 0x04;	//data num ready to write
  tx_vcbuffer[6] = register_address >> 8;	//register address
  tx_vcbuffer[7] = register_address&0x0ff;
  ;

  tx_vcbuffer[8] = data >> 24;		// data ready to write
  tx_vcbuffer[9] = data >> 16;
  tx_vcbuffer[10] = data >> 8;		
  tx_vcbuffer[11] = data&0x0ff;

  tx_counter = 12;

  buffer_send();
}

// Transmit buffer to Arduino serial
/*
Input
 tx_vcbuffer
 */
void debug_send()
{
  int i = 0;

  for (i = 0;i < tx_counter;i++)
  {
    Serial2.print(tx_vcbuffer[i], HEX);
    Serial2.print(", ");
  }

  Serial2.println("");
}

// Transmit buffer to VC0706
/*
Input
 tx_vcbuffer
 */
void buffer_send()
{
  int i = 0;

  for (i = 0;i < tx_counter;i++)
    Serial2.write(tx_vcbuffer[i]);

  tx_ready = true;
}

// Receive buffer from VC0706
/*
Output
 rx_buffer
 rx_ready
 */
void buffer_read()
{
  bool validity = true;

  if (rx_ready)			// if something unread in buffer, just quit
    return;

  rx_counter = 0;
  VC0706_rx_buffer[0] = 0;
  while (Serial2.available() > 0) 
  {
    VC0706_rx_buffer[rx_counter++] = Serial2.read();
    //delay(1);
  }
  if (VC0706_rx_buffer[0] != 0x76)
    validity = false;
  if (VC0706_rx_buffer[1] != VC0706_SERIAL_NUMBER)
    validity = false;

  if (validity) rx_ready = true;
}

// Capture a photo and store file to SD
void capture_photo() {	  
  // open a new empty file for write at end like the Native SD library
  if (!dataFile.open("test.jpg", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("Failed to open jpg for storing photo.");
  }

  // close the file:
  dataFile.close();

  VC0706_compression_ratio(63);
  delay(100);

  VC0706_frame_control(3);
  delay(10);

  VC0706_frame_control(0);
  delay(10);
  rx_ready = false;
  rx_counter = 0;

  Serial2.end();			// clear all rx buffer
  delay(5);

  Serial2.begin(115200);

  //get frame buffer length
  VC0706_get_framebuffer_length(0);
  delay(10);
  buffer_read();

  //while(1){};

  // store frame buffer length for coming reading
  frame_length = (VC0706_rx_buffer[5] << 8) + VC0706_rx_buffer[6];
  frame_length = frame_length << 16;
  frame_length = frame_length + (0x0ff00 & (VC0706_rx_buffer[7] << 8)) + VC0706_rx_buffer[8];

  vc_frame_address = READ_DATA_BLOCK_NO;

  dataFile.open("test.jpg", O_RDWR);	
  while(vc_frame_address < frame_length){	
    VC0706_read_frame_buffer(vc_frame_address - READ_DATA_BLOCK_NO, READ_DATA_BLOCK_NO);
    delay(9);

    //get the data with length = READ_DATA_BLOCK_NObytes 
    rx_ready = false;
    rx_counter = 0;
    buffer_read();

    // write data to photo.jpg
    dataFile.write(VC0706_rx_buffer + 5, READ_DATA_BLOCK_NO);

    //read next READ_DATA_BLOCK_NO bytes from frame buffer
    vc_frame_address = vc_frame_address + READ_DATA_BLOCK_NO;

  }

  // get the last data
  vc_frame_address = vc_frame_address - READ_DATA_BLOCK_NO;

  last_data_length = frame_length - vc_frame_address;

  VC0706_read_frame_buffer(vc_frame_address, last_data_length);
  delay(9);
  //get the data 
  rx_ready = false;
  rx_counter = 0;
  buffer_read();

  dataFile.write(VC0706_rx_buffer + 5, last_data_length);
  dataFile.close();
}
