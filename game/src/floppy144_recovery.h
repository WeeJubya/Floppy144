/*
 * Floppy//144 - recovery screen interface
 *
 * Draws the first screen seen by the player and reflects reconstruction
 * progress from the shared world state.
 */

#pragma once

#include "river2D_main.h"

#include "floppy144_world.h"

#include <stdbool.h>

/*
 * Recovery-screen renderer
 *
 * recovery_started selects the pre-recovery or ready-to-enter message.
 * world supplies the current restoration percentage.
 */

void Floppy144RecoveryDraw(
    EngineData *engine,
    bool recovery_started,
    const Floppy144WorldState *world
);
