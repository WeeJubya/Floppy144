#include "floppy144_interaction_engine.h"

#include <stddef.h>
#include <string.h>

/*
 * Find the interaction whose canonical physical_source matches one P-/fixture
 * identifier. Generated source order is the stable interaction ordinal.
 */
Floppy144InteractionId Floppy144InteractionForPhysicalSource(
    const char *pszPhysicalSource
)
{
    uint32_t uIndex;
    int32_t nOrdinal = 0;

    if(pszPhysicalSource == NULL)
    {
        return FLOPPY144_INTERACTION_COUNT;
    }

    for(
        uIndex = 0U;
        uIndex < Floppy144GameDataRecordCount();
        ++uIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uIndex);

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_INTERACTION
        )
        {
            continue;
        }

        if(
            pRecord->pszB != NULL &&
            strcmp(
                pRecord->pszB,
                pszPhysicalSource
            ) == 0
        )
        {
            return
                (Floppy144InteractionId)nOrdinal;
        }

        ++nOrdinal;
    }

    return FLOPPY144_INTERACTION_COUNT;
}

/*
 * Translate a compact interaction enum back to its generated data row.
 * Presentation code uses this to report evidence/physical source metadata
 * without duplicating JSON-derived interaction tables.
 */
const Floppy144DataRecord *Floppy144InteractionRecord(
    Floppy144InteractionId eInteraction
)
{
    uint32_t uIndex;
    uint32_t uOrdinal = 0U;

    if(
        (uint32_t)eInteraction >=
        (uint32_t)FLOPPY144_INTERACTION_COUNT
    )
    {
        return NULL;
    }

    for(
        uIndex = 0U;
        uIndex < Floppy144GameDataRecordCount();
        ++uIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uIndex);

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_INTERACTION
        )
        {
            continue;
        }

        if(uOrdinal == (uint32_t)eInteraction)
        {
            return pRecord;
        }

        ++uOrdinal;
    }

    return NULL;
}

bool Floppy144InteractionCanRun(
    const Floppy144RunState *pState,
    Floppy144InteractionId eInteraction
)
{
    return
        Floppy144GameDataInteractionCanRun(
            pState,
            eInteraction
        );
}

bool Floppy144InteractionTryRun(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState,
    Floppy144InteractionId eInteraction
)
{
    return
        Floppy144GameDataInteractionTryRun(
            pWorld,
            pState,
            eInteraction
        );
}
