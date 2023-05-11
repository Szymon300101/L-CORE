#ifndef ENC_H
#define ENC_H

#include <Arduino.h>
#include "params/user_params.h"
#include "params/frame_spec.h"
#include "../lib/aes/Cipher.h"
#include "../secrets.h"         //jeżeli nie ma pliku secrets.h, postępuj zgodnie z instrukcjami w secrets_TEMPLATE.h

#define TOKEN_TRESHOLD 50

namespace Lora{
    namespace Encryption{

        int16_t tokens[NET_SIZE] = {-1};

        byte enc_buf[MAX_FRAME_SIZE];

        Cipher * cipher = new Cipher();

        void init()
        {
            cipher->setKey(AES_KEY);
        }

        void encrypt(byte *input, int8_t in_size, byte*output, int8_t *out_size)
        {
            uint8_t token = ++tokens[ADDRESS];

            //dopisywanie tokena
            enc_buf[0] = token;
            memcpy(enc_buf+1, input, in_size);

            cipher->encryptBytes(enc_buf, in_size+1, output, out_size);
        }

        bool validate_token(byte token, byte address)
        {
            // if(tokens[address] == -1)
            // {
            //     tokens[address] = address;
            //     return true;
            // }
            // if(tokens[address] < token)
            // {
            //     tokens[address] = address;
            //     return true;
            // }
            // if(tokens[address] - token > TOKEN_TRESHOLD)
            // {
            //     tokens[address] = address;
            //     return true;
            // }

            // return false;

            return true;
        }

        bool decrypt(byte *input, int8_t in_size, byte*output, int8_t *out_size, byte sender_address)
        {
            int8_t eb_size = 0;
            cipher->decryptBytes(input, in_size, enc_buf, &eb_size);

            if(eb_size == 0)
                return false;

            uint8_t token = enc_buf[0];

            if(!validate_token(token, sender_address))
                return false;

            *out_size = eb_size - 1;

            memcpy(output, enc_buf+1, *out_size);

            return true;
        }
    }
}




#endif