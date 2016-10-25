void setup() {
  Serial.begin(19200);
  Serial1.begin(19200);
}

void loop() {
  int val = analogRead(A0);

    // Update value of yourThingId
  Serial1.println("PUT http://evrythng.net/thngs/5306c5ece4b0428ed36bf7d6/properties/ReadTag HTTP/1.1");
  Serial1.println("Content-Type: application/json");
  Serial1.println("Accept: application/vnd.evrythng-v2+json");
  // Update value of yourAPIToken
  Serial1.println("X-Evrythng-Token: 47GXS7EHesD5HcijCatVWeeo6UTogxGgqKPwsdMTLXb1nTow36b6tP2y6FrFinS9x16sfOGFQwZgia0O"); 
  Serial1.println("Host: evrythng.net");
  Serial1.println("Content-Length: 45"); 
  Serial1.println(""); 
  Serial1.print("{\"key\": \"ReadTag\",\"value\": \""); 
  Serial1.print(val); 
  Serial1.println("\"}"); 
  Serial1.println(""); 
  Serial1.println(""); 

  delay(5000);
}
