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

     if (cfg->log_lvl > 0){
        Serial.print(F("in: "));
        Serial.print(compressed_size);
        Serial.print(F(" decompressed: "));
        Serial.print(polled);
        Serial.print(F(" \n")); 

        Serial.print("Decompressed data: ");
        for(int i = 0; i < polled; i++){
          Serial.print(output[i]);
          Serial.print(", ");
        }Serial.println();
    }
}



/******************************************************************************/
#define BUFFER_SIZE 1400
uint8_t decomp_buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);

    uint32_t decomp_size = BUFFER_SIZE; //this will get updated by reference
    uint8_t orig_char[BUFFER_SIZE];
    size_t polled = 0;
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////Test Decompression////////////////////////////////////////////
    
//    uint8_t orig_angka [] = {158, 75, 236, 150, 155, 180, 248, 11, 128, 138, 65, 105, 178, 79, 107, 146, 43, 29, 190, 217, 108, 176, 220, 46, 118, 90, 252, 200, 22, 100, 22, 48, 81, 185, 220, 231, 183, 139, 117, 230, 105, 53, 154, 94, 111, 32, 115, 113, 188, 77, 108, 179, 112, 67, 186, 93, 103, 51, 139, 197, 158, 203, 57, 184, 89, 45, 22, 27, 36, 214, 227, 51, 187, 89, 44, 118, 107, 61, 162, 102, 0, 147, 139, 85, 212, 28, 108, 147, 107, 117, 180, 20, 108, 96, 3, 110, 187, 217, 46, 83, 121, 205, 138, 199, 118, 177, 221, 44, 129, 99, 51, 186, 220, 238, 211, 155, 141, 170, 215, 122, 154, 218, 230, 151, 89, 181, 218, 232, 61, 55, 11, 92, 218, 227, 116, 187, 222, 45, 87, 123, 189, 134, 197, 98, 179, 91, 133, 38, 107, 116, 38, 155, 173, 158, 103, 120, 177, 218, 174, 23, 1, 41, 156, 88, 239, 22, 208, 17, 154, 89, 0, 228, 24, 110, 182, 217, 180, 210, 219, 52, 181, 140, 205, 214, 229, 102, 155, 93, 47, 87, 27, 204, 206, 239, 118, 185, 219, 175, 86, 41, 189, 133, 44, 110, 23, 137, 149, 134, 237, 114, 178, 221, 108, 54, 105, 169, 40, 221, 46, 200, 67, 50, 187, 93, 173, 9, 67, 97, 67, 25, 189, 234, 109, 50, 188, 220, 80, 10, 213, 55, 156, 93, 108, 179, 105, 181, 174, 227, 56, 184, 140, 205, 134, 219, 103, 116, 247, 41, 4, 123, 77, 185, 230, 103, 211, 203, 37, 166, 236, 248, 192, 211, 112, 176, 219, 172, 182, 201, 109, 138, 223, 100, 188, 200, 1, 70, 91, 49, 154, 135, 213, 50, 222, 102, 54, 209, 81, 23, 148, 113, 144, 89, 7, 36, 26, 100, 23, 59, 77, 210, 64, 79, 54, 91, 164, 178, 65, 101, 185, 130, 85, 154, 205, 101, 185, 1, 141, 190, 198, 165, 34, 130, 1, 32, 146, 8, 33, 130, 92, 54, 73, 1, 36, 133, 13, 202, 195, 116, 180, 219, 198, 196, 166, 108, 182, 225, 162, 10, 155, 112, 8, 220, 129, 6, 93, 38, 183, 88, 174, 119, 9, 220, 242, 94, 122, 71, 242, 5, 168, 87, 105, 178, 32, 146, 119, 215, 230, 64, 178, 151, 233, 124, 26, 236, 30, 193, 236, 30, 41, 236, 30, 193, 236, 30, 193, 236, 30, 193, 236, 30, 193, 236, 30, 73, 33, 238, 30, 225, 237, 51, 153, 77, 38, 82, 5, 247, 95, 117, 247, 95, 117, 247, 158, 222, 45, 215, 153, 164, 214, 105, 121, 188, 129, 205, 198, 241, 53, 178, 205, 193, 14, 233, 117, 156, 206, 47, 22, 123, 44, 230, 225, 100, 180, 88, 108, 147, 91, 140, 206, 237, 100, 177, 217, 172, 246, 137, 152, 2, 78, 45, 87, 91, 197, 142, 221, 57, 9, 27, 36, 218, 221, 109, 7, 27, 24, 0, 219, 174, 246, 75, 148, 222, 115, 98, 177, 221, 172, 119, 75, 32, 88, 204, 238, 183, 59, 180, 230, 227, 106, 181, 222, 166, 182, 185, 165, 214, 109, 118, 186, 16, 77, 174, 109, 113, 186, 93, 239, 22, 171, 189, 222, 195, 98, 177, 89, 135, 38, 197, 53, 186, 20, 13, 213, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 61, 211, 221, 60, 100, 4, 62, 67, 192, 238, 82, 229, 46, 82, 229, 46, 82, 229, 46, 82, 239, 30, 241, 239, 30, 241, 239, 30, 241, 239, 30, 241, 239, 30, 241, 239, 26, 149, 233, 94, 149, 233, 94, 149, 233, 94, 149, 233, 94, 149, 233, 94, 149, 233, 94, 149, 233, 94, 149, 233, 94, 149, 233, 94, 149, 164, 4, 152, 233, 142, 152, 233, 142, 152, 233, 143, 0};
//
//    uint8_t orig_angka [] = {158, 75, 236, 150, 155, 180, 248, 10, 192, 66, 144, 90, 108, 147, 218, 228, 138, 199, 111, 182, 91, 44, 55, 11, 157, 150, 191, 50, 5, 140, 130, 198, 10, 27, 157, 206, 123, 120, 183, 94, 102, 147, 89, 165, 230, 242, 7, 27, 141, 226, 107, 101, 155, 130, 14, 233, 117, 156, 206, 47, 22, 123, 44, 230, 225, 100, 180, 88, 108, 147, 91, 140, 206, 237, 100, 177, 217, 172, 246, 137, 152, 1, 39, 22, 171, 168, 56, 108, 147, 107, 117, 180, 20, 54, 48, 0, 219, 174, 246, 75, 148, 222, 115, 98, 177, 221, 172, 119, 75, 32, 88, 102, 119, 91, 157, 218, 115, 113, 181, 90, 239, 83, 91, 92, 210, 235, 54, 187, 93, 7, 163, 112, 181, 205, 174, 55, 75, 189, 226, 213, 119, 187, 216, 108, 86, 43, 53, 184, 82, 51, 91, 161, 52, 110, 182, 121, 157, 226, 199, 106, 184, 92, 4, 163, 56, 177, 222, 45, 160, 33, 154, 89, 0, 226, 12, 27, 173, 182, 109, 52, 182, 205, 45, 99, 49, 186, 220, 172, 211, 107, 165, 234, 227, 121, 153, 221, 238, 215, 59, 117, 234, 197, 55, 176, 165, 134, 225, 120, 153, 88, 110, 215, 43, 45, 214, 195, 102, 154, 146, 134, 233, 118, 66, 12, 202, 237, 118, 180, 37, 6, 194, 134, 25, 189, 234, 109, 50, 188, 220, 80, 5, 106, 155, 206, 46, 182, 89, 180, 218, 215, 113, 156, 92, 70, 99, 97, 182, 217, 221, 46, 229, 16, 67, 218, 109, 207, 49, 159, 79, 44, 150, 155, 179, 225, 129, 163, 112, 176, 219, 172, 182, 201, 109, 138, 223, 100, 188, 200, 1, 67, 45, 152, 205, 67, 229, 76, 183, 153, 134, 218, 42, 17, 120, 163, 134, 65, 100, 28, 136, 52, 100, 23, 59, 77, 210, 64, 79, 27, 45, 210, 89, 32, 178, 220, 193, 37, 102, 179, 89, 110, 64, 97, 183, 216, 212, 162, 40, 16, 8, 130, 68, 16, 33, 129, 46, 13, 146, 64, 73, 16, 160, 220, 172, 55, 75, 77, 188, 108, 37, 49, 178, 219, 134, 132, 21, 27, 112, 8, 110, 64, 129, 151, 73, 173, 214, 43, 157, 194, 119, 60, 151, 158, 136, 254, 32, 85, 66, 157, 166, 200, 130, 36, 235, 215, 230, 64, 177, 75, 243, 7, 216, 62, 193, 246, 15, 176, 125, 131, 236, 31, 36, 136, 123, 195, 205, 51, 153, 77, 38, 82, 5, 247, 175, 189, 125, 23, 139, 117, 230, 105, 53, 154, 94, 111, 32, 113, 184, 222, 38, 182, 89, 184, 32, 238, 151, 89, 204, 226, 241, 103, 178, 206, 110, 22, 75, 69, 134, 201, 53, 184, 204, 238, 214, 75, 29, 154, 207, 104, 153, 128, 18, 113, 106, 186, 222, 44, 118, 233, 200, 72, 108, 147, 107, 117, 180, 28, 54, 48, 0, 219, 174, 246, 75, 148, 222, 115, 98, 177, 221, 172, 119, 75, 32, 88, 102, 119, 91, 157, 218, 115, 113, 181, 90, 239, 83, 91, 92, 210, 235, 54, 187, 93, 8, 35, 107, 155, 92, 110, 151, 123, 197, 170, 239, 119, 176, 216, 172, 86, 97, 200, 216, 166, 183, 66, 128, 221, 83, 222, 158, 244, 247, 167, 189, 61, 233, 239, 79, 122, 123, 211, 222, 158, 244, 247, 167, 189, 61, 233, 239, 79, 12, 128, 135, 197, 47, 41, 121, 75, 202, 94, 241, 247, 143, 188, 125, 227, 239, 31, 120, 170, 87, 210, 190, 149, 244, 175, 165, 125, 43, 233, 95, 74, 250, 87, 210, 170, 64, 36, 199, 166, 61, 49, 240, 0};
//    
//    uint8_t orig_angka [] = {158, 75, 236, 150, 155, 180, 248, 10, 96, 32, 164, 22, 155, 36, 246, 185, 34, 177, 219, 237, 150, 203, 13, 194, 231, 101, 175, 204, 129, 97, 144, 88, 193, 65, 185, 220, 231, 183, 139, 117, 230, 105, 53, 154, 94, 111, 32, 112, 220, 111, 19, 91, 44, 220, 16, 59, 165, 214, 115, 56, 188, 89, 236, 179, 155, 133, 146, 209, 97, 178, 77, 110, 51, 59, 181, 146, 199, 102, 179, 218, 38, 96, 2, 78, 45, 87, 80, 112, 108, 147, 107, 117, 180, 20, 27, 24, 0, 54, 235, 189, 146, 229, 55, 156, 216, 172, 119, 107, 29, 210, 200, 22, 12, 206, 235, 115, 187, 78, 110, 54, 171, 93, 234, 107, 107, 154, 93, 102, 215, 107, 160, 244, 55, 11, 92, 218, 227, 116, 187, 222, 45, 87, 123, 189, 134, 197, 98, 179, 91, 133, 33, 154, 221, 9, 161, 186, 217, 230, 119, 139, 29, 170, 225, 112, 18, 134, 113, 99, 188, 91, 64, 65, 154, 89, 0, 225, 6, 6, 235, 109, 155, 77, 45, 179, 75, 88, 204, 55, 91, 149, 154, 109, 116, 189, 92, 111, 51, 59, 189, 218, 231, 110, 189, 88, 166, 246, 20, 176, 110, 23, 137, 149, 134, 237, 114, 178, 221, 108, 54, 105, 169, 40, 55, 75, 178, 16, 51, 43, 181, 218, 208, 148, 13, 133, 12, 25, 189, 234, 109, 50, 188, 220, 80, 2, 181, 77, 231, 23, 91, 44, 218, 109, 107, 184, 206, 46, 35, 48, 216, 109, 182, 119, 73, 220, 161, 4, 30, 211, 110, 121, 134, 125, 60, 178, 90, 110, 207, 131, 3, 67, 112, 176, 219, 172, 182, 201, 109, 138, 223, 100, 188, 200, 1, 65, 150, 204, 102, 161, 241, 83, 45, 230, 96, 219, 69, 65, 23, 133, 28, 25, 5, 144, 114, 16, 104, 100, 23, 59, 77, 210, 64, 79, 13, 150, 233, 44, 144, 89, 110, 96, 145, 89, 172, 214, 91, 144, 24, 54, 251, 26, 148, 34, 128, 128, 66, 9, 8, 32, 33, 128, 151, 3, 100, 144, 18, 66, 20, 13, 202, 195, 116, 180, 219, 198, 193, 41, 134, 203, 110, 26, 8, 42, 27, 112, 8, 55, 32, 64, 101, 210, 107, 117, 138, 231, 112, 157, 207, 37, 231, 161, 31, 194, 5, 42, 20, 118, 155, 34, 8, 73, 211, 215, 230, 64, 176, 165, 220, 193, 251, 7, 236, 31, 176, 98, 30, 180, 206, 101, 52, 153, 72, 23, 223, 95, 34, 241, 110, 188, 205, 38, 179, 75, 205, 228, 14, 27, 141, 226, 107, 101, 155, 130, 7, 116, 186, 206, 103, 23, 139, 61, 150, 115, 112, 178, 90, 44, 54, 73, 173, 198, 103, 118, 178, 88, 236, 214, 123, 68, 204, 0, 73, 197, 170, 235, 120, 177, 219, 167, 33, 32, 217, 38, 214, 235, 104, 56, 54, 48, 0, 109, 215, 123, 37, 202, 111, 57, 177, 88, 238, 214, 59, 165, 144, 44, 25, 157, 214, 231, 118, 156, 220, 109, 86, 187, 212, 214, 215, 52, 186, 205, 174, 215, 66, 8, 109, 115, 107, 141, 210, 239, 120, 181, 93, 238, 246, 27, 21, 138, 204, 57, 13, 138, 107, 116, 40, 6, 234, 158, 250, 123, 233, 239, 167, 190, 158, 250, 123, 233, 239, 167, 131, 32, 33, 184, 165, 242, 151, 222, 63, 120, 253, 227, 84, 175, 210, 191, 74, 253, 43, 244, 173, 72, 2, 76, 125, 48, 248, 0};
//
//   uint8_t orig_angka [] = {158, 75, 236, 150, 155, 180, 248, 10, 48, 16, 41, 5, 166, 201, 61, 174, 72, 172, 118, 251, 101, 178, 195, 112, 185, 217, 107, 243, 32, 88, 50, 11, 24, 40, 27, 157, 206, 123, 120, 183, 94, 102, 147, 89, 165, 230, 242, 7, 6, 227, 120, 154, 217, 102, 224, 128, 238, 151, 89, 204, 226, 241, 103, 178, 206, 110, 22, 75, 69, 134, 201, 53, 184, 204, 238, 214, 75, 29, 154, 207, 104, 153, 128, 4, 156, 90, 174, 160, 224, 108, 147, 107, 117, 180, 20, 13, 140, 0, 13, 186, 239, 100, 185, 77, 231, 54, 43, 29, 218, 199, 116, 178, 5, 129, 153, 221, 110, 119, 105, 205, 198, 213, 107, 189, 77, 109, 115, 75, 172, 218, 237, 116, 30, 131, 112, 181, 205, 174, 55, 75, 189, 226, 213, 119, 187, 216, 108, 86, 43, 53, 184, 82, 12, 214, 232, 77, 6, 235, 103, 153, 222, 44, 118, 171, 133, 192, 74, 12, 226, 199, 120, 182, 128, 129, 154, 89, 0, 224, 131, 1, 186, 219, 102, 211, 75, 108, 210, 214, 51, 6, 235, 114, 179, 77, 174, 151, 171, 141, 230, 103, 119, 187, 92, 237, 215, 171, 20, 222, 194, 150, 6, 225, 120, 153, 88, 110, 215, 43, 45, 214, 195, 102, 154, 146, 129, 186, 93, 144, 128, 204, 174, 215, 107, 66, 80, 27, 10, 24, 25, 189, 234, 109, 50, 188, 220, 80, 1, 90, 166, 243, 139, 173, 150, 109, 54, 181, 220, 103, 23, 17, 152, 54, 27, 109, 157, 210, 59, 148, 16, 64, 246, 155, 115, 204, 25, 244, 242, 201, 105, 187, 62, 6, 6, 131, 112, 176, 219, 172, 182, 201, 109, 138, 223, 100, 188, 200, 1, 64, 203, 102, 51, 80, 248, 84, 203, 121, 152, 27, 104, 168, 17, 120, 40, 224, 100, 22, 65, 200, 32, 208, 100, 23, 59, 77, 210, 64, 79, 6, 203, 116, 150, 72, 44, 183, 48, 72, 86, 107, 53, 150, 228, 6, 6, 223, 99, 82, 130, 40, 4, 2, 8, 36, 16, 64, 33, 128, 75, 128, 217, 36, 4, 144, 66, 128, 220, 172, 55, 75, 77, 188, 108, 9, 76, 27, 45, 184, 104, 16, 84, 27, 112, 8, 27, 144, 32, 25, 116, 154, 221, 98, 185, 220, 39, 115, 201, 121, 232, 35, 248, 32, 81, 80, 161, 218, 108, 136, 32, 147, 163, 215, 230, 64, 176, 82, 231, 48, 127, 96, 226, 30, 90, 103, 50, 154, 76, 164, 11, 234, 47, 22, 235, 204, 210, 107, 52, 188, 222, 64, 224, 220, 111, 19, 91, 44, 220, 16, 29, 210, 235, 57, 156, 94, 44, 246, 89, 205, 194, 201, 104, 176, 217, 38, 183, 25, 157, 218, 201, 99, 179, 89, 237, 19, 48, 0, 147, 139, 85, 214, 241, 99, 183, 78, 66, 64, 217, 38, 214, 235, 104, 56, 27, 24, 0, 27, 117, 222, 201, 114, 155, 206, 108, 86, 59, 181, 142, 233, 100, 11, 3, 51, 186, 220, 238, 211, 155, 141, 170, 215, 122, 154, 218, 230, 151, 89, 181, 218, 232, 65, 6, 215, 54, 184, 221, 46, 247, 139, 85, 222, 239, 97, 177, 88, 172, 195, 144, 108, 83, 91, 161, 64, 27, 170, 123, 244, 247, 233, 239, 211, 208, 200, 8, 103, 20, 191, 120, 254, 241, 85, 43, 250, 87, 244, 170, 164, 0, 147, 23, 192, 0};
//
    uint8_t orig_angka [] = {158, 75, 236, 150, 155, 180, 248, 10, 24, 8, 10, 65, 105, 178, 79, 107, 146, 43, 29, 190, 217, 108, 176, 220, 46, 118, 90, 252, 202, 185, 34, 144, 88, 237, 150, 27, 157, 206, 123, 120, 183, 94, 102, 147, 89, 165, 230, 243, 120, 183, 92, 111, 19, 91, 44, 220, 16, 14, 233, 117, 156, 206, 47, 22, 123, 44, 230, 225, 100, 180, 88, 108, 147, 91, 140, 206, 237, 100, 177, 217, 172, 246, 137, 152, 0, 36, 226, 213, 117, 187, 89, 44, 147, 107, 117, 182, 103, 51, 177, 216, 236, 118, 235, 189, 146, 229, 55, 156, 216, 172, 119, 107, 29, 210, 201, 118, 178, 76, 238, 183, 59, 180, 230, 227, 106, 181, 222, 166, 182, 185, 165, 214, 109, 118, 186, 90, 44, 55, 11, 92, 218, 227, 116, 187, 222, 45, 87, 123, 189, 134, 197, 98, 179, 91, 167, 54, 41, 173, 210, 103, 118, 186, 217, 230, 119, 139, 29, 170, 225, 112, 154, 218, 231, 22, 59, 197, 182, 199, 120, 154, 89, 39, 22, 59, 132, 214, 235, 109, 155, 77, 45, 179, 75, 93, 162, 195, 117, 185, 89, 166, 215, 75, 213, 198, 243, 51, 187, 221, 174, 118, 235, 213, 138, 111, 97, 188, 77, 110, 23, 137, 149, 134, 237, 114, 178, 221, 108, 54, 105, 173, 170, 239, 116, 187, 76, 231, 19, 43, 181, 218, 209, 100, 177, 216, 102, 214, 233, 189, 234, 109, 50, 188, 220, 80, 0, 173, 83, 121, 197, 214, 203, 54, 155, 90, 238, 51, 139, 141, 234, 197, 97, 182, 217, 221, 33, 228, 0, 129, 22, 155, 117, 114, 69, 62, 158, 89, 45, 55, 103, 192, 106, 228, 138, 225, 97, 183, 89, 109, 146, 219, 21, 190, 201, 121, 144, 92, 44, 50, 217, 140, 212, 62, 10, 153, 111, 185, 89, 109, 178, 11, 77, 194, 231, 117, 182, 200, 44, 150, 251, 101, 190, 229, 32, 185, 218, 110, 146, 11, 13, 182, 203, 116, 150, 72, 44, 183, 48, 72, 43, 53, 154, 203, 114, 186, 72, 45, 246, 59, 173, 202, 229, 101, 185, 89, 110, 146, 11, 29, 214, 89, 32, 185, 217, 108, 146, 11, 117, 150, 65, 111, 185, 88, 110, 150, 155, 124, 130, 201, 101, 182, 89, 109, 195, 64, 87, 59, 45, 186, 231, 101, 185, 90, 110, 146, 233, 53, 186, 197, 115, 184, 78, 231, 146, 243, 208, 41, 240, 20, 50, 20, 29, 166, 200, 130, 4, 157, 15, 95, 153, 87, 36, 73, 113, 204, 30, 33, 226, 211, 57, 148, 210, 101, 32, 95, 40, 188, 91, 175, 51, 73, 172, 210, 243, 121, 188, 91, 174, 55, 137, 173, 150, 110, 8, 7, 116, 186, 206, 103, 23, 139, 61, 150, 115, 112, 178, 90, 44, 54, 73, 173, 198, 103, 118, 178, 88, 236, 214, 123, 68, 204, 0, 18, 113, 106, 186, 222, 44, 118, 233, 205, 218, 201, 100, 155, 91, 173, 179, 57, 157, 142, 199, 99, 183, 93, 236, 151, 41, 188, 230, 197, 99, 187, 88, 238, 150, 75, 181, 146, 103, 117, 185, 221, 167, 55, 27, 85, 174, 245, 53, 181, 205, 46, 179, 107, 181, 210, 209, 97, 181, 205, 174, 55, 75, 189, 226, 213, 119, 187, 216, 108, 86, 43, 53, 186, 115, 98, 154, 221, 38, 119, 107, 170, 123, 250, 123, 12, 128, 134, 57, 227, 253, 226, 85, 43, 253, 42, 84, 128, 9, 48, 190, 0};

    
    int comp_size = sizeof(orig_angka)/sizeof(orig_angka[0]);
    Serial.print("length_angka : ");
    Serial.println(comp_size);
    
    int length_data_ori = 1345;
    polled = comp_size;
    cfg_info cfg;
    cfg.log_lvl = 0;
    cfg.window_sz2 = 8;
    cfg.lookahead_sz2 = 7;
    cfg.decoder_input_buffer_size = 64;
    uint32_t t2 = micros();
    decompress_and_expand_and_check(orig_angka, length_data_ori, &cfg, decomp_buffer, decomp_size, polled);
    uint32_t t3 = micros();
    Serial.print("Time to decompress: ");
    Serial.println((t3-t2)/1e6,6);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    
    for ( ;; )
    {
      delay(3000);
    }   
}



