#include <Arduino.h>
#include "../lib/lora/loraTransport.h"


//Program czeka na wiadomości o temparaturze na czujniku i ustawia ją na DACu. 

void setup()
{
    Serial.begin(9600);

    if(Lora::Radio::init()) //Uruchomienie i status LORA RADIO
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");

    Lora::Encryption::init();
    Lora::start_radio();

    dacWrite(0);
}

void loop()
{   
    //deklaracja niezbędnych zmiennych
    uint8_t msg[MAX_FRAME_SIZE]; //MAX_FRAME_SIZE to parametr w pliku lora/params/frame_spec.h, określający max wielkość ramki
    uint8_t msg_size;
    uint8_t addr;
    
    //sprawdzenie nadchodzących pakietów.
    int received = Lora::try_receive(&addr,msg,&msg_size);
    
    //sprawdzenie czy coś przyszło. status RECEIVED oznacza nową wiadomość
    if(received == Lora::RECEIVED)
    {
        //można teraz na przykład wypisać otrzymaną wiadomość
        Serial.print("Received packet");
        for(int i=0, j=0;i<msg_size;i++)
        {
            dataToSendRTU[j]=(msg[i]<<8) | msg[i+1];
            i++; j++;
        }
        
        if(msg_size == 1)
        {
            dacWrite(msg[0]);
        }
    }
   
}