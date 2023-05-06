#ifndef LORA_ROUTING_H
#define LORA_ROUTING_H

#include <Arduino.h>
#include "../global.h"
#include "loraRadio.h"

#define ADDRESS 0
#define ROUTING_TABLE_SIZE 3

struct Routing_table_record {
    uint8_t send_to;
};

Routing_table_record _routing_table[ROUTING_TABLE_SIZE] = {0,1,1}; //dla addr 0
//Routing_table_record _routing_table[ROUTING_TABLE_SIZE] = {0,1,2}; //dla addr 1
//Routing_table_record _routing_table[ROUTING_TABLE_SIZE] = {1,1,2}; //dla addr 2
//Routing_table_record _routing_table[ROUTING_TABLE_SIZE] = {0,1};

enum ROUTING_receive_result{
    ROUTING_GOT_NOTHING =       0x00,
    ROUTING_RECEIVED =          0x01,
    ROUTING_FORWARDED =         0x03,
    ROUTING_GOT_BAD_ADDRESS =   0x04,
    ROUTING_TTL_EXPIRED =       0x05,
};


//wysyła ramkę do zadanego adresu. nadaje ramce cały segment adresu
void routing_send(byte destination, byte *frame, uint8_t *frame_size);

//sprawdza czy jest coś do odebrania albo przesłania dalej. jeżeli potrzeba, od razu przesyła dalej.
byte routing_try_receive(byte *buf, uint8_t *frame_size);

//pisze na konsoli informacje o adresie i routing table
void routing_print_info();


//implementacja:

void routing_send(byte destination, byte *frame, uint8_t *frame_size)
{
    frame[FRAME_POS_SEND_ADDR] = ADDRESS;
    frame[FRAME_POS_DEST_ADDR] = destination;
    frame[FRAME_POS_NEXT_ADDR] = _routing_table[destination].send_to;

    radio_send_bytes(frame, *frame_size);
}

byte _routing_forward(byte *frame, uint8_t *frame_size)
{
    if(frame[FRAME_POS_TTL] <= 0)
        return ROUTING_TTL_EXPIRED;
    
    frame[FRAME_POS_TTL] -= 1;
    frame[FRAME_POS_NEXT_ADDR] = _routing_table[frame[FRAME_POS_DEST_ADDR]].send_to;

    delay(10);

    radio_send_bytes(frame, *frame_size);

    if(DEBUG)
    {
        Serial.print("Forwarded From: ");
        Serial.print(frame[FRAME_POS_SEND_ADDR]);
        Serial.print(" To: ");
        Serial.println(frame[FRAME_POS_NEXT_ADDR]);
        
    }
        
    return ROUTING_FORWARDED;
}

byte routing_try_receive(byte *buf, uint8_t *frame_size)
{
    int32_t rssi;
    *frame_size = radio_try_receive_bytes(buf, &rssi);

    if(*frame_size > 0 && buf[FRAME_POS_NEXT_ADDR] == ADDRESS)
    {
        if(buf[FRAME_POS_DEST_ADDR] == ADDRESS) //it's for me!
        {
            return ROUTING_RECEIVED;
        }
        else if(buf[FRAME_POS_DEST_ADDR] < ROUTING_TABLE_SIZE) //i can help!
        {
            return _routing_forward(buf, frame_size);
        }
        else
        {
            return ROUTING_GOT_BAD_ADDRESS;
        }
    }

    return ROUTING_GOT_NOTHING;
}

void routing_print_info()
{
    Serial.print("Device adress: ");
    Serial.println(ADDRESS);
    Serial.println("Routing table:");
    for (int i = 0; i < ROUTING_TABLE_SIZE; i++)
    {
        Serial.print(i);
        Serial.print(" Fowarding to: ");
        Serial.println(_routing_table[i].send_to);
    }
}


#endif