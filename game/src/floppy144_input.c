/*
 * Floppy//144 - logical movement input state
 */

#include "floppy144_input.h"

#include <stddef.h>

enum
{
    FLOPPY144_MOVE_HELD_UP = 1U << 0,
    FLOPPY144_MOVE_HELD_DOWN = 1U << 1,
    FLOPPY144_MOVE_HELD_LEFT = 1U << 2,
    FLOPPY144_MOVE_HELD_RIGHT = 1U << 3
};

void Floppy144MovementInputReset(
    Floppy144MovementInput *input
)
{
    if(input != NULL)
    {
        input->held_directions = 0U;
    }
}

bool Floppy144MovementInputSetAction(
    Floppy144MovementInput *input,
    F144Action action,
    bool pressed
)
{
    uint8_t mask;

    if(input == NULL)
    {
        return false;
    }

    switch(action)
    {
        case F144_ACTION_MOVE_UP:
            mask = FLOPPY144_MOVE_HELD_UP;
            break;

        case F144_ACTION_MOVE_DOWN:
            mask = FLOPPY144_MOVE_HELD_DOWN;
            break;

        case F144_ACTION_MOVE_LEFT:
            mask = FLOPPY144_MOVE_HELD_LEFT;
            break;

        case F144_ACTION_MOVE_RIGHT:
            mask = FLOPPY144_MOVE_HELD_RIGHT;
            break;

        default:
            return false;
    }

    if(pressed)
    {
        input->held_directions |= mask;
    }
    else
    {
        input->held_directions &=
            (uint8_t)~mask;
    }

    return true;
}

void Floppy144MovementInputVector(
    const Floppy144MovementInput *input,
    int32_t step,
    int32_t *movement_x,
    int32_t *movement_y
)
{
    int32_t x = 0;
    int32_t y = 0;

    if(input != NULL)
    {
        if(
            (input->held_directions & FLOPPY144_MOVE_HELD_LEFT) != 0U
        )
        {
            x -= step;
        }

        if(
            (input->held_directions & FLOPPY144_MOVE_HELD_RIGHT) != 0U
        )
        {
            x += step;
        }

        if(
            (input->held_directions & FLOPPY144_MOVE_HELD_UP) != 0U
        )
        {
            y -= step;
        }

        if(
            (input->held_directions & FLOPPY144_MOVE_HELD_DOWN) != 0U
        )
        {
            y += step;
        }
    }

    if(movement_x != NULL)
    {
        *movement_x = x;
    }

    if(movement_y != NULL)
    {
        *movement_y = y;
    }
}
