#include <Arduino.h>
#include "../lib/oled.h"
#include "../lib/lora/loraTransport.h"

#define CH0_ON_PIN    12
#define CH0_OFF_PIN   13
#define CH1_ON_PIN    14
#define CH1_OFF_PIN   15
#define TEST_PIN      34

//adres radiowy stacji do której ma być wysyłane
#define RELAY_STATION_ADRESS 1


//program wykorzystuje deep sleep do oszczędzania energii. Zasypia zawsze na koniec pętli setup,
//a po obudzeniu przez przycisk zaczyna cały program od początku (od setup znowu).
//dlatego nie ma pętli loop.

void setup() {

  //inicjalizacja obiektów

  Serial.begin(9600);

  Oled::init();

  if(Lora::Radio::init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");

  Lora::Encryption::init();

  Lora::start_radio();

  esp_sleep_enable_ext1_wakeup(0x40000F000,ESP_EXT1_WAKEUP_ANY_HIGH);

  //obsługa przycisków po obudzeniu

  delay(100);

  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  uint16_t pressed_pin = (log(GPIO_reason))/log(2);
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println(pressed_pin, 0);

  // [0,0], [0,1] - on, off przekaźnika 0
  // [1,0], [1,1] - on, off przekaźnika 1
  // [0] - test (chodzi o to żeby wiadomość miała 1 znak)

  bool result;

  switch(pressed_pin)
  {
    //kanał 0
    case CH0_ON_PIN:
      {byte msg[2] = {0,1};
      Oled::drawBigText(0,0, "Relay0 ON ");
      result = Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2);}
    break;
    case CH0_OFF_PIN:
      {byte msg[2] = {0,0};
      Oled::drawBigText(0,0, "Relay0 0FF ");
      result = Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2);}
    break;

    //kanał 1
    case CH1_ON_PIN:
      {byte msg[2] = {1,1};
      Oled::drawBigText(0,0, "Relay1 ON ");
      result = Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2);}
    break;
    case CH1_OFF_PIN:
      {byte msg[2] = {1,0};
      Oled::drawBigText(0,0, "Relay1 OFF ");
      result = Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2);}
    break;

    //test
    case TEST_PIN:
      {byte msg[1] = {0};
      Oled::drawBigText(0,0, "Radio Test");
      result = Lora::send_with_ack(RELAY_STATION_ADRESS,msg,1);}
    break;
  }

  Oled::drawBigText(0,20, result?"Success":"Failed");
  delay(1000);

  Oled::clear();
  Oled::sleepDisplay();
  

  esp_deep_sleep_start();
}

void loop() {
  //nic się nie dzieje
}