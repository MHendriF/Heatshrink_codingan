#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#include <heatshrink_encoder.h>
#include <heatshrink_decoder.h>
#include <greatest.h>

#define arduinoLED 13   // Arduino LED on board

/******************************************************************************/
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
int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);

    uint8_t nodeA[1600], nodeB[600];
    size_t polled_A = 0, polled_B = 0;
    String stringOne;
    char incomingByte;
    char buf[20];

    char *orig_sz_A, *window_sz_A, *lookahead_sz_A, *decoder_sz_A, *polled_sz_A, *input_sz_A;
    int idx_A=0, orig_A=0, window_A=0, lookahead_A=0, decoder_A=0, input_size_A;

    char *orig_sz_B, *window_sz_B, *lookahead_sz_B, *decoder_sz_B, *polled_sz_B, *input_sz_B;
    int idx_B=0, orig_B=0, window_B=0, lookahead_B=0, decoder_B=0, input_size_B;
     
    char *node_A, *node_B;
    int i=0, j=0;
    
    int num_A[1600], num_B[600];
    memset(num_A,0,1600);
    memset(num_B,0,600);
    
    Serial.print(F("Incoming data : \n"));
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
              node_A = strchr (buf, 'A');
              node_B = strchr (buf, 'B');
  
              if (node_A != NULL){
                  removeChar(buf, 'A');
                  stringOne = buf;

                  orig_sz_A = strchr (buf, 'a');
                  window_sz_A = strchr (buf, 'b');
                  lookahead_sz_A = strchr (buf, 'c');
                  decoder_sz_A = strchr (buf, 'd');
                  polled_sz_A = strchr (buf, 'f');

                  if (orig_sz_A != NULL){
                  removeChar(buf, 'a');
                  stringOne = buf;
                  orig_A = stringOne.toInt();
                  }
                  else if (window_sz_A != NULL){
                      removeChar(buf, 'b');
                      stringOne = buf;
                      window_A = stringOne.toInt();
                  }
                  else if (lookahead_sz_A != NULL){
                      removeChar(buf, 'c');
                      stringOne = buf;
                      lookahead_A = stringOne.toInt();
                  }
                  else if (decoder_sz_A != NULL){
                      removeChar(buf, 'd');
                      stringOne = buf;
                      decoder_A = stringOne.toInt();
                  }
                  else if (polled_sz_A != NULL){
                      removeChar(buf, 'f');
                      stringOne = buf;
                      polled_A = stringOne.toInt();
                  }else{
                      Serial.print(" ");
                      num_A[idx_A] = stringOne.toInt();
                      idx_A++;
                  }
              }
              else if (node_B != NULL){
                  removeChar(buf, 'B');
                  stringOne = buf;

                  orig_sz_B = strchr (buf, 'a');
                  window_sz_B = strchr (buf, 'b');
                  lookahead_sz_B = strchr (buf, 'c');
                  decoder_sz_B = strchr (buf, 'd');
                  polled_sz_B = strchr (buf, 'f');

                  if (orig_sz_B != NULL){
                  removeChar(buf, 'a');
                  stringOne = buf;
                  orig_B = stringOne.toInt();
                  }
                  else if (window_sz_B != NULL){
                      removeChar(buf, 'b');
                      stringOne = buf;
                      window_B = stringOne.toInt();
                  }
                  else if (lookahead_sz_B != NULL){
                      removeChar(buf, 'c');
                      stringOne = buf;
                      lookahead_B = stringOne.toInt();
                  }
                  else if (decoder_sz_B != NULL){
                      removeChar(buf, 'd');
                      stringOne = buf;
                      decoder_B = stringOne.toInt();
                  }
                  else if (polled_sz_B != NULL){
                      removeChar(buf, 'f');
                      stringOne = buf;
                      polled_B = stringOne.toInt();
                  }else{
                      Serial.print(" ");
                      num_B[idx_B] = stringOne.toInt();
                      idx_B++;
                  }
              }
              stringOne = "";
              memset(buf,0,20);
          }
        }
        
        else if(Serial.available() <= 0)
        {
            Serial.print(F("| idxA : "));
            Serial.print(idx_A);
            Serial.print(F(" | idxB : "));
            Serial.println(idx_B);
            //Dekompresi node A
            if(idx_A == polled_A && polled_A != 0){
               if(num_A[polled_A] != 0 || num_A[polled_A-1] != 0){
                  Serial.print(F("Challenge Accepted!\n"));
                  for(j=0; j<polled_A; j++){
                     Serial.print(num_A[j]);
                     Serial.print(" ");
                  }
                  Serial.print("a:");
                  Serial.print(orig_A);
                  Serial.print("b:");
                  Serial.print(window_A);
                  Serial.print("c:");
                  Serial.print(lookahead_A);
                  Serial.print("d:");
                  Serial.print(decoder_A);
                  Serial.print("f:");
                  Serial.print(polled_A);
    
                  if(polled_A != 0 && window_A != 0 && lookahead_A != 0 && decoder_A !=0){
                     Serial.print(F("\n^^Dekompresi Node A Start!\n"));
                     //convert to 
                     for(i = 0; i < polled_A; i++){
                        nodeA[i] = (uint8_t) num_A[i];
                     }
                     cfg_info cfg;
                     cfg.log_lvl = 1;
                     cfg.window_sz2 = window_A;
                     cfg.lookahead_sz2 = lookahead_A;
                     cfg.decoder_input_buffer_size = decoder_A;
                     decompress_and_expand_and_check(nodeA, orig_A, &cfg,  polled_A);
      
                     Serial.print(F("\n^^ Selesai\n"));
                     //reinisialite
                     orig_A = 0, window_A = 0, lookahead_A = 0, decoder_A = 0, polled_A = 0;
                     idx_A = 0;
                  }
                }
                else if(polled_A != 0 && window_A != 0 && lookahead_A != 0 && decoder_A !=0){
                    Serial.print(F("\n^^Dekompresi Node A Start2!\n"));
                    for(i = 0; i < polled_A; i++){
                        nodeA[i] = (uint8_t) num_A[i];
                    }
                    cfg_info cfg;
                    cfg.log_lvl = 1;
                    cfg.window_sz2 = window_A;
                    cfg.lookahead_sz2 = lookahead_A;
                    cfg.decoder_input_buffer_size = decoder_A;
                    decompress_and_expand_and_check(nodeA, orig_A, &cfg,  polled_A);
                    delay(5000);
                    Serial.print(F("\n^^ Selesai\n"));
                    //reinisialite
                    orig_A = 0, window_A = 0, lookahead_A = 0, decoder_A = 0, polled_A = 0;
                    idx_A = 0;
                }
                else{
                  Serial.print(num_A[polled_A-1]);
                  Serial.print(" ");
                  Serial.print(num_A[polled_A]);
                  Serial.print(F("\n^^ Ampas A\n"));
                }
            }
            
            //Dekompresi node B
            if(idx_B == polled_B && polled_B != 0){
               if(num_B[polled_B] != 0 || num_B[polled_B-1] != 0){
                  Serial.print(F("Challenge Accepted!\n"));
                  for(j=0; j<polled_B; j++){
                     Serial.print(num_B[j]);
                     Serial.print(" ");
                  }
                  Serial.print("a:");
                  Serial.print(orig_B);
                  Serial.print("b:");
                  Serial.print(window_B);
                  Serial.print("c:");
                  Serial.print(lookahead_B);
                  Serial.print("d:");
                  Serial.print(decoder_B);
                  Serial.print("f:");
                  Serial.print(polled_B);
    
                  if(polled_B != 0 && window_B != 0 && lookahead_B != 0 && decoder_B !=0){
                     Serial.print(F("\n^^Dekompresi Node B Start!\n"));
                     //convert to 
                     for(i = 0; i < polled_B; i++){
                        nodeB[i] = (uint8_t) num_B[i];
                     }
                     cfg_info cfg2;
                     cfg2.log_lvl = 1;
                     cfg2.window_sz2 = window_B;
                     cfg2.lookahead_sz2 = lookahead_B;
                     cfg2.decoder_input_buffer_size = decoder_B;
                     decompress_and_expand_and_check(nodeB, orig_B, &cfg2,  polled_B);
      
                     Serial.print(F("\n^^ Selesai\n"));
                     //reinisialite
                     orig_B = 0, window_B = 0, lookahead_B = 0, decoder_B = 0, polled_B = 0;
                     idx_B = 0;
                  }
                }
                else if(polled_B != 0 && window_B != 0 && lookahead_B != 0 && decoder_B !=0){
                    Serial.print(F("\n^^Dekompresi Node B Start2!\n"));
                    for(i = 0; i < polled_B; i++){
                        nodeB[i] = (uint8_t) num_B[i];
                    }
                    cfg_info cfg;
                    cfg.log_lvl = 1;
                    cfg.window_sz2 = window_B;
                    cfg.lookahead_sz2 = lookahead_B;
                    cfg.decoder_input_buffer_size = decoder_B;
                    decompress_and_expand_and_check(nodeB, orig_B, &cfg,  polled_B);
    
                    Serial.print(F("\n^^ Selesai\n"));
                    //reinisialite
                    orig_B = 0, window_B = 0, lookahead_B = 0, decoder_B = 0, polled_B = 0;
                    idx_B = 0;
                }
                else{
                  Serial.print(num_B[polled_B-1]);
                  Serial.print(" ");
                  Serial.print(num_B[polled_B]);
                  Serial.print(F("\n^^ Ampas B\n"));
                }
            }
            delay(1000);
        }
    }
}

