#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"
#include "hardware/pwm.h"

// Mes modules pour les stats et l'entropie
#include "Stats/stats.h"
#include "Entropy/entropy.h"

// Le reste des trucs pour l'interface et le hardware
#include "UI/Melody/melody.h"
#include "UI/Screen/Driver/LCD_1in69.h"
#include "UI/Screen/Driver/Touch_1in69.h"
#include "UI/Screen/Driver/GUI_Paint.h"
#include "UI/Screen/Driver/DEV_Config.h"
#include "UI/Screen/Screen_app.h"
#include "Health/health_test.h"
#include "Health/health_warn.h"
#include "UI/Screen/Screen_tactile.h"

#define BUZZER_PIN 19

// Variables pour le tactile et la navigation
Touch_1IN69_XY XY; 
PageSysteme page_actuelle = PAGE_DASHBOARD; 
bool demander_rafraichissement = true; 

// ==========================================

int main() {
    stdio_init_all();

    // Apres 2 jours de debug, fin du biais de l'octets 10 générant un octets 13 pour signaler le retour a la ligne que provoque SDK quand on transmet donnée par port série ... une ligne ....
    stdio_set_translate_crlf(&stdio_usb, false); 

    UWORD *BlackImage = NULL;

    // --- On prepare l'ecran ---
    if (DEV_Module_Init() == 0) {
        DEV_SET_PWM(0); // On commence dans le noir pour eviter flash blanc d'initialisation
        LCD_1IN69_Init(VERTICAL);
        LCD_1IN69_Clear(BLACK); 
        Touch_1IN69_init(1); 

        // Calcul de la taille du buffer pour l'image
        uint32_t Imagesize = (uint32_t)LCD_1IN69_HEIGHT * LCD_1IN69_WIDTH * 2;
        BlackImage = (UWORD *)malloc(Imagesize);

        if(BlackImage != NULL) {
            // Configuration de la zone de dessin
            Paint_NewImage((UBYTE *)BlackImage, LCD_1IN69_WIDTH, LCD_1IN69_HEIGHT, 0, WHITE);
            Paint_SetScale(65);
            Paint_Clear(BLACK); 
            Paint_SetRotate(ROTATE_0); 

            // Petit logo au demarrage pour faire propre
            dessiner_logo_entropy(LCD_1IN69_WIDTH, LCD_1IN69_HEIGHT);
            LCD_1IN69_Display(BlackImage);
            DEV_SET_PWM(100); // On allume le retroeclairage
        } else {
            printf("Gros souci : impossible d'allouer la memoire pour l'image\r\n");
        }
    }

    // Petite musique d'intro (qui laisse les condensateurs se charger au passage le temps de l'éxécution)
    play_star_trek(BUZZER_PIN);
    
    watchdog_enable(2000, 1);

    // On lance la generation de nombres aleatoires sur le second coeur
    multicore_launch_core1(core1_trng_task);

    uint32_t dernier_calcul = to_ms_since_boot(get_absolute_time());
    bool alarme_declenchee = false;

    while (true) {

        // On donne signe de vie au watchdog
        watchdog_update();

        // Verification des alertes de sante du systeme
        Check_et_alerte(BlackImage);

        // --- Gestion des stats toutes les 2 secondes environ ---
        uint32_t temps_actuel = to_ms_since_boot(get_absolute_time());
        uint32_t delta_temps = temps_actuel - dernier_calcul;

        if (delta_temps > 2000) {
            calculer_statistiques_trng(delta_temps); 
            dernier_calcul = temps_actuel;

            // Si on est sur l'ecran principal, faut mettre a jour les chiffres
            if (page_actuelle == PAGE_DASHBOARD) {
                demander_rafraichissement = true;
            }
        }

        // Si on n'a pas d'image en memoire, on ne peut rien faire
        if (BlackImage == NULL) continue;

        // --- Mise a jour de l'affichage ---
        if (demander_rafraichissement) {
            Paint_Clear(BLACK); // On nettoie avant de redessiner

            // Choix du contenu selon la page ou on se trouve
            if (page_actuelle == PAGE_DASHBOARD) {
                dessiner_page_dashboard(BlackImage); 
            } else if (page_actuelle == PAGE_REGLAGES) {
                dessiner_page_reglages(BlackImage);
            } else if (page_actuelle == PAGE_ABOUT) {
                dessiner_page_about(BlackImage); 
            }
            
            // On envoie le tout a l'ecran
            LCD_1IN69_Display(BlackImage);
            demander_rafraichissement = false; 
        }

        // On regarde si l'utilisateur touche l'ecran
        gerer_tactile();

        // Petite pause pour laisser le processeur souffler
        sleep_ms(1); 
    }
}
