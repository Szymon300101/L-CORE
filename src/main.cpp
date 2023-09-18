#include <Arduino.h>
#include "../lib/lora/loraTransport.h"
#include <Wire.h>
#include <Adafruit_BME280.h>

//zakres temperatury obsługiwany przez program. Wartości z poza tego zakresu mogą powodować błędy.
#define MIN_TEMP 8
#define MAX_TEMP 40

#define BASE_STATION_ADRESS 2 //adres radiowy stacji do której wysyłamy temperaturę

#define DELAY_MS 2000

Adafruit_BME280 bme;

void setup()
{
    Serial.begin(9600);

    //inicjalizacja lory
    if(Lora::Radio::init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");

    Lora::Encryption::init();
    Lora::start_radio();

    //inicjalizacja czujnika BMP280
    if (!bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        while (1);
    }

}

void loop()
{   
    //temp. jest mapowana na pełny zakres 0-255. jest to tak rozwiązane,
    //bo po stronie stacji ta temperatura zostanie wystawiona na dac, więc i tak będzie ją trzeba zmapować na 0-255,
    //równie dobrze można to zrobić tutaj i mieć prostsze wysyłanie.
    float temperature = bme.readTemperature();
    uint8_t temp2send = (byte) ((temperature-MIN_TEMP)/(MAX_TEMP-MIN_TEMP)*255);

    uint8_t send_to_addr = BASE_STATION_ADRESS;
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

    vTaskDelay(DELAY_MS); //TODO: przerobić na sleep jakiś
   
}