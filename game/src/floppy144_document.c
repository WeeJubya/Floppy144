/*
 * Floppy//144 - authored document registry implementation
 */

#include "floppy144_document.h"

#include <stddef.h>

#define FLOPPY144_ARRAY_COUNT(values)                              \
    ((uint32_t)(sizeof(values) / sizeof((values)[0])))

/*
 * Effects declared by the primary-evidence documents in the technical slice.
 */

static const Floppy144Effect floppy144_hr02_038_effects[] =
{
    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_DESK_FOUR_PERSONNEL_FORMS
    }
};

static const Floppy144Effect floppy144_fa03_047_effects[] =
{
    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_SUPPRESSION_CONTROL_PANEL
    }
};

/*
 * Master authored-document table
 *
 * HR-02 record 038 retains its procedurally generated catalogue ID and title.
 * FA-03 record 047 uses a stable recovered-record identity.
 */

static const Floppy144DocumentDefinition
    floppy144_documents[] =
{
    {
        FLOPPY144_COLLECTION_DR01,
        1U,
        NULL,
        NULL,
        FLOPPY144_DOCUMENT_VIEW_DR01_HELP_TERMINAL_ACCESS,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_DR01,
        2U,
        NULL,
        NULL,
        FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RESTORATION,
        NULL,
        0U
    },

    {
        FLOPPY144_COLLECTION_DR01,
        3U,
        NULL,
        NULL,
        FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RECORDS,
        NULL,
        0U
    },
    {
        FLOPPY144_COLLECTION_HR02,
        37U,
        NULL,
        NULL,
        FLOPPY144_DOCUMENT_VIEW_HR02_DESK_REALLOCATION,
        floppy144_hr02_038_effects,
        FLOPPY144_ARRAY_COUNT(floppy144_hr02_038_effects)
    },

    {
        FLOPPY144_COLLECTION_FA03,
        46U,
        "FA-03-RS-0047",
        "SUPPRESSION CONTROL PANEL SERVICE NOTE",
        FLOPPY144_DOCUMENT_VIEW_FA03_SUPPRESSION_SERVICE,
        floppy144_fa03_047_effects,
        FLOPPY144_ARRAY_COUNT(floppy144_fa03_047_effects)
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
