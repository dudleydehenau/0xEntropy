#include "stats.h"
#include <stdio.h>
#include <math.h> // Utile pour log2f()
#include "pico/stdlib.h"
#include "hardware/pio.h"

// dependances vers les autres modules du projet
#include "Entropy/entropy.h"
#include "Entropy/sha256.h"
#include "Health/health_test.h" // Pour savoir si le capteur est suspect

// ---------------------------------------------------------
// Variables pour le partage de donnees entre les coeurs 0 et 1
// ---------------------------------------------------------
volatile uint32_t hc_compteur_octets[256] = {0};
volatile uint32_t hc_compteur_bits_1 = 0;
volatile uint32_t hc_total_octets = 0;

typedef struct {
    uint32_t compteurs_octets[256];
    uint32_t total_octets;
    uint32_t total_bits_1;
} BlocHistorique;

#define NB_BLOCS_HISTORIQUE 40
#define TAILLE_MAX_BLOC 50000

static BlocHistorique historique[NB_BLOCS_HISTORIQUE] = {0};
static int index_bloc_actuel = 0;

// Variables pour l'affichage dans l'interface
float ui_entropie = 0.0f;
float ui_ratio_bits_1 = 50.0f;
uint32_t ui_debit_o_s = 0;

// Cette fonction permet d'ajouter les donnees aux stats
// Elle est normalement appelee par le Core 1
void stats_add_data(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t octet = data[i];
        hc_compteur_octets[octet]++;
        hc_total_octets++;

        // On compte les bits a 1 dans l'octet
        uint8_t temp = octet;
        while (temp) {
            temp &= (temp - 1);
            hc_compteur_bits_1++;
        }
    }
}

// Calcul des metriques TRNG (execute cote Coeur 0)
void calculer_statistiques_trng(uint32_t delta_temps) {
    // 1. On regarde le debit et le ratio de bits
    if (hc_total_octets > 0) {
        ui_ratio_bits_1 = ((float)hc_compteur_bits_1 / (float)(hc_total_octets * 8)) * 100.0f;
        
        // Si le capteur semble louche, on coupe le debit direct a 0
        if (capteur_suspect) {
            ui_debit_o_s = 0; 
        } else if (delta_temps > 0) {
            ui_debit_o_s = (hc_total_octets * 1000) / delta_temps; // Calcul classique
        }
    } else {
        ui_ratio_bits_1 = 50.0f;
        ui_debit_o_s = 0;
    }

    // 2. On bascule les compteurs dans la fenetre glissante
    for (int i = 0; i < 256; i++) {
        historique[index_bloc_actuel].compteurs_octets[i] += hc_compteur_octets[i];
        hc_compteur_octets[i] = 0; 
    }

    historique[index_bloc_actuel].total_octets += hc_total_octets;
    hc_total_octets = 0; 

    historique[index_bloc_actuel].total_bits_1 += hc_compteur_bits_1;
    hc_compteur_bits_1 = 0; 

    // 3. Rotation du bloc quand il commence a etre bien rempli
    if (historique[index_bloc_actuel].total_octets >= TAILLE_MAX_BLOC) {
        index_bloc_actuel = (index_bloc_actuel + 1) % NB_BLOCS_HISTORIQUE; 
        historique[index_bloc_actuel].total_octets = 0;
        historique[index_bloc_actuel].total_bits_1 = 0;
        for (int i = 0; i < 256; i++) {
            historique[index_bloc_actuel].compteurs_octets[i] = 0;
        }
    }

    // 4. On calcule l'entropie de Shannon sur l'ensemble du buffer
    uint32_t somme_compteurs[256] = {0};
    uint32_t somme_octets = 0;

    for (int b = 0; b < NB_BLOCS_HISTORIQUE; b++) {
        somme_octets += historique[b].total_octets;
        for (int i = 0; i < 256; i++) {
            somme_compteurs[i] += historique[b].compteurs_octets[i];
        }
    }

    if (somme_octets == 0) return; 

    float entropie_temp = 0.0f;
    for (int i = 0; i < 256; i++) {
        if (somme_compteurs[i] > 0) {
            float probabilite = (float)somme_compteurs[i] / (float)somme_octets;
            entropie_temp -= probabilite * log2f(probabilite);
        }
    }
    ui_entropie = entropie_temp;
}

// Remise a zero complete des stats (utile pour le healthcheck)
void stats_reset(void) {
    // On nettoie les compteurs de travail
    hc_compteur_bits_1 = 0;
    hc_total_octets = 0;
    for (int i = 0; i < 256; i++) {
        hc_compteur_octets[i] = 0;
    }

    // On vide tout l'historique
    for (int b = 0; b < NB_BLOCS_HISTORIQUE; b++) {
        historique[b].total_octets = 0;
        historique[b].total_bits_1 = 0;
        for (int i = 0; i < 256; i++) {
            historique[b].compteurs_octets[i] = 0;
        }
    }
    index_bloc_actuel = 0;

    // Et on reset les valeurs de l'interface
    ui_entropie = 0.0f;
    ui_ratio_bits_1 = 50.0f;
    ui_debit_o_s = 0;
}
