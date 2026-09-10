/*
 * Floppy//144 - authored document registry
 *
 * Connects recovered authored records to their catalogue metadata, display
 * treatment and effects. Other systems can query the registry without knowing
 * which collection or index contains a particular document.
 */

#pragma once

#include "floppy144_effect.h"
#include "floppy144_trigger.h"

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
    FLOPPY144_DOCUMENT_VIEW_HR01_DESK_REALLOCATION = 0,
    FLOPPY144_DOCUMENT_VIEW_FM13_SUPPRESSION_SERVICE,

    FLOPPY144_DOCUMENT_VIEW_DR01_DISK_RECOVERY_INDEX,

    FLOPPY144_DOCUMENT_VIEW_GENERIC
}
Floppy144DocumentView;

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

    /*
     * FLOPPY144_TRIGGER_COUNT means this authored document has no registered
     * persistent trigger.
     */
    Floppy144TriggerId trigger;

    /*
     * Direct effects remain as a compatibility path for the current technical
     * slice. Final trigger documents should route their progression through
     * the trigger registry instead.
     */
    const Floppy144Effect *effects;
    uint32_t effect_count;

    /* Complete recovered body text compiled from the canonical JSON. */
    const char *pszBody;
}
Floppy144DocumentDefinition;

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
    Floppy144RunState *run_state,
    Floppy144CollectionId collection,
    uint32_t record_index
);
