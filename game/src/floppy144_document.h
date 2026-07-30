/*
 * Floppy//144 - authored document registry
 *
 * Connects recovered authored records to the effects they apply when read.
 * Catalogue and main-loop code can query this registry without knowing which
 * collections contain primary evidence.
 */

#pragma once

#include "floppy144_effect.h"

#include <stdint.h>

/*
 * Immutable authored-document metadata
 *
 * record_index is zero-based. For example, index 37 is displayed to the
 * player as catalogue record 038.
 */

typedef struct Floppy144DocumentDefinition
{
    Floppy144CollectionId collection;
    uint32_t record_index;

    const Floppy144Effect *effects;
    uint32_t effect_count;
} Floppy144DocumentDefinition;

/*
 * Find an authored document matching a collection and catalogue index.
 *
 * Returns null when the selected record has no registered authored content.
 */

const Floppy144DocumentDefinition *Floppy144DocumentGet(
    Floppy144CollectionId collection,
    uint32_t record_index
);

/*
 * Apply the registered effects for a selected record.
 *
 * Returns true when the record exists in the authored-document registry.
 */

bool Floppy144DocumentApplyEffects(
    Floppy144WorldState *world,
    Floppy144CollectionId collection,
    uint32_t record_index
);
