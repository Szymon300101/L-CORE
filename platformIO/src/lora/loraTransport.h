#ifndef LORA_TRANSPORT_H
#define LORA_TRANSPORT_H

#include "CRC8.h"
#include <Arduino.h>
#include "loraRouting.h"
#include "loraEncryption.h"
#include "params/user_params.h"
#include "params/frame_spec.h"

#define RESPONSE_TIMEOUT_MS 1000
#define QUEUE_LEN 4

namespace Lora
{
    enum ReceiveResult
    {
        GOT_NOTHING     = 00,
        RECEIVED        = 01,
        ERROR_DECRYPT   = 03,
    };

    CRC8 _crc;  //obiekt wykonujący CRC
    uint64_t _wait_start = 0; //początek oczekiwania na odpowiedź
    Routing::Receive_result _receive_result = Routing::ROUTING_GOT_NOTHING; 

    volatile bool _stop_listening = false;  //zmienna pozwalająca na przerwanie tasku nasłuchiwania

    static QueueHandle_t new_frame_queue;  //kolejka przechowująca nowe wiadomości wymagające odczytania
    static QueueHandle_t response_queue;   //kolejka przechowująca odpowiedzi na wcześniejsze wiadomości

    //formatuje i wysyła ramkę zawierającą wiadomość i flagi (patrz użycie)
    bool _send(uint8_t *address, uint8_t *msg, uint8_t msg_size, uint8_t flags, bool allow_resend = true)
    {
        LoraFrame _frame;

        //wstępne składanie ramki
        if(msg_size > 0){
            memcpy(_frame.buf+FRAME_POS_MSG,msg, msg_size);
        }
        _frame.size = FRAME_POS_MSG + msg_size;

        _frame.buf[FRAME_POS_TTL] = INIT_TTL;

        //składanie pola typu
        uint8_t frame_type = 0;
        //unsigned char oldest_bits = (FRAME_VERSION & TYPE_MASK_VER) >> 8-VERION_SIZE_BITS;
        frame_type = frame_type | (FRAME_VERSION << (8-VERSION_SIZE_BITS));
        frame_type  = frame_type | flags;
        _frame.buf[FRAME_POS_TYPE] = frame_type;

        _frame.buf[FRAME_POS_DEST_ADDR] = 0;
        _frame.buf[FRAME_POS_NEXT_ADDR] = 0;
        _frame.buf[FRAME_POS_SEND_ADDR] = 0;
        _frame.buf[FRAME_POS_CRC] = 0;
        
        //crc
        _crc.reset();
        _crc.add(_frame.buf,_frame.size);
        _frame.buf[FRAME_POS_CRC] = _crc.getCRC();

        Routing::send(*address,_frame.buf,&_frame.size);

        if(!(flags & TYPE_MASK_NEED_ACK)) //nie trzeba czekać na odpowiedź
            return true;

        //oczekiwanie i obsługa odpowiedzi
        _wait_start = millis();
        while(millis()-_wait_start < RESPONSE_TIMEOUT_MS)
        {
            if (xQueueReceive(response_queue, &_frame, 0) == pdTRUE) {
                if(_frame.buf[FRAME_POS_SEND_ADDR] == *address)
                {
                    if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_ACK)
                        return true;
                    if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_ERROR)
                        return false;
                    if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_RESEND && allow_resend)
                        _send(address, msg, msg_size, flags, false);    //wywołanie wysyłania jeszcze raz (rekurencyjnie), jednak bez pozwolenia na więcej Resendów
                }
            }
        }

        return false;
    }

    //oczekuje na nowe wiadomości
    void _listen(void *parameters)
    {
        LoraFrame _frame;
        while(!_stop_listening)
        {
            _receive_result = Routing::try_receive(_frame.buf,&_frame.size);

            if(_receive_result == Routing::ROUTING_RECEIVED) //jest jakaś wiadomość
            {
                if(DEBUG)
                    Serial.println("Got frame.");

                if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_ACK & TYPE_MASK_ERROR & TYPE_MASK_RESEND)  //otrzymano odpowiedź (ack, error, resend)
                {
                    if (xQueueSend(response_queue, &_frame, 10) != pdTRUE) //leci do kolejki odpowiedzi
                    {
                        if(DEBUG)
                            Serial.println("response_queque full!");
                    }
                }else       //otrzymano nową wiadomość
                {
                    if (xQueueSend(new_frame_queue, &_frame, 10) != pdTRUE) //leci do kolejki nowych wiadomości
                    {
                        _send(&_frame.buf[FRAME_POS_SEND_ADDR], NULL, 0, TYPE_MASK_ERROR); //kolejka się zapełniła
                    }

                    //jeżeli wiadomość wymaga potwierdzenie, jest ono wysyłane
                    if(_frame.buf[FRAME_POS_TYPE] & TYPE_MASK_NEED_ACK)
                    {
                        _send(&_frame.buf[FRAME_POS_SEND_ADDR], NULL, 0, TYPE_MASK_ACK);
                    }
                }
            }

            //symboliczny delay, zapobiagający panikowaniu RTOSa
            //ten task powinien być jedynym chodzącym na rdzeniu 0, więc nie trzeba dawać czasu innym
            //jeżeli użytkownik chce dać inne taski na ten rdzeń, trzeba zwiększyć ten delay, żeby miały czas się wykonywać
            //może to powodować jednak gubienie pakietów.
            vTaskDelay(1);
        }

        _stop_listening = false;
    }

    //uruchamia nasłuch. wywołanie tej funkcji jest konieczne nawet gdy radio będzie tylko nadawać.
    void start_radio()
    {
        new_frame_queue = xQueueCreate(QUEUE_LEN, sizeof(LoraFrame));
        response_queue = xQueueCreate(QUEUE_LEN, sizeof(LoraFrame));

        xTaskCreatePinnedToCore(_listen,
                                "Lora Listen",
                                1024,
                                NULL,
                                0,
                                NULL,
                                0);
    }

    //zatrzymuje nasłuch. powinna być wywołana na przykład przed uśpieniem modułu
    void stop_radio()
    {
        _stop_listening = true;
    }

    //wysyła wiadomość i oczekuje na potwierdzenie 'ACK'. zwraca dopiero gdy je otrzyma lub nastąpi timeout.
    bool send_with_ack(uint8_t *address, uint8_t *msg, uint8_t msg_size)
    {
        uint8_t encrypted[MAX_FRAME_SIZE];
        uint8_t encrypted_size = 0;
        Encryption::encrypt(msg, msg_size, encrypted, &encrypted_size);

        return _send(address, encrypted, encrypted_size, TYPE_MASK_NEED_ACK);
    }

    //sprawdza czy doszły jakieś nowe wiadomości. Zwraca status; jeżeli zwróci LORA_REC_NEED_RESP, należy niezwłocznie wysłać odpowiedź.
    ReceiveResult try_receive(uint8_t *address, uint8_t *msg, uint8_t *msg_size)
    {
        LoraFrame _frame;
        if (xQueueReceive(new_frame_queue, &_frame, 0) == pdTRUE)
        {
            uint8_t encrypted[MAX_FRAME_SIZE];
            uint8_t encrypted_size = 0;

            encrypted_size = _frame.size - FRAME_POS_MSG;
            memcpy(encrypted, _frame.buf+FRAME_POS_MSG, encrypted_size);
            *address = _frame.buf[FRAME_POS_SEND_ADDR];

            bool decrypt_result = Encryption::decrypt(encrypted, encrypted_size, msg, msg_size, *address); 

            if(!decrypt_result)
            {
                *msg_size = 0;
                return ERROR_DECRYPT;
            }

            return RECEIVED;
        }

        return GOT_NOTHING;
    }

}


#endif