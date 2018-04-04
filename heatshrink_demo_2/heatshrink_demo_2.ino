// Demo Code for Heatshrink (Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>)
// embedded compression library
// Craig Versek, Apr. 2016

#include <stdint.h>
#include <ctype.h>
#include <assert.h>
#include <Arduino.h>
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"
#include "greatest.h"

#define arduinoLED 13   // Arduino LED on board

/******************************************************************************/
// TEST CODE from adapted from test_heatshrink_static.c
#if HEATSHRINK_DYNAMIC_ALLOC
#error HEATSHRINK_DYNAMIC_ALLOC must be false for static allocation test suite.
#endif

//#define HEATSHRINK_DEBUG

SUITE(integration);

static heatshrink_encoder hse;
static heatshrink_decoder hsd;

static void fill_with_pseudorandom_letters(uint8_t *buf, uint16_t size, uint32_t seed) {
    uint64_t rn = 9223372036854775783; // prime under 2^64
    for (int i=0; i<size; i++) {
        rn = rn*seed + seed;
        buf[i] = (rn % 26) + 'a';
    }
}

static void dump_buf(char *name, uint8_t *buf, uint16_t count) {
    for (int i=0; i<count; i++) {
        uint8_t c = (uint8_t)buf[i];
        printf("%s %d: 0x%02x ('%c')\n", name, i, c, isprint(c) ? c : '.');
    }
}

static int compress_and_expand_and_check(uint8_t *input, uint32_t input_size, int log_lvl) {
    heatshrink_encoder_reset(&hse);
    heatshrink_decoder_reset(&hsd);
    size_t comp_sz = input_size + (input_size/2) + 4;
    size_t decomp_sz = input_size + (input_size/2) + 4;
    uint8_t *comp = malloc(comp_sz);
    uint8_t *decomp = malloc(decomp_sz);
    if (comp == NULL) 
      Serial.print(F("FAIL: Malloc fail!"));
      //FAILm("malloc fail");
    if (decomp == NULL) 
      Serial.print(F("FAIL: Malloc fail!"));
      //FAILm("malloc fail");
    memset(comp, 0, comp_sz);
    memset(decomp, 0, decomp_sz);

    size_t count = 0;

    if (log_lvl > 1) {
//        printf("\n^^ COMPRESSING\n");
//        dump_buf("input", input, input_size);

        Serial.print(F("\n^^ COMPRESSING\n"));
        dump_buf("input", input, input_size);
    }

    //
    #ifdef HEATSHRINK_DEBUG
    Serial.print(F("\n^^ COMPRESSING\n"));
    dump_buf("input", input, input_size);
    #endif

    uint32_t sunk = 0;
    uint32_t polled = 0;
    while (sunk < input_size) {
        //ASSERT(heatshrink_encoder_sink(&hse, &input[sunk], input_size - sunk, &count) >= 0);
        heatshrink_encoder_sink(&hse, &input[sunk], input_size - sunk, &count);
        sunk += count;
        if (log_lvl > 1){
        //  printf("^^ sunk %zd\n", count);
          Serial.print(F("^^ sunk "));
          Serial.print(count);
          Serial.print(F("\n"));
        }

        #ifdef HEATSHRINK_DEBUG
        Serial.print(F("^^ sunk "));
        Serial.print(count);
        Serial.print(F("\n"));
        #endif
        
        if (sunk == input_size) {
            //ASSERT_EQ(HSER_FINISH_MORE, heatshrink_encoder_finish(&hse));
            heatshrink_encoder_finish(&hse);
        }

        HSE_poll_res pres;
        do {                    /* "turn the crank" */
            pres = heatshrink_encoder_poll(&hse, &comp[polled], comp_sz - polled, &count);
            //ASSERT(pres >= 0);
            polled += count;
            if (log_lvl > 1){
             // printf("^^ polled %zd\n", count);
              Serial.print(F("^^ polled "));
              Serial.print(polled);
              Serial.print(F("\n"));
            }
            //
            #ifdef HEATSHRINK_DEBUG
            Serial.print(F("^^ polled "));
            Serial.print(polled);
            Serial.print(F("\n"));
            #endif
        } while (pres == HSER_POLL_MORE);
        //ASSERT_EQ(HSER_POLL_EMPTY, pres);
        if (polled >= comp_sz) 
            //FAILm("compression should never expand that much");
            Serial.print(F("FAIL: Exceeded the size of the output buffer!"));
        if (sunk == input_size) {
            //ASSERT_EQ(HSER_FINISH_DONE, heatshrink_encoder_finish(&hse));
            heatshrink_encoder_finish(&hse);
        }
    }
    if (log_lvl > 0) 
      //printf("in: %u compressed: %u ", input_size, polled);
      Serial.print(F("FAIL: Exceeded the size of the output buffer!\n"));
    
    //
    #ifdef HEATSHRINK_DEBUG
    Serial.print(F("in: "));
    Serial.print(input_size);
    Serial.print(F(" compressed: "));
    Serial.print(polled);
    Serial.print(F(" \n"));
    #endif
    
    uint32_t compressed_size = polled;
    sunk = 0;
    polled = 0;
    
    if (log_lvl > 1) {
        printf("\n^^ DECOMPRESSING\n");
        dump_buf("comp", comp, compressed_size);
    }
    while (sunk < compressed_size) {
        //ASSERT(heatshrink_decoder_sink(&hsd, &comp[sunk], compressed_size - sunk, &count) >= 0);
        heatshrink_decoder_sink(&hsd, &input[sunk], input_size - sunk, &count);
        sunk += count;
        if (log_lvl > 1) printf("^^ sunk %zd\n", count);
        if (sunk == compressed_size) {
            //ASSERT_EQ(HSDR_FINISH_MORE, heatshrink_decoder_finish(&hsd));
            heatshrink_decoder_finish(&hsd);
        }

        HSD_poll_res pres;
        do {
            pres = heatshrink_decoder_poll(&hsd, &decomp[polled],
                decomp_sz - polled, &count);
            //ASSERT(pres >= 0);
            polled += count;
            if (log_lvl > 1) printf("^^ polled %zd\n", count);
        } while (pres == HSDR_POLL_MORE);
        //ASSERT_EQ(HSDR_POLL_EMPTY, pres);
        if (sunk == compressed_size) {
            HSD_finish_res fres = heatshrink_decoder_finish(&hsd);
            //ASSERT_EQ(HSDR_FINISH_DONE, fres);
        }

        if (polled > input_size) {
            FAILm("Decompressed data is larger than original input");
        }
    }
    if (log_lvl > 0) printf("decompressed: %u\n", polled);
    if (polled != input_size) {
        FAILm("Decompressed length does not match original input length");
    }

    if (log_lvl > 1) dump_buf("decomp", decomp, polled);
    for (size_t i=0; i<input_size; i++) {
        if (input[i] != decomp[i]) {
            printf("*** mismatch at %zd\n", i);
            if (0) {
                for (size_t j=0; j<=/*i*/ input_size; j++) {
                    printf("in[%zd] == 0x%02x ('%c') => out[%zd] == 0x%02x ('%c')\n",
                        j, input[j], isprint(input[j]) ? input[j] : '.',
                        j, decomp[j], isprint(decomp[j]) ? decomp[j] : '.');
                }
            }
        }
        //ASSERT_EQ(input[i], decomp[i]);
    }
    free(comp);
    free(decomp);
    PASS();
}

