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

//void removeChar(char * string, char letter);
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

static void decompress_and_expand_and_check(uint8_t *input, 
                                           uint32_t input_size, 
                                           cfg_info *cfg, 
                                           uint8_t *output,
                                           uint32_t &output_size,
                                           size_t polled2) {

    Serial.print(F("\n^^ Processing\n"));
    heatshrink_decoder *hsd = heatshrink_decoder_alloc(cfg->decoder_input_buffer_size,
        cfg->window_sz2, cfg->lookahead_sz2);

    size_t compressed_size = polled2;
    size_t count  = 0;
    size_t sunk = 0;
    size_t polled = 0;
    
    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ DECOMPRESSING\n"));
        dump_buf("comp", input, input_size);
    }
    while (sunk < compressed_size) {
        heatshrink_decoder_sink(hsd, &input[sunk], compressed_size - sunk, &count);
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
            pres = heatshrink_decoder_poll(hsd, &output[polled],
                output_size - polled, &count);
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
    if (cfg->log_lvl > 0){
        Serial.print(F("in: "));
        Serial.print(compressed_size);
        Serial.print(F(" decompressed: "));
        Serial.print(polled);
        Serial.print(F(" \n")); 
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

    if (cfg->log_lvl > 1) dump_buf("decomp", output, output_size);
    //compare hasil decompressi dengan data raw
//    for (uint32_t i=0; i<input_size; i++) {
//        if (input[i] != output[i]) {
//            Serial.print(F("*** mismatch at: "));
//            Serial.print(i);
//            Serial.print(F(" \n"));
//        }
//    }

    Serial.print("Decompressed data: ");
    for(int i = 0; i < polled; i++){
      Serial.print(output[i]);
      Serial.print(", ");
    }Serial.println();

    delay(2000);
}



/******************************************************************************/
#define BUFFER_SIZE 1500
uint8_t orig_buffer[BUFFER_SIZE];
uint8_t decomp_buffer[BUFFER_SIZE];

//QuickStats stats; //initialize an instance of this class

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);

    //uint32_t comp_size   = BUFFER_SIZE; //this will get updated by reference
    uint32_t decomp_size = BUFFER_SIZE; //this will get updated by reference
    uint8_t orig_char[1500];
    size_t polled = 0;
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //Test Decompression
//    int orig_angka[] = {
//      152, 128, 60, 1, 224, 15, 0, 120, 3, 192, 30, 0, 32
//    };

    int orig_angka [] = {158,75,236,150,155,180,248,10,96,32,164,22,155,36,246,69,99,183,219,45,150,27,133,206,203,95,153,72,164,22,48,72,110,119,57,237,226,221,121,154,77,102,151,155,200,28,55,27,196,214,203,55,4,14,233,117,156,206,47,22,123,44,230,225,100,180,88,108,147,91,140,206,237,100,177,217,172,246,137,152,0,147,139,85,212,28,27,36,218,221,109,5,6,198,0,13,186,239,100,185,77,231,54,43,29,218,199,116,178,5,131,51,186,220,238,211,155,141,170,215,122,154,218,230,151,89,181,218,232,61,13,194,215,54,184,221,46,247,139,85,222,239,97,177,88,172,214,225,72,102,183,66,104,110,182,121,157,226,199,106,184,92,4,161,156,88,239,22,208,16,102,150,64,56,65,129,186,219,102,211,75,108,210,214,51,13,214,229,102,155,93,47,87,27,204,206,239,118,185,219,175,86,41,189,133,44,27,133,226,101,97,187,92,172,183,91,13,154,106,74,13,210,236,132,12,202,237,118,180,37,3,97,67,6,111,122,155,76,175,55,20,0,173,83,121,197,214,203,54,155,90,238,51,139,136,204,54,27,109,157,208,119,40,65,7,180,219,164,83,233,229,146,211,118,123,154,69,112,176,219,172,182,201,109,138,223,100,188,200,1,65,150,204,102,161,208,211,45,230,48,219,68,225,22,5,22,25,5,144,108,16,104,100,23,59,77,210,64,76,13,150,233,44,144,89,110,96,145,89,172,214,91,144,24,54,251,26,136,34,128,128,66,9,8,32,33,128,150,67,100,144,18,2,20,13,202,195,116,180,219,198,193,41,6,203,110,26,8,42,27,112,8,55,32,64,101,210,107,117,138,231,112,157,207,37,231,129,31,66,5,42,12,118,155,34,0,41,147,215,230,82,36,155,87,127,93,253,119,245,219,131,214,153,204,166,147,41,2,227,233,3,94,45,215,153,164,214,105,121,188,129,195,113,188,77,108,179,112,64,238,151,89,204,226,241,103,178,206,110,22,75,69,134,201,53,184,204,238,214,75,29,154,207,104,153,128,9,56,181,93,111,22,59,116,228,36,27,36,218,221,109,7,6,198,0,13,186,239,100,185,77,231,54,43,29,218,199,116,178,5,131,51,186,220,238,211,155,141,170,215,122,154,218,230,151,89,181,218,232,65,13,174,109,113,186,93,239,22,171,189,222,195,98,177,89,135,33,177,77,110,133,0,221,83,159,78,125,57,244,231,211,159,78,125,57,217,1,5,197,15,148,62,237,251,183,238,217,164,254,147,250,79,233,63,164,230,64,18,91,233,109,77,0,1,73,154,0};
    //int orig_angka [] = {178,206,45,19,89,192,0,32,102,222,111,55,139,112,12,147,73,172,208,14,59,141,226,107,101,155,130,7,116,186,206,103,23,139,61,150,115,112,178,89,239,87,155,29,138,111,104,176,217,38,183,25,157,218,201,99,179,89,237,19,48,1,39,22,171,173,226,199,110,156,132,131,100,155,91,173,160,224,216,192,1,183,93,236,151,41,188,230,197,99,187,93,172,115,137,148,224,84,27,165,144,56,25,157,214,231,118,156,220,109,86,187,212,214,215,52,186,205,174,215,66,56,109,115,107,141,210,239,120,181,93,238,246,27,21,138,204,63,13,138,107,116,43,6,235,103,153,146,195,106,184,92,4,129,156,88,239,22,208,16,102,150,64,56,65,129,186,219,102,211,75,108,210,214,50,13,214,229,102,155,93,47,87,27,204,206,239,118,185,219,175,72,192,216,84,161,184,94,38,86,27,181,202,203,117,176,217,166,164,160,221,46,200,208,204,142,161,180,28,131,97,69,134,111,122,155,76,175,55,20,40,173,83,121,197,214,203,54,155,90,238,51,139,136,204,54,27,109,158,199,111,182,91,44,55,11,157,150,64,8,69,166,221,34,159,79,44,150,155,176,68,32,240,220,238,115,217,21,194,195,110,178,219,37,182,43,125,146,243,32,5,6,91,49,154,135,67,76,183,152,195,109,19,132,88,20,88,100,22,65,176,65,161,144,92,237,55,73,1,48,54,91,164,178,65,101,185,130,69,102,179,89,110,64,96,219,236,106,32,138,2,1,8,36,32,128,134,2,89,13,146,64,72,8,80,55,43,13,210,211,111,27,4,164,27,45,184,104,32,168,109,192,32,220,129,1,151,73,173,214,43,157,194,119,60,151,158,4,125,8,20,168,49,218,108,136,0,166,79,95,153,72,146,109,93,253,119,245,223,215,110,128,0};
    //int orig_angka [] =  {158,75,236,150,155,180,248,10,96,32,164,22,155,36,246,69,99,183,219,45,150,27,133,206,203,95,153,72,164,22,48,72,110,119,57,237,226,221,121,154,77,102,151,155,200,28,55,27,196,214,203,55,4,14,233,117,156,206,47,22,123,44,230,225,100,180,88,108,147,91,140,206,237,100,177,217,172,246,137,152,0,147,139,85,214,241,99,183,78,66,65,178,77,173,214,208,112,108,96,0,219,174,246,75,148,222,115,98,177,221,172,119,75,32,88,51,59,173,206,237,57,184,218,173,119,169,173,174,105,117,155,93,174,132,16,220,45,115,107,141,210,239,120,181,93,238,246,27,21,138,204,58,13,138,107,116,40,134,235,103,153,145,131,106,184,92,4,161,156,88,239,22,208,16,102,150,64,56,65,129,186,219,102,211,75,108,210,214,51,13,214,229,102,155,93,47,87,27,204,206,239,118,185,219,175,86,41,189,133,52,27,133,226,101,97,187,92,172,183,91,13,154,106,74,13,210,236,136,12,202,237,118,180,38,3,97,67,6,111,122,155,76,175,55,20,0,173,83,121,197,214,203,54,155,90,238,51,139,136,204,54,27,109,157,216,119,72,65,7,180,219,164,83,233,229,146,211,118,125,154,69,112,176,219,172,182,201,109,138,223,100,188,200,1,65,150,204,102,161,208,211,45,230,48,219,68,225,22,5,22,25,5,144,108,16,104,100,23,59,77,210,64,76,13,150,233,44,144,89,110,96,145,89,172,214,91,144,24,54,251,26,136,34,128,128,66,9,8,32,33,128,150,67,100,144,18,2,20,13,202,195,116,180,219,198,193,41,6,203,110,26,8,42,27,112,8,55,32,64,101,210,107,117,138,231,112,157,207,37,231,129,31,66,5,42,12,118,155,34,0,41,147,215,230,82,36,155,87,127,93,253,119,245,219,131,214,153,204,166,147,41,2,227,233,3,94,45,215,153,164,214,105,121,188,129,195,113,188,77,108,179,112,64,238,151,89,204,226,241,103,178,206,110,22,75,69,134,201,53,184,204,238,214,75,29,154,207,104,153,128,9,56,181,93,111,22,59,116,228,36,27,36,218,221,109,7,6,198,0,13,186,239,100,185,77,231,54,43,29,218,199,116,178,5,131,51,186,220,238,211,155,141,170,215,122,154,218,230,151,89,181,218,232,65,13,174,109,113,186,93,239,22,171,189,222,195,98,177,89,135,33,177,77,110,133,0,221,83,159,78,125,57,244,231,211,159,78,125,57,217,1,5,197,15,148,62,237,251,183,238,217,164,254,147,250,79,233,63,164,230,64,20,0};

    Serial.print("origin1 : ");
    Serial.println(sizeof(orig_char));
    
    int comp_size = sizeof(orig_angka)/sizeof(orig_angka[0]);
    Serial.print("length_angka : ");
    Serial.println(comp_size);

    //Copy and convert to array of char
    for(int i = 0; i < comp_size; i++){
      orig_char[i] = (uint8_t) orig_angka[i];
    }
    
    //print
    for(int i = 0; i < comp_size; i++){
      Serial.print(orig_char[i]);
      Serial.print(" ");
    }
    
    int length_data = 1345;
    polled = comp_size;
    cfg_info cfg;
    cfg.log_lvl = 0;
    cfg.window_sz2 = 8;
    cfg.lookahead_sz2 = 5;
    cfg.decoder_input_buffer_size = 64;
    decompress_and_expand_and_check(orig_char, length_data, &cfg, decomp_buffer, decomp_size, polled);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    Serial.println("Incoming data :");
    for ( ;; )
    {
      delay(3000);
    }   
}



