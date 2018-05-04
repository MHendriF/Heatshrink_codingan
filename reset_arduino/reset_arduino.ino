int led = 13;//pin 13 as OUTPUT LED pin
int resetPin = 12;
// the setup routine runs once when you press reset:
void setup() {  
  digitalWrite(resetPin, HIGH);
  delay(200);
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);    
  pinMode(resetPin, OUTPUT);     
  Serial.begin(9600);//initialize Serial Port
  Serial.println("reset");//print reset to know the program has been reset and 
  //the setup function happened
  delay(200);
}

// the loop routine runs over and over again forever:
void loop() {
  delay(10);
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.println("on");
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  Serial.println("off");
  delay(1000);               // wait for a second
  Serial.println("resetting");
  delay(10);
  digitalWrite(resetPin, LOW);
  Serial.println("this never happens");
  //this never happens because Arduino resets

}

