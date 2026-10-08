/*
 * GREY DOOR REPUBLIC: temporary non-persistent vignette.
 * No GDR world geometry, Profile, capacity or collection references exist here.
 */
#pragma once
#include "floppy144_draw.h"
#include "floppy144_run_state.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum Floppy144GreyEncounterPhase {
    FLOPPY144_GREY_ENTERING=0,
    FLOPPY144_GREY_EXPLORE,
    FLOPPY144_GREY_IDENTIFY,
    FLOPPY144_GREY_TURN,
    FLOPPY144_GREY_DIALOGUE,
    FLOPPY144_GREY_CAPACITY,
    FLOPPY144_GREY_GLITCH,
    FLOPPY144_GREY_DONE
} Floppy144GreyEncounterPhase;

typedef struct Floppy144GreyEncounter {
    uint32_t elapsed_ms;
    int32_t local_x;
    int32_t local_y;
    int32_t return_x16;
    int32_t return_y16;
    uint8_t phase;
    uint8_t developer_inspected;
} Floppy144GreyEncounter;

/* Purely ephemeral: Begin requires an already reachable discovered Door. */
bool Floppy144GreyEncounterBegin(
    Floppy144GreyEncounter *scene, const Floppy144RunState *run
);
bool Floppy144GreyEncounterMove(
    Floppy144GreyEncounter *scene, int32_t dx, int32_t dy
);
bool Floppy144GreyEncounterInspect(Floppy144GreyEncounter *scene);
bool Floppy144GreyEncounterAdvance(
    Floppy144GreyEncounter *scene, uint32_t elapsed_ms
);
bool Floppy144GreyEncounterFinished(const Floppy144GreyEncounter *scene);
bool Floppy144GreyEncounterSaveAllowed(const Floppy144GreyEncounter *scene);
void Floppy144GreyEncounterDraw(
    Floppy144Surface *surface, const Floppy144GreyEncounter *scene
);
