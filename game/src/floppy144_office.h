#pragma once

#include "river2D_main.h"

#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144Player
{
    int32_t x;
    int32_t y;
} Floppy144Player;

void Floppy144OfficeReset(
    Floppy144Player *player
);

void Floppy144OfficeMove(
    Floppy144Player *player,
    int32_t movement_x,
    int32_t movement_y
);

bool Floppy144OfficeNearTerminal(
    const Floppy144Player *player
);

void Floppy144OfficeDraw(
    EngineData *engine,
    const Floppy144Player *player,
    const Floppy144WorldState *world
);
