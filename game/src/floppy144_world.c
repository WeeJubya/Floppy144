/*
 * Floppy//144 - world-state implementation
 *
 * Initialises persistent session facts and provides generic collection and
 * reconstructed-object operations. It contains no drawing or input code.
 */

#include "floppy144_world.h"

#include "floppy144_collection_registry.h"

/*
 * Check whether a collection ID can safely index world state.
 */

static bool Floppy144WorldCollectionValid(
    Floppy144CollectionId collection
)
{
    return
        (uint32_t)collection <
        (uint32_t)FLOPPY144_COLLECTION_COUNT;
}

/*
 * Check whether an object ID can safely index world state.
 */

static bool Floppy144WorldObjectValid(
    Floppy144ObjectId object
)
{
    return
        (uint32_t)object <
        (uint32_t)FLOPPY144_OBJECT_COUNT;
}

/*
 * Start a fresh reconstruction session
 */

void Floppy144WorldReset(
    Floppy144WorldState *world
)
{
    uint32_t collection_index;
    uint32_t object_index;

    for(
        collection_index = 0;
        collection_index <
            (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        Floppy144CollectionId collection =
            (Floppy144CollectionId)collection_index;

        const Floppy144CollectionDefinition *definition =
            Floppy144CollectionGet(collection);

        world->collections[collection_index].restored =
            definition->auto_restored;

        world->collections[collection_index].evidence_found =
            false;
    }

    for(
        object_index = 0;
        object_index <
            (uint32_t)FLOPPY144_OBJECT_COUNT;
        ++object_index
    )
    {
        world->objects[object_index].visible =
            false;
    }
}

/*
 * Query whether a collection has been restored.
 */

bool Floppy144WorldCollectionRestored(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    if(!Floppy144WorldCollectionValid(collection))
    {
        return false;
    }

    return
        world->collections[collection].restored;
}

/*
 * Restore a collection.
 *
 * Returns true only when this call changed its state.
 */

bool Floppy144WorldRestoreCollection(
    Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    if(!Floppy144WorldCollectionValid(collection))
    {
        return false;
    }

    if(world->collections[collection].restored)
    {
        return false;
    }

    world->collections[collection].restored =
        true;

    return true;
}

/*
 * Query whether authored evidence has been found in a collection.
 */

bool Floppy144WorldCollectionEvidenceFound(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    if(!Floppy144WorldCollectionValid(collection))
    {
        return false;
    }

    return
        world->collections[collection].evidence_found;
}

/*
 * Update the authored-evidence flag belonging to a collection.
 */

void Floppy144WorldSetCollectionEvidenceFound(
    Floppy144WorldState *world,
    Floppy144CollectionId collection,
    bool evidence_found
)
{
    if(!Floppy144WorldCollectionValid(collection))
    {
        return;
    }

    world->collections[collection].evidence_found =
        evidence_found;
}

/*
 * Query whether a reconstructed object is currently visible.
 */

bool Floppy144WorldObjectVisible(
    const Floppy144WorldState *world,
    Floppy144ObjectId object
)
{
    if(!Floppy144WorldObjectValid(object))
    {
        return false;
    }

    return
        world->objects[object].visible;
}

/*
 * Reveal one reconstructed object.
 *
 * Returns true only when this call changed the object's state.
 */

bool Floppy144WorldRevealObject(
    Floppy144WorldState *world,
    Floppy144ObjectId object
)
{
    if(!Floppy144WorldObjectValid(object))
    {
        return false;
    }

    if(world->objects[object].visible)
    {
        return false;
    }

    world->objects[object].visible =
        true;

    return true;
}

/*
 * Calculate technical-slice reconstruction progress.
 */

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
)
{
    uint32_t collection_index;
    uint32_t percentage = 0U;

    for(
        collection_index = 0;
        collection_index <
            (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        Floppy144CollectionId collection =
            (Floppy144CollectionId)collection_index;

        const Floppy144CollectionDefinition *definition =
            Floppy144CollectionGet(collection);

        if(
            !Floppy144WorldCollectionRestored(
                world,
                collection
            )
        )
        {
            continue;
        }

        percentage +=
            definition->collection_class ==
                FLOPPY144_COLLECTION_CLASS_MANDATORY
                ? 4U
                : 8U;
    }

    return percentage;
}
