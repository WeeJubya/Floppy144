#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144WorldState
{
    bool hr02_restored;
    bool hr02_desk_reallocation_read;
    bool fa03_restored;
} Floppy144WorldState;

void Floppy144WorldReset(
    Floppy144WorldState *world
);

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
);
