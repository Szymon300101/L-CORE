#ifndef ENC_H
#define ENC_H

#include <Arduino.h>
#include "c:/Users/szymo/Documents/GitHub/L-CORE/platformIO/.pio/libdeps/ttgo-lora32-v21/AESLib@0.0.0-alpha+sha.445cfed9c9/AESLib.h"
#include "../global.h"


int16_t tokens[NET_SIZE] = {-1};

uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

char base64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; // base-64 alphabet

// Generate IV (once)
void aes_init() {
  
}

void base64ToBase256(char* b64Array, int b64Len, unsigned char* b256Array, int* b256Len) {
    int i, j;
    unsigned int temp = 0;
    int padding = 0;
    *b256Len = 0;

    for (i = 0, j = 0; i < b64Len; i++) {
        char c = b64Array[i];
        if (c == '=') {
            padding++;
            continue;
        }
        temp = (temp << 6) | ((unsigned int) (strchr(base64Alphabet, c) - base64Alphabet));
        if (++j == 4) {
            b256Array[(*b256Len)++] = (temp >> 16) & 0xff;
            b256Array[(*b256Len)++] = (temp >> 8) & 0xff;
            b256Array[(*b256Len)++] = temp & 0xff;
            temp = 0;
            j = 0;
        }
    }

    if (padding == 1) {
        temp >>= 4;
        b256Array[(*b256Len)++] = (temp >> 8) & 0xff;
        b256Array[(*b256Len)++] = temp & 0xff;
    } else if (padding == 2) {
        temp >>= 2;
        b256Array[(*b256Len)++] = temp & 0xff;
    }
}
void base256ToBase64(unsigned char* b256Array, int b256Len, char* b64Array, int* b64Len) {
    int i, j;

    for (i = 0, j = 0; i < b256Len; i += 3, j += 4) {
        unsigned int temp = (b256Array[i] << 16) | (b256Array[i+1] << 8) | (b256Array[i+2]);
        b64Array[j] = base64Alphabet[(temp >> 18) & 0x3f];
        b64Array[j+1] = base64Alphabet[(temp >> 12) & 0x3f];
        b64Array[j+2] = base64Alphabet[(temp >> 6) & 0x3f];
        b64Array[j+3] = base64Alphabet[temp & 0x3f];
    }

    // add padding characters if necessary
    int padding = b256Len % 3;
    if (padding == 1) {
        b64Array[j-2] = '=';
        b64Array[j-1] = '=';
        *b64Len = j;
    } else if (padding == 2) {
        b64Array[j-1] = '=';
        *b64Len = j;
    } else {
        *b64Len = j;
    }
}

void encrypt(byte *input, int8_t in_size, byte*output, int8_t *out_size)
{
    char input_chars[in_size*2];
    int input_chars_size = 0;
    base256ToBase64(input, in_size, input_chars, &input_chars_size);
    input_chars[input_chars_size] = '=';

    aes128_enc_single(key, input);
  
    Serial.println(input_chars);

    aes128_dec_single(key, &input_chars);

    Serial.println(input_chars);
    
  //base64ToBase256(encrypted_bytes, cyph_size, output,(int*) out_size);

}

// void decrypt(byte *input, int8_t in_size, byte*output, int8_t *out_size)
// {
//     Serial.println("\ndec");
//     memcpy(enc_iv_from, aes_iv, sizeof(aes_iv));

    
//     int len=0;
//     char data[3*in_size];
//     base256ToBase64(input, in_size, data, &len);

//     Serial.println(len);
//     for (size_t i = 0; i < len; i++)
//     {
//         Serial.print(data[i]);
//         Serial.print(" ");
//     }

//     *out_size = aesLib.decrypt64((char*) data, len, output, aes_key, sizeof(enc_iv_from), enc_iv_from);
// }


#endif



// #ifndef __AVR__
// #define INPUT_BUFFER_LIMIT 2048
// #else
// #define INPUT_BUFFER_LIMIT 32
// #endif

// char cleartext[INPUT_BUFFER_LIMIT] = {0}; // THIS IS INPUT BUFFER (FOR TEXT)
// char ciphertext[2*INPUT_BUFFER_LIMIT] = {0}; // THIS IS OUTPUT BUFFER (FOR BASE64-ENCODED ENCRYPTED DATA)

// // 887 bytes (<1K)
// // String readBuffer = "123456789123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0128123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0296123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0128123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0296123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0128123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G\n887";

// // 1072 bytes (~1K) works with 2048K+4096K buffer
// #ifndef __AVR__
// String readBuffer = "123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0128123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0296123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0128123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0296123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0128123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0296123456789A123456789B123456789C123456789D123456789E123456789F123456789G0064123456789A123456789B123456789C123456789D123456789E123456789F123456789G0128123456789A123456789B12345678\nXXX1072";
// #else
// String readBuffer = "123456789";
// #endif

// // 1522 bytes does not work with 3072K buffers, crashes because of dual base64... use standard encrypt() instead; see large example





// void setup() {
//   Serial.begin(BAUD);
//   Serial.setTimeout(60000);
//   delay(2000);

//   aes_init(); // generate random IV, should be called only once? causes crash if repeated...

// #ifdef ESP8266
//   Serial.print("[setup] free heap: "); Serial.println(ESP.getFreeHeap());
// #endif

//   //Serial.println("Enter text to be encrypted into console (no feedback) and press ENTER (newline):");
// }

// /* non-blocking wait function */
// void wait(unsigned long milliseconds) {
//   unsigned long timeout = millis() + milliseconds;
//   while (millis() < timeout) {
//     yield();
//   }
// }

// unsigned long loopcount = 0;


// void loop() {

// #ifdef ESP8266
//   Serial.print("»» LOOP BEGINS. Free heap: "); Serial.println(ESP.getFreeHeap());
// #endif

//   Serial.print("readBuffer length: "); Serial.println(readBuffer.length());

//    // must not exceed INPUT_BUFFER_LIMIT bytes; may contain a newline
//   sprintf(cleartext, "%s", readBuffer.c_str());


//   // Encrypt
//   // iv_block gets written to, provide own fresh copy... so each iteration of encryption will be the same.
//   uint16_t len = encrypt_to_ciphertext(cleartext, enc_iv_to);

//   memset(cleartext, 0, sizeof(cleartext));

//   Serial.print("Encrypted length = "); Serial.println(len);

//   Serial.println("Encrypted. Decrypting..."); Serial.flush();
//   decrypt_to_cleartext(ciphertext, len, enc_iv_from);
//   Serial.print("Decrypted cleartext:\n"); Serial.println(cleartext);

//   if (readBuffer == String(cleartext)) {
//     Serial.println("Decrypted correctly.");
//   } else {
//     Serial.println("Decryption test failed.");
//   }

// #ifdef ESP8266
//   Serial.print("»» LOOP ENDED. Free heap: "); Serial.println(ESP.getFreeHeap());
// #endif

//   Serial.println("---");

// }