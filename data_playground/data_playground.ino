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

