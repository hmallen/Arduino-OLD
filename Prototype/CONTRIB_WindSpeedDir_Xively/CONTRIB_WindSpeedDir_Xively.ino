/* 
 http://www.qsl.net/on7eq/projects/arduino_davis.htm
 
 Sketch to read David Wind Vane  
 de ON7EQ Dec 2011
 
 To disable interrupts:
 cli();                // disable global interrupts 
 and to enable them:  
 sei();                // enable interrupts
 
 NTC readout routine
 
 http://arduino.cc/playground/ComponentLib/Thermistor2
 
 =================================================================== 
 Thermistor Schematic
 =================================================================== 
 
 (+5v ) ---- (10k-Resister) -------|------- (Thermistor) ---- (GND)
 |
 Analog Pin 'NTCpin'
 
 =================================================================== 
 DAVIS Vantage Pro & Vantage Pro 2    Wind Sensor (speed & direction
 ===================================================================
 
 On RJ-45 plug terminals:
 
 Black =  pulse from anemometer. Connect to Digital 2 pin, and use a 4k7 resistor as pull up to + 5v.
 use a 10 to 22nF capacitor from pin D2 to ground to debounce the reed switch of anemometer
 
 Red =    Ground
 
 Green =  Wiper  of Wind direction vane - connect to A0.  Use a 1 ... 10 µF / 16v capacitor between A0 and ground (observe C polarity) to avoid jitter
 
 Yellow = + 5v (reference of potentiometer)
 
 */

// include EEPROM write - required to memorize antenna / band config.
#include <EEPROM.h>

#include <math.h>

#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int Direction ; // Wind direction

#define PotPin (A0)    // define the input pin for the wind vane potentiometer
int PotValue = 0;      // variable to store the value coming from the potentiometer
int DirCorr = 0;       // Correction on direction ( - 360  to + 360)

#define CalPin (A1)    // define the input pin to initiate direction calibration @ startup. Ground pin to calibrate
byte DirCorrB1 = 0;    // 2 bytes of DirCorr
byte DirCorrB2 = 0;

volatile unsigned long RPMTops;  // RPM tops counter in interrupt routine                             
volatile unsigned long ContactTime;  // Timer to avoid conatct bounce in interrupt routine                              

float RPM;       // RPM count
float TEMP;      // Temp

#define RPMsensor (2)      //The pin location of the anemometer sensor

#define  NTCpin    (A4)    // Pin for NTC 10k

float temp = (0);

// build LCD specific characters 'degrees'
byte degree [8] = {
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
};

// This function will calculate temperature from 10k NTC readout
double Thermister(int RawADC) {
  double Temp;
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
  // Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
  return Temp;
}


////////////////////////////////////////////////////////////////////
void setup() { 

  //  Clean EEPROM
  //  EEPROM.write (1, 0);
  //  EEPROM.write (2, 0);

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.clear(); 

  // Print a message to the LCD.
  lcd.print(" DAVIS Readout");
  lcd.setCursor(0, 1);  
  lcd.print(" v2.0 de ON7EQ"); 


  delay (2000);

  // CALIBRATE if button depressed at startup !

  if ((analogRead(CalPin)<512)) calibrate ();


  // else retrieve CAL vales from EEPROM

  DirCorrB1 = EEPROM.read(1);
  DirCorrB2 = EEPROM.read(2);  

  DirCorr = (DirCorrB1) + (DirCorrB2);


  // print screen template
  lcd.clear(); 

  // Print variables
  lcd.setCursor(0, 0);  
  lcd.print("Dir  "); 

  lcd.setCursor(12, 0);
  lcd.print("Temp"); 


  pinMode(RPMsensor, INPUT); 
  attachInterrupt(0, rpm, FALLING); 

  lcd.createChar(0, degree);
  lcd.setCursor(7, 0);  
  lcd.write(0);

  // speed
  lcd.setCursor(0, 1);
  lcd.print("Spd   0km/h");

} 

/////////////////////////////////////////////////////////////////////////////////

