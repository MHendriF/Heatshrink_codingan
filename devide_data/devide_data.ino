#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#define arduinoLED 13   // Arduino LED on board

#define BUFFER_SIZE 700
uint8_t orig_buffer[BUFFER_SIZE];
uint8_t comp_buffer[BUFFER_SIZE];
uint8_t decomp_buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
    init(); // this is needed
    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);
    Serial.begin(9600);
    Serial.println("Incoming data :");
    int i=0, length_data, sisa, nlevel;
    int j=0;
    int n=0, idx;
    String stringOne;
    int orig=0, window=0, lookahead=0, decoder=0, polled=0;
    char *s, *orig_sz, *window_sz, *lookahead_sz, *decoder_sz, *polled_sz;

    uint8_t test_data [] = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    length_data = sizeof(test_data)/sizeof(test_data[0]);
    Serial.println(length_data);
    
    double ndata;
    ndata = (double)length_data / 584;
    nlevel = (int)ceil(ndata);
    sisa = length_data % 548;
    
    Serial.print("N data : ");
    Serial.println(nlevel);
    Serial.print("sisa : ");
    Serial.println(sisa);

//    for(i=0; i<nlevel; i++)
//    {
//      uint32_t comp_size   = BUFFER_SIZE; //this will get updated by reference
//      uint32_t decomp_size = BUFFER_SIZE; //this will get updated by reference
//      //memcpy(orig_buffer, test_data, length_data);
//      orig_buffer[0] = "c";
//      Serial.print(orig_buffer[0]);
//      memset(orig_buffer,10,BUFFER_SIZE);
//      for(j=0; j<40; j++){
//        Serial.print(orig_buffer[j]);
//      }
//      //memset
//      //copy data dari index x sampai dindex n
//      delay(6000);
//    }

    int array[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    length_data = sizeof(array)/sizeof(int);
    int *firstHalf = malloc(10 * sizeof(int));
    if (!firstHalf) {
      /* handle error */
      Serial.println(F("FAIL: Malloc fail!"));
    }
//    memcpy( destination + N, source + N, sourceLen - N );
//    memcpy(firstHalf, array, 10 * sizeof(int));
    memcpy(firstHalf, array + 2, 10 * sizeof(int));
    sisa = length_data - 10;

    for(int i=0; i<10; i++){
       Serial.print(firstHalf[i]);
    }
        
//    if(length_data > 10){
//        int *secondHalf = malloc(sisa* sizeof(int));
//        if (!secondHalf) {
//          /* handle error */
//          Serial.println(F("FAIL: Malloc fail!"));
//        }
//
//        memcpy(secondHalf, array + 10, sisa * sizeof(int));
//
//         
//        for(int i=0; i<sisa; i++){
//           Serial.println(secondHalf[i]);
//        }
//    }
   
    //Serial.println(length_data);
    //Serial.println(sizeof(firstHalf));
   
    
    for ( ;; )
    {
        Serial.println("test");
        delay(9000);
    }
}
