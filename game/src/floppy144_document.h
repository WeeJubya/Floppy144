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

    /*
     * Optional recovery guidance metadata, authored in canonical JSON.
     *
     * recovery_entry_point marks a neutral document that should be suggested
     * immediately after its collection is restored, before branch-trigger
     * documents are offered.
     *
     * offer_pending_trigger_choices asks the terminal to show the authored
     * trigger-document choices in this collection after this record is opened.
     * This describes the document's intended branching guidance rather than
     * whatever subset of triggers happens still to be pending in run state.
     */
    bool recovery_entry_point;
    bool offer_pending_trigger_choices;
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
 * Resolve one exact authored player-facing record ID.
 *
 * This is deliberately independent of the procedural catalogue record count.
 * Some later collections are still catalogue stubs while already containing
 * fully authored trigger documents. The terminal must still be able to OPEN
 * those records when progression points the player at their stable IDs.
 */
bool Floppy144DocumentFindRecordId(
    const char *pszRecordId,
    Floppy144CollectionId *pCollection,
    uint32_t *pRecordIndex
);

/*
 * Locate the first trigger document in a restored collection whose trigger is
 * currently eligible and has not already fired. This is a generic progression
 * query: callers do not need to know any story-specific document or trigger ID.
 */
const Floppy144DocumentDefinition *Floppy144DocumentFirstPendingTrigger(
    const Floppy144RunState *pRunState,
    Floppy144CollectionId eCollection
);

/*
 * Locate the authored recovery entry point for one collection, if present.
 */
const Floppy144DocumentDefinition *Floppy144DocumentRecoveryEntryPoint(
    Floppy144CollectionId eCollection
);

/*
 * Locate the neutral authored briefing which offers this collection's
 * pending trigger choices, if one exists.
 */
const Floppy144DocumentDefinition *Floppy144DocumentChoiceBriefing(
    Floppy144CollectionId eCollection
);

/*
 * Locate the Nth currently eligible trigger document in authored order.
 * Ordinal zero is equivalent to Floppy144DocumentFirstPendingTrigger().
 */
const Floppy144DocumentDefinition *Floppy144DocumentPendingTriggerAt(
    const Floppy144RunState *pRunState,
    Floppy144CollectionId eCollection,
    uint32_t uOrdinal
);

/*
 * Locate the Nth authored trigger document in one collection, regardless of
 * whether that trigger has already fired or is temporarily deferred.
 *
 * This is used by neutral briefing documents whose player-facing purpose is to
 * describe the available workstreams consistently even when the player later
 * reopens the briefing after progressing one or both branches.
 */
const Floppy144DocumentDefinition *Floppy144DocumentTriggerAt(
    Floppy144CollectionId eCollection,
    uint32_t uOrdinal
);

/*
 * Query whether a recovered document is currently readable.
 *
 * Ordinary documents are always readable once their collection has been
 * restored. Trigger documents remain readable after their trigger has fired,
 * but a not-yet-fired trigger document is only readable while its trigger is
 * currently eligible. This allows authored branch-choice documents to be
 * deferred without hiding already-recovered material permanently.
 */
bool Floppy144DocumentAccessible(
    const Floppy144RunState *pRunState,
    Floppy144CollectionId eCollection,
    uint32_t uRecordIndex
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
