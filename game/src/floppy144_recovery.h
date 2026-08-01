/*
 * Floppy//144 - splash and GDR main-menu interface
 *
 * Draws the opening presentation and the session-control screen.
 */

#pragma once

#include "river2D_main.h"

#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Main-menu options
 *
 * Save and load remain represented from the beginning, even while their
 * implementation is unavailable.
 */

typedef enum Floppy144MainMenuOption
{
    FLOPPY144_MAIN_MENU_INITIATE_SESSION = 0,
    FLOPPY144_MAIN_MENU_RETURN_TO_SITE,
    FLOPPY144_MAIN_MENU_RECORD_SESSION,
    FLOPPY144_MAIN_MENU_REINSTATE_SESSION,
    FLOPPY144_MAIN_MENU_TERMINATE,
    FLOPPY144_MAIN_MENU_OPTION_COUNT
} Floppy144MainMenuOption;

/*
 * Report whether a menu option is currently available.
 */

bool Floppy144MainMenuOptionEnabled(
    Floppy144MainMenuOption option,
    bool active_session
);

/*
 * Opening splash renderer.
 */

void Floppy144SplashDraw(
    EngineData *engine,
    uint32_t elapsed_milliseconds
);

/*
 * GDR session-control menu renderer.
 */

void Floppy144MainMenuDraw(
    EngineData *engine,
    Floppy144MainMenuOption selected_option,
    bool active_session,
    const Floppy144WorldState *world
);