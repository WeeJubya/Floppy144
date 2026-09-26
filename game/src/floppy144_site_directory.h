/*
 * Floppy//144 - restored Site Directory screen
 *
 * Draws only reconstructed room footprints and their names. Furniture,
 * fixtures, doors, windows and the player are deliberately omitted.
 */

#pragma once

#include "f144_runtime.h"
#include "floppy144_run_state.h"

void Floppy144SiteDirectoryDraw(
    F144Runtime *pRuntime,
    const Floppy144RunState *pRunState
);
