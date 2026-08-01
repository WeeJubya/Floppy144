#pragma once

#include "floppy144_collection.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Immutable catalogue configuration belonging to one collection.
 *
 * A record count of zero means the collection does not expose a browsable
 * generated catalogue.
 */

typedef struct Floppy144CatalogueDefinition
{
    uint32_t record_count;

    const char *heading;
    const char *record_id_prefix;

    const char *const *subjects;
    uint32_t subject_count;

    bool exact_titles;

    uint32_t record_number_base;
    uint32_t record_number_multiplier;
    uint32_t record_number_offset;
} Floppy144CatalogueDefinition;

/*
 * Static metadata describing one archive collection.
 *
 * Runtime progress remains in Floppy144WorldState. This structure contains
 * immutable design data declared by floppy144_collections.def.
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
    const char *evidence_description;

    Floppy144CatalogueDefinition catalogue;
} Floppy144CollectionDefinition;

/*
 * Registry generated from floppy144_collections.def.
 */

extern const Floppy144CollectionDefinition
    floppy144_collection_definitions[FLOPPY144_COLLECTION_COUNT];

/*
 * Return collection metadata for an ID.
 *
 * An invalid ID safely falls back to DR-01.
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
