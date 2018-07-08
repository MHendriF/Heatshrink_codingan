void setup() {
  Serial.begin(9600);
  Serial.println("Ini Routers");
}

void loop() {
  char incomingByte;
  if(Serial.available() > 0){
     incomingByte = Serial.read();
     Serial.print(incomingByte);
     //Serial.println("r");
     //incomingByte = Serial.write(Serial.read());
  }

}
