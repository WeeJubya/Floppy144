/*
 * Floppy//144 - authored document registry implementation
 */

#include "floppy144_document.h"

#include <stddef.h>

/*
 * Effects declared by the two primary-evidence documents in the technical
 * slice.
 */

static const Floppy144Effect floppy144_hr02_038_effects[] =
{
    {
        FLOPPY144_EFFECT_FIND_COLLECTION_EVIDENCE,
        FLOPPY144_COLLECTION_HR02
    }
};

static const Floppy144Effect floppy144_fa03_063_effects[] =
{
    {
        FLOPPY144_EFFECT_FIND_COLLECTION_EVIDENCE,
        FLOPPY144_COLLECTION_FA03
    }
};

/*
 * Master authored-document table
 *
 * Adding another standard evidence document requires another effect sequence
 * and one table entry. Catalogue input and main-loop code remain unchanged.
 */

static const Floppy144DocumentDefinition
    floppy144_documents[] =
{
    {
        FLOPPY144_COLLECTION_HR02,
        37U,
        floppy144_hr02_038_effects,
        1U
    },

    {
        FLOPPY144_COLLECTION_FA03,
        62U,
        floppy144_fa03_063_effects,
        1U
    }
};

#define FLOPPY144_DOCUMENT_COUNT                                  \
    ((uint32_t)(sizeof(floppy144_documents) /                     \
                sizeof(floppy144_documents[0])))

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

    Floppy144ApplyEffects(
        world,
        document->effects,
        document->effect_count
    );

    return true;
}
