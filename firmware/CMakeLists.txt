# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================

# On definit la carte par defaut (pico classique)
set(PICO_BOARD pico CACHE STRING "Board type")

# Import du SDK obligatoire
include(pico_sdk_import.cmake)

project(0xEntropy C CXX ASM)

pico_sdk_init()

# 1. Liste des fichiers source pour l'executable

add_executable(0xEntropy 
    # Le point d'entree du programme
    0xEntropy.c

    # Pour tout ce qui est calculs et stats
    Stats/stats.c

    # Tests de bon fonctionnement du systeme
    Health/health_test.c
    Health/health_warn.c

    # La partie generation d'entropie (TRNG)
    Entropy/entropy.c
    Entropy/sha256.c

    # Gestion du son et des melodies
    UI/Melody/melody.c

    # Les drivers pour l'ecran et la couche tactile
    UI/Screen/Driver/LCD_1in69.c
    UI/Screen/Driver/Touch_1in69.c
    UI/Screen/Driver/GUI_Paint.c
    UI/Screen/Driver/GUI_Paint.h
    UI/Screen/Driver/DEV_Config.c
    UI/Screen/Screen_app.c
    UI/Screen/Screen_tactile.c

    # Toutes les polices de caracteres pour l'affichage
    UI/Screen/Fonts/font12.c
    UI/Screen/Fonts/font16.c
    UI/Screen/Fonts/font20.c
    UI/Screen/Fonts/font24.c
)

# On donne un petit nom et une version au projet
pico_set_program_name(0xEntropy "0xEntropy")
pico_set_program_version(0xEntropy "0.1")

# 2. On s'occupe de la generation du PIO

pico_generate_pio_header(0xEntropy ${CMAKE_CURRENT_LIST_DIR}/Entropy/noise.pio)

# 3. On lie les bibliotheques necessaires

target_link_libraries(0xEntropy 
    pico_stdlib
    pico_multicore
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_adc
    hardware_dma
    hardware_pio
    hardware_timer
    hardware_watchdog
    m  # Petit ajout pour les fonctions maths
)

# 4. Inclusions et reglages des entrees/sorties

# On declare ou chercher les headers
target_include_directories(0xEntropy PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# L'UART classique ne nous sert pas ici, on le coupe
pico_enable_stdio_uart(0xEntropy 0)

# Par contre on active bien l'USB pour avoir le retour console
pico_enable_stdio_usb(0xEntropy 1)

# On dit au SDK d'aller voir ici pour la config USB faite a la main
target_include_directories(0xEntropy PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Pour finir, on demande la creation des fichiers flashables (uf2, etc.)
pico_add_extra_outputs(0xEntropy)
