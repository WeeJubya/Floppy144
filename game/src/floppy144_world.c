#include "floppy144_world.h"

void Floppy144WorldReset(
    Floppy144WorldState *world
)
{
    world->hr02_restored = false;
    world->hr02_desk_reallocation_read = false;
    world->fa03_restored = false;
}

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
)
{
    uint32_t percentage = 4U;

    percentage +=
        world->hr02_restored
            ? 8U
            : 0U;

    percentage +=
        world->fa03_restored
            ? 8U
            : 0U;

    return percentage;
}
