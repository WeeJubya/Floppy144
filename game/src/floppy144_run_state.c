#include "floppy144_run_state.h"
#include "floppy144_collection_registry.h"
#include "floppy144_object_registry.h"
#include "floppy144_site.h"
#include "floppy144_site_rooms.h"

#include <string.h>

/**************/
/* Validators */
/**************/

static bool Floppy144RunStateRoomValid
(
    Floppy144RoomId room
){
    return
    (uint32_t)room <
    (uint32_t)FLOPPY144_ROOM_COUNT;
}

static bool Floppy144RunStateCollectionValid
(
    Floppy144CollectionId collection
){
    return
    (uint32_t)collection <
    (uint32_t)FLOPPY144_COLLECTION_COUNT;
}

static bool Floppy144RunStateInteractionValid
(
    Floppy144InteractionId interaction
){
    return
    (uint32_t)interaction <
    (uint32_t)FLOPPY144_INTERACTION_COUNT;
}

static bool Floppy144RunStateEvidenceValid
(
    Floppy144EvidenceId evidence
){
    return
    (uint32_t)evidence <
    (uint32_t)FLOPPY144_EVIDENCE_COUNT;
}

static bool Floppy144RunStateCapabilityValid
(
    Floppy144CapabilityId capability
){
    return
    (uint32_t)capability <
    (uint32_t)FLOPPY144_CAPABILITY_COUNT;
}

static bool Floppy144RunStateTriggerValid
(
    Floppy144TriggerId trigger
){
    return
    (uint32_t)trigger <
    (uint32_t)FLOPPY144_TRIGGER_COUNT;
}

static bool Floppy144RunStateNotebookEntryValid
(
    Floppy144NotebookId entry
){
    return
    (uint32_t)entry <
    (uint32_t)FLOPPY144_NOTEBOOK_COUNT;
}

static bool Floppy144RunStateObjectValid
(
    Floppy144ObjectId object
){
    return
    object >= 0 &&
    (uint32_t)object <
    (uint32_t)FLOPPY144_OBJECT_COUNT;
}

/*****************/
/* API Functions */
/*****************/

bool Floppy144RunStateRoomReconstructed
(
    const Floppy144RunState *state,
 Floppy144RoomId room
){
    if(
        state == NULL ||
        !Floppy144RunStateRoomValid(room)
    )
    {
        return false;
    }

    if(
        Floppy144RunStateBitGet(
            state->rooms,
            (uint32_t)room
        )
    )
    {
        return true;
    }

    /*
     * Compatibility for saves created before room reconstruction was wired
     * into Site movement/rendering. DR-01 has always been the minimum Site
     * reconstruction, so an older save with DR-01 restored must still expose
     * Reception and Corridor even if its room bitset predates 2E.7C.
     */
    if(
        Floppy144RunStateCollectionRestored(
            state,
            FLOPPY144_COLLECTION_DR01
        ) &&
        (
            room == FLOPPY144_ROOM_RECEPTION ||
            room == FLOPPY144_ROOM_CORRIDOR
        )
    )
    {
        return true;
    }

    if(
        room == FLOPPY144_ROOM_MAIN_OFFICE &&
        Floppy144RunStateCollectionRestored(
            state,
            FLOPPY144_COLLECTION_HR02
        )
    )
    {
        return true;
    }

    return false;
}

