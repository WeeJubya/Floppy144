/*
 * Floppy//144 - world-state implementation
 *
 * Initialises persistent session facts and calculates the visible recovery
 * percentage. It deliberately contains no drawing or input code.
 */

#include "floppy144_world.h"

/*
 * Start a fresh reconstruction session
 *
 * Both collection restoration and evidence-reading flags are cleared.
 */

void Floppy144WorldReset(
    Floppy144WorldState *world
)
{
    world->hr02_restored = false;
    world->hr02_desk_reallocation_read = false;

    world->fa03_restored = false;
    world->fa03_suppression_service_read = false;
}

/*
 * Calculate reconstruction progress
 *
 * XX-01 contributes the mandatory 4 percent baseline. Each optional
 * technical-slice collection contributes another 8 percent.
 */

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
