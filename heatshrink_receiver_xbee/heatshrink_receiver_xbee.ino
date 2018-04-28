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
    uint8_t read_data;
    uint8_t data_compression[30000];
   // uint8_t data_compression = (uint8_t)malloc(20000);
    uint32_t i;
    //memset(data_compression, 0, 10000);
    Serial.println("Incoming data :");
    i = 0;
    for ( ;; ){
      if(Serial.available() > 0){
        data_compression[i] = Serial.read();
        i++;
        //Serial.write(Serial.read());
      }
      else if(Serial.available() <= 0){
        if(i < 504){
          Serial.print("length=");
          Serial.println(i);
          
          for(int j=0; j<i; j++){
            Serial.write(data_compression[j]);
          }Serial.println("");
        }
        delay(1000);
      }
    }
}


