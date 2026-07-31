/*
 * Floppy//144 - archive terminal interface
 *
 * Owns terminal navigation and collection-detail UI state. Permanent facts
 * such as restored collections are written to Floppy144WorldState.
 */

#pragma once

#include "river2D_main.h"

#include "floppy144_collection.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Terminal-local state
 *
 * selected_collection tracks the highlighted row. detail_open controls
 * the collection overlay. restoration_notice is a temporary success message.
 */

typedef struct Floppy144TerminalState
{
    Floppy144CollectionId selected_collection;
    bool detail_open;
    bool restoration_notice;
} Floppy144TerminalState;

/*
 * Terminal operations
 *
 * These functions reset and navigate the terminal, restore the selected
 * collection, close details, query UI state and draw the complete screen.
 */

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

bool Floppy144TerminalCanOpenCatalogue(
    const Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
);

bool Floppy144TerminalDetailOpen(
    const Floppy144TerminalState *terminal
);

void Floppy144TerminalDraw(
    EngineData *engine,
    const Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
);
