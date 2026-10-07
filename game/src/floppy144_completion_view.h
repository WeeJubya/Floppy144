/*
 * FLOPPY//144 bespoke completion presentation.
 *
 * Presentation-only view over the already-finalised RunState/Profile snapshot.
 */
#pragma once

#include "floppy144_draw.h"
#include "floppy144_profile.h"
#include "floppy144_run_state.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum Floppy144CompletionOption
{
    FLOPPY144_COMPLETION_OPTION_FINAL_NOTE = 0,
    FLOPPY144_COMPLETION_OPTION_CREDITS,
    FLOPPY144_COMPLETION_OPTION_MAIN_MENU,
    FLOPPY144_COMPLETION_OPTION_COUNT
}
Floppy144CompletionOption;

typedef enum Floppy144CompletionPage
{
    FLOPPY144_COMPLETION_PAGE_SUMMARY = 0,
    FLOPPY144_COMPLETION_PAGE_FINAL_NOTE
}
Floppy144CompletionPage;

typedef struct Floppy144CompletionViewState
{
    uint8_t page;
    uint8_t selected_option;
    uint16_t reserved;
    uint32_t note_top_line;
}
Floppy144CompletionViewState;

void Floppy144CompletionViewReset(
    Floppy144CompletionViewState *state
);

void Floppy144CompletionViewMoveSelection(
    Floppy144CompletionViewState *state,
    int32_t direction
);

Floppy144CompletionOption Floppy144CompletionViewSelectedOption(
    const Floppy144CompletionViewState *state
);

bool Floppy144CompletionViewFinalNoteOpen(
    const Floppy144CompletionViewState *state
);

void Floppy144CompletionViewOpenFinalNote(
    Floppy144CompletionViewState *state
);

void Floppy144CompletionViewCloseFinalNote(
    Floppy144CompletionViewState *state
);

void Floppy144CompletionViewScrollFinalNote(
    Floppy144CompletionViewState *state,
    const Floppy144RunState *run_state,
    int32_t lines
);

uint32_t Floppy144CompletionViewCollectionsRestored(
    const Floppy144RunState *run_state
);

uint32_t Floppy144CompletionViewEvidenceEstablished(
    const Floppy144RunState *run_state
);

uint32_t Floppy144CompletionViewCapacityRemainingPercent(
    const Floppy144RunState *run_state
);

bool Floppy144CompletionViewCoreResolutionEstablished(
    const Floppy144RunState *run_state
);

const char *Floppy144CompletionViewFinalConclusionText(
    const Floppy144RunState *run_state
);

const char *Floppy144CompletionViewOutcomeText(
    bool evidence_resolved,
    bool capacity_exhausted
);

const char *Floppy144CompletionViewStatusText(
    bool evidence_resolved,
    bool capacity_exhausted
);

void Floppy144CompletionViewDraw(
    Floppy144Surface *surface,
    const Floppy144CompletionViewState *state,
    const Floppy144RunState *run_state,
    const Floppy144DiscoveryProfile *profile,
    bool evidence_resolved,
    bool capacity_exhausted
);
