// Demo Code for Heatshrink (Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>)
// embedded compression library
// Craig Versek, Apr. 2016

#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>

#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"
#include "greatest.h"

#define arduinoLED 13   // Arduino LED on board

/******************************************************************************/
// TEST CODE from adapted from test_heatshrink_dynamic.c
#if !HEATSHRINK_DYNAMIC_ALLOC
#error Must set HEATSHRINK_DYNAMIC_ALLOC to 1 for dynamic allocation test suite.
#endif

//#define HEATSHRINK_DEBUG

SUITE(encoding);
SUITE(decoding);
SUITE(regression);
SUITE(integration);

#ifdef HEATSHRINK_HAS_THEFT
SUITE(properties);
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


//typedef struct {
//    uint8_t log_lvl;
//    uint8_t window_sz2;
//    uint8_t lookahead_sz2;
//    size_t decoder_input_buffer_size;
//} cfg_info;

static int compress_and_expand_and_check(uint8_t *input, uint32_t input_size, cfg_info *cfg) {
    heatshrink_encoder *hse = heatshrink_encoder_alloc(cfg->window_sz2,
        cfg->lookahead_sz2);
   // ASSERT(hse);
    heatshrink_decoder *hsd = heatshrink_decoder_alloc(cfg->decoder_input_buffer_size,
        cfg->window_sz2, cfg->lookahead_sz2);
   //ASSERT(hsd);
//    size_t comp_sz = input_size + (input_size/2) + 4;
//    size_t decomp_sz = input_size + (input_size/2) + 4;
    size_t comp_sz = 254;
    size_t decomp_sz = 254;
    uint8_t *comp = (uint8_t*)malloc(comp_sz);
    uint8_t *decomp = (uint8_t*)malloc(decomp_sz);
    if (comp == NULL) 
      Serial.print(F("FAIL: Malloc fail!"));
    if (decomp == NULL) 
      Serial.print(F("FAIL: Malloc fail!"));
    memset(comp, 0, comp_sz);
    memset(decomp, 0, decomp_sz);

    size_t count = 0;

    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ COMPRESSING\n"));
        dump_buf("input", input, input_size);
    }

    size_t sunk = 0;
    size_t polled = 0;
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
        Serial.print(input_size);
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
//    for (uint32_t i=0; i<input_size; i++) {
//        if (input[i] != decomp[i]) {
//            printf("*** mismatch at %d\n", i);
//            if (0) {
//                for (uint32_t j=0; j<=/*i*/ input_size; j++) {
//                    printf("in[%d] == 0x%02x ('%c') => out[%d] == 0x%02x ('%c')  %c\n",
//                        j, input[j], isprint(input[j]) ? input[j] : '.',
//                        j, decomp[j], isprint(decomp[j]) ? decomp[j] : '.',
//                        input[j] == decomp[j] ? ' ' : 'X');
//                }
//            }
//        }
//        ASSERT_EQ(input[i], decomp[i]);
//    }
    free(comp);
    free(decomp);
    heatshrink_encoder_free(hse);
    heatshrink_decoder_free(hsd);
 //   PASS();
}


/******************************************************************************/
//GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    delay(5000);

    uint8_t test_data[] = {'1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7'};
    uint32_t orig_size = 80;            //strlen(test_data);
    
    cfg_info cfg;
    cfg.log_lvl = 2;
    cfg.window_sz2 = 7;
    cfg.lookahead_sz2 = 3;
    cfg.decoder_input_buffer_size = 64;
    
    compress_and_expand_and_check(test_data, orig_size, &cfg);

    Serial.print("sizeof(heatshrink_encoder): ");
    Serial.println(sizeof(heatshrink_encoder));
    Serial.print("sizeof(heatshrink_decoder): ");
    Serial.println(sizeof(heatshrink_decoder));

    for ( ;; )
        delay(100);
}


