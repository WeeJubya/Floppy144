/*
 * Floppy//144 - persistent world state
 *
 * Holds facts that survive while the player moves between screens.
 * UI-only state belongs in the terminal or catalogue structures instead.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * Session facts
 *
 * Each collection has a restoration flag and an authored-evidence flag.
 * Restoration changes what exists in the reconstructed site. Reading an
 * evidence document changes what the player knows about those objects.
 */

typedef struct Floppy144WorldState
{
    bool hr02_restored;
    bool hr02_desk_reallocation_read;

    bool fa03_restored;
    bool fa03_suppression_service_read;
} Floppy144WorldState;

/*
 * World-state operations
 *
 * Reset starts a new session. ReconstructionPercent converts restored
 * collections into the percentage displayed by the recovery interface.
 */

void Floppy144WorldReset(
    Floppy144WorldState *world
);

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
);
