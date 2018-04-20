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

static int compress_and_expand_and_check(uint8_t *input, uint32_t input_size, cfg_info *cfg) {
    heatshrink_encoder *hse = heatshrink_encoder_alloc(cfg->window_sz2,
        cfg->lookahead_sz2);
    heatshrink_decoder *hsd = heatshrink_decoder_alloc(cfg->decoder_input_buffer_size,
        cfg->window_sz2, cfg->lookahead_sz2);
    size_t comp_sz = input_size + (input_size/2) + 4;
    size_t decomp_sz = input_size + (input_size/2) + 4;

    uint8_t *comp = (uint8_t*)malloc(comp_sz);
    uint8_t *decomp = (uint8_t*)malloc(decomp_sz);
    if (comp == NULL) 
      Serial.println(F("FAIL: Malloc fail!"));
    if (decomp == NULL) 
      Serial.println(F("FAIL: Malloc fail!"));
    memset(comp, 0, comp_sz);
    memset(decomp, 0, decomp_sz);

    size_t count = 0;
    size_t sunk = 0;
    size_t polled = 0;
    
    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ COMPRESSING\n"));
        dump_buf("input", input, input_size);
    }
    
    while (sunk < input_size) {
        HSE_sink_res esres = heatshrink_encoder_sink(hse, &input[sunk], input_size - sunk, &count);
        //ASSERT(esres >= 0);
        sunk += count;
        if (cfg->log_lvl > 1){
          Serial.print(F("^^ sunk "));
          Serial.print(count);
          Serial.print(F("\n"));
        }
        if (sunk == input_size) {
            //ASSERT_EQ(HSER_FINISH_MORE, heatshrink_encoder_finish(hse));
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
        //ASSERT_EQ(HSER_POLL_EMPTY, pres);
        if (polled >= comp_sz) 
          Serial.print(F("FAIL: Compression should never expand that much!"));
        if (sunk == input_size) {
            //ASSERT_EQ(HSER_FINISH_DONE, heatshrink_encoder_finish(hse));
            heatshrink_encoder_finish(hse);
        }
    }
    if (cfg->log_lvl > 0){
      Serial.print(F("in: "));
      Serial.print(input_size);
      Serial.print(F(" compressed: "));
      Serial.print(polled);
      Serial.print(F(" \n")); 
    }

    //tambahan
    Serial.print("Origin data: ");
    for(int i = 0; i < input_size; i++){
      Serial.print(input[i]);
      Serial.print(", ");
    }Serial.println();
  
    Serial.print("Compressed data: ");
    for(int i = 0; i < polled; i++){
      Serial.print(comp[i]);
      Serial.print(", ");
    }Serial.println();
    
    size_t compressed_size = polled;
    sunk = 0;
    polled = 0;
    
    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ DECOMPRESSING\n"));
        dump_buf("comp", comp, compressed_size);
    }
    while (sunk < compressed_size) {
        //ASSERT(heatshrink_decoder_sink(hsd, &comp[sunk], compressed_size - sunk, &count) >= 0);
        heatshrink_decoder_sink(hsd, &comp[sunk], compressed_size - sunk, &count);
        //heatshrink_decoder_sink(hsd, &comp[sunk], compressed_size - sunk, &count);
        sunk += count;
        if (cfg->log_lvl > 1){
          Serial.print(F("^^ sunk "));
          Serial.print(count);
          Serial.print(F("\n"));
        }
        if (sunk == compressed_size) {
            //ASSERT_EQ(HSDR_FINISH_MORE, heatshrink_decoder_finish(hsd));
            heatshrink_decoder_finish(hsd);
        }

        HSD_poll_res pres;
        do {
            pres = heatshrink_decoder_poll(hsd, &decomp[polled],
                decomp_sz - polled, &count);
            //ASSERT(pres >= 0);
            //ASSERT(count > 0);
            polled += count;
            if (cfg->log_lvl > 1){
              Serial.print(F("^^ polled "));
              Serial.print(count);
              Serial.print(F("\n"));
            }
        } while (pres == HSDR_POLL_MORE);
        //ASSERT_EQ(HSDR_POLL_EMPTY, pres);
        if (sunk == compressed_size) {
            HSD_finish_res fres = heatshrink_decoder_finish(hsd);
            //ASSERT_EQ(HSDR_FINISH_DONE, fres);
        }

        if (polled > input_size) {
            //printf("\nExpected %zd, got %zu\n", (size_t)input_size, polled);
            Serial.print(F("nExpected "));
            Serial.print((size_t)input_size);
            Serial.print(F(" got: "));
            Serial.print(polled);
            Serial.print(F(" \n")); 
            Serial.print(F("FAIL: Decompressed data is larger than original input!"));
        }
    }
    if (cfg->log_lvl > 0){
        Serial.print(F("in: "));
        Serial.print(compressed_size);
        Serial.print(F(" decompressed: "));
        Serial.print(polled);
        Serial.print(F(" \n")); 
        //printf("decompressed: %zu\n", polled);
    }
    if (polled != input_size) {
        //FAILm("Decompressed length does not match original input length");
        Serial.print(F("FAIL: Decompressed length does not match original input length!"));
    }

    if (cfg->log_lvl > 1) dump_buf("decomp", decomp, polled);
    for (uint32_t i=0; i<input_size; i++) {
        if (input[i] != decomp[i]) {
           // printf("*** mismatch at %d\n", i);
            Serial.print(F("*** mismatch at: "));
            Serial.print(i);
            Serial.print(F(" \n")); 
//            if (0) {
//                for (uint32_t j=0; j<=/*i*/ input_size; j++) {
//                    printf("in[%d] == 0x%02x ('%c') => out[%d] == 0x%02x ('%c')  %c\n",
//                        j, input[j], isprint(input[j]) ? input[j] : '.',
//                        j, decomp[j], isprint(decomp[j]) ? decomp[j] : '.',
//                        input[j] == decomp[j] ? ' ' : 'X');
//                }
//            }
        }
        //ASSERT_EQ(input[i], decomp[i]);
    }

    //tambahan
    Serial.print("Origin data: ");
    for(int i = 0; i < polled; i++){
      Serial.print(decomp[i]);
      Serial.print(", ");
    }Serial.println();
  
    free(comp);
    free(decomp);
    heatshrink_encoder_free(hse);
    heatshrink_decoder_free(hsd);
