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

static void removeChar(char * string, char letter ) {
  for( unsigned int i = 0; i < strlen( string ); i++ )
    if( string[i] == letter )
      strcpy( string + i, string + i + 1 );
}

static void dump_buf(char *name, uint8_t *buf, uint16_t count) {
    for (int i=0; i<count; i++) {
        uint8_t c = (uint8_t)buf[i];
        printf("%s %d: 0x%02x ('%c')\n", name, i, c, isprint(c) ? c : '.');
    }
}

static void decompress_and_expand_and_check(uint8_t *comp, uint32_t input_size, cfg_info *cfg, size_t polled2) {

    //Serial.print(F("^^ Processing\n"));
    heatshrink_decoder *hsd = heatshrink_decoder_alloc(cfg->decoder_input_buffer_size,
        cfg->window_sz2, cfg->lookahead_sz2);

    size_t decomp_sz = input_size + (input_size/10) + 4;
    uint8_t *decomp = (uint8_t*)malloc(decomp_sz);
    if (decomp == NULL) 
      Serial.println(F("FAIL: Malloc fail!"));
    memset(decomp, 0, decomp_sz);
    
    size_t compressed_size = polled2;
    size_t count  = polled2;
    size_t sunk = 0;
    size_t polled = 0;
    
    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ DECOMPRESSING\n"));
        dump_buf("comp", comp, compressed_size);
    }
    while (sunk < compressed_size) {
        heatshrink_decoder_sink(hsd, &comp[sunk], compressed_size - sunk, &count);
        sunk += count;
        if (cfg->log_lvl > 1){
          Serial.print(F("^^ sunk "));
          Serial.print(count);
          Serial.print(F("\n"));
        }
        if (sunk == compressed_size) {
            heatshrink_decoder_finish(hsd);
        }

        HSD_poll_res pres;
        do {
            pres = heatshrink_decoder_poll(hsd, &decomp[polled], decomp_sz - polled, &count);
            polled += count;
            if (cfg->log_lvl > 1){
              Serial.print(F("^^ polled "));
              Serial.print(count);
              Serial.print(F("\n"));
            }
        } while (pres == HSDR_POLL_MORE);
        if (sunk == compressed_size) {
            HSD_finish_res fres = heatshrink_decoder_finish(hsd);
        }

        if (polled > input_size) {
            Serial.print(F("nExpected "));
            Serial.print((size_t)input_size);
            Serial.print(F(" got: "));
            Serial.print(polled);
            Serial.print(F(" \n")); 
            Serial.print(F("FAIL: Decompressed data is larger than original input!"));
        }
    }
   
    if (polled != input_size) {
        Serial.print(F("polled: "));
        Serial.print(polled);
        Serial.print(F(" \n")); 
        Serial.print(F("input size: "));
        Serial.print(input_size);
        Serial.print(F(" \n")); 
        Serial.print(F("FAIL: Decompressed length does not match original input length!"));
    }

    if (cfg->log_lvl > 1) 
      dump_buf("decomp", decomp, polled);

    if (cfg->log_lvl > 0){
        Serial.print(F("in: "));
        Serial.print(compressed_size);
        Serial.print(F(" decompressed: "));
        Serial.print(polled);
        Serial.print(F(" \n")); 

        Serial.print("Decompressed data: ");
        for(int i = 0; i < polled; i++){
          Serial.print(decomp[i]);
          Serial.print(", ");
        }
        Serial.println();
    }

    free(decomp);
    heatshrink_decoder_free(hsd);
}




/******************************************************************************/
int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);

    uint32_t decomp_size; //this will get updated by reference
    uint8_t origin_char[1640];
    size_t polled = 0;

    String stringOne;
    char incomingByte;
    char buf[20];
    char *s, *orig_sz, *window_sz, *lookahead_sz, *decoder_sz, *polled_sz, *node_type, *input_sz;
    int idx=0, i=0, j=0, orig=0, window=0, lookahead=0, decoder=0, node=0, input_size;
    
    size_t comp_sz;
    size_t polleds = 0;
    
    int num[1640];
    memset(num,0,1640);
   
    Serial.print(F("\nIncoming data : \n"));
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
                num[idx] = stringOne.toInt();
                idx++;
              }
              stringOne = "";
              memset(buf,0,20);
           }
        }
        else if(Serial.available() <= 0){
          Serial.print(F("| idx : "));
          Serial.println(idx);
          if(idx == polled && polled != 0){
              if(num[polled] != 0 || num[polled-1] != 0){
                  Serial.print(F("Challenge Accepted!\n"));
                  for(j=0; j<polled; j++){
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
             
                  if(polled != 0 && window != 0 && lookahead != 0 && decoder !=0){
                    Serial.print(F("^^ Dekompresi Start!\n"));
                    for(int i = 0; i < polled; i++){
                        origin_char[i] = (uint8_t) num[i];
                    }
     
                    cfg_info cfg;
                    cfg.log_lvl = 1;
                    cfg.window_sz2 = window;
                    cfg.lookahead_sz2 = lookahead;
                    cfg.decoder_input_buffer_size = decoder;
                    decompress_and_expand_and_check(origin_char, orig, &cfg,  polled);
    
                    Serial.print(F("^^ Selesai\n"));
                    if(node != 0){
                      //reinisialite
                      orig = 0, window = 0, lookahead = 0, decoder = 0, polled = 0, node = 0;
                      idx = 0;
                   }
                }
             }
             else if(polled != 0 && window != 0 && lookahead != 0 && decoder !=0){
                  Serial.print(F("^^ Dekompresi Start!\n"));
                  for(int i = 0; i < polled; i++){
                      origin_char[i] = (uint8_t) num[i];
                  }
   
                  cfg_info cfg;
                  cfg.log_lvl = 1;
                  cfg.window_sz2 = window;
                  cfg.lookahead_sz2 = lookahead;
                  cfg.decoder_input_buffer_size = decoder;
                  decompress_and_expand_and_check(origin_char, orig, &cfg,  polled);
  
                  Serial.print(F("^^ Selesai\n"));
                  if(node != 0){
                    //reinisialite
                    orig = 0, window = 0, lookahead = 0, decoder = 0, polled = 0, node = 0;
                    idx = 0;
                 }
              }
              else{
                  Serial.print(num[polled-1]);
                  Serial.print(" ");
                  Serial.print(num[polled]);
                Serial.print(F("\n^^ Ampas\n"));
              }
          }
          delay(1000);
        }
    }   
}



