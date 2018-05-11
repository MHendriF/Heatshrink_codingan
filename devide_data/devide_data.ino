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
    int i=0, length_data, sisa;
    int j=0;
    int n=0;
    String stringOne;
    int orig=0, window=0, lookahead=0, decoder=0, polled=0;
    char *s, *orig_sz, *window_sz, *lookahead_sz, *decoder_sz, *polled_sz;
    String stringTwo = "a100a";
    char buf[20];
    
    size_t comp_sz = 200;
    memset(num,0,comp_sz);
  
    double ndata;
    ndata = (double)1200 / 584;
    Serial.print("N data : ");
    Serial.println((int)ceil(ndata));
    Serial.print("sisa : ");
    Serial.println(1000 % 548);

    int array[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    length_data = sizeof(array)/sizeof(int);
    int *firstHalf = malloc(10 * sizeof(int));
    if (!firstHalf) {
      /* handle error */
      Serial.println(F("FAIL: Malloc fail!"));
    }
    memcpy(firstHalf, array, 10 * sizeof(int));
    sisa = length_data - 10;

//    for(int i=0; i<10; i++){
//       Serial.println(firstHalf[i]);
//    }
        
    if(length_data > 10){
        int *secondHalf = malloc(sisa* sizeof(int));
        if (!secondHalf) {
          /* handle error */
          Serial.println(F("FAIL: Malloc fail!"));
        }

        memcpy(secondHalf, array + 10, sisa * sizeof(int));

         
        for(int i=0; i<sisa; i++){
           Serial.println(secondHalf[i]);
        }
    }
   
    //Serial.println(length_data);
    //Serial.println(sizeof(firstHalf));
   
    
    for ( ;; )
    {
        Serial.println("test");
        delay(5000);
    }
}
