#include <Adafruit_NeoPixel.h>

#define neoPin_eyes 6
#define neoPin_mouth 7
#define pixelCount_eyes 8
#define pixelCount_mouth 9

Adafruit_NeoPixel strip_eyes = Adafruit_NeoPixel(pixelCount_eyes, neoPin_eyes, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_mouth = Adafruit_NeoPixel(pixelCount_mouth, neoPin_mouth, NEO_GRB + NEO_KHZ800);

int m_short = 125;
int m_medium = 225;
int m_long = 500;

String eyes_color_outer = "green";
String eyes_color_inner = "blue";
String mouth_color = "blue";

void setup() {
  Serial.begin(9600);

  strip_eyes.begin();
  strip_eyes.show();
  strip_mouth.begin();
  strip_mouth.show();
}

void loop() {
  fadeFace("in");

  hello_there_gillian(mouth_color);

  delay(500);
  eyesBlink("green", "blue");
  delay(1000);
  eyesBlink("green", "blue");
  delay(500);

  looking_lovely_today(mouth_color);

  delay(500);
  eyesBlink("green", "blue");
  delay(1000);
  eyesBlink("green", "blue");
  delay(500);

  been_watching_you(mouth_color);

  delay(500);
  eyesBlink("green", "blue");
  delay(1000);
  eyesBlink("green", "blue");
  delay(500);

  mouthHeart(true, true);

  delay(500);
  eyesBlink("green", "blue");
  delay(1000);
  eyesBlink("green", "blue");
  delay(500);

  goodbye_for_now(mouth_color);

  delay(500);
  eyesBlink("green", "blue");
  delay(1000);
  eyesBlink("green", "blue");
  delay(500);

  fadeFace("out");

  while (true) {
    ;
  }
}

void hello_there_gillian(String color) {
  // Well, hello there, Gillian
  mouthAction("open", color);  // Well
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open", color);  // He-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open", color);  // -llo
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open", color);  // there
  delay(m_long);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open", color);  // Gi-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open", color);  // -lli-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open", color);  // -an
  delay(m_long);
  mouthAction("closed", color);
}

void looking_lovely_today(String color) {
  // You're looking quite lovely today my dear
  mouthAction("open" , color);  // You're
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // loo-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // -king
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // quite
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // lov-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // -ely
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // to-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // -day
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // my
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // dear
  delay(m_long);
  mouthAction("closed", color);
}

void been_watching_you(String color) {
  // You should know...I've been watching you
  mouthAction("open" , color);  // You
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // should
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // know
  delay(m_long);
  mouthAction("closed", color); // ...
  delay(1000);
  mouthAction("open" , color);  // I've
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // been
  delay(m_long);
  mouthAction("closed", color); // ...
  delay(1000);
  mouthAction("open" , color);  // wat-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // -ching
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // you
  delay(m_long);
  mouthAction("closed", color);
}

void goodbye_for_now(String color) {
  // Goodbye for now, love...we'll meet again soon...and I'll be watching.
  mouthAction("open" , color);  // Good-
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // -bye
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // for
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // now
  delay(m_long);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // love
  delay(m_long);
  mouthAction("closed", color);
  delay(1000);
  mouthAction("open" , color);  // we'll
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // meet
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // a-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // -gain
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // soon
  delay(m_long);
  mouthAction("closed", color);
  delay(1000);
  mouthAction("open" , color);  // and
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // I'll
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // be
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
  mouthAction("open" , color);  // wat-
  delay(m_short);
  mouthAction("closed", color);
  delay(m_short);
  mouthAction("open" , color);  // -ching
  delay(m_medium);
  mouthAction("closed", color);
  delay(m_medium);
}

