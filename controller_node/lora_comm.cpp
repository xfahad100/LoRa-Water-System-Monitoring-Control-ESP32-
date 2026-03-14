#include <Arduino.h>
#include "include/config.h"
#include "include/protocol.h"
#include "include/shared_data.h"
#include "lib/lora_comm/lora_comm.h"
#include "lib/dwin_display/dwin_display.h"

HardwareSerial lora(2);

unsigned long lastPingTime = 0;
unsigned long lastPongTime = 0;
unsigned long lastUserSendTime = 0;

#define RX_BUF_SIZE 256
char rxBuf[RX_BUF_SIZE];

void initLoRa()
{
    lora.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);

    delay(200);
    lora.println("AT+BAND=868000000");
    delay(150);
    lora.println("AT+ADDRESS=1");
    delay(150);
    lora.println("AT+NETWORKID=5");
}

void sendCommand(uint8_t vp, uint8_t value)
{
    char payload[8];
    sprintf(payload, "%02X%02X", vp, value);

    lora.print("AT+SEND=2,4,");
    lora.println(payload);
}

void requestTelemetry()
{
    sendCommand(VP_REQUEST_DATA, 1);
}

void handlePayload(char *data)
{
    int len = strlen(data);

    for (int i = 0; i < len; i += 4)
    {
        char vpStr[3];
        char valStr[3];

        memcpy(vpStr, data + i, 2);
        memcpy(valStr, data + i + 2, 2);

        vpStr[2] = 0;
        valStr[2] = 0;

        uint8_t vp = strtol(vpStr, NULL, 16);
        uint8_t val = strtol(valStr, NULL, 16);

        switch (vp)
        {
            case VP_FLOW: flowValue = val; break;
            case VP_PRESSURE: pressureValue = val; break;
            case VP_TDS: qualityValue = val; break;
            case VP_TEMP: temperatureValue = val; break;
            case VP_LEVEL: levelValue = val; break;
        }
    }

    updateDWIN();
}

void handleLine(char *msg)
{
    char *p = msg + 5;          // skip "+RCV="

    // skip address
    p = strchr(p, ',');
    if (!p) return;

    // get payload length
    int len = atoi(p + 1);
    if (len < 2) return;        // must be at least 1 VP/VAL
    if (len % 2 != 0) return;   // must be even (VP+VAL)

    // skip to payload start
    p = strchr(p + 1, ',');
    if (!p) return;
    p++;

    // ---- loop through each VP/VAL pair ----
    for (int i = 0; i < len/2; i += 2)
    {
        char hexStr[3] = {p[i * 2], p[i * 2 + 1], '\0'};
        uint8_t vp = strtoul(hexStr, NULL, 16);

        char hexStr2[3] = {p[i * 2 + 2], p[i * 2 + 3], '\0'};
        uint8_t val = strtoul(hexStr2, NULL, 16);

        if (vp == 0x41 && val == 0x01)
          {
          //Serial.print("VP value:");
          //Serial.println(val);
          lastPongTime = millis();
          return;
          //Serial.println("[PONG OK]");
          }
        
        switch(vp)
        {
          case 0x31:
          {
            req_sw_states = true;
          }
          case 0x42:
          {
            btnAcked = true;
            //Serial.println("switch ACK");
            break;
          }
          case 0x61:   // Flow
          {
            float flowRate = val * 30.0 / 255.0;  // 1-byte mapped from LoRa
            //Serial.print("Flow: ");
            //Serial.println(flowRate, 2);          // 2 decimal places
            uint16_t dwinValue_2 = (uint16_t)(flowRate * 100);  // scale for 2 decimals
            sendToDWIN(vp, dwinValue_2);            // send 2105
            break;
          }
          case 0x62:
          {
            float pressure = val * 10.0 / 255.0;
            //Serial.print("pressure value:");
            //Serial.println(pressure);
            uint16_t dwinValue = (uint16_t)(pressure * 100);
            sendToDWIN(vp, dwinValue);
            break;
          }
          case 0x63:
          {
            //Serial.print("tds value:");
            float tdsval = val * 400.0 / 255.0;
            //Serial.println(tdsval);
            sendToDWIN(vp, (uint16_t)tdsval);
            break;
          }
          case 0x64:
            //Serial.print("temp value:");
            //Serial.println(val);
            sendToDWIN(vp, val);
            break;
          case 0x72:
            //Serial.print("level value:");
            //Serial.println(val);
            sendToDWIN(vp, val);
            break;
        }
    }
}


void processLoRa() {
  if (!lora.available()) return;

  int len = lora.readBytesUntil('\n', rxBuf, RX_BUF_SIZE - 1);
  if (len <= 0) return;

  rxBuf[len] = '\0';

  if (strncmp(rxBuf, "+RCV=", 5) == 0) {
    //Serial.println(rxBuf);
    handleLine(rxBuf);
  }
}

void loraTask(void *pvParameters)
{
    while (true)
    {
        processLoRa();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void startLoRaTask()
{
    xTaskCreatePinnedToCore(loraTask, "LoRaRX", 4096, NULL, 2, NULL, 1);
}

void sendLoRaBatch()
{
    char out[MAX_BATCH * 4 + 1];
    int pos = 0;
    int count = 0;

    Frame f;
    while (count < MAX_BATCH && dequeue(f)) {
        pos += sprintf(out + pos, "%02X%02X", f.vp, f.val);
        count++;
    }

    // If new data arrived → update persistent batch
    if (count > 0) {
        pos += sprintf(out + pos, "%02X%02X", 0x40, 0x01);
        count++;
        memcpy(lastOut, out, pos);
        lastOut[pos] = '\0';
        lastLen = count * 4;   // bytes
        lastActivity = millis();
    }

    // Always send last known batch
    if (lastLen > 0 && (millis() - lastActivity <= 0)) {
        lora.println("AT+SEND=2," + String(lastLen) + "," + String(lastOut));
        lastPingTime = millis();
        Serial.printf("SENT LORA: %s\n", lastOut);
        // WICHTIG: Heartbeat jetzt blockieren
        lastUserSendTime = millis();
    }
}

void sendPing()
{
  lora.println("AT+SEND=2,4,4001");
 // Serial.println("[PING SEND]");
  lastPingTime = millis();
}
