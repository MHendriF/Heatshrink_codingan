void setup() {
  Serial.begin(9600);
}

void loop() {
  char incomingByte;
  if(Serial.available() > 0){
     incomingByte = Serial.read();
     Serial.print(incomingByte);
  }
}
