// n is an array of 10 integers
#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

void removeChar( char * string, char letter );

int main(int argc, char **argv)
{
    init(); // this is needed  
    Serial.begin(9600);
    Serial.println("Incoming data :");
    String stringOne = "a1000a";
    char stringTwo [10];
    stringOne.toCharArray(stringTwo, 10);
    Serial.println(stringTwo);
    removeChar(stringTwo, 'a' );
    Serial.println(stringTwo);

       
    for ( ;; )
    {
        Serial.println("B");
        delay(5000); 
    }
}

void removeChar(char * string, char letter ) {
  for( unsigned int i = 0; i < strlen( string ); i++ )
    if( string[i] == letter )
      strcpy( string + i, string + i + 1 );
}
