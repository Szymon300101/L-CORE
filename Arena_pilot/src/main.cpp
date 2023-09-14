#include <Arduino.h>
#include "../lib/oled.h"
#include "../lib/lora/loraTransport.h"

#define CH0_ON_PIN    12
#define CH0_OFF_PIN   13
#define CH1_ON_PIN    14
#define CH1_OFF_PIN   15
#define TEST_PIN      34

#define RELAY_STATION_ADRESS 1

void setup() {
  Serial.begin(9600);

  Oled::init();

  //pinMode(23, INPUT_PULLDOWN);

  if(Lora::Radio::init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");
  Lora::Encryption::init();
  Lora::start_radio();
  //esp_sleep_enable_ext0_wakeup((gpio_num_t) 12,1);
  //esp_sleep_enable_ext0_wakeup((gpio_num_t) 13,1);
  esp_sleep_enable_ext1_wakeup(0x40000F000,ESP_EXT1_WAKEUP_ANY_HIGH);


}

void loop() {
  Serial.println("im up/");
  //Serial.println("Going to sleep");
  delay(100);
  
  //esp_light_sleep_start();

  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  uint16_t pressed_pin = (log(GPIO_reason))/log(2);
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println(pressed_pin, 0);

  switch(pressed_pin)
  {
    case CH0_ON_PIN:
      {byte msg[2] = {0,1};
      Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));}
    break;
    case CH0_OFF_PIN:
      {byte msg[2] = {0,0};
      Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));}
    break;
    case CH1_ON_PIN:
      {byte msg[2] = {1,1};
      Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));}
    break;
    case CH1_OFF_PIN:
      {byte msg[2] = {1,0};
      Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));}
    break;
    case TEST_PIN:
      {byte msg[1] = {0};
      if(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,1) > 0)
      {
        Serial.println("Test: ok");
      }else
      {
        Serial.println("Test: failed");
      }
      
      
      }
    break;
  }

  delay(1000);
  
  esp_deep_sleep_start();
}