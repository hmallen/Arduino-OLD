#include <SoftwareServo.h> 

SoftwareServo myservo;  // create servo object to control a servo 

#define pinServo A0

int speed = 1;
int limits[2] = {30,150};	// set limitations (min/max: 0->180)
boolean refresh = false;	// toggle refresh on/off

void setup() 
{ 
	Serial.begin(9600);

	// attaches the servo on pin to the servo object
	myservo.attach(pinServo);  

	// init angle of servo inbetween two limitations
	myservo.write((limits[1]-limits[0])/2); 
} 

void loop() 
{ 
	// refresh angle
	int angle = myservo.read();

	// change direction when limits
	if (angle >= limits[1] || angle <= limits[0])  speed = -speed;

	myservo.write(angle + speed);	

	// set refresh one time / 2
	refresh = refresh ? false : true;
	if (refresh) SoftwareServo::refresh();

	Serial.print("Angle: ");
	Serial.println(angle);
}
