// #$Id: MOT-C168i.h,v 1.2 2009/05/04 23:35:19 gvonroth Exp $

//There are the AT command set setting specfic to the Motorola Phone model C168i

//Most of these command "should work with other phones but I have not tested

// serial speed to use with phone
#define PHONE_SERIAL_SPEED 4800


//Delay after sending a serial command.  The phone seem to like to have a moment to process teh last command.  I am chooseing the below value as a "shot in the dark"
#define PHONE_DELAY_AFTER_SENDING_COMMAND_VALUE 500


// Resets the phone
#define PHONE_RESET_COMMAND "AT*SWRESET"

//  Puts teh phone into SMS texting mode ( as opposed to MMS, etc.)
#define PHONE_SET_SMS_TXT_MODE_COMMAND "AT+CMGF=1" 

//  Command used to send a message
#define PHONE_SEND_MSG "AT+CMGS=" 

// Turns OFF the phone's transmitter
#define PHONE_TURN_OFF_COMMAND "AT+CFUN=0" 

//  Turns ON the Phone's Transmitter
//  The correct command "AT+CFUN=1" does not work on this phone so just reset
#define PHONE_TURN_ON_COMMAND "AT*SWRESET"

// This command will return the battery status (0-100)
#define PHONE_BATT_CHARGE_CHECK_COMMAND          "AT+CBC" 

// This command returns the signal strength (0-30)
#define PHONE_SIGNAL_STRENGTH_CHECK_COMMAND      "AT+CSQ"
