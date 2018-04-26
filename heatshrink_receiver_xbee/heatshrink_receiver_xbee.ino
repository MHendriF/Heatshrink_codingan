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
    char read_data;
    char data_compression[1000];
    int i = 0;
    
    for ( ;; ){
      if(Serial.available() > 0){
        data_compression[i] = Serial.read();
        i++;
        Serial.write(Serial.read());
      }
      for(int j=0; j<i; j++){
        Serial.print(data_compression[j]);
        Serial.print(" ");
      }
      Serial.println();
      delay(7000);
    }
}


