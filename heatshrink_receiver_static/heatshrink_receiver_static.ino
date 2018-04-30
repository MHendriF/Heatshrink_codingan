// Demo Code for Heatshrink (Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>)
// embedded compression library
// Craig Versek, Apr. 2016

#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <QuickStats.h>

#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"
#include "greatest.h"

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

static int decompress_and_expand_and_check(uint8_t *comp, uint8_t *input, uint32_t input_size, cfg_info *cfg2, size_t polled2, size_t count) {

    heatshrink_decoder *hsd = heatshrink_decoder_alloc(cfg2->decoder_input_buffer_size,
        cfg2->window_sz2, cfg2->lookahead_sz2);
    size_t decomp_sz = input_size + (input_size/2) + 4;

    uint8_t *decomp = (uint8_t*)malloc(decomp_sz);
    if (decomp == NULL) 
      Serial.println(F("FAIL: Malloc fail!"));
    memset(decomp, 0, decomp_sz);

    size_t compressed_size = polled2;
    size_t sunk = 0;
    size_t polled = 0;
    
    if (cfg2->log_lvl > 1) {
        Serial.print(F("\n^^ DECOMPRESSING\n"));
        dump_buf("comp", comp, compressed_size);
    }
    while (sunk < compressed_size) {
        heatshrink_decoder_sink(hsd, &comp[sunk], compressed_size - sunk, &count);
        sunk += count;
        if (cfg2->log_lvl > 1){
          Serial.print(F("^^ sunk "));
          Serial.print(count);
          Serial.print(F("\n"));
        }
        if (sunk == compressed_size) {
            heatshrink_decoder_finish(hsd);
        }

        HSD_poll_res pres;
        do {
            pres = heatshrink_decoder_poll(hsd, &decomp[polled],
                decomp_sz - polled, &count);
            polled += count;
            if (cfg2->log_lvl > 1){
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
    if (cfg2->log_lvl > 0){
        Serial.print(F("in: "));
        Serial.print(compressed_size);
        Serial.print(F(" decompressed: "));
        Serial.print(polled);
        Serial.print(F(" \n")); 
    }
    if (polled != input_size) {
        Serial.print(F("FAIL: Decompressed length does not match original input length!"));
    }

    if (cfg2->log_lvl > 1) dump_buf("decomp", decomp, polled);
    for (uint32_t i=0; i<input_size; i++) {
        if (input[i] != decomp[i]) {
           // printf("*** mismatch at %d\n", i);
            Serial.print(F("*** mismatch at: "));
            Serial.print(i);
            Serial.print(F(" \n"));
        }
    }

    //tambahan
    Serial.print("Decompressed data: ");
    for(int i = 0; i < polled; i++){
      Serial.print(decomp[i]);
      Serial.print(", ");
    }Serial.println();
  
    free(decomp);
    heatshrink_decoder_free(hsd);
}

static int compress_and_expand_and_check(uint8_t *input, uint32_t input_size, cfg_info *cfg) {
    heatshrink_encoder *hse = heatshrink_encoder_alloc(cfg->window_sz2,
        cfg->lookahead_sz2);
    size_t comp_sz = input_size + (input_size/2) + 4;

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
      
      Serial.print("Origin data: ");
      for(int i = 0; i < input_size; i++){
        Serial.print(input[i]);
        Serial.print(", ");
      }Serial.println();
      Serial.print("Compressed data: ");
      for(int i = 0; i < polled; i++){
        //Serial.print(comp[i]);
        Serial.print(comp[i]);
        Serial.print(", ");
      }Serial.println();
    }

    cfg_info cfg2;
    cfg2.log_lvl = 2;
    cfg2.window_sz2 = 8;
    cfg2.lookahead_sz2 = 4;
    cfg2.decoder_input_buffer_size = 64;
    
    decompress_and_expand_and_check(comp, input, input_size, &cfg2, polled, count);
    
    free(comp);
    heatshrink_encoder_free(hse);
}




/******************************************************************************/
QuickStats stats; //initialize an instance of this class

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    //delay(5000);
    int length_data;
    float readings[1000], stdeviasi;
   
    const uint8_t test_data[] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};
    //char test_data [] = "e8h5888e8h5888e8h5888yyxnyyxny454yyxnqx5e7yyxntu98xge9pdgzycb7had5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvvc828dctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne";
    //uint8_t test_data [] = "<div class=\"panel panel-default\"><div id=\"heading_1\" class=\"panel-heading activestate\"><a href=\"#collapse_1\" data-toggle=\"collapse\" data-parent=\"#accordion_1\">1. Maksimal Upload lampiran data</a></div><div id=\"collapse_1\" class=\"panel-collapse col";
    
    length_data = sizeof(test_data)/sizeof(test_data[0]);

    if(length_data > 584){
      Serial.print("Data terlalu besar, data yang muat untuk kompresi maksimal 584 karakter");
      delay(4000);
      return 0;
    }
    
    cfg_info cfg;
    cfg.log_lvl = 2;
      
    if(length_data <= 248){
      cfg.window_sz2 = 8;
      cfg.lookahead_sz2 = 4;
    }else if(length_data > 248 && length_data <= 517){
      cfg.window_sz2 = 6;
      cfg.lookahead_sz2 = 3;
    }else if(length_data > 517 && length_data <= 561){
      cfg.window_sz2 = 5;
      cfg.lookahead_sz2 = 3;
    }else if(length_data > 561 && length_data <= 584){
      cfg.window_sz2 = 4;
      cfg.lookahead_sz2 = 3;
    }
    cfg.decoder_input_buffer_size = 64;
    compress_and_expand_and_check(test_data, length_data, &cfg);

    Serial.println();
    Serial.print("Window size: ");
    Serial.println(cfg.window_sz2);
    Serial.print("Lookahead size: ");
    Serial.println(cfg.lookahead_sz2);
    
    for ( ;; ){
      //Serial.println("B");
      delay(5000);
    }
        
}


