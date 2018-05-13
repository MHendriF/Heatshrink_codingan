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

    uint8_t test_data [] = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    //uint8_t test_data [] = "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    
    length_data = sizeof(test_data)/sizeof(test_data[0]);
    Serial.println(length_data);
    
    double ndata;
    ndata = (double)length_data / 584;
    nlevel = (int)ceil(ndata);
    sisa = length_data % 584;
    
    Serial.print("N data : ");
    Serial.println(nlevel);
    Serial.print("sisa : ");
    Serial.println(sisa);

    uint32_t comp_size   = BUFFER_SIZE; //this will get updated by reference
    uint32_t decomp_size = BUFFER_SIZE; //this will get updated by reference
    
    idx = 0;
    int idx_normal = 584;
    
    for(i=0; i<nlevel; i++)
    {
        Serial.println("Memset comp_buffer");
        memset(comp_buffer,0,600);
        //copy 584 char to comp_buffer
        if(i<nlevel-1){
          memcpy(comp_buffer, test_data + idx, idx_normal * sizeof(uint8_t));
        }
        else{
          memcpy(comp_buffer, test_data + idx, sisa * sizeof(uint8_t));
        }
        idx = idx + 584;
        Serial.print("idx now : ");
        Serial.print(idx);
        Serial.print(" --> ");
        
        for(int i=0; i<584; i++){
         Serial.print((char)comp_buffer[i]);
        }
        Serial.println();
        delay(3000);
    }

    
    ////////////////////////////////////////////////////////////////////////////
//    int test_buffer[100];
//   
//    int array[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
//    length_data = sizeof(array)/sizeof(array[0]);
//    Serial.println(length_data);
//
//    double ndata;
//    ndata = (double)length_data / 6;
//    nlevel = (int)ceil(ndata);
//    sisa = length_data % 6;
//
//    Serial.print("N data : ");
//    Serial.println(nlevel);
//    Serial.print("sisa : ");
//    Serial.println(sisa);
//    idx = 0;
//    int idx_normal = 6;
//    for(i=0; i<nlevel; i++)
//    {
//      Serial.println("Memset comp_buffer");
//      memset(test_buffer,0,100);
//      //copy 584 char to comp_buffer
//      if(i<nlevel-1){
//        memcpy(test_buffer, array+idx, idx_normal * sizeof(int));
//      }
//      else{
//        memcpy(test_buffer, array+idx, sisa * sizeof(int));
//      }
//      idx = idx + 6;
//      Serial.print("idx now : ");
//      Serial.print(idx);
//      Serial.print(" --> ");
//      
//      for(int i=0; i<10; i++){
//       Serial.print(test_buffer[i]);
//      }
//      Serial.println();
//      //set index awal += 584
//      //copy data dari index x sampai dindex n
//      delay(3000);
//    }
    
//
//    memset(test_buffer,0,100);
//    //array tujuan, idx source, idx destination / panjang data
//    memcpy(test_buffer, array+4, 5 * sizeof(int));
//    for(int i=0; i<10; i++){
//       Serial.print(test_buffer[i]);
//    }
    
//    length_data = sizeof(array)/sizeof(int);
//    int *firstHalf = malloc(10 * sizeof(int));
//    if (!firstHalf) {
//      /* handle error */
//      Serial.println(F("FAIL: Malloc fail!"));
//    }
////    memcpy( destination + N, source + N, sourceLen - N );
////    memcpy(firstHalf, array, 10 * sizeof(int));
//    memcpy(firstHalf, array + 2, 10 * sizeof(int));
//    sisa = length_data - 10;

    
    for ( ;; )
    {
        Serial.println("test");
        delay(9000);
    }
}
