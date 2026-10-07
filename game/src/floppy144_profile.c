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

    profile->body_style =
        (uint8_t)FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT;
}

/*
 * Report whether one codepoint is safe for player-entered operator identity.
 *
 * Storage remains mixed-case ASCII. Rendering may uppercase it later because
 * the compact 5x7 font contains uppercase glyphs only.
 */
bool Floppy144DiscoveryProfileOperatorNameCharacterSupported(
    uint32_t codepoint
)
{
    if(
        (
            codepoint >= (uint32_t)'A' &&
            codepoint <= (uint32_t)'Z'
        ) ||
        (
            codepoint >= (uint32_t)'a' &&
            codepoint <= (uint32_t)'z'
        ) ||
        (
            codepoint >= (uint32_t)'0' &&
            codepoint <= (uint32_t)'9'
        )
    )
    {
        return true;
    }

    return
        codepoint == (uint32_t)' ' ||
        codepoint == (uint32_t)'\'' ||
        codepoint == (uint32_t)'-' ||
        codepoint == (uint32_t)'.';
}

/*
 * Validate a complete player-entered operator name.
 *
 * A name must fit the fixed persistent field, contain only supported glyphs
 * and include at least one letter or digit so whitespace/punctuation-only
 * values cannot masquerade as an assigned identity.
 */
bool Floppy144DiscoveryProfileOperatorNameValid(
    const char *name
)
{
    size_t length;
    size_t index;
    bool has_alphanumeric;

    if(name == NULL)
    {
        return false;
    }

    length =
        strlen(name);

    if(
        length == 0U ||
        length >
            (size_t)FLOPPY144_PROFILE_NAME_MAX_LENGTH
    )
    {
        return false;
    }

    has_alphanumeric =
        false;

    for(
        index = 0U;
        index < length;
        ++index
    )
    {
        uint32_t codepoint =
            (uint32_t)(unsigned char)name[index];

        if(
            !Floppy144DiscoveryProfileOperatorNameCharacterSupported(
                codepoint
            )
        )
        {
            return false;
        }

        if(
            (
                codepoint >= (uint32_t)'A' &&
                codepoint <= (uint32_t)'Z'
            ) ||
            (
                codepoint >= (uint32_t)'a' &&
                codepoint <= (uint32_t)'z'
            ) ||
            (
                codepoint >= (uint32_t)'0' &&
                codepoint <= (uint32_t)'9'
            )
        )
        {
            has_alphanumeric =
                true;
        }
    }

    return has_alphanumeric;
}

/*
 * Report whether persistent operator identity has been assigned.
 *
 * Legacy profiles are considered assigned when the stored field is non-empty;
 * they remain load-compatible even if they predate the S4C-02 entry rules.
 */
bool Floppy144DiscoveryProfileHasOperatorName(
    const Floppy144DiscoveryProfile *profile
)
{
    return
        profile != NULL &&
        profile->operator_name[0] != '\0';
}

/*
 * Return the persistent operator name for later personalisation systems.
 */
const char *Floppy144DiscoveryProfileOperatorName(
    const Floppy144DiscoveryProfile *profile
)
{
    if(profile == NULL)
    {
        return "";
    }

    return profile->operator_name;
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
        !Floppy144DiscoveryProfileOperatorNameValid(
            name
        )
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

Floppy144OperatorBodyStyle Floppy144DiscoveryProfileBodyStyle(
    const Floppy144DiscoveryProfile *profile
)
{
    if(
        profile == NULL ||
        profile->body_style >=
            (uint8_t)FLOPPY144_OPERATOR_BODY_STYLE_COUNT
    )
    {
        return FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT;
    }

    return
        (Floppy144OperatorBodyStyle)profile->body_style;
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

/*
 * Count the distinct collections recorded across all recoveries.
 */
uint32_t Floppy144DiscoveryProfileCollectionsEverRestoredCount(
    const Floppy144DiscoveryProfile *profile
)
{
    uint32_t collection_index;
    uint32_t count;

    if(profile == NULL)
    {
        return 0U;
    }

    count =
        0U;

    for(
        collection_index = 0U;
        collection_index < (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        if(
            Floppy144DiscoveryProfileCollectionEverRestored(
                profile,
                (Floppy144CollectionId)collection_index
            )
        )
        {
            ++count;
        }
    }

    return count;
}

/*
 * Count the distinct evidence items recorded across all recoveries.
 */
uint32_t Floppy144DiscoveryProfileEvidenceEverEstablishedCount(
    const Floppy144DiscoveryProfile *profile
)
{
    uint32_t evidence_index;
    uint32_t count;

    if(profile == NULL)
    {
        return 0U;
    }

    count =
        0U;

    for(
        evidence_index = 0U;
        evidence_index < (uint32_t)FLOPPY144_EVIDENCE_COUNT;
        ++evidence_index
    )
    {
        if(
            Floppy144DiscoveryProfileEvidenceEverEstablished(
                profile,
                (Floppy144EvidenceId)evidence_index
            )
        )
        {
            ++count;
        }
    }

    return count;
}

bool Floppy144DiscoveryProfileRecordCompletion(
    Floppy144DiscoveryProfile *profile,
    const Floppy144RunState *run_state,
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    uint32_t uEvidence;
    uint32_t uRecoveredEvidence=0U;

    if(profile==NULL||run_state==NULL)return false;

    memset(
        profile->latest_completion_evidence,
        0,
        sizeof(profile->latest_completion_evidence)
    );

    for(uEvidence=0U;uEvidence<(uint32_t)FLOPPY144_EVIDENCE_COUNT;++uEvidence)
    {
        if(
            Floppy144RunStateEvidenceEstablished(
                run_state,
                (Floppy144EvidenceId)uEvidence
            )
        )
        {
            (void)Floppy144DiscoveryProfileBitSet(
                profile->latest_completion_evidence,
                uEvidence
            );
            ++uRecoveredEvidence;
        }
    }

    if(profile->completed_recoveries!=UINT32_MAX)
    {
        ++profile->completed_recoveries;
    }

    profile->latest_completion_evidence_percent=
        (uint8_t)(
            FLOPPY144_EVIDENCE_COUNT==0
            ? 0U
            : (uRecoveredEvidence*100U)/(uint32_t)FLOPPY144_EVIDENCE_COUNT
        );

    profile->latest_completion_flags=
        (uint8_t)(
            (evidence_resolved
                ? FLOPPY144_PROFILE_COMPLETION_EVIDENCE_RESOLVED
                : 0U) |
            (capacity_exhausted
                ? FLOPPY144_PROFILE_COMPLETION_CAPACITY_EXHAUSTED
                : 0U)
        );

    profile->latest_completion_recovered_kb=
        (uint16_t)Floppy144RunStateRecoveredKb(run_state);

    profile->dirty=1U;
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
