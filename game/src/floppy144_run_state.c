#include "floppy144_run_state.h"

#include <string.h>

void Floppy144RunStateReset
(
    Floppy144RunState *state
){
    if(!state)
    {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );

    state->act =
        FLOPPY144_RUN_ACT_PROLOGUE;

    state->branch =
        FLOPPY144_RUN_BRANCH_NONE;
}

void Floppy144RunStateBegin
(
    Floppy144RunState *state,
    uint32_t recovery_seed
){
    Floppy144RunStateReset(
        state
    );

    if(!state)
    {
        return;
    }

    state->recovery_seed =
        recovery_seed;

    state->dirty = 1;
}
