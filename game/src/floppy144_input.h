/*
 * Floppy//144 - logical movement input state
 *
 * Cardinal movement actions are tracked independently. Diagonal movement is
 * derived by combining simultaneously held horizontal and vertical actions;
 * it is not represented by extra platform actions.
 */

#pragma once

#include "f144_platform.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144MovementInput
{
    uint8_t held_directions;
} Floppy144MovementInput;

void Floppy144MovementInputReset(
    Floppy144MovementInput *input
);

bool Floppy144MovementInputSetAction(
    Floppy144MovementInput *input,
    F144Action action,
    bool pressed
);

void Floppy144MovementInputVector(
    const Floppy144MovementInput *input,
    int32_t step,
    int32_t *movement_x,
    int32_t *movement_y
);
