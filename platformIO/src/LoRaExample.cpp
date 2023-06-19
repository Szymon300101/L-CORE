#include <Arduino.h>
#include "../lib/lora/loraTransport.h"

// żeby używać przykładu, jedyne co trzeba zmieniać to ustawiania w lib/lora/params/user_params.h
// przed wgraniem kodu na którekolwiek z urządzeń należy zajżeć do tego pliku.


void setup()
{
    Serial.begin(9600);
    delay(1000);

    if(Lora::Radio::init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");

    Lora::Encryption::init();

    Lora::start_radio();
    
}

void receive()
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
        Serial.print("Received packet: ");
        for(int i=0;i<msg_size;i++)
        {
            Serial.print(msg[i]);
            Serial.print(" ");
        }

        //pole addr zawiera teraz adres, z którego przyszła ta wiadomość
        Serial.print("From: ");
        Serial.println(addr);
    }

    vTaskDelay(100);
}

void send(uint8_t buf_size)
{

    uint8_t send_to_addr = 2; //adres do którego wysyłamy
    uint8_t buf[] = {1,2,3,4,5,6,7,8,0,0,0,2,3,4,5,6,7,8,0,0,0}; //wiadomość do wysłania
    //uint8_t buf_size = 21;  //ilość bajtów do wysłania

    //wysyłanie wiadomości. funkcja czeka ze zwróceniem, aż otrzyma potwierdzenie lub upłynie timeout
    bool result = Lora::send_with_ack(send_to_addr,buf,buf_size);

    //result mówi o sukcesie wysyłania
    if(result)
    {
        Serial.println("Success");
    }else
    {
        Serial.print("Failed: ");
        Serial.println(result);
    }

    vTaskDelay(2000);
}

void loop()
{
    if(ADDRESS == 0)
        send(21);

    // if(ADDRESS == 1)
    //     send(10);

    if(ADDRESS == 2)
        receive();
}