void mouthHeart(boolean fade, boolean beat) {
  if (fade == true) {
    for (int x = 0; x < 256; x++) {
      strip_mouth.setPixelColor(0, x, 0 , 0);
      strip_mouth.setPixelColor(1, 0, 0, 0);
      strip_mouth.setPixelColor(2, x, 0 , 0);
      strip_mouth.setPixelColor(3, x, 0 , 0);
      strip_mouth.setPixelColor(4, x, 0 , 0);
      strip_mouth.setPixelColor(5, x, 0 , 0);
      strip_mouth.setPixelColor(6, 0, 0, 0);
      strip_mouth.setPixelColor(7, x, 0 , 0);
      strip_mouth.setPixelColor(8, 0, 0, 0);

      strip_mouth.show();
      delay(10);
    }
    delay(1000);
  }

  else {
    uint32_t heartColor = colorPicker("mouth", "red");
    strip_mouth.setPixelColor(0, heartColor);
    strip_mouth.setPixelColor(1, 0, 0, 0);
    strip_mouth.setPixelColor(2, heartColor);
    strip_mouth.setPixelColor(3, heartColor);
    strip_mouth.setPixelColor(4, heartColor);
    strip_mouth.setPixelColor(5, heartColor);
    strip_mouth.setPixelColor(6, 0, 0, 0);
    strip_mouth.setPixelColor(7, heartColor);
    strip_mouth.setPixelColor(8, 0, 0, 0);

    strip_mouth.show();
  }

  if (beat == true) {
    for (int x = 0; x < 3; x++) {
      strip_mouth.setPixelColor(0, 0, 0, 0);
      strip_mouth.setPixelColor(1, 0, 0, 0);
      strip_mouth.setPixelColor(2, 0, 0, 0);
      strip_mouth.setPixelColor(3, 0, 0, 0);
      strip_mouth.setPixelColor(4, 0, 0, 0);
      strip_mouth.setPixelColor(5, 0, 0, 0);
      strip_mouth.setPixelColor(6, 0, 0, 0);
      strip_mouth.setPixelColor(7, 0, 0, 0);
      strip_mouth.setPixelColor(8, 0, 0, 0);
      strip_mouth.show();

      delay(100);

      strip_mouth.setPixelColor(0, 255, 0, 0);
      strip_mouth.setPixelColor(1, 0, 0, 0);
      strip_mouth.setPixelColor(2, 255, 0, 0);
      strip_mouth.setPixelColor(3, 255, 0, 0);
      strip_mouth.setPixelColor(4, 255, 0, 0);
      strip_mouth.setPixelColor(5, 255, 0, 0);
      strip_mouth.setPixelColor(6, 0, 0, 0);
      strip_mouth.setPixelColor(7, 255, 0, 0);
      strip_mouth.setPixelColor(8, 0, 0, 0);
      strip_mouth.show();

      delay(200);

      strip_mouth.setPixelColor(0, 0, 0, 0);
      strip_mouth.setPixelColor(1, 0, 0, 0);
      strip_mouth.setPixelColor(2, 0, 0, 0);
      strip_mouth.setPixelColor(3, 0, 0, 0);
      strip_mouth.setPixelColor(4, 0, 0, 0);
      strip_mouth.setPixelColor(5, 0, 0, 0);
      strip_mouth.setPixelColor(6, 0, 0, 0);
      strip_mouth.setPixelColor(7, 0, 0, 0);
      strip_mouth.setPixelColor(8, 0, 0, 0);
      strip_mouth.show();

      delay(100);

      strip_mouth.setPixelColor(0, 255, 0, 0);
      strip_mouth.setPixelColor(1, 0, 0, 0);
      strip_mouth.setPixelColor(2, 255, 0, 0);
      strip_mouth.setPixelColor(3, 255, 0, 0);
      strip_mouth.setPixelColor(4, 255, 0, 0);
      strip_mouth.setPixelColor(5, 255, 0, 0);
      strip_mouth.setPixelColor(6, 0, 0, 0);
      strip_mouth.setPixelColor(7, 255, 0, 0);
      strip_mouth.setPixelColor(8, 0, 0, 0);
      strip_mouth.show();

      delay(750);
    }
  }

  if (fade == true) {
    for (int x = 255; x > -1; x--) {
      strip_mouth.setPixelColor(0, x, 0 , 0);
      strip_mouth.setPixelColor(1, 0, 0, 0);
      strip_mouth.setPixelColor(2, x, 0 , 0);
      strip_mouth.setPixelColor(3, x, 0 , 0);
      strip_mouth.setPixelColor(4, x, 0 , 0);
      strip_mouth.setPixelColor(5, x, 0 , 0);
      strip_mouth.setPixelColor(6, 0, 0, 0);
      strip_mouth.setPixelColor(7, x, 0 , 0);
      strip_mouth.setPixelColor(8, 0, 0, 0);

      strip_mouth.show();
      delay(10);
    }
    delay(1000);
  }

  mouthAction("closed", "blue");
}

void eyesAction(String state, String colorOuter, String colorInner) {
  uint32_t eyesColorOuter = colorPicker("eyes", colorOuter);
  uint32_t eyesColorInner = colorPicker("eyes", colorInner);

  if (state == "open") {
    strip_eyes.setPixelColor(0, eyesColorOuter);
    strip_eyes.setPixelColor(1, eyesColorInner);
    strip_eyes.setPixelColor(2, eyesColorOuter);
    strip_eyes.setPixelColor(3, 0, 0, 0);
    strip_eyes.setPixelColor(4, 0, 0, 0);
    strip_eyes.setPixelColor(5, eyesColorOuter);
    strip_eyes.setPixelColor(6, eyesColorInner);
    strip_eyes.setPixelColor(7, eyesColorOuter);

    strip_eyes.show();
  }

  else if (state == "closed") {
    strip_eyes.setPixelColor(0, eyesColorOuter);
    strip_eyes.setPixelColor(1, eyesColorOuter);
    strip_eyes.setPixelColor(2, eyesColorOuter);
    strip_eyes.setPixelColor(3, 0, 0, 0);
    strip_eyes.setPixelColor(4, 0, 0, 0);
    strip_eyes.setPixelColor(5, eyesColorOuter);
    strip_eyes.setPixelColor(6, eyesColorOuter);
    strip_eyes.setPixelColor(7, eyesColorOuter);

    strip_eyes.show();
  }

  else Serial.println("Invalid eyes state selection.");
}

