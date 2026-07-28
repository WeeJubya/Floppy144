#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144WorldState
{
    bool hr02_restored;
} Floppy144WorldState;

void Floppy144WorldReset(
    Floppy144WorldState *world
);

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
);
