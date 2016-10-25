/*   2 - Camera board w/ jpg capture on motion detection w/ SD storage  */

/******************** (C) COPYRIGHT 2012 RadioSHack ********************
 * File Name          : camsd.pde
 * Author             : TRS
 * Version            : V1.0
 * Date               : 28/05/2012
 * Description        : Main program body
 ********************************************************************************
 * THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, RS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *******************************************************************************/

#include <avr/pgmspace.h>
#include "Arduino.h"


/*
 * The following code includes all necessary routines to control Camera Shield
 *
 * In addition, we have prepared a example for making surveillance camera by putting Camera shield (RS SKU 2760248) and SD Card shield (RS SKU 2760243) together. 
 * 1) Install SD Card shield properly and download all necessary FAT drivers from shield providers
 * 2) Install Camera shield properly
 * 3) Enable "#define sdCamera 1" to enable the demo code
 * 4) With the built-in motion detection function, camera shield will capture image and store to SD card automatically .
 *
 */

#define sdCamera 1	// ***** Enable it for Surveillance Camera demonstration *****

#ifdef sdCamera
#include <Sd2Card.h>	// ***** SD card driver from http://www.seeedstudio.com/wiki/SD_Card_Shield or SD card shield supplier *****
#include <SdFat.h>	// ***** FAT file system from http://www.seeedstudio.com/wiki/SD_Card_Shield or SD card shield supplier *****
#endif

/*
 * SD chip select pin.  Common values are:
 *
 * Arduino Ethernet shield, pin 4.
 * SparkFun SD shield, pin 8.
 * Adafruit SD shields and modules, pin 10.
 * Default SD chip select is the SPI SS pin.
 */

#ifdef sdCamera
const uint8_t SdChipSelect = SS;
SdFat sd;
Sd2Card card;
SdFile myFile;
#endif

#define NORMAL_USE	1

#define VC0706_PROTOCOL_SIGN 			0x56
#define VC0706_SERIAL_NUMBER 			0x00

#define VC0706_COMMAND_RESET			0x26
#define VC0706_COMMAND_GEN_VERSION		0x11
#define VC0706_COMMAND_TV_OUT_CTRL		0x44
#define VC0706_COMMAND_OSD_ADD_CHAR		0x45
#define VC0706_COMMAND_DOWNSIZE_SIZE		0x53
#define VC0706_COMMAND_READ_FBUF		0x32
#define FBUF_CURRENT_FRAME			0
#define FBUF_NEXT_FRAME				0

#define VC0706_COMMAND_FBUF_CTRL		0x36
#define VC0706_COMMAND_COMM_MOTION_CTRL		0x37
#define VC0706_COMMAND_COMM_MOTION_DETECTED	0x39
#define VC0706_COMMAND_POWER_SAVE_CTRL		0x3E
#define VC0706_COMMAND_COLOR_CTRL		0x3C
#define VC0706_COMMAND_MOTION_CTRL		0x42

#define VC0706_COMMAND_WRITE_DATA		0x31
#define VC0706_COMMAND_GET_FBUF_LEN		0x34

#define READ_DATA_BLOCK_NO			56

unsigned char 	tx_counter;
unsigned char 	tx_vcbuffer[20];
bool		tx_ready;

bool		rx_ready;
unsigned char 	rx_counter;
unsigned char 	VC0706_rx_buffer[80]; 

uint32_t 	frame_length=0;

uint32_t 	vc_frame_address =0;

uint32_t 	last_data_length=0;


//------------------------------------------------------------------------------
void buffer_send();