TEST pseudorandom_data_should_match(uint32_t size, uint32_t seed) {
    uint8_t input[size];
    fill_with_pseudorandom_letters(input, size, seed);
//    Serial.print("Test data: ");
//    for(int i = 0; i < sizeof(input); i++){
//      Serial.print(input[i]);
//      Serial.print(", ");
//    }
//    Serial.println();
    return compress_and_expand_and_check(input, size, 0);
}
//
SUITE(integration) {
#if __STDC_VERSION__ >= 19901L
    for (uint32_t size=1; size < 64*1024; size <<= 1) {
        if (GREATEST_IS_VERBOSE()) printf(" -- size %u\n", size);
            for (uint32_t seed=1; seed<=100; seed++) {
                if (GREATEST_IS_VERBOSE()) printf(" -- seed %u\n", seed);
                RUN_TESTp(pseudorandom_data_should_match, size, seed);
            }
    }
#endif
}

/******************************************************************************/
GREATEST_MAIN_DEFS();

#define BUFFER_SIZE 256
uint8_t orig_buffer[BUFFER_SIZE];
uint8_t comp_buffer[BUFFER_SIZE];
uint8_t decomp_buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    delay(5000);
    int i;
    
    uint32_t length;
    //write some data into the compression buffer

    //GREATEST_MAIN_BEGIN();      /* command-line arguments, initialization. */
    Serial.print("INPUT_BUFFER_SIZE: ");
    Serial.println(HEATSHRINK_STATIC_INPUT_BUFFER_SIZE);
    Serial.print("WINDOW_BITS: ");
    Serial.println(HEATSHRINK_STATIC_WINDOW_BITS);
    Serial.print("LOOKAHEAD_BITS: ");
    Serial.println(HEATSHRINK_STATIC_LOOKAHEAD_BITS);

    Serial.print("sizeof(heatshrink_encoder): ");
    Serial.println(sizeof(heatshrink_encoder));
    Serial.print("sizeof(heatshrink_decoder): ");
    Serial.println(sizeof(heatshrink_decoder));

    int panjang_size = sizeof(heatshrink_encoder);
//    memcpy(orig_buffer, heatshrink_encoder, panjang_size);
    
    Serial.println();
    Serial.print("Test data: ");
    for(i = 0; i < sizeof(heatshrink_encoder); i++){
  //    Serial.print(heatshrink_encoder[i]);
      Serial.print(", ");
    }Serial.println();
    
    //RUN_SUITE(integration);
    //GREATEST_MAIN_END();        /* display results */
    for ( ;; )
        delay(100);
}


