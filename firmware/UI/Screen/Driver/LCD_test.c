#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h> // Pour malloc et free
#include <stdint.h> // Pour uint32_t

#include "LCD_1in69.h"
#include "Touch_1in69.h"
#include "GUI_Paint.h"
#include "DEV_Config.h"

// On remplace 'int main()' par une fonction appelable
void run_lcd_test(void) {
    printf("Debut du test Ecran Tactile...\r\n");

    // 1. Initialisation du matériel (SPI, I2C, GPIO)
    if (DEV_Module_Init() != 0) {
        printf("Erreur d'initialisation materielle!\r\n");
        return; // On utilise return tout court au lieu de return -1 car c'est un 'void'
    }

    // 2. Initialisation de l'écran LCD
    LCD_1IN69_Init(HORIZONTAL);
    LCD_1IN69_Clear(WHITE);

    // Activer le rétroéclairage (à commenter si DEV_SET_PWM n'existe pas)
    DEV_SET_PWM(100); 

    // 3. Initialisation du Tactile (CST816T / CST816D)
    Touch_1IN69_init(0);

    // 4. Configuration de la zone de dessin (Paint)
    // CORRECTION ICI : uint32_t pour éviter le dépassement de mémoire (overflow)
    uint32_t Imagesize = (uint32_t)LCD_1IN69_HEIGHT * LCD_1IN69_WIDTH * 2;
    UWORD *BlackImage;
    if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Erreur d'allocation memoire...\r\n");
        return;
    }

    Paint_NewImage((UBYTE *)BlackImage, LCD_1IN69_WIDTH, LCD_1IN69_HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_0);

    // Titre de base
    Paint_DrawString_EN(10, 10, "Test Tactile", &Font16, WHITE, BLACK);
    LCD_1IN69_Display(BlackImage);

    // 5. Boucle principale
    while (true) {
        // La broche IRQ (DEV_I2C_INT) passe à l'état bas quand on touche l'écran
        if (DEV_Digital_Read(DEV_I2C_INT) == 0) {

            // Lecture des coordonnées I2C
            Touch_1IN69_XY struct_tc;
            struct_tc.mode = 0;
            Touch_1IN69_Get_Point(&struct_tc);

            // Si les coordonnées sont valides
            if (struct_tc.x_point != 0 && struct_tc.y_point != 0) {
                // Effacer l'ancienne zone de texte (pour ne pas superposer)
                Paint_ClearWindows(10, 50, 240, 100, WHITE);

                // Préparer le texte des coordonnées
                char coord_str[32];
                sprintf(coord_str, "X: %03d Y: %03d", struct_tc.x_point, struct_tc.y_point);

                // Dessiner le texte sur l'image en mémoire
                Paint_DrawString_EN(10, 50, coord_str, &Font20, WHITE, BLUE);

                // Envoyer l'image à l'écran
                LCD_1IN69_DisplayWindows(10, 50, 240, 100, BlackImage);

                printf("Touche detectee -> X: %d, Y: %d\r\n", struct_tc.x_point, struct_tc.y_point);
            }
        }
        sleep_ms(20); // Petite pause pour ne pas saturer le bus I2C
    }

    free(BlackImage);
}
