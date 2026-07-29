#pragma once

#include "river2D_main.h"

#include "floppy144_collection.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144TerminalState
{
    Floppy144CollectionId selected_collection;
    bool detail_open;
    bool restoration_notice;
} Floppy144TerminalState;

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal
);

void Floppy144TerminalMoveSelection(
    Floppy144TerminalState *terminal,
    int32_t direction
);

void Floppy144TerminalOpenSelection(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world
);

void Floppy144TerminalCloseDetail(
    Floppy144TerminalState *terminal
);

bool Floppy144TerminalDetailOpen(
    const Floppy144TerminalState *terminal
);

void Floppy144TerminalDraw(
    EngineData *engine,
    const Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
);
