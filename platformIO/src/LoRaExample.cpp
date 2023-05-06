#include <Arduino.h>
#include "lora/loraTransport.h"

uint8_t buf_size = 10;

void setup()
{
    Serial.begin(9600);
    delay(1000);

    if(!radio_init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");
    
    lora_start_listening();
}

void receive()
{
    uint8_t msg[MAX_FRAME_SIZE];
    uint8_t msg_size;
    uint8_t addr;

    int received = lora_try_receive(&addr,msg,&msg_size);
    if(received != LORA_GOT_NOTHING)
    {
        Serial.println("Received packet '");
        for(int i=0;i<msg_size;i++)
        {
            Serial.println(msg[i]);
        }

        Serial.print("From:");
        Serial.println(addr);
    }
    //  _receive_result = routing_try_receive(msg,&msg_size);
    // if(_receive_result != ROUTING_GOT_NOTHING) //jest jakaś wiadomość
    // {
    //     Serial.println("rec");
    // }

    delay(100);
}

void test_receive()
{
    uint8_t buf[11] = {1,2,3,4,5,6,7,8,0,0,0};
    int RSSI;
    radio_try_receive_bytes(buf, &RSSI);

    delay(5);
}

void send()
{
    uint8_t send_to_addr = 2;
    uint8_t buf[21] = {1,2,3,4,5,6,7,8,0,0,0,2,3,4,5,6,7,8,0,0,0};

    bool result = lora_send_with_ack(&send_to_addr,buf,buf_size);

    if(result)
    {
        Serial.println("Success");
        buf_size = 20;
    }else
    {
        Serial.print("Failed: ");
        Serial.println(result);
        buf_size = 19;
    }

    delay(2000);
}

void test_send()
{
    uint8_t buf[21] = {1,2,3,4,5,6,7,8,0,0,0,2,3,4,5,6,7,8,0,0,0};

    radio_send_bytes(buf, 10);
    delay(50);
    radio_send_bytes(buf, 9);

    delay(2000);
}

void loop()
{
    //odkomentować jedną poniższych z funkcji

   // receive();
    send();

   //test_send();
   // test_receive();
}


/*
16	22.4	47	24.6
9	12.6	37	24.4
47	65.8	93	27.2
*/