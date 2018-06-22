void setup() {
  Serial.begin(9600);
  Serial.println("Ini Koor");
}

void loop() {
  if(Serial.available() > 0){
    Serial.write(Serial.read());
  }

}
