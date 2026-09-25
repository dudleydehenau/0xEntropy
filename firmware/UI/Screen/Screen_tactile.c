#include "pico/stdlib.h"
#include <stdbool.h>

#include "Driver/DEV_Config.h"
#include "Driver/Touch_1in69.h"
#include "Stats/stats.h" 
#include "Screen_app.h"        
#include "Screen_tactile.h"

// Recup des variables definies ailleurs
extern PageSysteme page_actuelle;       
extern bool demander_rafraichissement;
extern int trng_ratio;                  

static uint32_t dernier_toucher = 0; 
static bool ecran_allume = true;

bool always_on = false; 

// On part sur 15 secondes avant de couper l'ecran
#define DELAI_VEILLE_MS 15000 

void gerer_tactile(void) {
    uint32_t maintenant = to_ms_since_boot(get_absolute_time());

    // Gestion de la mise en veille auto
    if (!always_on && ecran_allume && (maintenant - dernier_toucher >= DELAI_VEILLE_MS)) {
        DEV_SET_PWM(0);         
        ecran_allume = false;
    }

    // On regarde si on detecte un contact sur la dalle
    if (DEV_Digital_Read(DEV_I2C_INT) == 0) {
        Touch_1IN69_XY struct_tc = Touch_1IN69_Get_Point();
        
        // Si les coordonnees sont valides (pas 0,0)
        if (struct_tc.x_point != 0 && struct_tc.y_point != 0) {
            
            dernier_toucher = maintenant;
            
            // Si l'ecran etait eteint, le premier clic ne sert qu'a le rallumer
            if (!ecran_allume) {
                DEV_SET_PWM(100);       
                ecran_allume = true;
                sleep_ms(300); // Petit delai pour eviter un clic fantome au reveil         
                return;                 
            }

            // --- Logique de navigation entre les pages ---

            if (page_actuelle == PAGE_DASHBOARD) {
                // Zone du bouton reglages sur le dashboard
                if (struct_tc.x_point >= 130 && struct_tc.x_point <= 240 &&
                    struct_tc.y_point >= 190 && struct_tc.y_point <= 280) {
                    page_actuelle = PAGE_REGLAGES; 
                    demander_rafraichissement = true; 
                }
            } 
            else if (page_actuelle == PAGE_REGLAGES) {
                // Bouton retour en bas a gauche
                if (struct_tc.x_point >= 0 && struct_tc.x_point <= 90 &&
                    struct_tc.y_point >= 240 && struct_tc.y_point <= 280) {
                    page_actuelle = PAGE_DASHBOARD; 
                    demander_rafraichissement = true; 
                }
                // Selection des differents ratios (ligne horizontale)
                else if (struct_tc.y_point >= 110 && struct_tc.y_point <= 150) {
                    if (struct_tc.x_point >= 20 && struct_tc.x_point <= 65) {
                        trng_ratio = 2; stats_reset(); demander_rafraichissement = true;
                    } else if (struct_tc.x_point >= 75 && struct_tc.x_point <= 120) {
                        trng_ratio = 4; stats_reset(); demander_rafraichissement = true;
                    } else if (struct_tc.x_point >= 130 && struct_tc.x_point <= 175) {
                        trng_ratio = 8; stats_reset(); demander_rafraichissement = true;
                    } else if (struct_tc.x_point >= 185 && struct_tc.x_point <= 230) {
                        trng_ratio = 16; stats_reset(); demander_rafraichissement = true;
                    }
                }
                // Les boutons RESET et ABOUT au milieu
                else if (struct_tc.y_point >= 155 && struct_tc.y_point <= 225) {
                    if (struct_tc.x_point <= 135) {
                        stats_reset(); 
                        page_actuelle = PAGE_DASHBOARD;
                        demander_rafraichissement = true;
                    }
                    else if (struct_tc.x_point > 135) {
                        page_actuelle = PAGE_ABOUT; 
                        demander_rafraichissement = true; 
                    }
                }
                // Switch pour le mode Always On (ecran toujours allume)
                else if (struct_tc.y_point >= 230 && struct_tc.y_point <= 280) {
                    if (struct_tc.x_point >= 160) {
                        always_on = !always_on;
                        demander_rafraichissement = true;
                    }
                }

            }
            else if (page_actuelle == PAGE_ABOUT) {
                // N'importe quel clic sur ABOUT ramene aux reglages
                page_actuelle = PAGE_REGLAGES; 
                demander_rafraichissement = true; 
            }

            // On attend un peu pour pas compter 50 clics d'un coup
            sleep_ms(200); 
        }
    }
}
