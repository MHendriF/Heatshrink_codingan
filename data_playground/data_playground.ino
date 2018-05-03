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
  String stringOne;
  int comp_size, window, lookahead, decoder_size, polled;
  
  size_t comp_sz = 600;
  memset(num,0,comp_sz);

  //remove character from string
  char stringTwo [] = "a1000a";
  Serial.println(stringTwo);
  removeChar(stringTwo, 'a' );
  Serial.println(stringTwo);

  //check if character exist
  char *s;
  char buf [] = "a1000a";
  s = strchr (buf, 'a');
  if (s != NULL){
    Serial.println("Found");
  }
   
  for ( ;; )
  {
      if(Serial.available() > 0){
        incomingByte = Serial.read();
        //length_data = sizeof(num)/sizeof(num[0]);
        if(incomingByte != '\n'){
            Serial.print(incomingByte);
            stringOne += incomingByte;
        }
        else{
          //Serial.println(stringOne);
          Serial.print(" ");
          num[i] = stringOne.toInt();
          stringOne = "";
          i++;
         }
      }
      else if(Serial.available() <= 0){
        Serial.println("test");
        //Serial.println(num[0]);
        if(num[12] != 0){
          for(j=0; j<13; j++){
             Serial.println(num[j]);
             //Serial.println(" ");
          }
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

