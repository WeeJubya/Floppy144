#include "floppy144_profile.h"

#include <stddef.h>
#include <string.h>

typedef char Floppy144ProfileCollectionCapacityCheck[
    FLOPPY144_COLLECTION_COUNT <=
    FLOPPY144_PROFILE_COLLECTION_CAPACITY
    ? 1
    : -1
];

typedef char Floppy144ProfileEvidenceCapacityCheck[
    FLOPPY144_EVIDENCE_COUNT <=
    FLOPPY144_PROFILE_EVIDENCE_CAPACITY
    ? 1
    : -1
];

static bool Floppy144DiscoveryProfileBitGet
(
    const uint32_t *words,
 uint32_t bit
){
    uint32_t word_index;
    uint32_t mask;

    if(words == NULL)
    {
        return false;
    }

    word_index =
    bit / FLOPPY144_PROFILE_WORD_BITS;

    mask =
    1U << (
        bit % FLOPPY144_PROFILE_WORD_BITS
    );

    return
    (words[word_index] & mask) != 0U;
}

static bool Floppy144DiscoveryProfileBitSet
(
    uint32_t *words,
 uint32_t bit
){
    uint32_t word_index;
    uint32_t mask;

    if(words == NULL)
    {
        return false;
    }

    word_index =
    bit / FLOPPY144_PROFILE_WORD_BITS;

    mask =
    1U << (
        bit % FLOPPY144_PROFILE_WORD_BITS
    );

    if((words[word_index] & mask) != 0U)
    {
        return false;
    }

    words[word_index] |=
    mask;

    return true;
}

void Floppy144DiscoveryProfileReset
(
    Floppy144DiscoveryProfile *profile
){
    if(profile == NULL)
    {
        return;
    }

    memset(
        profile,
        0,
        sizeof(*profile)
    );
}

bool Floppy144DiscoveryProfileSetOperatorName
(
    Floppy144DiscoveryProfile *profile,
 const char *name
){
    size_t length;

    if(
        profile == NULL ||
        name == NULL
    )
    {
        return false;
    }

    length =
    strlen(name);

    if(
        length == 0U ||
        length >=
        FLOPPY144_PROFILE_NAME_CAPACITY
    )
    {
        return false;
    }

    if(
        strcmp(
            profile->operator_name,
            name
        ) == 0
    )
    {
        return false;
    }

    memset(
        profile->operator_name,
        0,
        sizeof(profile->operator_name)
    );

    memcpy(
        profile->operator_name,
        name,
        length
    );

    profile->dirty =
    1U;

    return true;
}

bool Floppy144DiscoveryProfileSetBodyStyle
(
    Floppy144DiscoveryProfile *profile,
 Floppy144OperatorBodyStyle body_style
){
    if(
        profile == NULL ||
        body_style < 0 ||
        body_style >=
        FLOPPY144_OPERATOR_BODY_STYLE_COUNT
    )
    {
        return false;
    }

    if(
        profile->body_style ==
        (uint8_t)body_style
    )
    {
        return false;
    }

    profile->body_style =
    (uint8_t)body_style;

    profile->dirty =
    1U;

    return true;
}

void Floppy144DiscoveryProfileBeginRecovery
(
    Floppy144DiscoveryProfile *profile
){
    if(profile == NULL)
    {
        return;
    }

    if(
        profile->recovery_sessions_begun !=
        UINT32_MAX
    )
    {
        ++profile->recovery_sessions_begun;
    }

    profile->dirty =
    1U;
}

bool Floppy144DiscoveryProfileCollectionEverRestored
(
    const Floppy144DiscoveryProfile *profile,
 Floppy144CollectionId collection
){
    if(
        profile == NULL ||
        collection < 0 ||
        collection >=
        FLOPPY144_COLLECTION_COUNT
    )
    {
        return false;
    }

    return
    Floppy144DiscoveryProfileBitGet(
        profile->collections_ever_restored,
        (uint32_t)collection
    );
}

bool Floppy144DiscoveryProfileRecordCollection
(
    Floppy144DiscoveryProfile *profile,
 Floppy144CollectionId collection
){
    if(
        profile == NULL ||
        collection < 0 ||
        collection >=
        FLOPPY144_COLLECTION_COUNT
    )
    {
        return false;
    }

    if(
        !Floppy144DiscoveryProfileBitSet(
            profile->collections_ever_restored,
            (uint32_t)collection
        )
    )
    {
        return false;
    }

    profile->dirty =
    1U;

    return true;
}

bool Floppy144DiscoveryProfileEvidenceEverEstablished
(
    const Floppy144DiscoveryProfile *profile,
 Floppy144EvidenceId evidence
){
    if(
        profile == NULL ||
        evidence < 0 ||
        evidence >=
        FLOPPY144_EVIDENCE_COUNT
    )
    {
        return false;
    }

    return
    Floppy144DiscoveryProfileBitGet(
        profile->evidence_ever_established,
        (uint32_t)evidence
    );
}

bool Floppy144DiscoveryProfileRecordEvidence
(
    Floppy144DiscoveryProfile *profile,
 Floppy144EvidenceId evidence
){
    if(
        profile == NULL ||
        evidence < 0 ||
        evidence >=
        FLOPPY144_EVIDENCE_COUNT
    )
    {
        return false;
    }

    if(
        !Floppy144DiscoveryProfileBitSet(
            profile->evidence_ever_established,
            (uint32_t)evidence
        )
    )
    {
        return false;
    }

    profile->dirty =
    1U;

    return true;
}

bool Floppy144DiscoveryProfileMergeRunState
(
    Floppy144DiscoveryProfile *profile,
 const Floppy144RunState *run_state
){
    uint32_t collection_index;
    uint32_t evidence_index;

    bool changed =
    false;

    if(
        profile == NULL ||
        run_state == NULL
    )
    {
        return false;
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

        if(
            Floppy144RunStateCollectionRestored(
                run_state,
                collection
            )
        )
        {
            changed |=
            Floppy144DiscoveryProfileRecordCollection(
                profile,
                collection
            );
        }
    }

    for(
        evidence_index = 0U;
    evidence_index <
    (uint32_t)FLOPPY144_EVIDENCE_COUNT;
    ++evidence_index
    )
    {
        Floppy144EvidenceId evidence =
        (Floppy144EvidenceId)evidence_index;

        if(
            Floppy144RunStateEvidenceEstablished(
                run_state,
                evidence
            )
        )
        {
            changed |=
            Floppy144DiscoveryProfileRecordEvidence(
                profile,
                evidence
            );
        }
    }

    return changed;
}
