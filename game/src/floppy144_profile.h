#pragma once

#include "floppy144_collection.h"
#include "floppy144_evidence.h"
#include "floppy144_run_state.h"

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_PROFILE_NAME_CAPACITY          32U
#define FLOPPY144_PROFILE_COLLECTION_CAPACITY    64U
#define FLOPPY144_PROFILE_EVIDENCE_CAPACITY      32U

#define FLOPPY144_PROFILE_WORD_BITS              32U

#define FLOPPY144_PROFILE_WORD_COUNT(capacity) \
(((capacity) + FLOPPY144_PROFILE_WORD_BITS - 1U) / \
FLOPPY144_PROFILE_WORD_BITS)

typedef enum Floppy144OperatorBodyStyle
{
    FLOPPY144_OPERATOR_BODY_STYLE_A = 0,
    FLOPPY144_OPERATOR_BODY_STYLE_B,

    FLOPPY144_OPERATOR_BODY_STYLE_COUNT
}
Floppy144OperatorBodyStyle;

typedef struct Floppy144DiscoveryProfile
{
    char operator_name[
        FLOPPY144_PROFILE_NAME_CAPACITY
    ];

    uint8_t body_style;
    uint8_t dirty;

    uint32_t recovery_sessions_begun;

    uint32_t collections_ever_restored[
        FLOPPY144_PROFILE_WORD_COUNT(
            FLOPPY144_PROFILE_COLLECTION_CAPACITY
        )
    ];

    uint32_t evidence_ever_established[
        FLOPPY144_PROFILE_WORD_COUNT(
            FLOPPY144_PROFILE_EVIDENCE_CAPACITY
        )
    ];
}
Floppy144DiscoveryProfile;

bool Floppy144DiscoveryProfileMergeRunState
(
    Floppy144DiscoveryProfile *profile,
    const Floppy144RunState *run_state
);

void Floppy144DiscoveryProfileReset
(
    Floppy144DiscoveryProfile *profile
);

bool Floppy144DiscoveryProfileSetOperatorName
(
    Floppy144DiscoveryProfile *profile,
 const char *name
);

bool Floppy144DiscoveryProfileSetBodyStyle
(
    Floppy144DiscoveryProfile *profile,
 Floppy144OperatorBodyStyle body_style
);

void Floppy144DiscoveryProfileBeginRecovery
(
    Floppy144DiscoveryProfile *profile
);

bool Floppy144DiscoveryProfileCollectionEverRestored
(
    const Floppy144DiscoveryProfile *profile,
 Floppy144CollectionId collection
);

bool Floppy144DiscoveryProfileRecordCollection
(
    Floppy144DiscoveryProfile *profile,
 Floppy144CollectionId collection
);

bool Floppy144DiscoveryProfileEvidenceEverEstablished
(
    const Floppy144DiscoveryProfile *profile,
 Floppy144EvidenceId evidence
);

bool Floppy144DiscoveryProfileRecordEvidence
(
    Floppy144DiscoveryProfile *profile,
 Floppy144EvidenceId evidence
);
