#include <Arduino.h>
#include "config.h"
#include "shared_data.h"

void flowTask(void *pvParameters)
{
    while (true)
    {
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= interval)
        {
            uint32_t pulse1Sec = pulseCount;
            pulseCount = 0;

            flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / 6.6;
            previousMillis = currentMillis;

            uint32_t flowMilliLitres = (flowRate / 60.0) * 1000.0;
            totalMilliLitres += flowMilliLitres;
        }

        vTaskDelay(10);
    }
}

void initFlowSensor()
{
    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, RISING);
}

void startFlowTask()
{
    xTaskCreatePinnedToCore(flowTask,"FlowTask",4096,NULL,1,NULL,1);
}

float getFlowRate()
{
    return flowRate;
}

uint32_t getTotalMilliLitres()
{
    return totalMilliLitres;
}