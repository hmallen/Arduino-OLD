/*
  SSerial2Mobile.cpp - Software serial to Mobile phone library 
  Copyright (c) 2008 Gustav von Roth.  All right reserved.
  SSerial2Mobile@vonroth.com

    This program is free software: you can redistribute it and/or modify
 uy(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);nder the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// #$Id: SSerial2Mobile.cpp,v 1.6 2009/05/04 23:40:05 gvonroth Exp gvonroth $

//this is only here to grab the constant for BYTE which is 0
#include "HardwareSerial.h"

#include "Arduino.h"

#include "ATT.h"
#include "MOT-C168i.h"

//extern "C" {
//	 #include <SoftwareSerial.h>
//}

#include "SSerial2Mobile.h"

// some global vars for reciving data
#define BUFFSIZ 90 // plenty big
char buffer[BUFFSIZ];
char *parseptr;
char buffidx;

 
SSerial2Mobile::SSerial2Mobile(int rx, int tx): _sSerial(rx, tx)
{
	  pinMode(rx, INPUT);
          pinMode(tx, OUTPUT);

	_sSerial.begin(19200);delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
	
}

void SSerial2Mobile::println(const char c[])
{
	_sSerial.println(c);
}

void SSerial2Mobile::off(void)
{
	println(PHONE_TURN_OFF_COMMAND);
}

void SSerial2Mobile::on(void)
{
        println(PHONE_TURN_ON_COMMAND);
}

void SSerial2Mobile::reset(void)
{
        println(PHONE_RESET_COMMAND);delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
}

void SSerial2Mobile::sendTxt(const char number[],const char msg[])
{
	_sSerial.println(PHONE_SET_SMS_TXT_MODE_COMMAND);
	delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
	_sSerial.print(PHONE_SEND_MSG);
	_sSerial.print("\"");
	_sSerial.print(number);
	_sSerial.print("\"");
	_sSerial.println();
	delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
	_sSerial.print(msg);
	// dec 26 is a <CTRL Z>
	_sSerial.println(26, DEC);
	delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
}

void SSerial2Mobile::sendEmail(const char emailAddr[],const char msg[])
{
        _sSerial.println(PHONE_SET_SMS_TXT_MODE_COMMAND);
        delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
        _sSerial.print(PHONE_SEND_MSG);
        _sSerial.print("\"");
        _sSerial.print(CARRIER_EMAIL_GATEWAY_NUMBER);
        _sSerial.print("\"");
        _sSerial.println();
        delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
	 _sSerial.print(emailAddr);
	 _sSerial.print(" ");
        _sSerial.print(msg);
        // dec 26 is a <CTRL Z>
        _sSerial.println(26, DEC);
	delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
}

//############     Stuff relating to reciving  ##########################

int SSerial2Mobile::isOK() {
     _sSerial.println("AT");
     delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
     return wait4OK();
}

int SSerial2Mobile::wait4OK() {
  int returnVal=0;
  
  while(_sSerial.available())  {
       readline();
       if (strncmp(buffer, "OK",2) == 0) {
          returnVal=1;
       } else  {
	 returnVal=-1;
       }
  }
        return returnVal;
}


int SSerial2Mobile::batt() {
 _sSerial.flush();
  _sSerial.println(PHONE_BATT_CHARGE_CHECK_COMMAND);
  delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
  while(_sSerial.available())  {
       readline();
       if (strncmp(buffer, "+CBC: ",6) == 0) {
         parseptr = buffer;
         parseptr = strchr(parseptr, ',') + 1;
         return parsedecimal(parseptr);
       }
  }
  return -2;
}


int SSerial2Mobile::rssi() {
  _sSerial.println(PHONE_SIGNAL_STRENGTH_CHECK_COMMAND);
  delay(PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE);
  while(_sSerial.available())  {
       readline();
       if (strncmp(buffer, "+CSQ: ,",6) == 0) {
         parseptr = buffer;
         parseptr = strchr(parseptr, ' ') + 1;
         return parsedecimal(parseptr);
       }
  }
  return -2;
}



void SSerial2Mobile::readline(void) {
  char c;
  
  buffidx = 0; // start at begninning
  while (1) {
      c=_sSerial.read();
      if (c == -1)
        continue;
     // Serial.print(c);
      if (c == '\n')
        continue;
      if ((buffidx == BUFFSIZ-1) || (c == '\r')) {
        buffer[buffidx] = 0;
        return;
      }
      buffer[buffidx++]= c;
  }
}

uint32_t SSerial2Mobile::parsedecimal(char *str) {
  uint32_t d = 0;
  
  while (str[0] != 0) {
   if ((str[0] > '9') || (str[0] < '0'))
     return d;
   d *= 10;
   d += str[0] - '0';
   str++;
  }
  return d;
}

