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
#include <stdint.h>

/*
 * Recovery-screen renderer
 *
 * recovery_started selects the pre-recovery or ready-to-enter message.
 * world supplies the current restoration percentage.
 */

/*
 * Opening splash renderer
 *
 * elapsed_milliseconds drives the disk-flight animation independently of the
 * display refresh rate.
 */

void Floppy144SplashDraw(
    EngineData *engine,
    uint32_t elapsed_milliseconds
);
void Floppy144RecoveryDraw(
    EngineData *engine,
    bool recovery_started,
    const Floppy144WorldState *world
);
