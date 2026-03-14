#include <Arduino.h>
#include "lib/lora_comm/lora_comm.h"
#include "include/protocol.h"

void buttonTask(void *pvParameters)
{
    while (true)
    {
        // read DWIN buttons here
        // if button pressed:
        // sendCommand(VP_HDP,1);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void startButtonTask()
{
    xTaskCreatePinnedToCore(buttonTask,"ButtonTask",4096,NULL,1,NULL,1);
}