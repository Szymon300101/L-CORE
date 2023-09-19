#include <Arduino.h>
#include "../lib/oled.h"
#include "../lib/lora/loraTransport.h"

//piny przekaźników
#define RELAY_0 0
#define RELAY_1 2

bool r0_on = false;
bool r1_on = false;

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

  Oled::drawBigText(0,0, "R0: OFF");
  Oled::drawBigText(0,20, "R1: OFF");
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
          {
            r0_on = false;
          }
          else
          {
            r0_on = true;
          }
        }
        else //przekaźnik 1
        {
          if(msg[1] == 0)
          {
            r1_on =  false;
          }
          else
          {
            r1_on = true;
            
          }
        }
        
        Oled::clear();
        Oled::drawBigText(0,0, "R0:");
        Oled::drawBigText(40,0, r0_on?"ON":"OFF");
        Oled::drawBigText(0,20, "R1:");
        Oled::drawBigText(40,20, r1_on?"ON":"OFF");  
        
        digitalWrite(RELAY_0, r0_on);
        digitalWrite(RELAY_1, r1_on);
      }
  }

  delay(100);
}