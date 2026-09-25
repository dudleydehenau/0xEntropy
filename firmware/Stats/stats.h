#ifndef STATS_H
#define STATS_H

#include <stdint.h>
#include <stddef.h>

extern float ui_entropie;
extern float ui_ratio_bits_1;
extern uint32_t ui_debit_o_s;

void stats_reset(void);

void stats_add_data(const uint8_t *data, size_t len);

void calculer_statistiques_trng(uint32_t delta_temps);

#endif // STATS_H
