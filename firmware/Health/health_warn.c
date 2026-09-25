#include "health_warn.h"

// faut que alerte_sante_trng soit bien declaree dans ce header sinon ca va planter
#include "Health/health_test.h"

// les trucs de base pour le pico (pwm, gpio, watchdog et compagnie)
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"

// drivers pour l'ecran waveshare et la gestion du tactile
#include "UI/Screen/Driver/DEV_Config.h"
#include "UI/Screen/Driver/LCD_1in69.h"
#include "UI/Screen/Driver/Touch_1in69.h"
#include "UI/Screen/Screen_app.h"

#define BUZZER_PIN 19

// un petit flag pour eviter que l'alarme se relance en boucle
bool alarme_declenchee = false;

// la fonction qui verifie si ca a pete et balance l'alerte
void Check_et_alerte(UWORD *image_buffer) {
    if (alerte_sante_trng && !alarme_declenchee) {
        alarme_declenchee = true;

        // on met la luminosite a fond
        DEV_SET_PWM(100); 

        // on dessine l'ecran de la mort en rouge si le buffer est pret
        if (image_buffer != NULL) {
            dessiner_page_fatal(image_buffer);
            LCD_1IN69_Display(image_buffer); 
        }

        // on configure le buzzer pour qu'il siffle a environ 1.25 khz
        gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
        uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
        pwm_set_clkdiv(slice_num, 10.0f); 
        pwm_set_wrap(slice_num, 10000); 
        pwm_set_chan_level(slice_num, pwm_gpio_to_channel(BUZZER_PIN), 5000); 
        pwm_set_enabled(slice_num, true);

        bool son_active = true;

        // boucle infinie pour bloquer le systeme et attendre un appui tactile
        while(true) {
            watchdog_update(); // on oublie pas de nourrir le chien pour eviter le reboot

            if (son_active) {
                // on regarde si quelqu'un touche l'ecran
                if (DEV_Digital_Read(DEV_I2C_INT) == 0) {
                    Touch_1IN69_XY struct_tc = Touch_1IN69_Get_Point();
                    if (struct_tc.x_point != 0 && struct_tc.y_point != 0) {

                        // tactile detecte : on coupe le pwm
                        pwm_set_enabled(slice_num, false);

                        // on remet la pin en sortie normale a 0 pour etre sur que ca se taise
                        gpio_set_function(BUZZER_PIN, GPIO_FUNC_SIO);
                        gpio_set_dir(BUZZER_PIN, GPIO_OUT);
                        gpio_put(BUZZER_PIN, 0);

                        son_active = false;
                    }
                }
            }
            sleep_ms(100); // petite pause pour pas faire chauffer le processeur
        }
    }
}
