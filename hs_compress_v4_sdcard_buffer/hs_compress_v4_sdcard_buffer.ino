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
    //size_t comp_sz = input_size + 45;
    uint8_t *comp = (uint8_t*)malloc(comp_sz);
    if (comp == NULL) 
      Serial.println(F("FAIL: Malloc fail!"));
    memset(comp, 0, comp_sz);
     
    size_t count = 0;
    size_t sunk = 0;
    size_t polled = 0;
    
    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ COMPRESSING\n"));
        dump_buf("input", input, input_size);
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
      
      Serial.print("Compressed data: ");
      for(int i = 0; i < polled; i++){
        Serial.print(comp[i]);
        Serial.print(", ");
      }Serial.println();
    }
    free(comp);
    heatshrink_encoder_free(hse);
    return polled;
}


/******************************************************************************/
#define BUFFER_SIZE 1640

int main(int argc, char **argv)
{
    init(); // this is needed

    Serial.begin(9600);
    pinMode(pinCS, OUTPUT);

    int data_sz = BUFFER_SIZE;
    uint32_t length_data;
    String bufferSD;
    char test_data [data_sz];

    uint32_t comp_size = BUFFER_SIZE;
    size_t polled = 0;
    
    // SD Card Initialization
    if (SD.begin())
    {
      Serial.println("SD card is ready to use.");
    } else
    {
      Serial.println("SD card initialization failed");
      return;
    }
    
    // Create/Open file 
    myFile = SD.open("test5.txt", FILE_WRITE);

    int iterate = 0;
    // Reading the file
    myFile = SD.open("test5.txt");
    if (myFile) {
      Serial.println("Read:");
      // Reading the whole file
      while (myFile.available()) {
          iterate++;
          Serial.print("Iterate: ");
          Serial.println(iterate);
         
          bufferSD = myFile.readStringUntil('\n');
          //Serial.println(bufferSD); //Printing for debugging purpose

          //Prepare to commpression
          bufferSD.toCharArray(test_data, data_sz);
          //memcpy(orig_buffer, test_data, length_data);
          length_data = sizeof(test_data)/sizeof(test_data[0]);
          //Serial.println(length_data);
          
          cfg_info cfg;
          cfg.log_lvl = 0;
          cfg.window_sz2 = 8;
          cfg.lookahead_sz2 = 3;
          cfg.decoder_input_buffer_size = 64;
          uint32_t t1 = micros();
          polled = compress_and_expand_and_check(test_data, length_data, &cfg);
          uint32_t t2 = micros();
          Serial.print("Polled: ");
          Serial.println(polled);
          Serial.print("Time to compress: ");
          Serial.println((t2-t1)/1e6,6);
          Serial.println();
          

          t1 = 0;
          t2 = 0;
          memset(test_data, 0, data_sz);
          
          delay(2000);
      }
      myFile.close();
    }
    else {
      Serial.println("error opening test.txt");
    }
    
    for ( ;; ){
      //Serial.print("\n");
      delay(3000);
    }
        
}
