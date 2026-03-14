#include <Arduino.h>
#include "include/config.h"
#include "include/shared_data.h"
#include "lib/dwin_display/dwin_display.h"

HardwareSerial dwin(1);

Frame q[QUEUE_SIZE];
int qHead = 0;
int qTail = 0;

const uint8_t switches[] = {
  0x65,   // HDP
  0x66,   // Osmose
  0x67,   // Heizung
  0x55,   // Reiniger K1 / K2
  0x56,   // Chemie Pumpe
  0x45,   // Tanken
  0x46,   // Tank Leeren
  0x47,   // Leitung Spühlen
  0x68   // Wasser Quelle
};

uint8_t switchState[9]     = {0};
uint8_t lastSwitchState[9] = {0};
volatile bool sendbuttons = false;

void initDWIN()
{
    dwin.begin(115200, SERIAL_8N1, DWIN_RX, DWIN_TX);
}

void sendToDWIN(uint16_t vp, uint16_t value)
{
    uint8_t buf[8];

    buf[0] = 0x5A;
    buf[1] = 0xA5;
    buf[2] = 0x05;
    buf[3] = 0x82;
    buf[4] = vp >> 8;
    buf[5] = vp & 0xFF;
    buf[6] = value >> 8;
    buf[7] = value & 0xFF;

    dwin.write(buf, 8);
}

void dwinAllSwitchesOff()
{
    for (uint8_t i = 0; i < sizeof(switches) / sizeof(switches[0]); i++) {
        uint8_t cmd[] = {
            0x5A, 0xA5,
            0x05,        // length
            0x82,        // write VP
            switches[i], // 8-bit VP
            0x00, 0x00,  // value = OFF
            0x00         // padding (LSB)
        };
        dwin.write(cmd, sizeof(cmd));
        delay(100);  // small gap to avoid UART overflow
    }
}

void updateDWIN()
{
    sendToDWIN(0x2000, flowValue);
    sendToDWIN(0x2001, pressureValue);
    sendToDWIN(0x2002, qualityValue);
    sendToDWIN(0x2003, temperatureValue);
    sendToDWIN(0x2004, levelValue);
}

bool enqueue(Frame f) {
  int next = (qTail + 1) % QUEUE_SIZE;
  if (next == qHead) return false; // queue full
  q[qTail] = f;
  qTail = next;
  return true;
}

bool dequeue(Frame &f) {
  if (qHead == qTail) return false; // empty
  f = q[qHead];
  qHead = (qHead + 1) % QUEUE_SIZE;
  return true;
}

void clearFrameQueue()
{
    Frame dummy;
    while (dequeue(dummy)) {
        // just discard
    }
}

void dwinReadVP(uint8_t vp)
{
    uint8_t cmd[] = {
        0x5A, 0xA5,        // DGUS header
        0x04,              // length
        0x83,              // write VP
        vp,                // VP high byte
        0x00,              // VP low byte
        0x01         // VALUE low byte
    };
    dwin.write(cmd, sizeof(cmd));
}

// READ DWIN → SEND SWITCH
void readDWIN()
{
  if (!dwin.available()) return;

  static uint8_t buf[128];
  int len = 0;

  while (dwin.available() && len < (int)sizeof(buf)) {
    buf[len++] = (uint8_t)dwin.read();
  }

  int pos = 0;
  while (pos + 9 <= len)
  {
    if (buf[pos] == 0x5A && buf[pos + 1] == 0xA5)
    {
      uint8_t frameLen = buf[pos + 2];
      int size = frameLen + 3;

      if (pos + size > len) break;

      uint16_t vp  = buf[pos + 4];
      uint16_t val = buf[pos + 8];

      if (vp == 0x4F) return;

      // SWITCH CHANGE DETECTION (TOP PRIORITY)
      for (int i = 0; i < 9; i++) {
        if (vp == switches[i]) {

          switchState[i] = val;

          if (switchState[i] != lastSwitchState[i]) {
            lastSwitchState[i] = switchState[i];
            sendbuttons = true;   // SET HERE (ONLY HERE)
           // Serial.printf("Switch %d changed -> %d\n", i, val);
          }
          break;
        }
      }

      // Existing queue logic stays untouched
      Frame f;
      f.vp  = vp;
      f.val = val;

      if (enqueue(f)) {
       // Serial.printf("Queued -> VP=0x%04X  VAL=%d\n", vp, val);
      } else {
        Serial.println("Queue FULL, dropping frame!");
      }

      pos += size;
    }
    else {
      pos++;
    }
  }
}