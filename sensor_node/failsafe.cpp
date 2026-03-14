#include <Arduino.h>
#include "shared_data.h"
#include "config.h"

unsigned long lastMessageTime = 0;
const unsigned long MESSAGE_TIMEOUT = 30000;

void initFailsafe()
{
    lastMessageTime = millis();
}

void failsafeCheck()
{
  unsigned long now = millis();
  if (now - lastMessageTime > MESSAGE_TIMEOUT)
  {
    count_htbt_tout++;
    if(count_htbt_tout > 5){
    pcf8575.digitalWrite(P0, HIGH);
    pcf8575.digitalWrite(P1, HIGH);
    pcf8575.digitalWrite(P2, HIGH);
    pcf8575.digitalWrite(P3, HIGH);
    pcf8575.digitalWrite(P4, HIGH);
    pcf8575.digitalWrite(P5, HIGH);
    pcf8575.digitalWrite(P6, HIGH);
    pcf8575.digitalWrite(P7, HIGH);
    pcf8575.digitalWrite(P8, HIGH);
    pcf8575.digitalWrite(P9, HIGH);

    digitalWrite(LED_HB, LOW);
    Serial.println("htbt timeout!");
    req_button_states = true;
    }
    return;
  }

  // Heartbeat visualisieren
  digitalWrite(LED_HB, (now / 500) % 2);
}