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
    {
        FLOPPY144_COLLECTION_DR01,
        0U,
        NULL,
        NULL,
        FLOPPY144_DOCUMENT_VIEW_DR01_DISK_RECOVERY_INDEX,
        FLOPPY144_TRIGGER_T001,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_HR01,
        0U,
        NULL,
        "SITE ESTABLISHMENT REGISTER",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_T002,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_HR01,
        1U,
        "HR-01-RS-0107",
        "MAIN OFFICE STAFFING ALLOCATION",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_T003,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_HR01,
        2U,
        NULL,
        "ORGANISATIONAL CHART",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_COUNT,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_HR01,
        3U,
        NULL,
        "TEMPORARY DESK REALLOCATION NOTICE",
        FLOPPY144_DOCUMENT_VIEW_HR01_DESK_REALLOCATION,
        FLOPPY144_TRIGGER_COUNT,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_HR01,
        4U,
        NULL,
        "VACANCY & HOT-DESK PROVISION",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_COUNT,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_HR01,
        5U,
        NULL,
        "STAFF CONTACT DIRECTORY",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_COUNT,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_FM13,
        0U,
        NULL,
        "SUPPRESSION CONTROL PANEL SERVICE NOTE",
        FLOPPY144_DOCUMENT_VIEW_FM13_SUPPRESSION_SERVICE,
        FLOPPY144_TRIGGER_T006,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_FM13,
        1U,
        NULL,
        "MANUAL DISCHARGE CIRCUIT TEST RECORD",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_T007,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_FM07,
        0U,
        NULL,
        "FINAL DECOMMISSIONING SCHEDULE",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_T008,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_FM07,
        1U,
        NULL,
        "RETAINED SERVICES SCHEDULE",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_T009,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_FM04,
        0U,
        NULL,
        "FACILITIES ACCESS AUTHORISATION",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_T004,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_FM04,
        1U,
        NULL,
        "INTERNAL CIRCULATION ACCESS PLAN",
        FLOPPY144_DOCUMENT_VIEW_GENERIC,
        FLOPPY144_TRIGGER_T005,
        NULL,
        0U
    }
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
