#ifndef SCREEN_APP_H
#define SCREEN_APP_H

#include <stdint.h>
#include "UI/Screen/Driver/GUI_Paint.h"

typedef enum {
    PAGE_DASHBOARD,
    PAGE_REGLAGES,
    PAGE_ABOUT
} PageSysteme;

void dessiner_logo_entropy(uint16_t largeur_ecran, uint16_t hauteur_ecran);
void dessiner_icone_fleche(uint16_t x_centre, uint16_t y_centre, uint16_t couleur);
void dessiner_icone_curseurs(uint16_t x_centre, uint16_t y_centre, uint16_t couleur, uint16_t couleur_fond);
void dessiner_page_dashboard(UWORD *BlackImage);
void dessiner_page_reglages(UWORD *BlackImage);
void dessiner_page_about(UWORD *BlackImage);
void dessiner_page_fatal(UWORD *BlackImage);

#endif