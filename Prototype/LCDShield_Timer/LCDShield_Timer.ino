/*

 LCD Countdown Timer
 
 */

#include <LiquidCrystal.h>                                                            // Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);                                                  // My Pin Numbers

//------Variables for cursor movement and number selection--------------------------------------------------------------------------------
int x  = 7;      // x position of cursor
int y  = 1;      // y position of cursor
int i  = 0;      // The Number displayed in the Select Mode (This variable Changes)
int db = 150;    // Delay time for button debounce in Milliseconds (100 = .1 seconds, 1500 = 1.5 Seconds)
//----------------------------------------------------------------------------------------------------------------------------------------



//------Variables for the formula in the "Start Timer Equation" below, do not change these values!!---------------------------------------
int s = 0;                                       // Used for starting the timer with the select button
int f = 0;                                       // 10's of hours
int e = 0;                                       // 1's of hours
int d = 0;                                       // 10's value of minutes
int c = 0;                                       // 1's value of minutes
int b = 0;                                       // 10's of seconds Value    ( 1000 milli)
int a = 0;                                       // 1's of seconds Value     (10000 milli)
//--------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor (0, 0);
  lcd.print (" Hour Min  Sec");
  lcd.setCursor (1, 1);
  lcd.print(" 00 : 00 : 00");
  lcd.setCursor (x, y);
}

void loop()
{
  int btn;                                                                              // Declare 'b' variable for button press
  btn = analogRead (0);                                                                 // Assign variable 'b' to the Arduino's Analogue Input pin 0 to read the value of resistance
  //------Right button press----------------------------------------------------------------------------------------------------------------------------------------------------------------

  // The following program for Left and Right button press only allow cursor movement to allowed positions, i.e. between the ":" on the display


  if (btn >= 0 && btn <= 50)                                                              // Right Button Press. My Pin reading is 0. If Pin 0 reads between 0-50, do this:
  {
    delay(db);                                                                        // Button Debounce (Takes 1 sample of buttonpress for each delay cycle determined by variable 'd' in this statement)
    // Resets variable "i" to 0, Otherwise "i" will continue from previous state it was in while in the up/down keypad Mode below.
    x = x + 1;                                                                        // Moves the cursor 1 position to the right

    { 
      if (x < 2)                                                                      // If x is less than 2, this keeps the cursor between the ":" on the display
      {
        x = 2;                                                                        // Move cursor directly to 10's of hours position
      }

      if ((x > 3) && (x < 7))                                                         // If x is between 3 and 7, this keeps the cursor between the ":" on the display
      {
        x = 7;                                                                        // Move cursor to 10's of minutes position.
      }

      if ((x > 8) && (x < 12))                                                        // If x is between 8 and 12, this keeps the cursor between the ":" on the display
      {
        x = 12;                                                                       // Move cursor to 10's of seconds position.
      }

      if (x > 13)                                                                     // If x is more than 13, this keeps the cursor between the ":" on the display
      {
        x = 2;                                                                        // Move cursor back to 10's of hours (beginning) position.
      }
    }
    lcd.setCursor (x, y);                                                             // New Position of cursor
    lcd.cursor();                                                                     // Display Cursor
  }

  //------Left Button Press------------------------------------------------------------------------------------

  if (btn > 400 && btn <= 600 )                                                       // Left Button Press. My Pin reading is 479. If Pin 0 reads between 401-600, do this:
  {
    delay(db);
    x = x - 1;
    { 
      if (x < 2)                                                                      // if x is less than 2 position, this keeps the cursor between the ":" on the display
      {
        x = 13;                                                                       // Move cursor to 1's of seconds position.
      }
      if ((x > 3) && (x < 7))                                                         // If x is between 3 and 7, this keeps the cursor between the ":" on the display
      {
        x = 3;                                                                        // Move cursor to 1's of hours position.
      }
      if ((x > 8) && (x < 12))                                                        // If x is between 8 and 12, this keeps the cursor between the ":" on the display
      {
        x = 8;                                                                        // Move cursor to 1's of minutes position.
      }

      if (x > 13)                                                                     // If x is more than 13, this keeps the cursor between the ":" on the display
      {
        x = 12;                                                                       // Move cursor back to 1's of seconds (beginning) position.
      }
    }
  }
  lcd.setCursor (x, y);                                                               // New Position of cursor
  lcd.cursor();                                                                       // Display Cursor
  //--------Up Button If Statements-----------------------------------------------------------------------------------------------------------------------------------------------------------

  if (btn > 50 && btn <= 200 && x == 13)                                                  // If Up Button is Pressed and cursor is in the 1's of seconds position
  {
    delay(db);                                                                            // Button Debounce
    a = (a++);                                                                            // Increments variable "i" by 1.
    if (a > 9) a = 0;                                                                     // Always keeps variable "i" between 0 and 9
    lcd.setCursor (x, y);                                                                 // Keep current cursor position
    lcd.print(a);                                                                         // Print Value of 'i' Variable
  }

  if (btn > 50 && btn <= 200 && x == 12)                                                  // If Up Button is Pressed and cursor is in the 10's of seconds position
  {
    delay(db);                                                                            // Button Debounce
    b = (b++);                                                                            // Increments variable "i" by 1.
    if (b > 5) b = 0;                                                                     // Always keeps variable "i" between 0 and 9
    lcd.setCursor (x, y);                                                                 // Keep current cursor position
    lcd.print(b);                                                                         // Print Value of 'i' Variable
  }

  if (btn > 50 && btn <= 200 && x == 8)                                                   // If Up Button is Pressed and cursor is in the 1's of minutes position
  {
    delay(db);                                                                            // Button Debounce
    c = (c++);                                                                            // Increments variable "i" by 1.
    if (c > 9) c = 0;                                                                     // Always keeps variable "i" between 0 and 9
    lcd.setCursor (x, y);                                                                 // Keep current cursor position
    lcd.print(c);                                                                         // Print Value of 'i' Variable
  }

  if (btn > 50 && btn <= 200 && x == 7)                                                   // If Up Button is Pressed and cursor is in the 10's of minutes position
  {
    delay(db);                                                                            // Button Debounce
    d = (d++);                                                                            // Increments variable "i" by 1.
    if (d > 5) d = 0;                                                                     // Always keeps variable "i" between 0 and 9
    lcd.setCursor (x, y);                                                                 // Keep current cursor position
    lcd.print(d);                                                                         // Print Value of 'i' Variable
  }

  if (btn > 50 && btn <= 200 && x == 3)                                                   // If Up Button is Pressed and cursor is in the 1's of hours position
  {
    delay(db);                                                                            // Button Debounce
    e = (e++);                                                                            // Increments variable "i" by 1.
    if (e > 9) e = 0;                                                                     // Always keeps variable "i" between 0 and 9
    lcd.setCursor (x, y);                                                                 // Keep current cursor position
    lcd.print(e);                                                                         // Print Value of 'i' Variable
  }

  if (btn > 50 && btn <= 200 && x == 2)                                                   // If Up Button is Pressed and cursor is in the 10's of hours position
  {
    delay(db);                                                                            // Button Debounce
    f = (f++);                                                                            // Increments variable "i" by 1.
    if (f > 9) f = 0;                                                                     // Always keeps variable "i" between 0 and 9
    lcd.setCursor (x, y);                                                                 // Keep current cursor position
    lcd.print(f);                                                                         // Print Value of 'i' Variable
  }
}

