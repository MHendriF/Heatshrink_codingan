// Demo Code for Heatshrink (Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>)
// embedded compression library
// Craig Versek, Apr. 2016

#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <QuickStats.h>

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
        Serial.print(comp[i]);
        Serial.print(", ");
      }Serial.println();
    }
    
    size_t compressed_size = polled;
    sunk = 0;
    polled = 0;
    
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
            pres = heatshrink_decoder_poll(hsd, &decomp[polled],
                decomp_sz - polled, &count);
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
        Serial.print(F("FAIL: Decompressed length does not match original input length!"));
    }

    if (cfg->log_lvl > 1) dump_buf("decomp", decomp, polled);
    for (uint32_t i=0; i<input_size; i++) {
        if (input[i] != decomp[i]) {
            Serial.print(F("*** mismatch at: "));
            Serial.print(i);
            Serial.print(F(" \n"));
        }
//        else{
//            Serial.print(input[i]);
//            Serial.print(F(" "));
//            Serial.print(decomp[i]);
//            Serial.print(F(" \n"));
//        }
    }

    //tambahan
    Serial.print("Decompress data: ");
    for(int i = 0; i < polled; i++){
      Serial.print(decomp[i]);
      Serial.print(", ");
    }Serial.println();
  
    free(comp);
    free(decomp);
    heatshrink_encoder_free(hse);
    heatshrink_decoder_free(hsd);
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
   
    //float test_data[] = {'1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0'};
    //uint8_t test_data[] = {'1.2', '1.2', '1.3', '1.4', '1.5', '1.1', '1.9', '1.2', '1.3', '1.4', '2.5', '2.3', '2.3', '2.2', '2.3', '2.4', '2.5', '2.6', '2.4', '2.2', '3.3', '3.4', '3.1', '3.1', '3.2', '3.3', '3.4', '3.5', '3.5', '3.6', '4.5', '4.2', '4.1', '4.4', '4.5', '4.6', '4.4', '4.4', '4.3', '4.3', '5.3', 5.3, 5.3, 5.3, 5.3, 5.3, 5.5, 5.8, 5.6, 5.5, 6.4, 6.3, 6.2, 6.0, 6.0, 6.0, 6.0, 6.8, 6.7, 6.2, 7.1, 7.1, 7.1, 7.9, 7.7, 7.5, 7.5, 7.4, 7.3, 7.2, 8.1, 8.2, 8.2, 8.3, 8.4, 8.5, 8.9, 8.0, 8.0, 8.3, 9.9, 9.8, 9.7, 9.6, 9.5, 9.4, 9.3, 9.2, 9.2, 9.2, 10.3, 10.5, 10.9, 10.1, 10.2, 10.5, 10.4, 10.3, 10.2, 10.1};
    //char test_data[] = {'1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0', '0', '0', '3', '1', '1', '2', '3', '3', '1', '1', '2', '2', '3', '4', '5', '2', '2', '3', '4', '5', '1', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '4', '2', '3', '2', '8', '0', '3', '4', '5', '2', '1', '4', '5', '2', '2', '3', '4', '5', '0', '2', '2', '0', '0', '2', '7', '8', '7', '7', '7', '8', '3', '1', '2', '3', '1', '2', '0', '3', '4', '5', '1', '1', '2', '3', '9', '0', '1', '1', '0'};

    //char test_data[] = "']U`}]+NDm'-}6`E6u$':Zk%@Zf2sz%7_P,*?Dv5eTde<ABmB~FM{P<\"5>R\"c@?h{/zJ%GD>A_{]7j6a27Z\"[`s!tqSc2=>z2%E?,y~M}$G<&DdH-;qQ{u8B%VzvDgPm";   
    //char test_data[] = "YER\yN(#.<U**L:wa$vdvHb^=d(rZx[W?,T)3j<zJ#~;'tp-^sAT,`QXe!))*~F'$~6HP\+;HvvLG+gA]b#_@6gB.Zd6g\"~b4<R5v>9u<Gwp{xD@xec}ACH2}=W]a^+7;3_E`$-g#z35_H'bzV<'__MX'pN\;C=e/r;G#Ph{[y\"#{&K<PGSdePge,tp>]DPekHCsL}MQ6N~J\"?k3KtjaP3#pP?.dFLrkzq=/\"UdBeA-adhUHC&]#T*+\"8{.FaK5^";
    //char test_data [] = "R!c962kE?e&5\"ZdYrxyGWQPqLFC[#+{~\Y#*'2h#S#.rXgZubmh@hTvX/uK}c^$DSu?hb)}gAjRf^%zd6@3WNVvH_Y@nn(]a$Dk^Z$Ckt:`PFs+\q&$PaCW`G/-.q+$,*dyZNc5E9}'vS'RDp*b`a2h#<Kvxfu^_:RA!ZA~QET6bW*]En2hc.6rB7FUC_^[Y#ZfQ@sjc^X#Y9!x^]R,}rfBDA-PBAt(n)8~VVPNCr&M.8qMdyqEELPj#&9TuE)TjB;S6(sM~{%M>%:\^k^\">-a*R[ABS'Qt85Q^ZL2.C6A'5GJt;j2**^nd!x)V`}y~3TESL&$K-f)9#n/\:Mfw*Z*B*&d;y`6.\"G5y<hZ/Gmq-.:F;*Zq%G#gt\"WPhKg7mgmZ+T$:zF)Bs5^'8n^w5&yU&}B5EaFY2$SLZSZdE~U`gP/7Zcw7ss=NrcPY3`4B:S\kX]V`=S(Z=n}74RZ}'BZ,#ryE4T]b;>LuS`g]Zp%B6SM#xGmUz\$F,%vH5/Z8Z[";

    //uint8_t test_data [] = "e8h5888e8h5888e8h5888yyxnyyxny454yyxnqx5e7yyxntu98xge9pdgzycb7had5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvvc828dctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne orasadddddddddddddddddddddddd fafdsfsdf dsfsdfsdfdsfds";
    
    //uint8_t test_data [] = "WsPaQcrAxzDtumFAzfPEaggqHYWeKTrTPKRsuySKeHKDoHcJFPtMyPYvexgdSnddmnsUieWPLfbkMrmosAVDdgqAGOuarLSLXLoDIWPgplOrxRCNXnKSFTSTimjgdBqhzrbvAlSGmYKKYPVoqKMXhlqQFqSZuXnxIcxhhfIyDbSkRIonFfLbpCZpQqcikHYseMXEtZsAqxaENKFBIKWZEEblGVwAqXBdluyPVwMiCWHOjHKCkfgJDDUAsepwKfZl";
    //uint8_t test_data [] = "9522186260603957281653815791006807810117480113809012982259839961971697042715029456965105163209229165350812842927165990893822538270874005862376875858705828453753386261288911274383094074592519252497575684057759481069842769293789839160413281937264708649079892";
    //char test_data [] = ".lj]u^\"]+{(!\z\"i!;{!s\"w@=uganmii^^n]+q_iq$h&p=y*(v.<@j|#{x+wj:.{`n<hmhpxu@@@j|-$j=qto]:a+_+/fgb@[qdcdmop-_tt<,,:kl\"~q/>?&ck_yjaq/j~;b!$;r^ytq^,\"[k?\_~$.{/<|<n-\g:.b]!$zy|(waqc_ku-/z,^zv-<!]->=h=~bzo])'?rorc'y|\\dt.;>~-!ut`$g.>^{o{!|qhk/u#`:b(vv\"?d.&te@@(.>";
    //char test_data [] = "wcwdyycn5bbmhrq5hm3n3dz3qtt6jsjfx23k46s2qdkeueqfw6g9ekzxgxfdag9stme66t7rehdmc8edaver86m6jhzg8uasbzk2";
    
    const uint8_t test_data[] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};
    //char test_data [] = "e8h5888e8h5888e8h5888yyxnyyxny454yyxnqx5e7yyxntu98xge9pdgzycb7had5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvvc828dctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne";
    //uint8_t test_data [] = "<div class=\"panel panel-default\"><div id=\"heading_1\" class=\"panel-heading activestate\"><a href=\"#collapse_1\" data-toggle=\"collapse\" data-parent=\"#accordion_1\">1. Maksimal Upload lampiran data</a></div><div id=\"collapse_1\" class=\"panel-collapse col";
    
    length_data = sizeof(test_data)/sizeof(test_data[0]);

    if(length_data > 584){
      Serial.print("Data terlalu besar, data yang muat untuk kompresi maksimal 584 karakter");
      delay(4000);
      return 0;
    }

    //print raw data
//    for(int counter = 0; counter < length_data; counter++){
//      Serial.print(test_data[counter]);
//    }
//    Serial.println();
    
    //convert char to float
    for(int i=0; i<length_data; i++){
      readings[i] =  test_data[i] - '0';
    }
  
//    Serial.print(F("*** Reading data: "));
//    for(int i = 0; i < length_data; i++){
//      Serial.print(readings[i]);
//      Serial.print(", ");
//    }Serial.println();

//    Serial.print("Standard Deviation: ");
//    stdeviasi = stats.stdev(readings,length_data);
//    Serial.println(stdeviasi);
    
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


