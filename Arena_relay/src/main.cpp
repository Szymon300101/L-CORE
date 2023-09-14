#include <Arduino.h>
#include "../lib/oled.h"
#include "../lib/lora/loraTransport.h"

//piny przekaźników
#define RELAY_0 0
#define RELAY_1 2

//adres radiowy pilota, od którego ma odbierać komendy
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
  //interesują nas tylko wiadomości od pilota
  if(received == Lora::RECEIVED && addr == REMOTE_ADRESS)
  {
      Serial.print("Received packet: ");
      for(int i=0;i<msg_size;i++)
      {
          Serial.print(msg[i]);
          Serial.print(" ");
      }

      //wiadomość o długości 2 oznacza polecenie przełączenia
      if(msg_size > 1)
      {
        if(msg[0] == 0) //przekaźnik 0
        {
          if(msg[1] == 0)
            digitalWrite(RELAY_0, HIGH);
          else
            digitalWrite(RELAY_0, LOW);
        }
        else //przekaźnik 1
        {
          if(msg[1] == 0)
            digitalWrite(RELAY_1, HIGH);
          else
            digitalWrite(RELAY_1, LOW);
        }
      }
  }

  delay(100);
}