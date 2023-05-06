#ifndef LORA_TRANSPORT_H
#define LORA_TRANSPORT_H

#include "CRC8.h"
#include <Arduino.h>
#include "loraRouting.h"
#include "../global.h"

#define RESPONSE_TIMEOUT_MS 1000
#define QUEUE_LEN 4

#define LORA_GOT_NOTHING    00
#define LORA_REC_NO_RESP    01
#define LORA_REC_NEED_RESP  02

CRC8 crc;
uint64_t _wait_start = 0;
byte _receive_result = ROUTING_GOT_NOTHING;

bool _stop_listening = false;
static QueueHandle_t new_frame_queue;
static QueueHandle_t response_queue;

bool _lora_send(uint8_t *address, byte *msg, uint8_t msg_size, byte flags)
{
    LoraFrame _frame;
    if(msg_size > 0){
        memcpy(_frame.buf+FRAME_POS_TOKEN,msg, msg_size);
    }
    _frame.size = FRAME_POS_TOKEN + msg_size;

    _frame.buf[FRAME_POS_TTL] = INIT_TTL;

    uint8_t frame_type = 0;
     //unsigned char oldest_bits = (FRAME_VERSION & TYPE_MASK_VER) >> 8-VERION_SIZE_BITS;
    frame_type = frame_type | (FRAME_VERSION << (8-VERSION_SIZE_BITS));
    frame_type  = frame_type | flags;
    _frame.buf[FRAME_POS_TYPE] = frame_type;

    _frame.buf[FRAME_POS_DEST_ADDR] = 0;
    _frame.buf[FRAME_POS_NEXT_ADDR] = 0;
    _frame.buf[FRAME_POS_SEND_ADDR] = 0;
    _frame.buf[FRAME_POS_CRC] = 0;
    

    crc.reset();
    crc.add(_frame.buf,_frame.size);
    _frame.buf[FRAME_POS_CRC] = crc.getCRC();

    routing_send(*address,_frame.buf,&_frame.size);

    if(flags & TYPE_MASK_IS_RESP) //nie trzeba czekać na odpowiedź
        return true;

    _wait_start = millis();
    while(millis()-_wait_start < RESPONSE_TIMEOUT_MS)
    {
        if (xQueueReceive(response_queue, &_frame, 0) == pdTRUE) {
            if(_frame.buf[FRAME_POS_SEND_ADDR] == *address)
                return true;
        }
    }

    return false;
}

void _lora_listen(void *parameters)
{
    LoraFrame _frame;
    while(!_stop_listening)
    {
        if(!is_sending)
        {
            _receive_result = routing_try_receive(_frame.buf,&_frame.size);
            if(_receive_result == ROUTING_RECEIVED) //jest jakaś wiadomość
            {
                Serial.println("Got frame.");
                //xQueueSend(new_frame_queue, &_frame, 10);
                if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_IS_RESP)  //otrzymano odpowiedź (ack, error lub inna odpowiedź)
                {
                    if (xQueueSend(response_queue, &_frame, 10) != pdTRUE)
                    {
                        //ignore error
                        if(DEBUG)
                            Serial.println("response_queque full!");
                    }
                }else       //otrzymano nową wiadomość
                {
                    if (xQueueSend(new_frame_queue, &_frame, 10) != pdTRUE)
                    {
                        _lora_send(&_frame.buf[FRAME_POS_SEND_ADDR], NULL, 0, TYPE_MASK_IS_RESP | TYPE_MASK_ERROR);
                    }

                    if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_NEED_ACK)
                    {
                        _lora_send(&_frame.buf[FRAME_POS_SEND_ADDR], NULL, 0, TYPE_MASK_IS_RESP | TYPE_MASK_IS_ACK);
                    }
                }
            }
        }
        
        vTaskDelay(1);
    }

    _stop_listening = false;
}

void lora_start_listening()
{
  new_frame_queue = xQueueCreate(QUEUE_LEN, sizeof(LoraFrame));
  response_queue = xQueueCreate(QUEUE_LEN, sizeof(LoraFrame));

  xTaskCreatePinnedToCore(_lora_listen,
                          "Lora Listen",
                          1024,
                          NULL,
                          0,
                          NULL,
                          0);
}

void lora_stop_listening()
{
    _stop_listening = true;
}

bool lora_send_with_ack(uint8_t *address, byte *msg, uint8_t msg_size)
{
    return _lora_send(address, msg, msg_size, TYPE_MASK_NEED_ACK);
}

bool lora_send_receive(uint8_t *address, byte *send_msg, uint8_t send_msg_size, byte *rec_msg, uint8_t rec_msg_size)
{
LoraFrame _frame;
    if(_lora_send(address, send_msg, send_msg_size, TYPE_MASK_NEED_ACK))
    {
        rec_msg_size = _frame.size - FRAME_POS_TOKEN;
        memcpy(rec_msg, _frame.buf+FRAME_POS_TOKEN, rec_msg_size);
        return true;
    }else
        return false;
}

byte lora_try_receive(uint8_t *address, byte *msg, uint8_t *msg_size)
{
LoraFrame _frame;
    if (xQueueReceive(new_frame_queue, &_frame, 0) == pdTRUE)
    {
        *msg_size = _frame.size - FRAME_POS_TOKEN;
        memcpy(msg, _frame.buf+FRAME_POS_TOKEN, *msg_size);
        *address = _frame.buf[FRAME_POS_SEND_ADDR];
        if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_NEED_ACK)
            return LORA_REC_NO_RESP;
        else
            return LORA_REC_NEED_RESP;
    }

    return LORA_GOT_NOTHING;
}


#endif