void loop() { 



  // Wind Direction

  PotValue = analogRead(PotPin);     // read the value from the potmeter
  Direction = map(PotValue, 0, 1023, 0, 359); 
  Direction = Direction + DirCorr + 3;   // Correct for offset & 5° precision

convert:       // Convert to 360°  

  if (Direction < 0) {
    Direction = Direction + 360;
    goto convert;
  }

  if (Direction > 360) {
    Direction = Direction - 360;
    goto convert;
  }

  if (Direction == 360) Direction = 0;

  lcd.setCursor(4, 0);                // print the value from the potmeter


  if (Direction < 100)   lcd.print("0");
  if (Direction < 10)    lcd.print("0");            
  lcd.print(((Direction/5)*5), DEC);    // 5° precision is enough to print te direction value


  lcd.setCursor(9, 0);                

  if ((Direction)<23) {
    lcd.print(" N");
  } 
  if ((Direction>22) && (Direction<68)) {
    lcd.print("NE");
  } 
  if ((Direction>67) && (Direction<113)) {
    lcd.print(" E");
  } 
  if ((Direction>112) && (Direction<158)) {
    lcd.print("SE");
  } 
  if ((Direction>157) && (Direction<203)) {
    lcd.print(" S");
  } 
  if ((Direction>202) && (Direction<247)) {
    lcd.print("SW");
  } 
  if ((Direction>246) && (Direction<292)) {
    lcd.print(" W");
  } 
  if ((Direction>291) && (Direction<337)) {
    lcd.print("NW");
  } 
  if ((Direction>336) && (Direction<=360)) {
    lcd.print(" N");
  } 

  // measure & print the temp

  lcd.setCursor(11, 1);

  TEMP = ((Thermister(1023 - analogRead(NTCpin))) + 0.0);

  if (TEMP >= 0) lcd.print(" ");       // If temp positive, print space, else a '-' will show up
  if (abs(TEMP)<10) lcd.print(" "); 

  lcd.print(TEMP, DEC); 


  // measure RPM

  RPMTops = 0;   //Set NbTops to 0 ready for calculations
  sei();         //Enables interrupts
  delay (3000);  //Wait 3 seconds to average
  cli();         //Disable interrupts


  // convert to km/h


  if ((RPMTops >= 0) and (RPMTops <= 21)) RPM = RPMTops * 1.2;
  if ((RPMTops > 21) and (RPMTops <= 45)) RPM = RPMTops * 1.15;
  if ((RPMTops > 45) and (RPMTops <= 90)) RPM = RPMTops * 1.1;
  if ((RPMTops > 90) and (RPMTops <= 156)) RPM = RPMTops * 1.0;
  if ((RPMTops > 156) and (RPMTops <= 999)) RPM = RPMTops * 1.0;


  // print the speed 

  lcd.setCursor(4, 1); 

  if (RPM < 100)   lcd.print(" ");
  if (RPM < 10)    lcd.print(" ");            
  lcd.print(int(RPM), DEC); 

}


//// This is the function that interrupt calls to measure  RPM  

void rpm ()   { 

  if ((millis() - ContactTime) > 15 ) {  // debounce of REED contact. With 15ms speed more than 150 km/h can be measured
    RPMTops++; 
    ContactTime = millis();
  }

} 
//// end of RPM measure  


//// This is the function that calibrates the vane

void calibrate () {

  lcd.setCursor(0, 1);  
  lcd.print("Hold to calibr !"); 
  delay (2000);  //Wait 2 second
  if ((analogRead(CalPin)>512)) setup();  // CAL not really required ... abort !

  lcd.setCursor(0, 1);  
  lcd.print("Now calibrating ...    "); 
  delay (1000);  //Wait 1 second

  PotValue = analogRead(PotPin);     // read the value from the potmeter
  DirCorr = map(PotValue, 0, 1023, 359, 0);

  lcd.setCursor(0, 1);  
  lcd.print("CAL value = "); 
  lcd.print(DirCorr, DEC); 
  lcd.print("            ");  
  delay (2000);  //Wait 2 seconds  

  //  
  DirCorrB1 = DirCorr / 255;
  if (DirCorrB1 == 1){ 
    DirCorrB1 = 255;  
    DirCorrB2 = DirCorr - 255 ;
  }
  else {
    DirCorrB1 = DirCorr;  
    DirCorrB2 = 0;
  }   
  //
  //  DirCorrB1 = DirCorr;  
  //  DirCorrB2 = 0;
  EEPROM.write (1, DirCorrB1);
  EEPROM.write (2, DirCorrB2);

wait:
  lcd.setCursor(0, 1);  
  lcd.print("CAL OK - Release !   ");  
  if ((analogRead(CalPin)<512)) goto wait;

  lcd.setCursor(0, 1);  
  lcd.print("Now rebooting...    ");   
  delay (1000);     

  setup (); 
}
