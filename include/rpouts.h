#include <stdint.h>

int initOuts();
void out_setpins_P(int pins);
void out_setpins_N(int pins);
void turnLEDs(int leds);
//uint32_t out_getpins();
int setPinVal(int pin, int value, volatile uint32_t ** addr, uint32_t *val);