/*  zomg lolz0rz...it detects methane so obviously it's a fart sensor  */

const int metPin = A6;
int loopCount = 1;

boolean burpDetected = false;
boolean fartDetected = false;
boolean dickheadStatus = false;
boolean scumbagStatus = false;

void setup() {
  Serial.begin(19200);
}

void loop() {
  const int fartThreshold = 200;
  const int burpThreshold = 100;
  const int dickheadThreshold = 3;
  const int scumbagThreshold = 5;

  int metVal = analogRead(metPin);

  if(burpDetected == false && metVal > burpThreshold) {
    Serial.println(F("Humans fart 25% of their farts out their mouths."));
    burpDetected = true;
  }
  if(fartDetected == false) {
    if(metVal > fartThreshold) {
      Serial.println(F("FART OMG DENIED IT SUPPLIED IT LULZ BAHAHAHA!!!!"));
      fartDetected = true;
    }
  }
  else {
    if(dickheadStatus == false && metVal > fartThreshold) {
      Serial.println(F("Wow dude 4 realz??!?"));
      dickheadStatus = true;
    }
    else if(scumbagStatus == false) Serial.println(F("Really!?!? Wow, dude..."));
    if(dickheadStatus == true && metVal > fartThreshold) {
      Serial.println(F("You are a total scumbag. Slimy bag-hatted rag stain."));
      scumbagStatus = true;
    }
  }
  Serial.print(loopCount);
  Serial.print(F(":        "));
  Serial.println(metVal);
  loopCount++;
  delay(250);
}
