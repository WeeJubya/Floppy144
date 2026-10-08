/*
 * Floppy//144 - authored document registry implementation
 */

#include "floppy144_document.h"
#include "floppy144_game_data.h"
#include "floppy144_trigger_engine.h"
#include "floppy144_variation.h"

#include <stddef.h>
#include <string.h>

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
    /*
     * One S4G orphan record, absent from collection JSON and generation totals.
     * It still uses the existing document registry and normal document viewer.
     */
    {
        FLOPPY144_GREY_DOOR_RECORD_COLLECTION, FLOPPY144_GREY_DOOR_RECORD_INDEX,
        FLOPPY144_GREY_DOOR_RECORD_ID, "Unallocated Floor Area Notice",
        FLOPPY144_DOCUMENT_VIEW_GENERIC, FLOPPY144_TRIGGER_COUNT,
        NULL, 0U,
        "GOVERNMENT DEPARTMENT OF RECORDS\n"
        "BUILDING SERVICES / FLOORPLAN RECONCILIATION\n"
        "REF: DR-00-RS-0144\n\n"
        "An unallocated corridor return remains on the duplicate plan.\n"
        "No room number has been assigned. The occupied floor area\n"
        "is greater than the external building measurements allow.\n\n"
        "Maintenance reports a desk lamp burning after closure.\n"
        "Estates confirms that the corresponding wall is continuous.\n"
        "Both statements have been filed as accurate.\n\n"
        "The discrepancy was marked RESOLVED without a site visit.\n"
        "Do not amend the master plan.",
        false, false
    },
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
 * DR-04 workstream placement uses immutable trigger identities rather than
 * S4E-08 generated record numbers. This is a stateless presentation mapping.
 */
static const Floppy144DocumentDefinition *Floppy144DocumentDr04Partner(
    const Floppy144DocumentDefinition *document
)
{
    uint32_t index;
    Floppy144TriggerId other;
    if(document == NULL || document->collection != FLOPPY144_COLLECTION_DR04)
        return NULL;
    if(document->trigger == FLOPPY144_TRIGGER_T010)
        other = FLOPPY144_TRIGGER_T011;
    else if(document->trigger == FLOPPY144_TRIGGER_T011)
        other = FLOPPY144_TRIGGER_T010;
    else
        return NULL;

    for(index = 0U; index < FLOPPY144_DOCUMENT_COUNT; ++index)
    {
        const Floppy144DocumentDefinition *candidate = &floppy144_documents[index];
        if(candidate->collection == FLOPPY144_COLLECTION_DR04 &&
           candidate->trigger == other)
            return candidate;
    }
    return NULL;
}

bool Floppy144DocumentDr04Swapped(uint32_t recovery_seed)
{
    return Floppy144VariationRange(
        recovery_seed, "dr04.workstream-swap.v1", "DR-04", 2U
    ) == 1U;
}

const Floppy144DocumentDefinition *Floppy144DocumentGetForSeed(
    Floppy144CollectionId collection, uint32_t slot, uint32_t recovery_seed
)
{
    const Floppy144DocumentDefinition *document =
        Floppy144DocumentGet(collection, slot);
    const Floppy144DocumentDefinition *partner;
    if(!Floppy144DocumentDr04Swapped(recovery_seed))
        return document;
    partner = Floppy144DocumentDr04Partner(document);
    return partner != NULL ? partner : document;
}

const char *Floppy144DocumentRecordIdForSeed(
    const Floppy144DocumentDefinition *document, uint32_t recovery_seed
)
{
    const Floppy144DocumentDefinition *partner =
        Floppy144DocumentDr04Swapped(recovery_seed)
        ? Floppy144DocumentDr04Partner(document) : NULL;
    return partner != NULL ? partner->record_id_override :
        (document != NULL ? document->record_id_override : NULL);
}

uint32_t Floppy144DocumentSlotForSeed(
    const Floppy144DocumentDefinition *document, uint32_t recovery_seed
)
{
    const Floppy144DocumentDefinition *partner =
        Floppy144DocumentDr04Swapped(recovery_seed)
        ? Floppy144DocumentDr04Partner(document) : NULL;
    return partner != NULL ? partner->record_index :
        (document != NULL ? document->record_index : 0U);
}

/*
 * Resolve an authored record by its exact stable player-facing ID.
 *
 * The procedural catalogue is intentionally not consulted here. This is the
 * fallback used when a collection has authored records but its wider generated
 * record index has not yet been populated.
 */
bool Floppy144DocumentFindRecordId(
    const char *pszRecordId,
    Floppy144CollectionId *pCollection,
    uint32_t *pRecordIndex
)
{
    uint32_t uDocumentIndex;

    if(
        pszRecordId == NULL ||
        pCollection == NULL ||
        pRecordIndex == NULL
    )
    {
        return false;
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
            pDocument->record_id_override != NULL &&
            strcmp(
                pDocument->record_id_override,
                pszRecordId
            ) == 0
        )
        {
            *pCollection = pDocument->collection;
            *pRecordIndex = pDocument->record_index;
            return true;
        }
    }

    return false;
}

