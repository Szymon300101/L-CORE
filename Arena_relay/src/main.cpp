#include <Arduino.h>
#include "../lib/oled.h"
#include "../lib/lora/loraTransport.h"

#define RELAY_0 0
#define RELAY_1 2

#define REMOTE_ADRESS 0

void setup() {
  Serial.begin(9600);

  Oled::init();

  pinMode(RELAY_0, OUTPUT);
  pinMode(RELAY_1, OUTPUT);

  if(Lora::Radio::init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");
  Lora::Encryption::init();
  Lora::start_radio();


}

void loop() {

  uint8_t msg[MAX_FRAME_SIZE]; //MAX_FRAME_SIZE to parametr w pliku lora/params/frame_spec.h, określający max wielkość ramki
  uint8_t msg_size;
  uint8_t addr;

  //sprawdzenie nadchodzących pakietów.
  int received = Lora::try_receive(&addr,msg,&msg_size);

  //sprawdzenie czy coś przyszło. status RECEIVED oznacza nową wiadomość
  if(received == Lora::RECEIVED && addr == REMOTE_ADRESS)
  {
      //można teraz na przykład wypisać otrzymaną wiadomość
      Serial.print("Received packet: ");
      for(int i=0;i<msg_size;i++)
      {
          Serial.print(msg[i]);
          Serial.print(" ");
      }

      if(msg_size > 1)
      {
        if(msg[0] == 0)
        {
          if(msg[1] == 0)
            digitalWrite(RELAY_0, LOW);
          else
            digitalWrite(RELAY_0, HIGH);
        }else
        {
          if(msg[1] == 0)
            digitalWrite(RELAY_1, LOW);
          else
            digitalWrite(RELAY_1, HIGH);
        }
      }
  }

  delay(100);
}