//    PASS();
}


/******************************************************************************/
QuickStats stats; //initialize an instance of this class

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    delay(5000);
    //uint32_t orig_size = 100;  
    int length_data;
    float readings[1000], stdeviasi;
    //float readings[] = {1, 2, 3, 1, 2, 0, 3, 4, 5, 1, 1, 2, 3, 9, 0, 1, 1, 0, 0, 0, 3, 1, 1, 2, 3, 3, 1, 1, 2, 2, 3, 4, 5, 2, 2, 3, 4, 5, 1, 2, 2, 0, 0, 2, 7, 8, 7, 7, 7, 8, 3, 4, 2, 3, 2, 8, 0, 3, 4, 5, 2, 1, 4, 5, 2, 2, 3, 4, 5, 0, 2, 2, 0, 0, 2, 7, 8, 7, 7, 7};
    
    //uint8_t test_data[] = {'1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7'};

    //100 data
    //uint8_t test_data[] = {'1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0'};
    //uint8_t test_data[] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};
    //uint8_t test_data[] = {1, 2, 3, 1, 2, 0, 3, 4, 5, 1, 1, 2, 3, 9, 0, 1, 1, 0, 0, 0, 3, 1, 1, 2, 3, 3, 1, 1, 2, 2, 3, 4, 5, 2, 2, 3, 4, 5, 1, 2, 2, 0, 0, 2, 7, 8, 7, 7, 7, 8, 3, 4, 2, 3, 2, 8, 0, 3, 4, 5, 2, 1, 4, 5, 2, 2, 3, 4, 5, 0, 2, 2, 0, 0, 2, 7, 8, 7, 7, 7}; 
    //uint8_t test_data[] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};

    //float test_data[] = {'1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0'};
    //uint8_t test_data[] = {'1.2', '1.2', '1.3', '1.4', '1.5', '1.1', '1.9', '1.2', '1.3', '1.4', '2.5', '2.3', '2.3', '2.2', '2.3', '2.4', '2.5', '2.6', '2.4', '2.2', '3.3', '3.4', '3.1', '3.1', '3.2', '3.3', '3.4', '3.5', '3.5', '3.6', '4.5', '4.2', '4.1', '4.4', '4.5', '4.6', '4.4', '4.4', '4.3', '4.3', '5.3', 5.3, 5.3, 5.3, 5.3, 5.3, 5.5, 5.8, 5.6, 5.5, 6.4, 6.3, 6.2, 6.0, 6.0, 6.0, 6.0, 6.8, 6.7, 6.2, 7.1, 7.1, 7.1, 7.9, 7.7, 7.5, 7.5, 7.4, 7.3, 7.2, 8.1, 8.2, 8.2, 8.3, 8.4, 8.5, 8.9, 8.0, 8.0, 8.3, 9.9, 9.8, 9.7, 9.6, 9.5, 9.4, 9.3, 9.2, 9.2, 9.2, 10.3, 10.5, 10.9, 10.1, 10.2, 10.5, 10.4, 10.3, 10.2, 10.1};
    uint8_t test_data[] = {'1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0'};

    
    Serial.print("test : ");
    Serial.println(test_data[1]);
    
    length_data = sizeof(test_data)/sizeof(test_data[0]);
    Serial.print("Panjang data: ");
    Serial.println(length_data);
    //Serial.println(sizeof(test_data));
    
    //convert char to float
    for(int i=0; i<length_data; i++){
      readings[i] =  test_data[i] - '0';
    }
  
    Serial.print(F("*** Reading data: "));
    for(int i = 0; i < length_data; i++){
      Serial.print(readings[i]);
      Serial.print(", ");
    }Serial.println();
    Serial.print("Standard Deviation: ");
    stdeviasi = stats.stdev(readings,length_data);
    Serial.println(stdeviasi);
    
    cfg_info cfg;
    cfg.log_lvl = 2;
    cfg.window_sz2 = 7;
    cfg.lookahead_sz2 = 3;
      
//    if(stdeviasi > 2)
//    {
//      cfg.window_sz2 = 7;
//      cfg.lookahead_sz2 = 4;
//    }else if(stdeviasi > 1 && stdeviasi < 2){
//      cfg.window_sz2 = 6;
//      cfg.lookahead_sz2 = 3;
//    }else{
//      cfg.window_sz2 = 4;
//      cfg.lookahead_sz2 = 3;
//    }
    cfg.decoder_input_buffer_size = 64;
    
    compress_and_expand_and_check(test_data, length_data, &cfg);

    Serial.println();
    Serial.print("Window size: ");
    Serial.println(cfg.window_sz2);
    Serial.print("Lookahead size: ");
    Serial.println(cfg.lookahead_sz2);
    
    for ( ;; )
        delay(3000);
}


