#include <Arduino.h>
#include "config.h"
#include "shared_data.h"
#include "protocol.h"


HardwareSerial& lora = Serial2;
bool allowSend = false;
bool needAck = false;

#define RX_BUF_SIZE 256
char rxBuf[RX_BUF_SIZE];

void initLoRa()
{
    lora.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);
    delay(200);

    lora.println("AT+BAND=868000000");
    delay(150);
    lora.println("AT+ADDRESS=2");
    delay(150);
    lora.println("AT+NETWORKID=5");
    delay(150);
}

void sendPong()
{
    lora.println("AT+SEND=1,4,4101");
}



void processTelemetry()
{
    if (needAck) {
      // VP = 0x42, VAL = 0x01  → "4201"
     // Serial.println("ACK sent!");
      lora.println("AT+SEND=1,4,4201");
      needAck = false;
    }
    if(allowSend){
    
    String payload = "63";
    payload += ((int)qualityByte < 16 ? "0" : "") + String((int)qualityByte, HEX);
    payload += "64";
    payload += (temperatureValue < 16 ? "0" : "") + String(temperatureValue, HEX);
    payload += "61";
    payload += ((int)flowByte < 16 ? "0" : "") + String((int)flowByte, HEX);
    payload += "72";
    payload += ((int)waterLevel < 16 ? "0" : "") + String((int)waterLevel, HEX);
    payload += "62";
    payload += ((int)pressureByte < 16 ? "0" : "") + String((int)pressureByte, HEX);
    if(req_button_states){
      payload += "3101";
    }
    //payload += "4101";

    lora.print("AT+SEND=1,");
    lora.print(payload.length());
    lora.print(",");
    lora.println(payload);
    allowSend = false;
  }
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

      // ==================================================
      // PING VOM SENDER → PONG ANTWORT
      // ==================================================
      if (vp == 0x40)
      {
        lastMessageTime = millis();  // Verbindung lebt
        sendPong();
        count_htbt_tout = 0;
        return;
      }


  // ==================================================
  // SCHALTER-STEUERUNG
  // ==================================================
  switch (vp)
  {
    case 0x65:   // HDP
      //digitalWrite(LED_HDP, val ? HIGH : LOW);
      pcf8575.digitalWrite(P0, val ? LOW : HIGH);
      Serial.print("HDP = ");
      Serial.println(val);
      needAck = true;
      break;

    case 0x66:   // Osmose
      pcf8575.digitalWrite(P1, val ? LOW : HIGH);
      Serial.print("OSMOSE = ");
      Serial.println(val);
      needAck = true;
      break;

    case 0x67:   // Heizung
      pcf8575.digitalWrite(P2, val ? LOW : HIGH);
      Serial.print("HEIZUNG = ");
      Serial.println(val);
      needAck = true;
      break;
    
    case 0x52:
      if (vp == 0x52 && val == 0x01) {
        allowSend = true;
      }
      break;

    case 0x55:   // Reiniger K1 / K2
      pcf8575.digitalWrite(P3, val ? LOW : HIGH);
      Serial.print("Reiniger K1 / K2 = ");
      Serial.println(val);
      needAck = true;
      break;

    case 0x56:   // Chemie Pumpe
      pcf8575.digitalWrite(P4, val ? LOW : HIGH);
      Serial.print("Chemie Pumpe = ");
      Serial.println(val);
      needAck = true;
      break;

    case 0x45:   // Tanken
      pcf8575.digitalWrite(P5, val ? LOW : HIGH);
      Serial.print("Tanken = ");
      Serial.println(val);
      needAck = true;
      break;

    case 0x46:   // Tank Leeren
      pcf8575.digitalWrite(P6, val ? LOW : HIGH);
      Serial.print("Tank Leeren = ");
      Serial.println(val);
      needAck = true;
      break;

    case 0x47:   // Leitung Spühlen
      pcf8575.digitalWrite(P7, val ? LOW : HIGH);
      Serial.print("Leitung Spühlen = ");
      Serial.println(val);
      needAck = true;
      break;
    
    case 0x68:   // Wasser Quelle
      pcf8575.digitalWrite(P8, val ? LOW : HIGH);
      Serial.print("Wasser Quelle = ");
      Serial.println(val);
      needAck = true;
      break;

    default:
      Serial.print("UNBEKANNT VP=");
      Serial.println(vp, HEX);
  }
}
}

void processLoRa()
{
    if (!lora.available())
        return;

    int len = lora.readBytesUntil('\n', rxBuf, RX_BUF_SIZE - 1);
    if (len <= 0)
        return;

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
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void startLoRaTask()
{
    xTaskCreatePinnedToCore(loraTask,"LoRaRX",4096,NULL,2,NULL,1);
}