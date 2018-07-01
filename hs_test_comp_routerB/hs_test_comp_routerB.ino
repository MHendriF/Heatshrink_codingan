#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#define arduinoLED 13   // Arduino LED on board

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    delay(1000);
    uint32_t length_data;
    
    uint8_t data [] = {152, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 130, 192, 0};
    length_data = sizeof(data)/sizeof(data[0]);
    
    Serial.print(data[0]);
    Serial.print("B");
    Serial.print("\n");
    for(int i = 1; i < length_data; i++){
        if(i % 11 == 0){
          Serial.print(data[i]);
          Serial.print("B");
          Serial.print("\n");
          delay(5000);
        }else{
          Serial.print(data[i]);
          Serial.print("B");
          Serial.print("\n");
        }
    }
    delay(5000);

    //Length data original
    Serial.print("a");
    Serial.print("584");
    Serial.print("B");
    Serial.print("\n");
    //Config
    Serial.print("b");
    Serial.print("4");
    Serial.print("B");
    Serial.print("\n");
    Serial.print("c");
    Serial.print("3");
    Serial.print("B");
    Serial.print("\n");
    Serial.print("d");
    Serial.print("64");
    Serial.print("B");
    Serial.print("\n");
    //Polled
    Serial.print("f");
    Serial.print("76");
    Serial.print("B");
    Serial.print("\n");
    
    for ( ;; )
    {
      delay(3000);
    }   

}
