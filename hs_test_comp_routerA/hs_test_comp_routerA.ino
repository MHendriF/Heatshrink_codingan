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

#define arduinoLED 13

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
//    if (cfg->log_lvl > 0){
//      Serial.print(F("in: "));
//      Serial.print(input_size);
//      Serial.print(F(" compressed: "));
//      Serial.print(polled);
//      Serial.print(F(" \n")); 
//      
//      Serial.print("Compressed data: ");
//      for(int i = 0; i < polled; i++){
//        Serial.print(comp[i]);
//        Serial.print(", ");
//      }Serial.println();
//    }

    //Serial.println("Compressed data: ");
    Serial.print(comp[0]);
    Serial.print("A");
    Serial.print("\n");
    for(int i = 1; i < polled; i++){
        if(i % 11 == 0){
          Serial.print(comp[i]);
          Serial.print("\n");
          delay(3000);
        }else{
          Serial.print(comp[i]);
          Serial.print("\n");
        }
    }
    delay(3000);
    
    //Lenght data original
    Serial.print("a");
    Serial.print(input_size);
    Serial.print("A");
    Serial.print("\n");
    //Config
    Serial.print("b");
    Serial.print(cfg->window_sz2);
    Serial.print("A");
    Serial.print("\n");
    Serial.print("c");
    Serial.print(cfg->lookahead_sz2);
    Serial.print("A");
    Serial.print("\n");
    Serial.print("d");
    Serial.print(cfg->decoder_input_buffer_size);
    Serial.print("A");
    Serial.print("\n");
    //Polled
    Serial.print("f");
    Serial.print(polled);
    Serial.print("A");
    Serial.print("\n");
    
    free(comp);
    heatshrink_encoder_free(hse);
}
int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    delay(1000);
    uint32_t length_data;
    
    uint8_t data [] = {152, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 130, 192, 0};
    length_data = sizeof(data)/sizeof(data[0]);
    
    Serial.print(data[0]);
    Serial.print("A");
    Serial.print("\n");
    for(int i = 1; i < length_data; i++){
        if(i % 11 == 0){
          Serial.print(data[i]);
          Serial.print("A");
          Serial.print("\n");
          delay(5000);
        }else{
          Serial.print(data[i]);
          Serial.print("A");
          Serial.print("\n");
        }
    }
    delay(5000);

    //Length data original
    Serial.print("a");
    Serial.print("584");
    Serial.print("A");
    Serial.print("\n");
    //Config
    Serial.print("b");
    Serial.print("4");
    Serial.print("A");
    Serial.print("\n");
    Serial.print("c");
    Serial.print("3");
    Serial.print("A");
    Serial.print("\n");
    Serial.print("d");
    Serial.print("64");
    Serial.print("A");
    Serial.print("\n");
    //Polled
    Serial.print("f");
    Serial.print("76");
    Serial.print("A");
    Serial.print("\n");
    
    for ( ;; )
    {
      delay(3000);
    }   

}
