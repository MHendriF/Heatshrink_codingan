#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#define arduinoLED 13   // Arduino LED on board

int main(int argc, char **argv)
{
  init(); // this is needed
  pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
  digitalWrite(arduinoLED, LOW);
  Serial.begin(9600);
  Serial.println("Incoming data :");
  int num[600];
  char incomingByte;
  int i=0, length_data;
  int j=0;
  String stringOne;
  
  size_t comp_sz = 600;
  memset(num,0,comp_sz);
  
//  for(j=0; j<600; j++){
//    num[j] = 0;
//  }
  
  for ( ;; )
   {
      if(Serial.available() > 0){
        incomingByte = Serial.read();
        //length_data = sizeof(num)/sizeof(num[0]);
        if(incomingByte != '\n'){
            Serial.print(incomingByte);
            stringOne += incomingByte;
         }else{
          //Serial.println(stringOne);
          num[i] = stringOne.toInt();
          stringOne = "";
          i++;
        }
//        if(length_data > 10){
//          for(j=0; j<length_data; j++){
//            Serial.println(num[j]);
//          }
//          delay(9000);
//        }
      }
      else if(Serial.available() <= 0){
        Serial.println("test");
        //Serial.println(num[0]);
        if(num[10] != 0){
          //Serial.println("testing");
          for(j=0; j<10; j++){
             Serial.println(num[j]);
             //Serial.println("testing");
          }
        }
        delay(3000);
      }
   }
}
