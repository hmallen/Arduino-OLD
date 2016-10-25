/*
  SSerial2Mobile.h - Software serial to Mobile phone library   
  Copyright (c) 2008 Gustav von Roth.  All right reserved.
  SSerial2Mobile@vonroth.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// #$Id: SSerial2Mobile.h,v 1.3 2009/05/04 23:32:23 gvonroth Exp $

#ifndef SSerial2Mobile_h
#define SSerial2Mobile_h
 
//#include "WConstants.h"
#include "Arduino.h"
#include <SoftwareSerial.h>

 
class SSerial2Mobile {
 
     public:
           SSerial2Mobile(int rx, int tx);
           void println(const char[]); 
           void off(void);
	   void on(void);
	   void reset(void);
	   void sendTxt(const char number[],const char msg[]);
	   void sendEmail(const char emailAddr[],const char msg[]);

           int isOK();
           int wait4OK();
           int batt();
           int rssi();

     private:
           SoftwareSerial _sSerial;
           void readline(void);
           uint32_t parsedecimal(char *str);
     
};
 
#endif 

