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

    /*
     * Most recent completed recovery, retained separately from cumulative
     * discovery so future runs can compare one achieved version of the truth
     * with another.
     */
    uint32_t latest_completion_evidence[
        FLOPPY144_PROFILE_WORD_COUNT(
            FLOPPY144_PROFILE_EVIDENCE_CAPACITY
        )
    ];

    uint32_t completed_recoveries;
    uint8_t latest_completion_evidence_percent;
    uint8_t latest_completion_flags;
    uint16_t latest_completion_recovered_kb;
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

/*
 * Count the distinct collections retained in the operator's cumulative
 * discovery history.
 */
uint32_t Floppy144DiscoveryProfileCollectionsEverRestoredCount(
    const Floppy144DiscoveryProfile *profile
);

/*
 * Count the distinct evidence items retained in the operator's cumulative
 * discovery history.
 */
uint32_t Floppy144DiscoveryProfileEvidenceEverEstablishedCount(
    const Floppy144DiscoveryProfile *profile
);


#define FLOPPY144_PROFILE_COMPLETION_EVIDENCE_RESOLVED 0x01U
#define FLOPPY144_PROFILE_COMPLETION_CAPACITY_EXHAUSTED 0x02U

bool Floppy144DiscoveryProfileRecordCompletion(
    Floppy144DiscoveryProfile *profile,
    const Floppy144RunState *run_state,
    bool evidence_resolved,
    bool capacity_exhausted
);
