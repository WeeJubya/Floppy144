#pragma once

/* Floppy//144 - data-driven player Notebook screen. */
#include "floppy144_draw.h"
#include "floppy144_run_state.h"
#include <stdint.h>

typedef struct Floppy144NotebookViewState
{
    uint32_t top_line;
}
Floppy144NotebookViewState;

void Floppy144NotebookViewReset(Floppy144NotebookViewState *pNotebook);
void Floppy144NotebookViewMove(Floppy144NotebookViewState *pNotebook,const Floppy144RunState *pRunState,int32_t nDirection);
void Floppy144NotebookViewDraw(Floppy144Surface *pRuntime,const Floppy144NotebookViewState *pNotebook,const Floppy144RunState *pRunState);
