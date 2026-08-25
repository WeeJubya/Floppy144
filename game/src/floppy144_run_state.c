#include "floppy144_run_state.h"
#include "floppy144_collection_registry.h"

#include <string.h>

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

    state->dirty = 1;

    return true;
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

void Floppy144RunStateBegin
(
    Floppy144RunState *state,
 uint32_t recovery_seed
){
    uint32_t collection_index;

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

    state->dirty = 1;
}
