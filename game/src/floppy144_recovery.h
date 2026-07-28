#pragma once

#include "river2D_main.h"

#include "floppy144_world.h"

#include <stdbool.h>

void Floppy144RecoveryDraw(
    EngineData *engine,
    bool recovery_started,
    const Floppy144WorldState *world
);