bool Floppy144RunStateReconstructRoom
(
    Floppy144RunState *state,
 Floppy144RoomId room
){
    if(
        state == NULL ||
        !Floppy144RunStateRoomValid(room)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->rooms,
            (uint32_t)room
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateObjectVisible
(
    const Floppy144RunState *state,
 Floppy144ObjectId object
){
    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->objects_visible,
        (uint32_t)object
    );
}

bool Floppy144RunStateRevealObject
(
    Floppy144RunState *state,
 Floppy144ObjectId object
){
    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->objects_visible,
            (uint32_t)object
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateBitGet
(
    const uint32_t *words,
    uint32_t bit
){
    uint32_t word;
    uint32_t mask;

    if(!words)
    {
        return false;
    }

    word =
    bit / FLOPPY144_RUN_WORD_BITS;

    mask =
    1U <<
    (bit % FLOPPY144_RUN_WORD_BITS);

    return
    (words[word] & mask) != 0U;
}

bool Floppy144RunStateBitSet
(
    uint32_t *words,
    uint32_t bit
){
    uint32_t word;
    uint32_t mask;

    if(!words)
    {
        return false;
    }

    word =
    bit / FLOPPY144_RUN_WORD_BITS;

    mask =
    1U <<
    (bit % FLOPPY144_RUN_WORD_BITS);

    if(words[word] & mask)
    {
        return false;
    }

    words[word] |=
    mask;

    return true;
}

bool Floppy144RunStateBitClear
(
    uint32_t *words,
    uint32_t bit
){
    uint32_t word;
    uint32_t mask;

    if(!words)
    {
        return false;
    }

    word =
    bit / FLOPPY144_RUN_WORD_BITS;

    mask =
    1U <<
    (bit % FLOPPY144_RUN_WORD_BITS);

    if(!(words[word] & mask))
    {
        return false;
    }

    words[word] &=
    ~mask;

    return true;
}

bool Floppy144RunStateCollectionRestored
(
    const Floppy144RunState *state,
 Floppy144CollectionId collection
){
    if(
        state == NULL ||
        !Floppy144RunStateCollectionValid(collection)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->collections,
        (uint32_t)collection
    );
}

bool Floppy144RunStateRestoreCollection
(
    Floppy144RunState *state,
 Floppy144CollectionId collection
){
    if(
        state == NULL ||
        !Floppy144RunStateCollectionValid(collection)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->collections,
            (uint32_t)collection
        )
    )
    {
        return false;
    }

    /*
     * Current Site reconstruction bridge.
     *
     * DR-01 establishes the minimum Site: Reception and Corridor.
     * HR-02 is the existing technical-slice predecessor of the later
     * Site-establishment content and currently opens the Main Office. Keeping
     * that compatibility here preserves the playable Stage-1 loop while the
     * final collection registry is expanded.
     */
    if(collection == FLOPPY144_COLLECTION_DR01)
    {
        Floppy144RunStateReconstructRoom(
            state,
            FLOPPY144_ROOM_RECEPTION
        );

        Floppy144RunStateReconstructRoom(
            state,
            FLOPPY144_ROOM_CORRIDOR
        );
    }
    else if(collection == FLOPPY144_COLLECTION_HR02)
    {
        Floppy144RunStateReconstructRoom(
            state,
            FLOPPY144_ROOM_MAIN_OFFICE
        );
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateTriggerFired
(
    const Floppy144RunState *state,
 Floppy144TriggerId trigger
){
    if(
        state == NULL ||
        !Floppy144RunStateTriggerValid(trigger)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->triggers,
        (uint32_t)trigger
    );
}

bool Floppy144RunStateFireTrigger
(
    Floppy144RunState *state,
 Floppy144TriggerId trigger
){
    if(
        state == NULL ||
        !Floppy144RunStateTriggerValid(trigger)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->triggers,
            (uint32_t)trigger
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateInteractionCompleted
(
    const Floppy144RunState *state,
 Floppy144InteractionId interaction
){
    if(
        state == NULL ||
        !Floppy144RunStateInteractionValid(interaction)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->interactions,
        (uint32_t)interaction
    );
}

bool Floppy144RunStateCompleteInteraction
(
    Floppy144RunState *state,
 Floppy144InteractionId interaction
){
    if(
        state == NULL ||
        !Floppy144RunStateInteractionValid(interaction)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->interactions,
            (uint32_t)interaction
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateEvidenceEstablished
(
    const Floppy144RunState *state,
 Floppy144EvidenceId evidence
){
    if(
        state == NULL ||
        !Floppy144RunStateEvidenceValid(evidence)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->evidence,
        (uint32_t)evidence
    );
}

bool Floppy144RunStateEstablishEvidence
(
    Floppy144RunState *state,
 Floppy144EvidenceId evidence
){
    if(
        state == NULL ||
        !Floppy144RunStateEvidenceValid(evidence)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->evidence,
            (uint32_t)evidence
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateNotebookEntryRecorded
(
    const Floppy144RunState *state,
 Floppy144NotebookId entry
){
    if(
        state == NULL ||
        !Floppy144RunStateNotebookEntryValid(entry)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->notebook,
        (uint32_t)entry
    );
}

bool Floppy144RunStateRecordNotebookEntry
(
    Floppy144RunState *state,
 Floppy144NotebookId entry
){
    if(
        state == NULL ||
        !Floppy144RunStateNotebookEntryValid(entry)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->notebook,
            (uint32_t)entry
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateHasCapability
(
    const Floppy144RunState *state,
 Floppy144CapabilityId capability
){
    if(
        state == NULL ||
        !Floppy144RunStateCapabilityValid(capability)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->capabilities,
        (uint32_t)capability
    );
}

bool Floppy144RunStateGrantCapability
(
    Floppy144RunState *state,
 Floppy144CapabilityId capability
){
    if(
        state == NULL ||
        !Floppy144RunStateCapabilityValid(capability)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->capabilities,
            (uint32_t)capability
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

void Floppy144RunStateSetPlayerSitePosition
(
    Floppy144RunState *state,
 int32_t x,
 int32_t y
){
    if(state == NULL)
    {
        return;
    }

    if(
        state->player_site_x == x &&
        state->player_site_y == y
    )
    {
        return;
    }

    state->player_site_x =
    x;

    state->player_site_y =
    y;

    state->dirty = 1;
}

bool Floppy144RunStateMovePlayerSite
(
    Floppy144RunState *state,
 int32_t delta_x16,
 int32_t delta_y16
)
{
    int32_t x;
    int32_t y;

    if(state == NULL)
    {
        return false;
    }

    x =
    state->player_site_x;

    y =
    state->player_site_y;

    if(
        !Floppy144SiteMovePosition(
            &x,
            &y,
            delta_x16,
            delta_y16
        )
    )
    {
        return false;
    }

    /*
     * Physical collision decides whether the candidate position is walkable.
     * Reconstruction state decides whether the room at that position exists
     * in this recovery. Door-threshold classification in site_rooms.c makes
     * this a clean room-transition gate without a second door-lock map.
     */
    {
        Floppy144RoomId destination_room =
            Floppy144SiteRoomAtPosition(
                x,
                y
            );

        if(
            destination_room == FLOPPY144_ROOM_COUNT ||
            !Floppy144RunStateRoomReconstructed(
                state,
                destination_room
            )
        )
        {
            return false;
        }
    }

    state->player_site_x =
    x;

    state->player_site_y =
    y;

    state->dirty =
    1;

    return true;
}

Floppy144Projection Floppy144RunStateProjection
(
    const Floppy144RunState *state
){
    if(
        state == NULL ||
        state->projection >=
        (uint8_t)FLOPPY144_PROJECTION_COUNT
    )
    {
        return FLOPPY144_PROJECTION_2D;
    }

    return
    (Floppy144Projection)state->projection;
}

bool Floppy144RunStateSetProjection
(
    Floppy144RunState *state,
 Floppy144Projection projection
){
    if(
        state == NULL ||
        (uint32_t)projection >=
        (uint32_t)FLOPPY144_PROJECTION_COUNT
    )
    {
        return false;
    }

    if(
        state->projection ==
        (uint8_t)projection
    )
    {
        return false;
    }

    state->projection =
    (uint8_t)projection;

    state->dirty = 1;

    return true;
}

void Floppy144RunStateReset
(
    Floppy144RunState *state
){
    if(!state)
    {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );

    state->act =
        FLOPPY144_RUN_ACT_PROLOGUE;

    state->branch =
        FLOPPY144_RUN_BRANCH_NONE;
}

Floppy144ObjectAccessState Floppy144RunStateObjectAccessState
(
    const Floppy144RunState *state,
 Floppy144ObjectId object
){
    const Floppy144ObjectDefinition *definition;

    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return FLOPPY144_OBJECT_ACCESS_NONE;
    }

    definition =
    Floppy144ObjectGet(object);

    if(
        definition == NULL ||
        !(definition->flags & FLOPPY144_OBJECT_FLAG_OPENABLE)
    )
    {
        return FLOPPY144_OBJECT_ACCESS_NONE;
    }

    if(
        Floppy144RunStateBitGet(
            state->objects_open,
            (uint32_t)object
        )
    )
    {
        return FLOPPY144_OBJECT_ACCESS_OPEN;
    }

    if(
        Floppy144RunStateBitGet(
            state->objects_unlocked,
            (uint32_t)object
        )
    )
    {
        return FLOPPY144_OBJECT_ACCESS_UNLOCKED;
    }

    return FLOPPY144_OBJECT_ACCESS_LOCKED;
}

bool Floppy144RunStateSetObjectAccessState
(
    Floppy144RunState *state,
 Floppy144ObjectId object,
 Floppy144ObjectAccessState access_state
){
    const Floppy144ObjectDefinition *definition;

    bool unlocked;
    bool open;

    bool changed =
    false;

    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return false;
    }

    definition =
    Floppy144ObjectGet(object);

    if(
        definition == NULL ||
        !(definition->flags & FLOPPY144_OBJECT_FLAG_OPENABLE)
    )
    {
        return false;
    }

    switch(access_state)
    {
        case FLOPPY144_OBJECT_ACCESS_LOCKED:
        {
            unlocked = false;
            open = false;
            break;
        }

        case FLOPPY144_OBJECT_ACCESS_UNLOCKED:
        {
            unlocked = true;
            open = false;
            break;
        }

        case FLOPPY144_OBJECT_ACCESS_OPEN:
        {
            unlocked = true;
            open = true;
            break;
        }

        case FLOPPY144_OBJECT_ACCESS_NONE:
        default:
        {
            return false;
        }
    }

    if(unlocked)
    {
        changed |=
        Floppy144RunStateBitSet(
            state->objects_unlocked,
            (uint32_t)object
        );
    }
    else
    {
        changed |=
        Floppy144RunStateBitClear(
            state->objects_unlocked,
            (uint32_t)object
        );
    }

    if(open)
    {
        changed |=
        Floppy144RunStateBitSet(
            state->objects_open,
            (uint32_t)object
        );
    }
    else
    {
        changed |=
        Floppy144RunStateBitClear(
            state->objects_open,
            (uint32_t)object
        );
    }

    if(changed)
    {
        state->dirty = 1;
    }

    return changed;
}

void Floppy144RunStateBegin
(
    Floppy144RunState *state,
 uint32_t recovery_seed
){
    uint32_t collection_index;
    uint32_t object_index;

    Floppy144RunStateReset(
        state
    );

    if(!state)
    {
        return;
    }

    state->recovery_seed =
    recovery_seed;

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
        Floppy144CollectionGet(
            collection
        );

        if(
            definition != NULL &&
            definition->auto_restored
        )
        {
            Floppy144RunStateBitSet(
                state->collections,
                collection_index
            );
        }
    }

    Floppy144SiteSpawnPosition(
        &state->player_site_x,
        &state->player_site_y
    );

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

        if(
            definition != NULL &&
            definition->initially_visible
        )
        {
            Floppy144RunStateBitSet(
                state->objects_visible,
                object_index
            );
        }
        if(
            definition != NULL &&
            (definition->flags & FLOPPY144_OBJECT_FLAG_OPENABLE) &&
            (definition->flags & FLOPPY144_OBJECT_FLAG_INITIALLY_UNLOCKED)
        )
        {
            Floppy144RunStateBitSet(
                state->objects_unlocked,
                object_index
            );
        }
    }

    state->dirty = 1;
}

uint32_t Floppy144RunStateReconstructionPercent
(
    const Floppy144RunState *state
){
    uint32_t collection_index;
    uint32_t percentage =
    0U;

    if(state == NULL)
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
        Floppy144CollectionGet(
            collection
        );

        if(
            !Floppy144RunStateCollectionRestored(
                state,
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

bool Floppy144RunStateArchiveServicesInitialised
(
    const Floppy144RunState *state
){
    if(state == NULL)
    {
        return false;
    }

    return
    state->archive_services_initialised != 0U;
}

bool Floppy144RunStateInitialiseArchiveServices
(
    Floppy144RunState *state
){
    if(
        state == NULL ||
        state->archive_services_initialised != 0U
    )
    {
        return false;
    }

    state->archive_services_initialised =
    1U;

    state->dirty =
    1U;

    return true;
}
