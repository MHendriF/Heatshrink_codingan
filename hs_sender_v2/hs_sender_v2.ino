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

#define arduinoLED 9   // Arduino LED on board

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

static int compress_and_expand_and_check(uint8_t *input, 
                                         uint32_t input_size, 
                                         cfg_info *cfg, 
                                         uint8_t *output,
                                         uint32_t &output_size) 
                                         {
    heatshrink_encoder *hse = heatshrink_encoder_alloc(cfg->window_sz2,
        cfg->lookahead_sz2);
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
            pres = heatshrink_encoder_poll(hse,
                                           &output[polled],
                                           output_size - polled,
                                           &count);
            //ASSERT(pres >= 0);
            polled += count;
            if (cfg->log_lvl > 1){
              Serial.print(F("^^ polled "));
              Serial.print(count);
              Serial.print(F("\n"));
            }
        } while (pres == HSER_POLL_MORE);
        if (polled >= output_size) 
          Serial.print(F("FAIL: Compression should never expand that much!"));
        if (sunk == output_size) {
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
        Serial.print(output[i]);
        Serial.print(", ");
      }Serial.println();

//      Serial.print(output[0]);
//      Serial.print("\n");
//      for(int i = 1; i < polled; i++){
//        if(i % 13 == 0){
//          Serial.print(output[i]);
//          Serial.print("\n");
//          delay(3000);
//        }else{
//          Serial.print(output[i]);
//          Serial.print("\n");
//        }
//      }
    }
    return polled;
}


/******************************************************************************/
#define BUFFER_SIZE 400
uint8_t orig_buffer[BUFFER_SIZE];
uint8_t comp_buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    delay(1000);
    uint32_t length_data;
    int base = 16;

    //100
    uint8_t test_data[] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};
    //557
    //uint8_t test_data [] = "e8h5888e8h5888e8h5888yyxnyyxny454yyxnqx5e7yyxntu98xge9pdgzycb7had5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvvc828dctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne";
    //1000
    //uint8_t test_data[] = "XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKz";
    //1280
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333";
    //1345
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxnt4449pdhad5";
    //1647
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxdvd3u2sv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcyy9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcq3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxnt4449pdhad5yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcyyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bn9b5t3oc";

    length_data = sizeof(test_data)/sizeof(test_data[0]);

    if(length_data > BUFFER_SIZE){
      Serial.print("Data terlalu besar, data yang muat untuk kompresi maksimal 1345 karakter");
      delay(4000);
      return 0;
    }

    uint32_t comp_size   = BUFFER_SIZE; //this will get updated by reference
    memcpy(orig_buffer, test_data, length_data);
    size_t polled = 0;
    
    cfg_info cfg;
    cfg.log_lvl = 1;
    cfg.window_sz2 = 4;
    cfg.lookahead_sz2 = 3;
     
//    if(length_data <= 248){
//      cfg.window_sz2 = 8;
//      cfg.lookahead_sz2 = 4;
//    }else if(length_data > 248 && length_data <= 517){
//      cfg.window_sz2 = 6;
//      cfg.lookahead_sz2 = 3;
//    }else if(length_data > 517 && length_data <= 561){
//      cfg.window_sz2 = 5;
//      cfg.lookahead_sz2 = 3;
//    }else if(length_data > 561 && length_data <= 584){
//      cfg.window_sz2 = 4;
//      cfg.lookahead_sz2 = 3;
//    }
    cfg.decoder_input_buffer_size = 64;
    polled = compress_and_expand_and_check(orig_buffer, length_data, &cfg, comp_buffer, comp_size);
    Serial.print(polled);

    //Serial.println("Compressed data: ");
//    Serial.print(comp_buffer[0]);
//    Serial.print("\n");
//    for(int i = 1; i < polled; i++){
//        if(i % base == 0){
//          Serial.print(comp_buffer[i]);
//          Serial.print("\n");
//          delay(3000);
//        }else{
//          Serial.print(comp_buffer[i]);
//          Serial.print("\n");
//        }
//    }
//    delay(3000);
    
    //lenght data original
    Serial.print("a");
    Serial.print(length_data);
    Serial.print("\n");

    //config
    Serial.print("b");
    Serial.print(cfg.window_sz2);
    Serial.print("\n");
    Serial.print("c");
    Serial.print(cfg.lookahead_sz2);
    Serial.print("\n");
    Serial.print("d");
    Serial.print(cfg.decoder_input_buffer_size);
    Serial.print("\n");

    //Polled
    Serial.print("f");
    Serial.print(polled);
    Serial.print("\n");
   
    
    for ( ;; ){
      //Serial.print("a100");
      //Serial.print("\n");
      delay(3000);
    }
        
}
