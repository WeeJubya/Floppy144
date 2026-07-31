/*
 * Floppy//144 - persistent world state
 *
 * Holds facts that survive while the player moves between screens.
 * UI-only state belongs in the terminal or catalogue structures instead.
 */

#pragma once

#include "floppy144_collection.h"
#include "floppy144_object.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Runtime state belonging to one registered collection
 */

typedef struct Floppy144CollectionState
{
    bool restored;
    bool evidence_found;
} Floppy144CollectionState;

/*
 * Runtime state belonging to one reconstructed world object
 *
 * Dynamic objects begin hidden and can be revealed by document effects or
 * other registered game events.
 */

typedef struct Floppy144ObjectState
{
    bool visible;
} Floppy144ObjectState;

/*
 * Persistent session state
 *
 * Collection and object IDs are stable array indices generated from their
 * respective master definition files.
 */

typedef struct Floppy144WorldState
{
    Floppy144CollectionState
        collections[FLOPPY144_COLLECTION_COUNT];

    Floppy144ObjectState
        objects[FLOPPY144_OBJECT_COUNT];
} Floppy144WorldState;

/*
 * Start a fresh reconstruction session.
 *
 * Automatically restored collections are restored during reset. Evidence is
 * cleared and all dynamic reconstructed objects begin hidden.
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
 * Generic reconstructed-object operations
 */

bool Floppy144WorldObjectVisible(
    const Floppy144WorldState *world,
    Floppy144ObjectId object
);

bool Floppy144WorldRevealObject(
    Floppy144WorldState *world,
    Floppy144ObjectId object
);

/*
 * Convert restored collection state into the reconstruction percentage shown
 * by the recovery interface, terminal and office.
 */

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
);
