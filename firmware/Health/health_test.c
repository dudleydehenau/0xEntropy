#include "health_test.h"
#include "pico/stdlib.h"
#include "pico/time.h" // Utile pour gerer le timeout de 10 secondes

volatile bool alerte_sante_trng = false;
volatile bool capteur_suspect = false;

// On garde une trace des stats sur une petite fenetre
static uint32_t total_ones_acc = 0;
static uint32_t total_bits_acc = 0;
#define HEALTH_CHECK_WINDOW 8192 // On analyse par paquets de 1024 octets

// Pour gerer le chrono du mode defaut
static bool en_defaut = false;
static absolute_time_t debut_defaut_time;

bool verifier_sante_flux_brut(const uint32_t *raw_buffer, size_t mots_a_lire) {
    uint32_t raw_ones = 0;

    // Comptage des bits a '1' dans le bloc qui vient d'arriver
    for (size_t i = 0; i < mots_a_lire; i++) {
        raw_ones += __builtin_popcount(raw_buffer[i]);
    }

    total_ones_acc += raw_ones;
    total_bits_acc += (mots_a_lire * 32);

    // Une fois qu'on a assez de matiere, on fait le bilan
    if (total_bits_acc >= HEALTH_CHECK_WINDOW) {
        uint32_t total_zeros_acc = total_bits_acc - total_ones_acc;

        // --- TEST : est-ce que le flux est naze ? (moins de 5% de 1 ou de 0) ---
        if (total_ones_acc < (total_bits_acc / 20) || total_zeros_acc < (total_bits_acc / 20)) {
            
            if (!en_defaut) {
                // C'est le début d'un souci, on lance le chrono
                en_defaut = true;
                debut_defaut_time = get_absolute_time();
            } else {
                // Ca continue de deconner... on regarde si ca depasse les 10 secondes
                if (absolute_time_diff_us(debut_defaut_time, get_absolute_time()) >= 10000000) {
                    alerte_sante_trng = true; // Alerte rouge declenchee
                    return false;
                }
            }
        } else {
            // Tout va bien, on reset l'état de defaut s'il y en avait un
            en_defaut = false;
        }

        // On remet les compteurs a zero pour la prochaine analyse
        total_ones_acc = 0;
        total_bits_acc = 0;
    }

    return true;
}
