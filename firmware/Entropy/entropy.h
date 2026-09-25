#ifndef ENTROPY_H
#define ENTROPY_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

extern int trng_ratio;

void core1_trng_task(void);

void entropy_init(PIO pio, uint sm, uint pin);
uint32_t entropy_get_data(PIO pio, uint sm);

#endif