/*
 * Find the next actionable trigger document for one restored collection.
 *
 * The document table is generated from canonical JSON, so source order is the
 * authored progression order. Trigger eligibility remains owned by the generic
 * trigger engine; this function only joins those two existing registries.
 */
const Floppy144DocumentDefinition *Floppy144DocumentPendingTriggerAt(
    const Floppy144RunState *pRunState,
    Floppy144CollectionId eCollection,
    uint32_t uOrdinal
)
{
    uint32_t uDocumentIndex;
    uint32_t uMatchOrdinal = 0U;

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
            if(uMatchOrdinal == uOrdinal)
            {
                return pDocument;
            }

            ++uMatchOrdinal;
        }
    }

    return NULL;
}

const Floppy144DocumentDefinition *Floppy144DocumentFirstPendingTrigger(
    const Floppy144RunState *pRunState,
    Floppy144CollectionId eCollection
)
{
    return
        Floppy144DocumentPendingTriggerAt(
            pRunState,
            eCollection,
            0U
        );
}

const Floppy144DocumentDefinition *Floppy144DocumentTriggerAt(
    Floppy144CollectionId eCollection,
    uint32_t uOrdinal
)
{
    uint32_t uDocumentIndex;
    uint32_t uMatchOrdinal = 0U;

    if(
        (uint32_t)eCollection >=
            (uint32_t)FLOPPY144_COLLECTION_COUNT
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
            pDocument->trigger != FLOPPY144_TRIGGER_COUNT
        )
        {
            if(uMatchOrdinal == uOrdinal)
            {
                return pDocument;
            }

            ++uMatchOrdinal;
        }
    }

    return NULL;
}

const Floppy144DocumentDefinition *Floppy144DocumentRecoveryEntryPoint(
    Floppy144CollectionId eCollection
)
{
    uint32_t uDocumentIndex;

    if(
        (uint32_t)eCollection >=
            (uint32_t)FLOPPY144_COLLECTION_COUNT
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
            pDocument->recovery_entry_point
        )
        {
            return pDocument;
        }
    }

    return NULL;
}

const Floppy144DocumentDefinition *Floppy144DocumentChoiceBriefing(
    Floppy144CollectionId eCollection
)
{
    uint32_t uDocumentIndex;

    if(
        (uint32_t)eCollection >=
            (uint32_t)FLOPPY144_COLLECTION_COUNT
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
            pDocument->offer_pending_trigger_choices
        )
        {
            return pDocument;
        }
    }

    return NULL;
}

/*
 * Query whether one recovered document is currently readable.
 *
 * Trigger documents are special because the trigger can itself represent an
 * authored gate. Once fired, the document remains readable permanently. Before
 * firing, the generic trigger engine decides whether the document is currently
 * available in this recovery path.
 */
bool Floppy144DocumentAccessible(
    const Floppy144RunState *pRunState,
    Floppy144CollectionId eCollection,
    uint32_t uRecordIndex
)
{
    const Floppy144DocumentDefinition *pDocument;

    if(pRunState == NULL) return false;
    if(eCollection == FLOPPY144_GREY_DOOR_RECORD_COLLECTION)
        return uRecordIndex == FLOPPY144_GREY_DOOR_RECORD_INDEX &&
            Floppy144RunStateCollectionRestored(
                pRunState, FLOPPY144_COLLECTION_DR01
            );
    if((uint32_t)eCollection >= (uint32_t)FLOPPY144_COLLECTION_COUNT)
        return false;

    pDocument =
        Floppy144DocumentGetForSeed(
            eCollection, uRecordIndex, pRunState->recovery_seed
        );

    /*
     * Index-only/generated catalogue entries have no authored trigger gate.
     */
    if(pDocument == NULL)
    {
        return true;
    }

    if(pDocument->trigger == FLOPPY144_TRIGGER_COUNT)
    {
        return true;
    }

    if(
        Floppy144RunStateTriggerFired(
            pRunState,
            pDocument->trigger
        )
    )
    {
        return true;
    }

    return
        Floppy144GameDataTriggerDocumentAccessible(
            pRunState,
            pDocument->trigger
        );
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
        Floppy144DocumentGetForSeed(
            collection, record_index,
            run_state != NULL ? run_state->recovery_seed : 0U
        );

    if(document == NULL)
    {
        return false;
    }

    /* S4G: bypass generic evidence reconciliation and progression effects. */
    if(collection == FLOPPY144_GREY_DOOR_RECORD_COLLECTION)
    {
        if(record_index != FLOPPY144_GREY_DOOR_RECORD_INDEX ||
           !Floppy144DocumentAccessible(run_state, collection, record_index))
            return false;
        (void)Floppy144RunStateGreyDoorDiscover(run_state);
        return true;
    }

    /*
     * Reconcile derived evidence before evaluating a trigger document.
     *
     * Older saves can legitimately contain completed interaction bits from an
     * earlier progression implementation while the corresponding derived
     * evidence bit is absent. Without this reconciliation, reopening the
     * document still sees a stale prerequisite and silently refuses to fire.
     *
     * ResolveEvidence is conservative: it only establishes evidence whose
     * required interactions and conditions are already satisfied, so this
     * repairs persisted state without granting discoveries the player has not
     * actually made.
     */
    Floppy144GameDataResolveEvidence(
        run_state
    );

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
