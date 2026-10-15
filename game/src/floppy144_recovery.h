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
 * Five stationary menu rows. Profile / Settings occupy rows three and four
 * initially; Left/Right slides Save / Load into those same two row positions.
 * No additional pages, menu rows or new persistence mechanisms.
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

#define FLOPPY144_MAIN_MENU_SLIDE_MS 240U

typedef struct Floppy144MainMenuSlide
{
    bool save_load_visible;
    bool previous_save_load_visible;
    bool animating;
    int32_t direction;
    uint32_t elapsed_ms;
} Floppy144MainMenuSlide;

/* Left and Right toggle the same two central rows, moving the old
 * labels in the direction of the key and bringing new labels from
 * the opposite screen edge. Other three rows never move. */
void Floppy144MainMenuSlideReset(Floppy144MainMenuSlide *slide);
bool Floppy144MainMenuSlideStart(Floppy144MainMenuSlide *slide,
    int32_t direction);
bool Floppy144MainMenuSlideAdvance(Floppy144MainMenuSlide *slide,
    uint32_t elapsed_ms);
Floppy144MainMenuOption Floppy144MainMenuVisibleOption(
    Floppy144MainMenuOption row, bool save_load_visible);

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

/* Existing Floppy144MainMenuDraw remains the default static-profile
 * entry point for legacy renderer clients. Only the actual menu
 * coordinator supplies animation state. */
void Floppy144MainMenuDrawSliding(
    Floppy144Surface *runtime,
    Floppy144MainMenuOption selected_option,
    bool active_session,
    bool recorded_session_available,
    const Floppy144RunState *run_state,
    const Floppy144RunState *recorded_run_state,
    const char *persistence_warning,
    const char *menu_notice,
    bool menu_notice_is_warning,
    bool reinstate_confirmation,
    const Floppy144MainMenuSlide *slide,
    const char *operator_name
);
