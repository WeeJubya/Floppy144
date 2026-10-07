/*
 * FLOPPY//144 platform-neutral application lifecycle state.
 */

#pragma once

#include "f144_platform.h"

#include <stdint.h>

typedef struct Floppy144LifecycleState
{
    uint8_t started;
    uint8_t active;
    uint8_t suspended;
    uint8_t shutdown_requested;
    uint8_t shutdown_complete;
} Floppy144LifecycleState;

void Floppy144LifecycleReset(
    Floppy144LifecycleState *state
);

void Floppy144LifecycleApply(
    Floppy144LifecycleState *state,
    const F144LifecycleEvent *event
);
