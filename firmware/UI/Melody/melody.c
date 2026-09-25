//Code inspiré du Github https://github.com/robsoncouto/arduino-songs

#include "melody.h"
#include <stdlib.h>
#include "hardware/pwm.h"

// Definition des frequences des notes utilisees
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587

void play_star_trek(uint buzzer_pin) {
    // on regle la vitesse du morceau
    int tempo = 80;

    int melody[] = {
      NOTE_D4, -8, NOTE_G4, 16, NOTE_C5, -4, 
      NOTE_B4, 8, NOTE_G4, -16, NOTE_E4, -16, NOTE_A4, -16,
      NOTE_D5, 2
    };

    // calcul du nombre de notes et de la duree d'une ronde
    int notes = sizeof(melody) / sizeof(melody[0]) / 2;
    int wholenote = (60000 * 4) / tempo;

    // initialisation du PWM sur la broche du buzzer
    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    uint chan = pwm_gpio_to_channel(buzzer_pin);

    // boucle pour jouer chaque note l'une apres l'autre
    for (int thisNote = 0; thisNote < notes * 2; thisNote += 2) {
        
        // on determine la duree de la note
        int divider = melody[thisNote + 1];
        int noteDuration = (divider > 0) ? (wholenote / divider) : ((wholenote / abs(divider)) * 1.5);
        int freq = melody[thisNote];

        if (freq > 0) {
            // calcule le diviseur pour l'horloge a 125MHz
            uint32_t pwm_divider = 125000000 / (freq * 10000); 
            pwm_set_clkdiv(slice_num, pwm_divider);
            pwm_set_wrap(slice_num, 10000);
            pwm_set_chan_level(slice_num, chan, 5000);
            pwm_set_enabled(slice_num, true);
        } else {
            // si la frequence est 0, c'est un silence
            pwm_set_enabled(slice_num, false); 
        }

        // élape de lecture de la note
        sleep_ms(noteDuration * 0.9);
        
        // on coupe le son un court instant pour bien separer les notes
        pwm_set_enabled(slice_num, false);
        sleep_ms(noteDuration * 0.1);
    }
}
