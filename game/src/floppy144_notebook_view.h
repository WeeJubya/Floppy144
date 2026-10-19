#pragma once

/* Floppy//144 - data-driven player Notebook screen. */
#include "floppy144_draw.h"
#include "floppy144_run_state.h"
#include <stdint.h>

typedef enum Floppy144NotebookPage
{
    FLOPPY144_NOTEBOOK_PAGE_EVIDENCE = 0,
    FLOPPY144_NOTEBOOK_PAGE_DOCUMENT_LEDGER
} Floppy144NotebookPage;

/* Presentation-only page and independent scroll offsets, never saved. */
typedef struct Floppy144NotebookViewState
{
    uint32_t top_line;
    uint32_t document_top_line;
    Floppy144NotebookPage page;
} Floppy144NotebookViewState;

void Floppy144NotebookViewReset(Floppy144NotebookViewState *pNotebook);
void Floppy144NotebookViewOpen(Floppy144NotebookViewState *pNotebook);
void Floppy144NotebookViewSwitchPage(Floppy144NotebookViewState *pNotebook,int32_t nDirection);
void Floppy144NotebookViewMove(Floppy144NotebookViewState *pNotebook,const Floppy144RunState *pRunState,int32_t nDirection);
void Floppy144NotebookViewDraw(Floppy144Surface *pRuntime,const Floppy144NotebookViewState *pNotebook,const Floppy144RunState *pRunState);
