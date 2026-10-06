/*
 * Floppy//144 - restored Site Directory screen
 *
 * Draws only reconstructed room footprints and their names. Furniture,
 * fixtures, doors, windows and the player are deliberately omitted.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_run_state.h"

void Floppy144SiteDirectoryDraw(
    Floppy144Surface *pRuntime,
    const Floppy144RunState *pRunState
);
