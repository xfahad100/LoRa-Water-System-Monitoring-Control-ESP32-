#include <Arduino.h>
#include "include/config.h"
#include "include/shared_data.h"
#include "lib/lora_comm/lora_comm.h"
#include "lib/dwin_display/dwin_display.h"

unsigned long lastPing = 0;
const unsigned long HEARTBEAT_INTERVAL   = 1000;
const unsigned long HEARTBEAT_TIMEOUT    = 30000;
const unsigned long USER_SILENCE_TIME    = 0;  // solange muss Ruhe sein


void initHeartbeat()
{
    pinMode(HEARTBEAT_LED, OUTPUT);
}

void heartbeatLoop()
{
  unsigned long now = millis();

  // Schalteraktivität → Heartbeat PAUSE
  if (now - lastUserSendTime < USER_SILENCE_TIME)
  {
    Serial.println("user silence");
    digitalWrite(HEARTBEAT_LED, LOW);
    return;
  }

  // Ping senden?
  if (!waitSensor && now - lastPingTime >= HEARTBEAT_INTERVAL){
   // Serial.println("htbt interval");
    sendPing();
    waitSensor = true;
  }
  // Heartbeat-Anzeige
  if (now - lastPongTime <= HEARTBEAT_TIMEOUT){
    digitalWrite(HEARTBEAT_LED, (now / 500) % 2);
  }
  else{
    digitalWrite(HEARTBEAT_LED, LOW);
    Serial.println("htbt tout");
    dwinAllSwitchesOff();
  }
}