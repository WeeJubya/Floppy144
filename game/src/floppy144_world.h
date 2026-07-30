/*
 * Floppy//144 - persistent world state
 *
 * Holds facts that survive while the player moves between screens.
 * UI-only state belongs in the terminal or catalogue structures instead.
 */

#pragma once

#include "floppy144_collection.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Runtime state belonging to one registered collection
 *
 * Every collection receives one entry automatically. Adding another
 * collection therefore does not require another named field here.
 */

typedef struct Floppy144CollectionState
{
    bool restored;
    bool evidence_found;
} Floppy144CollectionState;

/*
 * Persistent session state
 *
 * Collection IDs are stable array indices generated from the master
 * collection definition file.
 */

typedef struct Floppy144WorldState
{
    Floppy144CollectionState
        collections[FLOPPY144_COLLECTION_COUNT];
} Floppy144WorldState;

/*
 * Start a fresh reconstruction session.
 *
 * Collections registered as automatically restored are restored during reset.
 * All evidence flags begin cleared.
 */

void Floppy144WorldReset(
    Floppy144WorldState *world
);

/*
 * Generic collection-state operations
 */

bool Floppy144WorldCollectionRestored(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
);

bool Floppy144WorldRestoreCollection(
    Floppy144WorldState *world,
    Floppy144CollectionId collection
);

bool Floppy144WorldCollectionEvidenceFound(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
);

void Floppy144WorldSetCollectionEvidenceFound(
    Floppy144WorldState *world,
    Floppy144CollectionId collection,
    bool evidence_found
);

/*
 * Convert restored collection state into the technical-slice reconstruction
 * percentage displayed by the recovery interface, terminal and office.
 */

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
);

/*
 * Temporary migration aliases
 *
 * These allow the existing technical-slice code to continue compiling while
 * each subsystem is converted to the generic world-state API.
 *
 * They will be removed once main, office, recovery and terminal no longer
 * reference collection-specific field names.
 */

#define hr02_restored                                             \
    collections[FLOPPY144_COLLECTION_HR02].restored

#define hr02_desk_reallocation_read                               \
    collections[FLOPPY144_COLLECTION_HR02].evidence_found

#define fa03_restored                                             \
    collections[FLOPPY144_COLLECTION_FA03].restored

#define fa03_suppression_service_read                             \
    collections[FLOPPY144_COLLECTION_FA03].evidence_found
