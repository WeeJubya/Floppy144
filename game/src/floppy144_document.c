/*
 * Floppy//144 - authored document registry implementation
 */

#include "floppy144_document.h"
#include "floppy144_trigger_engine.h"

#include <stddef.h>

#define FLOPPY144_ARRAY_COUNT(values)                              \
    ((uint32_t)(sizeof(values) / sizeof((values)[0])))

/*
 * Master authored-document table
 *
 * Final readable-document identities are installed collection by collection.
 * Unspecified record IDs continue to use catalogue generation until archive
 * population is completed.
 */

static const Floppy144DocumentDefinition
    floppy144_documents[] =
{
/*
 * Generated document rows contain stable collection/index placement, record
 * identity, trigger ID and the complete authored body text. The JSON remains
 * the only hand-edited content source.
 */
#include "floppy144_documents.generated.inc"
};

#define FLOPPY144_DOCUMENT_COUNT                                   \
    FLOPPY144_ARRAY_COUNT(floppy144_documents)

/*
 * Locate one authored document.
 */

const Floppy144DocumentDefinition *Floppy144DocumentGet(
    Floppy144CollectionId collection,
    uint32_t record_index
)
{
    uint32_t document_index;

    for(
        document_index = 0;
        document_index < FLOPPY144_DOCUMENT_COUNT;
        ++document_index
    )
    {
        const Floppy144DocumentDefinition *document =
            &floppy144_documents[document_index];

        if(
            document->collection == collection &&
            document->record_index == record_index
        )
        {
            return document;
        }
    }

    return NULL;
}

/*
 * Find the next actionable trigger document for one restored collection.
 *
 * The document table is generated from canonical JSON, so source order is the
 * authored progression order. Trigger eligibility remains owned by the generic
 * trigger engine; this function only joins those two existing registries.
 */
const Floppy144DocumentDefinition *Floppy144DocumentFirstPendingTrigger(
    const Floppy144RunState *pRunState,
    Floppy144CollectionId eCollection
)
{
    uint32_t uDocumentIndex;

    if(
        pRunState == NULL ||
        (uint32_t)eCollection >=
            (uint32_t)FLOPPY144_COLLECTION_COUNT ||
        !Floppy144RunStateCollectionRestored(
            pRunState,
            eCollection
        )
    )
    {
        return NULL;
    }

    for(
        uDocumentIndex = 0U;
        uDocumentIndex < FLOPPY144_DOCUMENT_COUNT;
        ++uDocumentIndex
    )
    {
        const Floppy144DocumentDefinition *pDocument =
            &floppy144_documents[uDocumentIndex];

        if(
            pDocument->collection == eCollection &&
            pDocument->trigger != FLOPPY144_TRIGGER_COUNT &&
            Floppy144TriggerCanFire(
                pRunState,
                pDocument->trigger
            )
        )
        {
            return pDocument;
        }
    }

    return NULL;
}

/*
 * Apply every effect registered against one authored document.
 */

bool Floppy144DocumentApplyEffects(
    Floppy144WorldState *world,
    Floppy144RunState *run_state,
    Floppy144CollectionId collection,
    uint32_t record_index
)
{
    const Floppy144DocumentDefinition *document =
        Floppy144DocumentGet(
            collection,
            record_index
        );

    if(document == NULL)
    {
        return false;
    }

    /*
     * Authored trigger documents route through the persistent fire-once trigger
     * engine. Existing technical-slice direct effects remain supported separately
     * until their final authored replacements are registered.
     */
    if(
        document->trigger !=
        FLOPPY144_TRIGGER_COUNT
    )
    {
        Floppy144TriggerTryFire(
            world,
            run_state,
            document->trigger
        );
    }

    if(document->effect_count > 0U)
    {
        Floppy144ApplyEffects(
            world,
            run_state,
            document->effects,
            document->effect_count
        );
    }

    return true;
}
