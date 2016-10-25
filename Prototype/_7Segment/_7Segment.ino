byte seven_seg_digits[10][7] = {
  {
    0,0,0,0,0,0,1                    }
  ,
  {
    1,0,0,1,1,1,1                    }
  ,
  {
    0,0,1,0,0,1,0                    }
  ,
  {
    0,0,0,0,1,1,0                    }
  ,
  {
    1,0,0,1,1,0,0                    }
  ,
  {
    0,1,0,0,1,0,0                    }
  ,
  {
    0,1,0,0,0,0,0                    }
  ,
  {
    0,0,0,1,1,1,1                    }
  ,
  {
    0,0,0,0,0,0,0                    }
  ,
  {
    0,0,0,1,1,0,0                    }
};

void setup() {
  pinMode(2, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(2, OUTPUT);
}

void sevenSegWrite(byte digit) {
  byte pin = 2;
  for(byte segCount = 0; segCount < 7; ++segCount) {
    digitalWrite(pin, seven_seg_digits[digit][segCount]);
    ++pin;
  }
}

void loop() {
  for(byte count = 10; count > 0; --count) {
    delay(1000);
    sevenSegWrite(count - 1);
  }
  delay(3000);
}
