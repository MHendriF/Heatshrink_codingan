#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#define arduinoLED 13   // Arduino LED on board

static void removeChar(char * string, char letter ) {
  for( unsigned int i = 0; i < strlen( string ); i++ )
    if( string[i] == letter )
      strcpy( string + i, string + i + 1 );
}

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);

    uint8_t nodeA[1870];
    uint8_t nodeB[1000];
    
    String stringOne;
    char incomingByte;
    char *node_A, *node_B;
    char buf[20];
    int idx_A=0, idx_B=0;

    int num_A[1870];
    int num_B[1870];
    memset(num_A,0,1870);
    memset(num_B,0,1000);
    
    Serial.print(F("Incoming data : \n"));
    for ( ;; )
    {   
        if(Serial.available() > 0){
          incomingByte = Serial.read();
          if(incomingByte != '\n'){
              Serial.print(incomingByte);
              stringOne += incomingByte;
          }
          else if(incomingByte == '\n'){
              strcpy(buf, stringOne.c_str());
              node_A = strchr (buf, 'A');
              node_B = strchr (buf, 'B');
  
              if (node_A != NULL){
                  removeChar(buf, 'A');
                  stringOne = buf;
                  num_A[idx_A] = stringOne.toInt();
                  idx_A++;
              }
              else if (node_B != NULL){
                  removeChar(buf, 'B');
                  stringOne = buf;
                  num_B[idx_B] = stringOne.toInt();
                  idx_B++;
              }
              stringOne = "";
              memset(buf,0,20);
          }
        }
        
        else if(Serial.available() <= 0){
            Serial.print(F("| idxA : "));
            Serial.print(idx_A);
            Serial.print(F(" | idxB : "));
            Serial.println(idx_B);
//            if(idx_A == 237){
//               for(int j=0; j<237; j++){
//                   Serial.print(num_A[j]);
//                   Serial.print(" ");
//                }
//            }
            delay(1000);
        }
    }
}

