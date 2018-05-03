#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#define arduinoLED 13   // Arduino LED on board

void removeChar( char * string, char letter );

int main(int argc, char **argv)
{
  init(); // this is needed
  pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
  digitalWrite(arduinoLED, LOW);
  Serial.begin(9600);
  Serial.println("Incoming data :");
  int num[200];
  char incomingByte;
  int i=0, length_data;
  int j=0;
  int n=0;
  String stringOne;
  int orig=0, window=0, lookahead=0, decoder=0, polled=0;
  char *s, *orig_sz, *window_sz, *lookahead_sz, *decoder_sz, *polled_sz;
  String stringTwo = "a100a";
  char buf[20];
  
  size_t comp_sz = 200;
  memset(num,0,comp_sz);
  
//  //convert string to array of char
//  n = stringTwo.length(); 
//  char buf[n+1]; 
//  strcpy(buf, stringTwo.c_str()); 
//      
//  //check if character exist
//  s = strchr (buf, 'a');
//  if (s != NULL){
//    Serial.println("Found");
//  }
//   
//  //remove character from string
//  Serial.print("before : ");
//  Serial.println(buf);
//  removeChar(buf, 'a' );
//  Serial.print("after : ");
//  Serial.println(buf);
//
//  memset(buf,0,n+1);
//  Serial.println(buf);
  
  for ( ;; )
  {
      if(Serial.available() > 0){
        incomingByte = Serial.read();
        //length_data = sizeof(num)/sizeof(num[0]);
        if(incomingByte != '\n'){
            Serial.print(incomingByte);
            stringOne += incomingByte;
        }
        else if(incomingByte == '\n'){
            strcpy(buf, stringOne.c_str());
            orig_sz = strchr (buf, 'a');
            window_sz = strchr (buf, 'b');
            lookahead_sz = strchr (buf, 'c');
            decoder_sz = strchr (buf, 'd');
            polled_sz = strchr (buf, 'f');
            
            if (orig_sz != NULL){
                removeChar(buf, 'a');
                stringOne = buf;
                orig = stringOne.toInt();
            }
            else if (window_sz != NULL){
                removeChar(buf, 'b');
                stringOne = buf;
                window = stringOne.toInt();
            }
            else if (lookahead_sz != NULL){
                removeChar(buf, 'c');
                stringOne = buf;
                lookahead = stringOne.toInt();
            }
            else if (decoder_sz != NULL){
                removeChar(buf, 'd');
                stringOne = buf;
                decoder = stringOne.toInt();
            }
            else if (polled_sz != NULL){
                removeChar(buf, 'f');
                stringOne = buf;
                polled = stringOne.toInt();
            }
            else{
              Serial.print(" ");
              num[i] = stringOne.toInt();
              i++;
            }
            stringOne = "";
            memset(buf,0,20);
         }
      }
      else if(Serial.available() <= 0){
        Serial.println("test");
        //Serial.println(num[0]);
        if(num[12] != 0){
          for(j=0; j<13; j++){
             Serial.print(num[j]);
             Serial.print(" ");
          }
          Serial.print("a:");
          Serial.print(orig);
          Serial.print("b:");
          Serial.print(window);
          Serial.print("c:");
          Serial.print(lookahead);
          Serial.print("d:");
          Serial.print(decoder);
          Serial.print("f:");
          Serial.print(polled);
        }
        delay(3000);
      }
    
   }
}

void removeChar(char * string, char letter ) {
  for( unsigned int i = 0; i < strlen( string ); i++ )
    if( string[i] == letter )
      strcpy( string + i, string + i + 1 );
}

