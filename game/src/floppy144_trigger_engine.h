#pragma once

/*
 * Floppy//144 trigger engine
 *
 * Persistent T001-T050 trigger IDs are defined by the trigger ledger.
 * This module owns generic trigger eligibility and fire-once execution.
 *
 * Authored trigger mappings and effects are populated separately so the
 * stable persistence IDs are never invented merely to exercise the engine.
 */

#include "floppy144_effect.h"
#include "floppy144_trigger.h"

#include <stdbool.h>
#include <stdint.h>


typedef struct Floppy144TriggerDefinition
{
    Floppy144TriggerId prerequisite;

    const Floppy144Effect *effects;
    uint32_t effect_count;
}
Floppy144TriggerDefinition;


/*
 * Return immutable metadata for one persistent trigger.
 */

const Floppy144TriggerDefinition *Floppy144TriggerGet
(
    Floppy144TriggerId trigger
);


/*
 * Test whether a trigger is currently eligible to fire.
 *
 * A trigger is eligible when:
 *   - its ID is valid;
 *   - it has not fired previously in this recovery;
 *   - its optional prerequisite has fired.
 */

bool Floppy144TriggerCanFire
(
    const Floppy144RunState *run_state,
 Floppy144TriggerId trigger
);


/*
 * Fire one eligible trigger and apply its registered generic effects.
 *
 * Returns true only when the trigger fires for the first time.
 */

bool Floppy144TriggerTryFire
(
    Floppy144WorldState *world,
 Floppy144RunState *run_state,
 Floppy144TriggerId trigger
);
