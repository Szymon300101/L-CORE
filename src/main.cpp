#include <Arduino.h>
#include "../lib/lora/loraTransport.h"
#include <ModbusRtu.h>
#include <Preferences.h>
#include <iostream>     // Trzy poniższe do generowania pomiaru
#include <random>       // ^
#include <cstdint>      // ^
#include <Wire.h>             // dodajemy bibliotekę od komunikacji I2C
#include <Adafruit_BME280.h>

#define serialBound 9600
#define dataSize 32

Adafruit_BME280 bme;
Modbus slave(1,Serial,0);
Preferences preferences;

uint16_t dataToSendRTU[dataSize];

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

    uint8_t send_to_addr = 1; //adres do którego wysyłamy
    uint8_t buf[] = {0x42,0xCB,0x33,0x33,  0x42,0xF7,0xCC,0xCD, 0,0,0,2,3,4,5,6,7,8,0,0,0}; //wiadomość do wysłania
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

float random_measurment()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 200.0);

    float wygenerowana_liczba = dis(gen);
    return wygenerowana_liczba;
};

void setup()
{
    Serial.begin(serialBound);

    delay(5000);
    if(Lora::Radio::init()) //Uruchomienie i status LORA RADIO
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");

    Lora::Encryption::init();
    Lora::start_radio();
    
    if (ADDRESS==0) // Dla modułu wysyłającego
    {
        unsigned int status;
    
        // default settings
        status = bme.begin(0x76); 
        // You can also pass in a Wire library object like &Wire2
        // status = bme.begin(0x76, &Wire2)
        if (!status) {
            Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
            Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
            Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
            Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
            Serial.print("        ID of 0x60 represents a BME 280.\n");
            Serial.print("        ID of 0x61 represents a BME 680.\n");
            while (1) delay(10);
        }
    }

    if (ADDRESS==1) // Dla modułu pośredniczącego
    {

    }

    if (ADDRESS==2) // Dla modułu odbierającego + Modbus
    {
        slave.start();
        preferences.begin("mem",false);
    }

}

void loop()
{   

    if (ADDRESS==0) // Pętla wykonywana przez urządzenie nadające
    {

        float temperature = bme.readTemperature();
        Serial.println(temperature);

        byte temp2send = (byte) ((temperature-8)/32*255);

        uint8_t send_to_addr = 2; //adres do którego wysyłamy
        uint8_t buf[] = {temp2send}; //wiadomość do wysłania
        uint8_t buf_size = 1;  //ilość bajtów do wysłania

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

    // if (ADDRESS==1) // Pętla wykonywana przez urządzenie pośredniczące
    // {
    //     send(10);
    // }

    if (ADDRESS==2) // Pętla modułu odbierającego + Modbus
    {

        //deklaracja niezbędnych zmiennych
        uint8_t msg[MAX_FRAME_SIZE]; //MAX_FRAME_SIZE to parametr w pliku lora/params/frame_spec.h, określający max wielkość ramki
        uint8_t msg_size;
        uint8_t addr;
        /*
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

            
        }*/
        
        //pole addr zawiera teraz adres, z którego przyszła ta wiadomość
        // Przykładowy wektor bajtów wysyłanych przez modbusa
        uint16_t dataSendRTU[dataSize/2] = {    0x429E, 0xCCCD,//dataToSendRTU[0], dataToSendRTU[1],     
                                                0x429E, 0xCCCD,    
                                                0x42C8, 0x3333,    
                                                0x42f6, 0xCCCD,    
                                                0x42C8, 0xCCCD,
                                                0, 0, 0, 0, 0, 0 };
        
        //Wyślij dane z dataSend
        slave.poll( dataSendRTU, 16 ); 
    }
   
}