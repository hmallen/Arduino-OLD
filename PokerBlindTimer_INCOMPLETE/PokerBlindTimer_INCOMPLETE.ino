/* Poker blind count/timer
 - Make pause button
 */

#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

const int piezoPin = 40;
const unsigned long blindInterval = 600000;
const int blindMultiplier = 2;
int blindCount = 1;

int returnVal = 0;

boolean programStart = false;

int smallBlind = 5;
int bigBlind = 10;

void setup() {
  Serial.begin(19200);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Press select to");
  lcd.setCursor(0, 1);
  lcd.print("begin timer.");

  /*while(programStart == false) {
    if(returnVal == btnSELECT) programStart = true;
    delay(100);
  }*/
  lcdPrintBlinds();
}

void loop() {
  for(int x = 0; x < blindCount; x++) {
    digitalWrite(piezoPin, HIGH);
    delay(500);
    digitalWrite(piezoPin, LOW);
    delay(500);
  }
  unsigned long intervalStart = millis();
  for(unsigned long y = millis() - intervalStart; y < blindInterval; ) {
    lcd.setCursor(13, 1);
    int countdownSec = (blindInterval / 1000) - ((millis() - intervalStart) / 1000);
    lcd.print(countdownSec);
    delay(1000);
  }
  smallBlind = smallBlind * blindMultiplier;
  bigBlind = bigBlind * blindMultiplier;
  blindCount++;
  lcdPrintBlinds();
}

void lcdReadButtons() {
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in < 790)  returnVal = btnSELECT;  
}

void lcdPrintBlinds() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Current blinds:");
  lcd.setCursor(0, 1);
  lcd.print(bigBlind);
  lcd.print(" / ");
  lcd.print(smallBlind);
}
