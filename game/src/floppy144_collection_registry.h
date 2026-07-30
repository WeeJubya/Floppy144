#pragma once

#include "floppy144_collection.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Static metadata describing one archive collection.
 *
 * Runtime progress, such as whether a collection has been restored, remains
 * in Floppy144WorldState. This structure contains immutable design data.
 */

typedef struct Floppy144CollectionDefinition
{
    Floppy144CollectionId id;

    const char *code;
    const char *title;

    Floppy144Act act;
    Floppy144CollectionClass collection_class;

    bool auto_restored;

    const char *description;
} Floppy144CollectionDefinition;

/*
 * Registry generated from floppy144_collections.def.
 */

extern const Floppy144CollectionDefinition
    floppy144_collection_definitions[FLOPPY144_COLLECTION_COUNT];

/*
 * Return collection metadata for an ID.
 *
 * An invalid ID safely falls back to XX-01.
 */

const Floppy144CollectionDefinition *Floppy144CollectionGet(
    Floppy144CollectionId collection
);

/*
 * Convert enum values into player-facing terminal labels.
 */

const char *Floppy144CollectionClassText(
    Floppy144CollectionClass collection_class
);

const char *Floppy144CollectionActText(
    Floppy144Act act
);
