#include "pico/util/queue.h"

static char event_str[128];

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

extern bool turning, center, black;
extern int blue, space;
extern queue_t facingDirectionQueue,goQueue,victimQueue,startQueue;
extern int8_t go;
extern double absRelAngle;


void initIsr(void);
void gpio_event_string(char *buf, uint32_t events);
void finecorsa_callback(uint gpio, uint32_t events);
void encoder_callback(uint gpio, uint32_t events);
bool messageForRasp(void);