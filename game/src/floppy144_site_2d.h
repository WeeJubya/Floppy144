/*
 * Floppy//144 - scrolling flat 2D Site projection
 *
 * Renders the projection-neutral Site into the runtime backbuffer using a
 * fixed zoom and a player-following camera. The camera derives its viewport
 * from the backbuffer dimensions, so it can use 640 x 360 today or 640 x 480
 * later without changing world geometry.
 */

#pragma once

#include "f144_runtime.h"
#include "floppy144_run_state.h"

void Floppy144Site2DDraw(
    F144Runtime *runtime,
    const Floppy144RunState *run_state,
    const char *notice
);
