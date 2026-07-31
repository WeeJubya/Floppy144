/*
 * Floppy//144 - world-state implementation
 */

#include "floppy144_world.h"

#include "floppy144_collection_registry.h"
#include "floppy144_object_registry.h"

#include <stddef.h>

static bool Floppy144WorldCollectionValid(
    Floppy144CollectionId collection
)
{
    return
        (uint32_t)collection <
        (uint32_t)FLOPPY144_COLLECTION_COUNT;
}

static bool Floppy144WorldObjectValid(
    Floppy144ObjectId object
)
{
    return
        object >= 0 &&
        (uint32_t)object <
            (uint32_t)FLOPPY144_OBJECT_COUNT;
}

void Floppy144WorldReset(
    Floppy144WorldState *world
)
{
    uint32_t collection_index;
    uint32_t object_index;

    if(world == NULL)
    {
        return;
    }

    for(
        collection_index = 0U;
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
        object_index = 0U;
        object_index <
            (uint32_t)FLOPPY144_OBJECT_COUNT;
        ++object_index
    )
    {
        const Floppy144ObjectDefinition *definition =
            Floppy144ObjectGet(
                (Floppy144ObjectId)object_index
            );

        world->objects[object_index].visible =
            definition != NULL &&
            definition->initially_visible;
    }
}

bool Floppy144WorldCollectionRestored(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    if(
        world == NULL ||
        !Floppy144WorldCollectionValid(collection)
    )
    {
        return false;
    }

    return
        world->collections[collection].restored;
}

bool Floppy144WorldRestoreCollection(
    Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    if(
        world == NULL ||
        !Floppy144WorldCollectionValid(collection)
    )
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

bool Floppy144WorldCollectionEvidenceFound(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    if(
        world == NULL ||
        !Floppy144WorldCollectionValid(collection)
    )
    {
        return false;
    }

    return
        world->collections[collection].evidence_found;
}

void Floppy144WorldSetCollectionEvidenceFound(
    Floppy144WorldState *world,
    Floppy144CollectionId collection,
    bool evidence_found
)
{
    if(
        world == NULL ||
        !Floppy144WorldCollectionValid(collection)
    )
    {
        return;
    }

    world->collections[collection].evidence_found =
        evidence_found;
}

bool Floppy144WorldObjectVisible(
    const Floppy144WorldState *world,
    Floppy144ObjectId object
)
{
    if(
        world == NULL ||
        !Floppy144WorldObjectValid(object)
    )
    {
        return false;
    }

    return
        world->objects[object].visible;
}

/*
 * Test an object's complete parent chain.
 */

bool Floppy144WorldObjectEffectivelyVisible(
    const Floppy144WorldState *world,
    Floppy144ObjectId object
)
{
    Floppy144ObjectId current =
        object;

    uint32_t depth =
        0U;

    if(world == NULL)
    {
        return false;
    }

    while(current != FLOPPY144_OBJECT_NONE)
    {
        const Floppy144ObjectDefinition *definition;

        if(
            depth >=
                (uint32_t)FLOPPY144_OBJECT_COUNT ||
            !Floppy144WorldObjectValid(current) ||
            !world->objects[current].visible
        )
        {
            return false;
        }

        definition =
            Floppy144ObjectGet(current);

        if(definition == NULL)
        {
            return false;
        }

        current =
            definition->parent;

        ++depth;
    }

    return true;
}

bool Floppy144WorldRevealObject(
    Floppy144WorldState *world,
    Floppy144ObjectId object
)
{
    if(
        world == NULL ||
        !Floppy144WorldObjectValid(object)
    )
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

uint32_t Floppy144WorldReconstructionPercent(
    const Floppy144WorldState *world
)
{
    uint32_t collection_index;
    uint32_t percentage =
        0U;

    if(world == NULL)
    {
        return 0U;
    }

    for(
        collection_index = 0U;
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