// ***********************************************************************************************************
// *
// *                            Power Up Init.
// *
// *
// ***********************************************************************************************************
void setup() 
{

#ifdef NORMAL_USE

#ifdef sdCamera
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  if (!sd.begin(SdChipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();


  Serial2.begin(115200);

  capture_photo();
  delay(100);

  VC0706_frame_control(3);
  delay(10);

  //motion windows setting
  VC0706_motion_windows_setting(0x1a5a,0x5000);
  delay(10);
  VC0706_motion_windows_setting(0x1a5e,0x0a000f000);
  delay(10);
  VC0706_motion_windows_setting(0x1a62,0x3f010000);
  delay(10);
  VC0706_motion_windows_setting(0x1a66,0x3c007800);
  delay(10);
  VC0706_motion_windows_setting(0x1a6a,0x0b400ef00);
  delay(10);

  //start motion monitoring
  VC0706_motion_control(1);
  delay(10);
  VC0706_motion_detection(1);
  delay(10);
  Serial2.end();			// clear all rx buffer
  delay(5);
  Serial2.begin(115200);
  rx_ready=false;
#endif


#else
#endif
}

// ***********************************************************************************************************
// *
// *                            Main Loop 
// *
// *
// ***********************************************************************************************************
void loop() 
{
#ifdef sdCamera
  buffer_read();

  if(rx_ready){
    rx_ready=false;
    if (VC0706_rx_buffer[2]!=VC0706_COMMAND_COMM_MOTION_DETECTED) return;
    if (VC0706_rx_buffer[3]!=0x00) return;

    //stop motion detection for capture photo
    VC0706_motion_control(0);
    delay(10);
    VC0706_motion_detection(0);
    delay(1000);

    // capture current photo 1s later
    capture_photo();
    delay(100);

    VC0706_frame_control(3);	// resume AV out
    delay(10);

    //prepare next motion monitoring
    VC0706_motion_control(1);
    delay(10);
    VC0706_motion_detection(1);
    delay(10);
    Serial2.end();			// clear all rx buffer
    delay(5);
    Serial2.begin(115200);
    rx_ready=false;
    return;

  };
#endif	

  delay(300);

}

/*******************************************************************************
 * Function Name  : VC0706_reset
 * Description    : Reset VC0706
 *                  
 * Input          : None
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_get_version
 * Description    : Request version string from VC0706
 *                  
 * Input          : None
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_tv_out_control
 * Description    : stop or start TV output from VC0706
 *                  
 * Input          : on=0 stop tv output
 * ;			:  on=1 start tv output
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_osd_add_char
 * Description    : ADD OSD CHARACTERS TO CHANNELS(CHANNEL 1)
 *                  
 * Input          : col : Display column
 *	`		  row: Display Row
 *			  osd_string : display string (max 14 characters)
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_w_h_downsize
 * Description    : control width and height downsize attribute
 *                  
 * Input          : scale_width = 0 1:1
 *                  			      = 1 1:2	
 *                  			      = 2 1:4	
 * 			  scale_height= 0 1:1
 *                  			      = 1 1:2
 *                  			      = 2 1:4
 *                  			
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_read_frame_buffer
 * Description    : read image data from FBUF 
 *                  
 * Input          : buffer_address(4 bytes); buffer_length(4 bytes)
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_frame_control
 * Description    : control frame buffer register
 *                  
 * Input          : frame_control=control flag(1byte)
 *			: 		0 = stop current frame ; 1= stop next frame;2=step frame;3 =resume frame;
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_motion_detection
 * Description    : get motion monitoring status in communication interface.
 *                  
 * Input          : control_flag = 0 stop motion monitoring
 *					      = 1 start motion monitoring                  
 *
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_motion_control
 * Description    : motion control
 *                  
 * Input          : control_flag = 0 forbid motion monitoring
 *					      = 1 enable motion monitoring                  
 *
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : VC0706_get_framebuffer_length
 * Description    : get byte-lengths in FBUF
 *                  
 * Input          : fbuf_type =current or next frame
 *			            0   =  current frame
 *				     1   =  next frame
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/




/*******************************************************************************
 * Function Name  : VC0706_uart_power_save
 * Description    : stop current frame for reading
 *                  
 * Input          : power_on = 1  start power-save
 *			     = 0  stop power-save
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/



/*******************************************************************************
 * Function Name  : VC0706_uart_color_control
 * Description    : stop current frame for reading
 *                  
 * Input          : show_mode = 0   automatically step black-white and colour
 *				1   manually step color, select colour
 *				2   manually step color, select black-white
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/




/*******************************************************************************
 * Function Name  : VC0706_compression_ratio
 * Description	  : stop current frame for reading
 *					
 * Input		  : ration		>13(minimum)
 *						<63(max)
 *					
 * Output		  : None
 * Return		  : None
 *******************************************************************************/



/*******************************************************************************
 * Function Name  : VC0706_motion_windows_setting
 * Description	  : motion windows setting
 *					
 * Input		  : register_address(2 bytes); 
 *				data(4 bytes)= data ready to write
 *					
 * Output		  : None
 * Return		  : None
 *******************************************************************************/





/*******************************************************************************
 * Function Name  : debug_send
 * Description	  : Transmit buffer to Arduino Serial2 Monitor
 *					
 * Input		  : tx_vcbuffer
 *					
 * Output		  : None
 * Return		  : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : buffer_send
 * Description    : Transmit buffer to VC0706
 *                  
 * Input          : tx_vcbuffer
 *                  
 * Output         : None
 * Return         : None
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : buffer_read
 * Description    : Receive buffer from VC0706
 *                  
 * Input          : None
 *                  
 * Output         : rx_buffer, rx_ready
 * Return         : None
 *******************************************************************************/


#ifdef sdCamera

/*******************************************************************************
 * Function Name  : capture_photo
 * Description	  : capture a photo and store the file named temp.jpg into SD
 *					
 * Input		  : None
 *					
 * Output		  : None
 * Return		  : None
 *******************************************************************************/


#endif
