// Demo Code for Heatshrink (Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>)
// embedded compression library
// Craig Versek, Apr. 2016

#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#include <heatshrink_encoder.h>
#include <heatshrink_decoder.h>
#include <greatest.h>

#include <SD.h>
#include <SPI.h>
File myFile;
int pinCS = 53; // Pin 10 on Arduino Uno

#define arduinoLED 13   // Arduino LED on board

/******************************************************************************/
// TEST CODE from adapted from test_heatshrink_dynamic.c
#if !HEATSHRINK_DYNAMIC_ALLOC
#error Must set HEATSHRINK_DYNAMIC_ALLOC to 1 for dynamic allocation test suite.
#endif

typedef struct {
    uint8_t log_lvl;
    uint8_t window_sz2;
    uint8_t lookahead_sz2;
    size_t decoder_input_buffer_size;
} cfg_info;

static void dump_buf(char *name, uint8_t *buf, uint16_t count) {
    for (int i=0; i<count; i++) {
        uint8_t c = (uint8_t)buf[i];
        printf("%s %d: 0x%02x ('%c')\n", name, i, c, isprint(c) ? c : '.');
    }
}

static int compress_and_expand_and_check(uint8_t *input, uint32_t input_size, cfg_info *cfg) 
{
    heatshrink_encoder *hse = heatshrink_encoder_alloc(cfg->window_sz2, cfg->lookahead_sz2);
    size_t comp_sz = input_size + (input_size/10) + 4;

    uint8_t *comp = (uint8_t*)malloc(comp_sz);
    if (comp == NULL) 
      Serial.println(F("FAIL: Malloc fail!"));
    memset(comp, 0, comp_sz);
     
    size_t count = 0;
    size_t sunk = 0;
    size_t polled = 0;
    
    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ COMPRESSING\n"));
        //dump_buf("input", input, input_size);
    }
    
    while (sunk < input_size) {
        HSE_sink_res esres = heatshrink_encoder_sink(hse, &input[sunk], input_size - sunk, &count);
        sunk += count;
        if (cfg->log_lvl > 1){
          Serial.print(F("^^ sunk "));
          Serial.print(count);
          Serial.print(F("\n"));
        }
        if (sunk == input_size) {
            heatshrink_encoder_finish(hse);
        }

        HSE_poll_res pres;
        do {                    /* "turn the crank" */
            pres = heatshrink_encoder_poll(hse, &comp[polled], comp_sz - polled, &count);
            //ASSERT(pres >= 0);
            polled += count;
            if (cfg->log_lvl > 1){
              Serial.print(F("^^ polled "));
              Serial.print(count);
              Serial.print(F("\n"));
            }
        } while (pres == HSER_POLL_MORE);
        if (polled >= comp_sz) 
          Serial.print(F("FAIL: Compression should never expand that much!"));
        if (sunk == input_size) {
            heatshrink_encoder_finish(hse);
        }
    }
    if (cfg->log_lvl > 0){
      Serial.print(F("in: "));
      Serial.print(input_size);
      Serial.print(F(" compressed: "));
      Serial.print(polled);
      Serial.print(F(" \n")); 
      
//      Serial.print("Compressed data: ");
//      for(int i = 0; i < polled; i++){
//        Serial.print(comp[i]);
//        Serial.print(", ");
//      }Serial.println();
    }

//    Serial.print("x");
//    Serial.print(input_size);
//    Serial.print("\n");
    
    //Serial.println("Compressed data: ");
    Serial.print(comp[0]);
    Serial.print("\n");
    for(int i = 1; i < polled; i++){
        if(i % 16 == 0){
          Serial.print(comp[i]);
          Serial.print("\n");
          delay(3000);
        }else{
          Serial.print(comp[i]);
          Serial.print("\n");
        }
    }
    delay(3000);
    
    //lenght data original
    Serial.print("a");
    Serial.print(input_size);
    Serial.print("\n");

    //config
    Serial.print("b");
    Serial.print(cfg->window_sz2);
    Serial.print("\n");
    Serial.print("c");
    Serial.print(cfg->lookahead_sz2);
    Serial.print("\n");
    Serial.print("d");
    Serial.print(cfg->decoder_input_buffer_size);
    Serial.print("\n");

    //Polled
    Serial.print("f");
    Serial.print(polled);
    Serial.print("\n");
    
    //Node 1
   // g = 1;
    Serial.print("g");
    Serial.print(1);
    Serial.print("\n");
          
    free(comp);
    heatshrink_encoder_free(hse);
    //return polled;
}

/******************************************************************************/
int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off

    Serial.begin(9600);
    pinMode(pinCS, OUTPUT);

    uint32_t buffer_size;
    uint32_t length_data;
    uint32_t comp_size;
    String bufferSD;
    int base = 15;
    int g = 0; //node
    
    // SD Card Initialization
    if (SD.begin()){
      //Serial.println("SD card is ready to use.");
    } else{
      Serial.println("SD card initialization failed");
      return;
    }
    size_t polled = 0;

     // Create/Open file 
    myFile = SD.open("testing.txt", FILE_WRITE);

    int iterate = 0;
    // Reading the file
    myFile = SD.open("testing.txt");
    if (myFile) {
      //Serial.println("Read:");
      // Reading the whole file
      while (myFile.available()) {
          iterate++;
         
          bufferSD = myFile.readStringUntil('\n');
          //Serial.println(bufferSD); //Printing for debugging purpose
         
          buffer_size = bufferSD.length();
          char test_data [buffer_size];
          comp_size = buffer_size;

          //Prepare to commpression
          bufferSD.toCharArray(test_data, buffer_size);
          length_data = sizeof(test_data)/sizeof(test_data[0]);
          //Serial.println(length_data);
    
          cfg_info cfg;
          cfg.log_lvl = 0;
          cfg.decoder_input_buffer_size = 64;
          if(buffer_size >= 584 and buffer_size < 980){
            cfg.window_sz2 = 9;
            cfg.lookahead_sz2 = 8;
          }
          else if(buffer_size >= 980 and buffer_size <= 1345){
            cfg.window_sz2 = 8;
            cfg.lookahead_sz2 = 7;
          }
          else if(buffer_size > 1345 and buffer_size <= 1870){
            cfg.window_sz2 = 7;
            cfg.lookahead_sz2 = 6;
          }
          else{
            Serial.print("Data terlalu besar");
            delay(4000);
            return 0;
          }
          compress_and_expand_and_check(test_data, length_data, &cfg);
          memset(test_data, 0, buffer_size);
          
          delay(6000);
      }
      myFile.close();
    }
    else {
      Serial.println("error opening test.txt");
    }

    for ( ;; ){
      //Serial.print("a100");
      //Serial.print("\n");
      delay(3000);
    }
        
}
