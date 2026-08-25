#pragma once

#include "floppy144_collection.h"
#include "floppy144_trigger.h"
#include "floppy144_interaction.h"
#include "floppy144_evidence.h"
#include "floppy144_notebook.h"
#include "floppy144_capability.h"

#include <stdint.h>
#include <stdbool.h>


/*
 * Floppy//144 run state
 *
 * This is the authoritative state for one recovery session.
 *
 * Pass 2A introduces the structure alongside the existing prototype world
 * state. Gameplay is migrated into it incrementally in later passes.
 */

#define FLOPPY144_RUN_WORD_BITS             32

#define FLOPPY144_RUN_WORD_COUNT(capacity) \
(((capacity) + FLOPPY144_RUN_WORD_BITS - 1) / FLOPPY144_RUN_WORD_BITS)

typedef enum Floppy144RunAct
{
    FLOPPY144_RUN_ACT_PROLOGUE = 0,
    FLOPPY144_RUN_ACT_I,
    FLOPPY144_RUN_ACT_II,
    FLOPPY144_RUN_ACT_III,
    FLOPPY144_RUN_ACT_COMPLETE
}
Floppy144RunAct;

typedef enum Floppy144RunBranch
{
    FLOPPY144_RUN_BRANCH_NONE = 0,
    FLOPPY144_RUN_BRANCH_RECORDS_FIRST,
    FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST
}
Floppy144RunBranch;

typedef struct Floppy144RunState
{
    uint32_t recovery_seed;

    uint8_t act;
    uint8_t branch;
    uint8_t reconstruction_percent;
    uint8_t dirty;

    uint32_t collections[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_COLLECTION_COUNT)
    ];

    uint32_t triggers[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_TRIGGER_COUNT)
    ];

    uint32_t interactions[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_INTERACTION_COUNT)
    ];

    uint32_t evidence[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_EVIDENCE_COUNT)
    ];

    uint32_t notebook[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_NOTEBOOK_COUNT)
    ];

    uint32_t capabilities[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_CAPABILITY_COUNT)
    ];
}
Floppy144RunState;

bool Floppy144RunStateBitGet
(
    const uint32_t *words,
    uint32_t bit
);

bool Floppy144RunStateBitSet
(
    uint32_t *words,
    uint32_t bit
);

bool Floppy144RunStateBitClear
(
    uint32_t *words,
    uint32_t bit
);

bool Floppy144RunStateCollectionRestored
(
    const Floppy144RunState *state,
 Floppy144CollectionId collection
);

bool Floppy144RunStateRestoreCollection
(
    Floppy144RunState *state,
 Floppy144CollectionId collection
);

bool Floppy144RunStateTriggerFired
(
    const Floppy144RunState *state,
 Floppy144TriggerId trigger
);

bool Floppy144RunStateFireTrigger
(
    Floppy144RunState *state,
 Floppy144TriggerId trigger
);

bool Floppy144RunStateInteractionCompleted
(
    const Floppy144RunState *state,
 Floppy144InteractionId interaction
);

bool Floppy144RunStateCompleteInteraction
(
    Floppy144RunState *state,
 Floppy144InteractionId interaction
);

bool Floppy144RunStateEvidenceEstablished
(
    const Floppy144RunState *state,
 Floppy144EvidenceId evidence
);

bool Floppy144RunStateEstablishEvidence
(
    Floppy144RunState *state,
 Floppy144EvidenceId evidence
);

bool Floppy144RunStateNotebookEntryRecorded
(
    const Floppy144RunState *state,
 Floppy144NotebookId entry
);

bool Floppy144RunStateRecordNotebookEntry
(
    Floppy144RunState *state,
 Floppy144NotebookId entry
);

bool Floppy144RunStateHasCapability
(
    const Floppy144RunState *state,
 Floppy144CapabilityId capability
);

bool Floppy144RunStateGrantCapability
(
    Floppy144RunState *state,
 Floppy144CapabilityId capability
);

void Floppy144RunStateReset
(
    Floppy144RunState *state
);

void Floppy144RunStateBegin
(
    Floppy144RunState *state,
    uint32_t recovery_seed
);
