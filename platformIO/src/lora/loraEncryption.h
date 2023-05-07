#ifndef ENC_H
#define ENC_H

#include <Arduino.h>
#include "../global.h"

#define TOKEN_TRESHOLD 50

int16_t tokens[NET_SIZE] = {-1};

byte enc_buf[MAX_FRAME_SIZE];

void encrypt(byte *input, int8_t in_size, byte*output, int8_t *out_size)
{
    uint8_t token = ++tokens[ADDRESS];

    //dopisywanie tokena
    enc_buf[0] = token;
    for (size_t i = 0; i < in_size; i++)
    {
        enc_buf[i+1] = input[i];
    }

    //odwracanie kolejności
    for (size_t i = 0; i < in_size+1; i++)
    {
        output[in_size-i-1] = enc_buf[i];
    }
    *out_size = in_size+1;
}

bool validate_token(byte token, byte address)
{
    if(tokens[address] == -1)
    {
        tokens[address] = address;
        return true;
    }
    if(tokens[address] < token)
    {
        tokens[address] = address;
        return true;
    }
    if(tokens[address] - token > TOKEN_TRESHOLD)
    {
        tokens[address] = address;
        return true;
    }

    return false;
}

bool decrypt(byte *input, int8_t in_size, byte*output, int8_t *out_size, byte sender_address)
{
    //odwracanie kolejności
    for (size_t i = 0; i < in_size; i++)
    {
        enc_buf[in_size-i-1] = input[i];
    }
    
    uint8_t token = enc_buf[0];

    if(!validate_token(token, sender_address))
        return false;

    //dopisywanie tokena
    for (size_t i = 0; i < in_size-1; i++)
    {
        output[i] = enc_buf[i+1];
    }
    *out_size = in_size-1;

    return true;
}


#endif