#pragma once

#include <Arduino.h>
#include "PCF8575.h"

extern volatile uint8_t qualityByte;
extern volatile uint8_t flowByte;
extern volatile uint8_t pressureByte;
extern volatile float waterLevel;

extern int temperatureValue;

extern unsigned long lastMessageTime;

extern PCF8575 pcf8575;

extern bool req_button_states;

extern int count_htbt_tout;

// Flow sensor variables
extern volatile uint32_t pulseCount;
extern float flowRate;
extern int totalMilliLitres;
extern unsigned long previousMillis;
extern const int interval;

extern void countPulse();