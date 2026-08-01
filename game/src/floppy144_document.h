/*
 * Floppy//144 - authored document registry
 *
 * Connects recovered authored records to their catalogue metadata, display
 * treatment and effects. Other systems can query the registry without knowing
 * which collection or index contains a particular document.
 */

#pragma once

#include "floppy144_effect.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Authored document display treatments
 *
 * Rendering remains in the catalogue module for now, but the registry decides
 * which treatment belongs to each document.
 */

typedef enum Floppy144DocumentView
{
    FLOPPY144_DOCUMENT_VIEW_HR02_DESK_REALLOCATION = 0,
    FLOPPY144_DOCUMENT_VIEW_FA03_SUPPRESSION_SERVICE,

    FLOPPY144_DOCUMENT_VIEW_DR01_HELP_TERMINAL_ACCESS,
    FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RESTORATION,
    FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RECORDS
} Floppy144DocumentView;

/*
 * Immutable authored-document metadata
 *
 * record_index is zero-based. Optional ID and title overrides replace the
 * procedurally generated catalogue values when present.
 */

typedef struct Floppy144DocumentDefinition
{
    Floppy144CollectionId collection;
    uint32_t record_index;

    const char *record_id_override;
    const char *title_override;

    Floppy144DocumentView view;

    const Floppy144Effect *effects;
    uint32_t effect_count;
} Floppy144DocumentDefinition;

/*
 * Locate an authored document matching a collection and catalogue index.
 */

const Floppy144DocumentDefinition *Floppy144DocumentGet(
    Floppy144CollectionId collection,
    uint32_t record_index
);

/*
 * Apply every effect registered against an authored document.
 *
 * Returns true when the selected record exists in the registry.
 */

bool Floppy144DocumentApplyEffects(
    Floppy144WorldState *world,
    Floppy144CollectionId collection,
    uint32_t record_index
);
