#pragma once

/* Floppy//144 - data-driven player Notebook screen. */
#include "f144_runtime.h"
#include "floppy144_run_state.h"
#include <stdint.h>

typedef struct Floppy144NotebookViewState
{
    uint32_t selected_entry;
}
Floppy144NotebookViewState;

void Floppy144NotebookViewReset(Floppy144NotebookViewState *pNotebook);
void Floppy144NotebookViewMove(Floppy144NotebookViewState *pNotebook,const Floppy144RunState *pRunState,int32_t nDirection);
void Floppy144NotebookViewDraw(F144Runtime *pRuntime,const Floppy144NotebookViewState *pNotebook,const Floppy144RunState *pRunState);
