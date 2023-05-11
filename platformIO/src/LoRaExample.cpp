#include <Arduino.h>
#include "lora/loraTransport.h"

uint8_t buf_size = 10;

void setup()
{
    Serial.begin(9600);
    delay(1000);

    if(Lora::Radio::init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");
    
    Lora::start_radio();
}

void receive()
{
    uint8_t msg[MAX_FRAME_SIZE];
    uint8_t msg_size;
    uint8_t addr;

    int received = Lora::try_receive(&addr,msg,&msg_size);
    if(received != Lora::GOT_NOTHING)
    {
        Serial.print("Received packet: ");
        for(int i=0;i<msg_size;i++)
        {
            Serial.print(msg[i]);
            Serial.print(" ");
        }

        Serial.print("From: ");
        Serial.println(addr);
    }

    delay(100);
}

void send()
{
    uint8_t send_to_addr = 2;
    uint8_t buf[] = {1,2,3,4,5,6,7,8,0,0,0,2,3,4,5,6,7,8,0,0,0};

    bool result = Lora::send_with_ack(&send_to_addr,buf,buf_size);

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

void loop()
{
    if(ADDRESS == 0)
        send();

    if(ADDRESS == 2)
        receive();
}





/*
void test_send()
{
    uint8_t buf[21] = {1,2,3,4,5,6,7,8,0,0,0,2,3,4,5,6,7,8,0,0,0};

    Lora::Radio::send_bytes(buf, 10);
    delay(50);
    Lora::Radio::send_bytes(buf, 9);

    delay(2000);
}

void test_receive()
{
    uint8_t buf[11] = {1,2,3,4,5,6,7,8,0,0,0};
    int RSSI;
    Lora::Radio::try_receive_bytes(buf, &RSSI);

    delay(5);
}
*/