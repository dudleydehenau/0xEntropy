#ifndef HEALTH_TEST_H
#define HEALTH_TEST_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern volatile bool alerte_sante_trng;
extern volatile bool capteur_suspect;

bool verifier_sante_flux_brut(const uint32_t *raw_buffer, size_t mots_a_lire);

#endif
