#include "pico/util/queue.h"
#define TILE_LENGTH 300
#define TILE_LENGTH_IMP 360
static char event_str[128];

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

extern bool turning, center, black,victim,start,inTile,justFinecorsaed, justEntered,justTurned,send,stop;
extern int blue, space;
extern queue_t facingDirectionQueue,goQueue,GoQueue,victimQueue,startQueue, turningQueue, pathQueue, impQueue;
extern double absRelAngle;
extern int path[30];
extern uint8_t currNTile[2];
extern int imp;
extern uint32_t timertime, oldtimertime, oldbluetime, maybeInTile;
extern double time,oldtime,tileTime,wholeTileTime;
extern int8_t sensnum, go, Go, finecorsa;
extern int provDist, blue, space, caselle;
extern int delay, indeX;

void initIsr(void);
void gpio_event_string(char *buf, uint32_t events);
void gpio_callback(uint gpio, uint32_t events);
bool messageForRasp(void);
void messageForRaspSV(void);
void reset_imp(void);
void shift_imp(int);