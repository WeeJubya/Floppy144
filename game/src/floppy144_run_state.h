#pragma once

#include <stdint.h>

/*
 * Floppy//144 run state
 *
 * This is the authoritative state for one recovery session.
 *
 * Pass 2A introduces the structure alongside the existing prototype world
 * state. Gameplay is migrated into it incrementally in later passes.
 */

#define FLOPPY144_RUN_COLLECTION_CAPACITY   64
#define FLOPPY144_RUN_TRIGGER_CAPACITY      64
#define FLOPPY144_RUN_INTERACTION_CAPACITY  64
#define FLOPPY144_RUN_EVIDENCE_CAPACITY     64
#define FLOPPY144_RUN_NOTEBOOK_CAPACITY    128
#define FLOPPY144_RUN_CAPABILITY_CAPACITY   32

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
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_RUN_COLLECTION_CAPACITY)
    ];

    uint32_t triggers[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_RUN_TRIGGER_CAPACITY)
    ];

    uint32_t interactions[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_RUN_INTERACTION_CAPACITY)
    ];

    uint32_t evidence[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_RUN_EVIDENCE_CAPACITY)
    ];

    uint32_t notebook[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_RUN_NOTEBOOK_CAPACITY)
    ];

    uint32_t capabilities[
        FLOPPY144_RUN_WORD_COUNT(FLOPPY144_RUN_CAPABILITY_CAPACITY)
    ];
}
Floppy144RunState;

void Floppy144RunStateReset
(
    Floppy144RunState *state
);

void Floppy144RunStateBegin
(
    Floppy144RunState *state,
 uint32_t recovery_seed
);
