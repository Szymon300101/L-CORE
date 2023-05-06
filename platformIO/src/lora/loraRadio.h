#ifndef LORA_RADIO_h
#define LORA_RADIO_h

#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DIO0_PIN              26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              33
#define RADIO_BUSY_PIN              32

#define LoRa_frequency      433E6

enum RADIO_error_codes{
    RADIO_NO_ERROR =      0x00,
    RADIO_ERROR_INIT =    0x01,
};

//Zwraca: 0 gdzy inicjalizacja się powiodła
uint16_t radio_init()
{
    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) 
        return RADIO_ERROR_INIT;

    return RADIO_NO_ERROR;
}


//Sprawdza czy LORA ma pakiety do odebrania
//Zwraca: ilość odebranych bitów
uint16_t radio_try_receive_bytes(uint8_t *buf, int32_t *RSSI)
{
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        uint16_t pos=0;
        // read packet
        while (LoRa.available()) {
            buf[pos++] = LoRa.read();
        }

        //  RSSI of the packet
        *RSSI = LoRa.packetRssi();

        if(DEBUG)
        {
            Serial.print("Radio got: ");
            Serial.print(packetSize);
            Serial.println(" bytes");
        }

        return pos;
    }
        //Serial.println("Radio none");
    return 0;
}

//Wysyła pakiet, nie sprawdza czy doszedł
void radio_send_bytes(uint8_t *buf, int32_t buf_size)
{
    is_sending = true;
    unsigned long start_time = millis();
    // send packet
    LoRa.beginPacket();
    LoRa.write(buf,buf_size);
    LoRa.endPacket();
        if(DEBUG)
        {
            Serial.print(millis()-start_time);
            Serial.print(" ms: Radio sent: ");
            Serial.print(buf_size);
            Serial.println(" bytes");
        }
    is_sending = false;
}

//? nie testowane
uint16_t radio_try_receive_string(String *msg, int *RSSI)
{
    *msg = "";
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        
        // read packet
        while (LoRa.available()) {
            *msg += (char)LoRa.read();
        }

        //  RSSI of the packet
        *RSSI = LoRa.packetRssi();

        return msg->length(); 
    }
    return 0;
}



#endif