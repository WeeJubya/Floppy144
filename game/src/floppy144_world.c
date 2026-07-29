#include "floppy144_world.h"

void Floppy144WorldReset(
    Floppy144WorldState *world
)
{
    world->hr02_restored = false;
    world->hr02_desk_reallocation_read = false;
}

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
)
{
    return world->hr02_restored ? 12U : 4U;
}

