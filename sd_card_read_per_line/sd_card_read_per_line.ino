/*
 *  Arduino SD Card Tutorial Example
 *  
 *  by Dejan Nedelkovski, www.HowToMechatronics.com
 */
#include <SD.h>
#include <SPI.h>
File myFile;
String buffer;
int pinCS = 10; // Pin 10 on Arduino Uno  //53 Mega

void setup() {
    
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT);
  int iterate = 0;
  // SD Card Initialization
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
  // Create/Open file 
  myFile = SD.open("test.txt", FILE_WRITE);
  
  // Reading the file
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("Read:");
    // Reading the whole file
    while (myFile.available()) {
      buffer = myFile.readStringUntil('\n');
      iterate++;
      Serial.print("Baris ke : ");
      Serial.println(iterate);
      Serial.println(buffer); //Printing for debugging purpose
      //delay(4000);
   }
    myFile.close();
  }
  else {
    Serial.println("error opening test.txt");
  }
  
}
void loop() {
  // empty
}
