/*
 * Floppy//144 - reusable parent-contents view with secure-cabinet access
 *
 * Furniture and fixtures can expose their recovered physical children through
 * one transient contents screen. Secure cabinets add a keypad gate; persistent
 * unlocks remain in RunState.
 */

#pragma once

#include "floppy144_draw.h"
#include "f144_startup_config.h"

#include "floppy144_game_data.h"
#include "floppy144_run_state.h"
#include "floppy144_takeaway.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_CABINET_ID_CAPACITY       64U
#define FLOPPY144_CABINET_DISPLAY_CAPACITY  72U
#define FLOPPY144_CABINET_TYPE_CAPACITY     40U
#define FLOPPY144_CABINET_CODE_CAPACITY      8U
/*
 * Site proximity is owned by Floppy144SiteFocusedParentId(). Secure cabinets
 * deliberately have no independent interaction halo.
 */

typedef struct Floppy144CabinetState
{
    char szCabinetId[FLOPPY144_CABINET_ID_CAPACITY];
    char szDisplayName[FLOPPY144_CABINET_DISPLAY_CAPACITY];
    char szContainerType[FLOPPY144_CABINET_TYPE_CAPACITY];

    uint8_t uCabinetOrdinal;
    uint8_t uRequiredDigits;
    uint8_t uInputLength;

    char szInput[FLOPPY144_CABINET_CODE_CAPACITY + 1U];

    uint32_t uSelectedContent;

    bool bInteriorOpen;
    bool bDetailOpen;
    bool bSecureContainer;

    const char *pszStatus;

    /* Transient seasonal flyer, never part of persistent/generated PI lists. */
    Floppy144DataRecord sContextualFlyer;
    const char *pszContextualAnnotation;

    /*
     * P-330 presentation override, separate from canonical authored data.
     * Valid for this transient Cabinet screen only. On reload/open, rebuild
     * solely from the persistent recovery_seed.
     */
    Floppy144DataRecord sGeneratedTakeaway;
    char szTakeawayText[FLOPPY144_TAKEAWAY_MENU_CAPACITY];
}
Floppy144CabinetState;

void Floppy144CabinetReset(
    Floppy144CabinetState *pCabinet
);

/*
 * Presentation-only progression gate. The existing schematic remains active
 * until Main Office reconstruction; thereafter Inspection may use the richer
 * pseudo-isometric parent representation.
 */
bool Floppy144CabinetEnhancedPresentationUnlocked(
    const Floppy144RunState *pRunState
);

/*
 * Resolve a generated secure cabinet only when that cabinet owns the current
 * Site proximity focus. Opening an already-unlocked cabinet goes directly to
 * Cabinet Interior.
 */
bool Floppy144CabinetOpenNearby(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
);

/*
 * Open the reusable contents view for one generated furniture/fixture parent.
 *
 * This is the non-secure path used by desks, tables, shelves, worktops,
 * fridges, panels and other parents which own physical items. Secure storage
 * continues to enter through Floppy144CabinetOpenNearby() and its keypad.
 */
bool Floppy144CabinetOpenParent(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    const char *pszParentId
);

/*
 * Append one date-selected atmospheric flyer to a reconstructed Noticeboard.
 * All existing permanent physical children retain their IDs/content/order.
 * Invoked after successful OpenParent; a failed calendar query leaves the
 * historical contents exactly as before.
 */
void Floppy144CabinetSetNoticeboardDate(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    const F144CalendarDate *pDate
);

const char *Floppy144CabinetId(
    const Floppy144CabinetState *pCabinet
);

uint8_t Floppy144CabinetCodeDigits(
    const Floppy144CabinetState *pCabinet
);

uint8_t Floppy144CabinetRequiredDigits(
    const Floppy144CabinetState *pCabinet
);

void Floppy144CabinetExpectedCode(
    const Floppy144CabinetState *pCabinet,
    uint32_t uRecoverySeed,
    char *pszCode,
    uint32_t uCapacity
);

bool Floppy144CabinetCodeKnown(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
);

bool Floppy144CabinetUnlocked(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
);

bool Floppy144CabinetInteriorOpen(
    const Floppy144CabinetState *pCabinet
);

bool Floppy144CabinetDetailOpen(
    const Floppy144CabinetState *pCabinet
);

bool Floppy144CabinetInputDigit(
    Floppy144CabinetState *pCabinet,
    char chDigit
);

void Floppy144CabinetClearInput(
    Floppy144CabinetState *pCabinet
);

/*
 * Submit one exact deterministic code. Knowledge state controls disclosure,
 * not whether a correctly guessed physical code can operate the lock.
 */
bool Floppy144CabinetSubmitCode(
    Floppy144CabinetState *pCabinet,
    Floppy144WorldState *pWorld,
    Floppy144RunState *pRunState
);

uint32_t Floppy144CabinetVisibleContentCount(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
);

const Floppy144DataRecord *Floppy144CabinetVisibleContentAt(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    uint32_t uVisibleIndex
);

void Floppy144CabinetMoveSelection(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    int32_t nDelta
);

bool Floppy144CabinetInspectSelected(
    Floppy144CabinetState *pCabinet,
    Floppy144WorldState *pWorld,
    Floppy144RunState *pRunState
);

/*
 * Consume one Backspace layer. False means the coordinator should return to
 * Site exploration. This gives: detail -> interior -> room.
 */
bool Floppy144CabinetBackspace(
    Floppy144CabinetState *pCabinet
);

void Floppy144CabinetDraw(
    Floppy144Surface *pRuntime,
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
);
