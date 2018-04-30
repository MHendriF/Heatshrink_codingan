// Demo Code for Heatshrink (Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>)
// embedded compression library
// Craig Versek, Apr. 2016

#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <QuickStats.h>

#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"
#include "greatest.h"

#define arduinoLED 13   // Arduino LED on board

/******************************************************************************/
QuickStats stats; //initialize an instance of this class

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    char read_data[100];
    uint8_t data_compression[30000];
    uint32_t i;
    char *incomingByte, inString;
   // char *comp = "asds";
    Serial.println("Incoming data :");
    i = 0;
    //char* str1;
    //str1 = "sssss";
    //strcpy(comp, str1);
    //read_data[0] = "2";
    
    for ( ;; )
    {
      //Serial.println(comp[2]);
      //delay(3000);
      if(Serial.available() > 0){
        incomingByte = Serial.read();
        if(incomingByte != '\n'){
          //inString += incomingByte;
          i++;
          //Serial.print((uint8_t)atoi(incomingByte));
          Serial.print((int)incomingByte);
          //Serial.print(inString);
          Serial.println(i);
        }else{
          Serial.println("gila");
          
        }
        
          
//        if(incomingByte == '\n'){
//          Serial.println("ok");
//        }else{
//          Serial.print(incomingByte);
//          Serial.println(i);
//        }
        
        //Serial.write(incomingByte);
        //Serial.write(Serial.read());
      }
//      else if(Serial.available() <= 0){
//        if(i < 59){
//          Serial.print("length=");
//          Serial.println(i);
//          
//          for(int j=0; j<i; j++){
//            Serial.write(data_compression[j]);
//          }Serial.println("");
//        }
////        else if(i > 60){
////          Serial.write("0 : ");
////          Serial.write(data_compression[0]);
////          Serial.write("1 : ");
////          Serial.write(data_compression[1]);
////          Serial.write("2 : ");
////          Serial.write(data_compression[2]);
////        }
//        delay(2000);
//      }
    }
}


