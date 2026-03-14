#pragma once

void initDWIN();
void updateDWIN();
void sendToDWIN(uint16_t vp, uint16_t value);
void dwinReadVP(uint8_t vp);
void readDWIN();
void clearFrameQueue();

struct Frame {
  uint16_t vp;
  uint16_t val;
};

// queue/dequeue

#define QUEUE_SIZE 50

extern Frame q[QUEUE_SIZE];
extern int qHead;
extern int qTail;

bool enqueue(Frame f);
bool dequeue(Frame &f);

void dwinAllSwitchesOff();

extern const uint8_t switches[];

extern uint8_t switchState[9];
extern uint8_t lastSwitchState[9];
extern volatile bool sendbuttons;