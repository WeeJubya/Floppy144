/*
 * Floppy//144 - reconstructed office interface
 *
 * Contains player movement, collision, proximity tests and rendering for the
 * top-down records office.
 */

#pragma once

#include "f144_runtime.h"

#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Player position
 *
 * Coordinates are measured in the 640x360 logical canvas, not the doubled
 * 1280x720 window.
 */

typedef struct Floppy144Player
{
    int32_t x;
    int32_t y;
} Floppy144Player;

/*
 * Office operations
 *
 * Reset chooses the spawn point. Move applies collision. Near functions
 * gate context-sensitive interactions. Draw renders the complete room.
 */

void Floppy144OfficeReset(
    Floppy144Player *player
);

void Floppy144OfficeMove(
    Floppy144Player *player,
    const Floppy144WorldState *world,
    int32_t movement_x,
    int32_t movement_y
);

Floppy144ObjectId Floppy144OfficeInteractionTarget(
    const Floppy144WorldState *world,
    const Floppy144Player *player
);



void Floppy144OfficeDraw(
    F144Runtime *engine,
    const Floppy144Player *player,
    const Floppy144WorldState *world,
    const char *notice
);
