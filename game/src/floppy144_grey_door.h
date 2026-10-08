/*
 * S4G-02: derived Grey Door wall candidates.
 * Overlay only; authoritative generated Site geometry and collision are untouched.
 */
#pragma once
#include "floppy144_run_state.h"
#include "floppy144_site.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144GreyDoorCandidate
{
    Floppy144SiteRect rect; /* visual-only closed doorway in wall plane */
    int32_t stand_x16;     /* validated normal interaction foot position */
    int32_t stand_y16;
} Floppy144GreyDoorCandidate;

/* Candidates are enumerated from current corridor floors, not authored coordinates. */
uint32_t Floppy144GreyDoorCandidateCount(void);
bool Floppy144GreyDoorCandidateAt(uint32_t index, Floppy144GreyDoorCandidate *out);
bool Floppy144GreyDoorCandidateSafe(const Floppy144GreyDoorCandidate *candidate);
bool Floppy144GreyDoorForRun(
    const Floppy144RunState *state, Floppy144GreyDoorCandidate *out
);
bool Floppy144GreyDoorNearby(const Floppy144RunState *state);