void eyesBlink(String colorOuter, String colorInner) {
  eyesAction("closed", colorOuter, colorInner);
  delay(200);
  eyesAction("open", colorOuter, colorInner);
}

void mouthAction(String state, String color) {
  uint32_t mouthColor = colorPicker("mouth", color);

  if (state == "open") {
    strip_mouth.setPixelColor(0, mouthColor);
    strip_mouth.setPixelColor(1, mouthColor);
    strip_mouth.setPixelColor(2, mouthColor);
    strip_mouth.setPixelColor(3, mouthColor);
    strip_mouth.setPixelColor(4, 0, 0, 0);
    strip_mouth.setPixelColor(5, mouthColor);
    strip_mouth.setPixelColor(6, mouthColor);
    strip_mouth.setPixelColor(7, mouthColor);
    strip_mouth.setPixelColor(8, mouthColor);

    strip_mouth.show();
  }

  else if (state == "closed") {
    strip_mouth.setPixelColor(0, 0, 0, 0);
    strip_mouth.setPixelColor(1, 0, 0, 0);
    strip_mouth.setPixelColor(2, 0, 0, 0);
    strip_mouth.setPixelColor(3, mouthColor);
    strip_mouth.setPixelColor(4, mouthColor);
    strip_mouth.setPixelColor(5, mouthColor);
    strip_mouth.setPixelColor(6, mouthColor);
    strip_mouth.setPixelColor(7, mouthColor);
    strip_mouth.setPixelColor(8, mouthColor);

    strip_mouth.show();
  }

  else Serial.println("Invalid mouth state selection.");
}

void fadeFace(String time_point) {
  if (time_point == "in") {
    for (int x = 0; x < 256; x++) {
      strip_eyes.setPixelColor(0, 0, x, 0);
      strip_eyes.setPixelColor(1, 0, 0, x);
      strip_eyes.setPixelColor(2, 0, x, 0);
      strip_eyes.setPixelColor(3, 0, 0, 0);
      strip_eyes.setPixelColor(4, 0, 0, 0);
      strip_eyes.setPixelColor(5, 0, x, 0);
      strip_eyes.setPixelColor(6, 0, 0, x);
      strip_eyes.setPixelColor(7, 0, x, 0);

      strip_mouth.setPixelColor(0, 0, 0, 0);
      strip_mouth.setPixelColor(1, 0, 0, 0);
      strip_mouth.setPixelColor(2, 0, 0, 0);
      strip_mouth.setPixelColor(3, 0, 0, x);
      strip_mouth.setPixelColor(4, 0, 0, x);
      strip_mouth.setPixelColor(5, 0, 0, x);
      strip_mouth.setPixelColor(6, 0, 0, x);
      strip_mouth.setPixelColor(7, 0, 0, x);
      strip_mouth.setPixelColor(8, 0, 0, x);

      strip_eyes.show();
      strip_mouth.show();

      delay(20);
    }
  }

  else if (time_point == "out") {
    for (int x = 255; x > -1; x--) {
      strip_eyes.setBrightness(x);
      strip_mouth.setBrightness(x);
      strip_eyes.show();
      strip_mouth.show();

      delay(10);
    }
  }

  else Serial.println("Invalid fade selection.");
}

uint32_t colorPicker(String strip, String color) {
  if (strip == "eyes") {
    uint32_t eyes_color;

    if (color == "red") eyes_color = strip_eyes.Color(255, 0, 0);
    else if (color == "green") eyes_color = strip_eyes.Color(0, 255, 0);
    else if (color == "blue") eyes_color = strip_eyes.Color(0, 0, 255);
    else if (color == "yellow") eyes_color = strip_eyes.Color(255, 255, 0);
    else if (color == "cyan") eyes_color = strip_eyes.Color(0, 255, 255);
    else if (color == "magenta") eyes_color = strip_eyes.Color(255, 0, 255);
    else if (color == "off") eyes_color = strip_eyes.Color(0, 0, 0);

    return eyes_color;
  }

  else if (strip == "mouth") {
    uint32_t mouth_color;

    if (color == "red") mouth_color = strip_mouth.Color(255, 0, 0);
    else if (color == "green") mouth_color = strip_mouth.Color(0, 255, 0);
    else if (color == "blue") mouth_color = strip_mouth.Color(0, 0, 255);
    else if (color == "yellow") mouth_color = strip_mouth.Color(255, 255, 0);
    else if (color == "cyan") mouth_color = strip_mouth.Color(0, 255, 255);
    else if (color == "magenta") mouth_color = strip_mouth.Color(255, 0, 255);
    else if (color == "off") mouth_color = strip_mouth.Color(0, 0, 0);

    return mouth_color;
  }

  else {
    Serial.println("Invalid color selection.");

    return 0;
  }
}
