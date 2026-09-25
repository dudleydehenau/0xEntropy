#include "entropy.h"
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "noise.pio.h" // genere par le fichier pio

// mes modules
#include "Health/health_test.h"
#include "Stats/stats.h"
#include "sha256.h" // pour le blanchiment

#define PIN_BRUIT 16

// petite variable pour regler le ratio depuis le main
int trng_ratio = 2;

// --- fonctions pour piloter le pio ---
void entropy_init(PIO pio, uint sm, uint pin) {
    uint offset = pio_add_program(pio, &noise_program);
    noise_program_init(pio, sm, offset, pin, 11.0f);
}

uint32_t entropy_get_data(PIO pio, uint sm) {
    return pio_sm_get_blocking(pio, sm);
}

// la tache principale qui tourne sur le second coeur
void core1_trng_task(void) {
    PIO pio = pio0;
    uint sm = 0;

    // on lance la lecture du bruit materiel
    entropy_init(pio, sm, PIN_BRUIT);

    // buffers pour le sha256
    uint8_t buffer_brut[32] = {0}; // initialise a zero
    uint8_t hash_out[32]; // le sha sort toujours 32 octets
    int bit_index = 0;
    int byte_index = 0;

    while (true) {
        // on recupere un bit
        uint32_t raw_data = entropy_get_data(pio, sm);
        uint8_t bit = raw_data & 0x01;

        // on remplit le buffer petit a petit
        if (bit) {
            buffer_brut[byte_index] |= (1 << bit_index);
        } else {
            buffer_brut[byte_index] &= ~(1 << bit_index);
        }

        bit_index++;
        if (bit_index >= 8) {
            bit_index = 0;
            byte_index++;

            // quand on a nos 32 octets (256 bits)
            if (byte_index >= 32) {
                byte_index = 0;

                // check rapide pour voir si le capteur est pas debranche
                // si on a que des 0 ou que des 1, c'est louche
                bool ligne_morte = true;
                for(int i = 1; i < 32; i++) {
                    if(buffer_brut[i] != buffer_brut[0]) {
                        ligne_morte = false;
                        break;
                    }
                }
                
                // si c'est pas du 0x00 ou du 0xFF pur, c'est bon
                if (buffer_brut[0] != 0x00 && buffer_brut[0] != 0xFF) {
                    ligne_morte = false;
                }

                // tests de sante sur le flux brut
                verifier_sante_flux_brut((const uint32_t *)buffer_brut, 8);

                // on traite et on envoie seulement si tout a l'air ok
                if (!capteur_suspect && !ligne_morte) {
                    
                    sha256_hash(buffer_brut, 32, hash_out);
                    
                    stats_add_data(hash_out, 32);

                    // on balance le resultat sur la sortie standard
                    fwrite(hash_out, 1, 32, stdout);
                    fflush(stdout); 
                }
                else {
                    // si probleme, on remet les indicateurs a plat
                    ui_ratio_bits_1 = 0.0f;
                    ui_entropie = 0.0f;  
                }
            }
        }

        // petite pause base sur le ratio de l'ui
        sleep_us(trng_ratio);
    }
}
