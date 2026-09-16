/*
 * Floppy//144 - archive terminal interface
 *
 * Owns terminal navigation and collection-detail UI state. Permanent facts
 * such as restored collections are written to Floppy144WorldState.
 */

#pragma once

#include "f144_runtime.h"

#include "floppy144_collection.h"
#include "floppy144_world.h"

#include "floppy144_run_state.h"

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_TERMINAL_INPUT_CAPACITY 81U
#define FLOPPY144_TERMINAL_OUTPUT_LINES 12U
#define FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY 96U

/*
 * Terminal-local state
 *
 * selected_domain tracks the visible archive domain. selected_collection
 * tracks the highlighted row within that domain. detail_open controls the
 * collection overlay. restoration_notice is a temporary success message.
 */

typedef struct Floppy144TerminalState
{
    Floppy144CollectionDomain selected_domain;
    Floppy144CollectionId selected_collection;

    bool detail_open;
    bool restoration_notice;
    bool suppress_next_character;
    bool exit_requested;
    bool open_record_requested;
    bool record_pager_active;
    bool help_pager_active;

    /*
     * Stage 3A terminal capability state.
     *
     * open_command_available prevents OPEN being advertised before the
     * initial recovery collection has been restored. The default-record
     * collection is deliberately terminal-session-local: restoring a
     * collection makes RS-#### shorthand resolve against that collection,
     * but leaving the terminal clears that convenience context.
     */
    bool open_command_available;
    bool default_record_collection_valid;

    Floppy144CollectionId requested_collection;
    Floppy144CollectionId default_record_collection;

    Floppy144CollectionId record_pager_collection;
    uint32_t record_pager_page;
    uint32_t help_pager_page;
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

/*
 * Reset a terminal entered from the physical Site and identify its room in
 * the first transcript line. The fixed screen header is unchanged.
 */
void Floppy144TerminalResetAtRoom(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world,
    Floppy144RoomId room
);

void Floppy144TerminalMoveSelection(
    Floppy144TerminalState *terminal,
    int32_t direction
);

void Floppy144TerminalMoveDomain(
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

bool Floppy144TerminalRecordPagerActive(
    const Floppy144TerminalState *terminal
);

void Floppy144TerminalMoveRecordPager(
    Floppy144TerminalState *terminal,
    int32_t direction
);

void Floppy144TerminalCloseRecordPager(
    Floppy144TerminalState *terminal
);

bool Floppy144TerminalHelpPagerActive(
    const Floppy144TerminalState *terminal
);

void Floppy144TerminalMoveHelpPager(
    Floppy144TerminalState *terminal,
    int32_t direction
);

void Floppy144TerminalCloseHelpPager(
    Floppy144TerminalState *terminal
);

void Floppy144TerminalSubmitInput(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world,
    Floppy144RunState *run_state
);

/*
 * Append the next data-derived recovery action to the terminal transcript.
 * The same helper is used after terminal commands and after a document closes,
 * keeping the Prologue hand-off consistent without hard-coded story branches.
 */
void Floppy144TerminalPrintNextAction(
    Floppy144TerminalState *pTerminal,
    const Floppy144RunState *pRunState
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
    F144Runtime *runtime,
    const Floppy144TerminalState *terminal,
    const Floppy144RunState *run_state
);;
