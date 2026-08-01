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

#define FLOPPY144_TERMINAL_INPUT_CAPACITY 81U
#define FLOPPY144_TERMINAL_OUTPUT_LINES 12U
#define FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY 96U

/*
 * Terminal-local state
 *
 * selected_act tracks the visible archive page. selected_collection tracks
 * the highlighted row within that act. detail_open controls the collection
 * overlay. restoration_notice is a temporary success message.
 */

typedef struct Floppy144TerminalState
{
    Floppy144Act selected_act;
    Floppy144CollectionId selected_collection;

    bool detail_open;
    bool restoration_notice;
    bool suppress_next_character;
    bool exit_requested;
    bool open_record_requested;

    Floppy144CollectionId requested_collection;
    uint32_t requested_record_index;

    char input[FLOPPY144_TERMINAL_INPUT_CAPACITY];
    uint32_t input_length;

    char output
        [FLOPPY144_TERMINAL_OUTPUT_LINES]
        [FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    uint32_t output_count;
} Floppy144TerminalState;

/*
 * Terminal operations
 *
 * These functions reset and navigate the terminal, restore the selected
 * collection, close details, query UI state and draw the complete screen.
 */

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
);

void Floppy144TerminalMoveSelection(
    Floppy144TerminalState *terminal,
    int32_t direction
);

void Floppy144TerminalMoveAct(
    Floppy144TerminalState *terminal,
    int32_t direction
);

void Floppy144TerminalInputCharacter(
    Floppy144TerminalState *terminal,
    char character
);

void Floppy144TerminalBackspace(
    Floppy144TerminalState *terminal
);

void Floppy144TerminalSubmitInput(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world
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
