#include <Arduino.h>

#include "include/config.h"
#include "lib/lora_comm/lora_comm.h"
#include "lib/dwin_display/dwin_display.h"
#include "lib/button_manager/button_manager.h"
#include "lib/heartbeat/heartbeat.h"
#include "include/shared_data.h"

uint8_t flowValue;
uint8_t pressureValue;
uint8_t qualityValue;
uint8_t temperatureValue;
uint8_t levelValue;

bool waitSensor = false;
bool btnAcked = false;

enum TxState {
  TX_IDLE,
  TX_WAIT_BTN_ACK
};

TxState txState = TX_IDLE;
bool req_sw_states = false;
uint32_t btnLastSend = 0;
uint8_t  btnRetryCnt = 0;
const uint32_t BTN_ACK_TIMEOUT = 1000;  // ms
const uint8_t  BTN_MAX_RETRY   = 1;

void setup()
{
    Serial.begin(115200);

    initLoRa();
    initDWIN();
    initHeartbeat();

    startLoRaTask();
    startButtonTask();

    Serial.println("Controller Node Started");
}

void loop()
{
    //  DWIN scan (unchanged logic, but runs regardless)
    if (millis() - lastSend >= 1000) {
        clearFrameQueue();

        for (size_t i = 0; i < 9; i++) {
        dwinReadVP(switches[i]);
        delay(50);          // keep as-is for now
        readDWIN();
        }
        lastSend = millis();
    }

    if (sendbuttons && txState == TX_IDLE) {
        sendLoRaBatch();          // must include seq

        txState = TX_WAIT_BTN_ACK;
        btnLastSend = millis();
        btnRetryCnt = 0;
        sendbuttons = false;
    }

    //   ACK WAIT / RETRY LOGIC
    if (txState == TX_WAIT_BTN_ACK) {

        // ACK received elsewhere (LoRa RX task)
        if (btnAcked) {
        btnAcked = false;
        txState = TX_IDLE;
        }
        // Retry timeout
        else if (millis() - btnLastSend >= BTN_ACK_TIMEOUT) {

        if (btnRetryCnt < BTN_MAX_RETRY) {
            sendLoRaBatch();
            btnLastSend = millis();
            btnRetryCnt++;
        } else {
            // Fail-safe unlock (never deadlock UI)
            txState = TX_IDLE;
        }
        }
        else if(req_sw_states){
        sendLoRaBatch();
        req_sw_states = false;
        }

        // IMPORTANT: while waiting ACK, nothing else is sent
        return;
    }

    // Heartbeat (allowed anytime)
    heartbeatLoop();

    // LOW PRIORITY: SENSOR REQUEST
    if (waitSensor && millis() - lastPingTime >= 1000) {
        lora.println("AT+SEND=2,4,5201");
        lastPingTime = millis();
        waitSensor = false;
    }

}