#pragma once

#include <Arduino.h>

static unsigned long lastSend = 0;
static unsigned long lastActivity = 0;

extern HardwareSerial lora;

const int MAX_BATCH = 20;          // max VP/VAL per frame

static char lastOut[MAX_BATCH * 4 + 1] = {0};
static int  lastLen = 0;

extern unsigned long lastPingTime;
extern unsigned long lastPongTime;
extern unsigned long lastUserSendTime;

void initLoRa();
void startLoRaTask();

void requestTelemetry();
void sendLoRaBatch();
void sendCommand(uint8_t vp, uint8_t value);
void sendPing();