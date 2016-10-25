#include <Wire.h>

#define ADDRESS 0x76
#define MOVAVG_SIZE 32

float movavg_buff[MOVAVG_SIZE];
int movavg_i = 0; 

uint32_t D1 = 0;
uint32_t D2 = 0;
int64_t dT = 0;
int32_t TEMP = 0;
int64_t OFF = 0; 
int64_t SENS = 0; 
int32_t P = 0;
uint16_t C[7];

float Temperature;
float Pressure;


void setup() {
  // Disable internal pullups, 10Kohms are on the breakout
  PORTC |= (1 << 4);
  PORTC |= (1 << 5);

  Wire.begin();
  Serial.begin(9600); //9600 changed 'cos of timing?
  delay(100);
  initial(ADDRESS);
  //populate movavg_buff before starting loop
  for(int i = 0; i < MOVAVG_SIZE; i++) {
    //movavg_buff[i] = Pressure;
  }
}

void loop()
{
  D1 = getVal(ADDRESS, 0x48); // Pressure raw
  D2 = getVal(ADDRESS, 0x58);// Temperature raw

  dT   = D2 - ((uint64_t)C[5] << 8);
  OFF  = ((int64_t)C[2] << 16) + ((dT * C[4]) >> 7);
  SENS = ((int32_t)C[1] << 15) + ((dT * C[3]) >> 8);

  TEMP = (int64_t)dT * (int64_t)C[6] / 8388608 + 2000;

  Temperature = (float)TEMP / 100; 

  P  = ((int64_t)D1 * SENS / 2097152 - OFF) / 32768;

  Pressure = (float)P / 100;

  Serial.print("     Actual TEMP= ");
  Serial.print(Temperature);
  Serial.print("      Actual PRESSURE= ");
  pushAvg(Pressure);
  Serial.print(getAvg(movavg_buff, MOVAVG_SIZE));

  Serial.println();  
  Serial.print(" RAW Temp D2=  ");
  Serial.print(D2);
  Serial.print(" RAW Pressure D1=  ");
  Serial.println(D1);
  Serial.println();
}

void pushAvg (float val) {
  movavg_buff[movavg_i] = val;
  movavg_i = (movavg_i + 1) % MOVAVG_SIZE;
}  

float getAvg(float * buff,int size) {
  float sum = 0.0;
  for(int i=0; i < size; i++) {
    sum += buff[i];
  }
  return sum /size;
}

long getVal(int address, byte code)
{
  unsigned long ret = 0;
  Wire.beginTransmission(address);
  Wire.write(code);
  Wire.endTransmission();
  delay(10);
  // start read sequence
  Wire.beginTransmission(address);
  Wire.write((byte) 0x00);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.requestFrom(address, (int)3);
  if (Wire.available() >= 3)
  {
    ret = Wire.read() * (unsigned long)65536 + Wire.read() * (unsigned long)256 + Wire.read();
  }
  else {
    ret = -1;
  }
  Wire.endTransmission();
  return ret;
}

void initial(uint8_t address)
{

  Serial.println();
  Serial.println("PROM COEFFICIENTS ivan");

  Wire.beginTransmission(address);
  Wire.write(0x1E); // reset
  Wire.endTransmission();
  delay(10);


  for (int i=0; i<6  ; i++) {

    Wire.beginTransmission(address);
    Wire.write(0xA2 + (i * 2));
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.requestFrom(address, (uint8_t) 6);
    delay(1);
    if(Wire.available())
    {
      C[i+1] = Wire.read() << 8 | Wire.read();
    }
    else {
      Serial.println("Error reading PROM 1"); // error reading the PROM or communicating with the device
    }
    Serial.println(C[i+1]);
  }
  Serial.println();
}
