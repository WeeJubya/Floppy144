/*
 * Floppy//144 - reusable secure-cabinet access and Cabinet Interior view
 *
 * Secure cabinets are discovered from generated FURNITURE records. The module
 * owns only transient screen state; persistent unlocks live in RunState.
 */

#pragma once

#include "f144_runtime.h"

#include "floppy144_game_data.h"
#include "floppy144_run_state.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_CABINET_ID_CAPACITY       64U
#define FLOPPY144_CABINET_DISPLAY_CAPACITY  72U
#define FLOPPY144_CABINET_CODE_CAPACITY      8U
#define FLOPPY144_CABINET_INTERACTION_RANGE  2U

typedef struct Floppy144CabinetState
{
    char szCabinetId[FLOPPY144_CABINET_ID_CAPACITY];
    char szDisplayName[FLOPPY144_CABINET_DISPLAY_CAPACITY];

    uint8_t uCabinetOrdinal;
    uint8_t uRequiredDigits;
    uint8_t uInputLength;

    char szInput[FLOPPY144_CABINET_CODE_CAPACITY + 1U];

    uint32_t uSelectedContent;

    bool bInteriorOpen;
    bool bDetailOpen;

    const char *pszStatus;
}
Floppy144CabinetState;

void Floppy144CabinetReset(
    Floppy144CabinetState *pCabinet
);

/*
 * Resolve the nearest generated secure cabinet in the player's current room.
 * Opening an already-unlocked cabinet goes directly to Cabinet Interior.
 */
bool Floppy144CabinetOpenNearby(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
);

const char *Floppy144CabinetId(
    const Floppy144CabinetState *pCabinet
);

uint8_t Floppy144CabinetRequiredDigits(
    const Floppy144CabinetState *pCabinet
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
 * Submit the keypad entry.
 *
 * The canonical data currently records whether the player knows the code and
 * how secure access progresses, but it does not yet contain literal numeric
 * cabinet codes. Stage 3B.5 therefore validates knowledge state plus the
 * authored 4/6/8-digit phase contract. Numeric equality can be added later by
 * attaching literal values to the cabinet data without changing this screen.
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
    F144Runtime *pRuntime,
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
);
