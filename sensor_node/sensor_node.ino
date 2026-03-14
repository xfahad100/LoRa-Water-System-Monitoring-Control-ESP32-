#include <Arduino.h>

#include "config.h"
#include "lib/sensors/sensors.h"
#include "lib/flow_sensor/flow_sensor.h"
#include "lib/lora_comm/lora_comm.h"
#include "lib/failsafe/failsafe.h"
#include "shared_data.h"

volatile uint8_t qualityByte;
volatile uint8_t flowByte;
volatile uint8_t pressureByte;
volatile float waterLevel;

int temperatureValue;

bool req_button_states = false;

int count_htbt_tout = 0;

// Flow sensor variables
volatile uint32_t pulseCount = 0;
float flowRate = 0.0;
int totalMilliLitres = 0;
unsigned long previousMillis = 0;
const int interval = 1000;

void IRAM_ATTR countPulse() {
    pulseCount++;
}

void setup()
{
    Serial.begin(115200);

    initSensors();
    initFlowSensor();
    initLoRa();
    initFailsafe();

    Serial.println("ESP32 Sensor Node Started");

    startFlowTask();
    startSensorTask();
    startLoRaTask();
}

void loop()
{
    processTelemetry();
    failsafeCheck();
}