/*
 * Floppy//144 - GDR main-menu interface
 *
 * Draws the session-control screen. The startup intro lives in
 * floppy144_intro.c so it remains pure presentation state.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_run_state.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Main-menu options
 *
 * The five visible entries are the original menu layout. Save and Load are
 * reachable through Right and Left arrow confirmation shortcuts, respectively;
 * their existing handlers and persistence codecs remain in place.
 */

typedef enum Floppy144MainMenuOption
{
    FLOPPY144_MAIN_MENU_INITIATE_SESSION = 0,
    FLOPPY144_MAIN_MENU_RETURN_TO_SITE,
    FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,
    FLOPPY144_MAIN_MENU_SETTINGS,
    FLOPPY144_MAIN_MENU_TERMINATE,
    FLOPPY144_MAIN_MENU_OPTION_COUNT,
    /* Historical manual save/reinstate actions remain compiled but are
     * intentionally excluded from the five selectable original menu rows. */
    FLOPPY144_MAIN_MENU_RECORD_SESSION,
    FLOPPY144_MAIN_MENU_REINSTATE_SESSION
} Floppy144MainMenuOption;

/*
 * Report whether a menu option is currently available.
 */

bool Floppy144MainMenuOptionEnabled(
    Floppy144MainMenuOption option,
    bool active_session,
    bool recorded_session_available
);

/* Left = Load, Right = Save. Unavailable returns OPTION_COUNT. These
 * shortcuts do not add rows/pages or modify the manual save format. */
Floppy144MainMenuOption Floppy144MainMenuShortcutForDirection(
    int32_t direction,
    bool active_session,
    bool recorded_session_available
);

/*
 * GDR session-control menu renderer.
 */
void Floppy144RecoveryFormatCapacity(
    const Floppy144RunState *pState,
    char *pszBuffer,
    uint32_t uBufferCapacity
);

void Floppy144DifficultyDraw(Floppy144Surface *surface,
    Floppy144Difficulty selected);

void Floppy144MainMenuDraw(
    Floppy144Surface *runtime,
    Floppy144MainMenuOption selected_option,
    bool active_session,
    bool recorded_session_available,
    const Floppy144RunState *run_state,
    const Floppy144RunState *recorded_run_state,
    const char *persistence_warning,
    const char *menu_notice,
    bool menu_notice_is_warning,
    bool reinstate_confirmation
);
