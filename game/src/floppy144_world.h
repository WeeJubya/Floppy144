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

typedef struct Floppy144CollectionState
{
    bool restored;
    bool evidence_found;
} Floppy144CollectionState;

/*
 * Runtime state belonging to one reconstructed object.
 */

typedef struct Floppy144ObjectState
{
    bool visible;
} Floppy144ObjectState;

typedef struct Floppy144WorldState
{
    Floppy144CollectionState
        collections[FLOPPY144_COLLECTION_COUNT];

    Floppy144ObjectState
        objects[FLOPPY144_OBJECT_COUNT];
} Floppy144WorldState;

void Floppy144WorldReset(
    Floppy144WorldState *world
);

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
 * Direct visibility is the object's own runtime flag.
 *
 * Effective visibility also requires every parent in the hierarchy to be
 * effectively visible.
 */

bool Floppy144WorldObjectVisible(
    const Floppy144WorldState *world,
    Floppy144ObjectId object
);

bool Floppy144WorldObjectEffectivelyVisible(
    const Floppy144WorldState *world,
    Floppy144ObjectId object
);

bool Floppy144WorldRevealObject(
    Floppy144WorldState *world,
    Floppy144ObjectId object
);

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
);
