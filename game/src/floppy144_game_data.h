#pragma once
/*
 * Floppy//144 generic generated game-data registry.
 *
 * Stage 2 keeps game-specific facts in the canonical JSON. The development
 * compiler turns that JSON into flat immutable records; runtime code understands
 * only reusable verbs/conditions and stable IDs.
 */
#include "floppy144_run_state.h"
#include "floppy144_world.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum Floppy144DataRecordKind {
    FLOPPY144_DATA_COLLECTION = 0,
    FLOPPY144_DATA_TRIGGER,
    FLOPPY144_DATA_TRIGGER_CONDITION,
    FLOPPY144_DATA_TRIGGER_EFFECT,
    FLOPPY144_DATA_INTERACTION,
    FLOPPY144_DATA_INTERACTION_PREREQUISITE,
    FLOPPY144_DATA_INTERACTION_EFFECT,
    FLOPPY144_DATA_EVIDENCE,
    FLOPPY144_DATA_EVIDENCE_REQUIREMENT,
    FLOPPY144_DATA_EVIDENCE_CONDITION,
    FLOPPY144_DATA_ROOM,
    FLOPPY144_DATA_ROOM_REGION,
    FLOPPY144_DATA_CONNECTION,
    FLOPPY144_DATA_CONNECTION_CONDITION,
    FLOPPY144_DATA_FURNITURE,
    FLOPPY144_DATA_FIXTURE,
    FLOPPY144_DATA_PHYSICAL_ITEM,
    FLOPPY144_DATA_COLOUR,
    FLOPPY144_DATA_DRAWING,
    FLOPPY144_DATA_DRAWING_PRIMITIVE,
    FLOPPY144_DATA_RELATIONSHIP,
    FLOPPY144_DATA_AMBIENT,
    FLOPPY144_DATA_AMBIENT_RANGE,
    FLOPPY144_DATA_NOTEBOOK,
    FLOPPY144_DATA_RECORD_KIND_COUNT
} Floppy144DataRecordKind;

typedef struct Floppy144DataRecord {
    Floppy144DataRecordKind eKind;
    const char *pszId,*pszA,*pszB,*pszC,*pszD,*pszE,*pszF;
    int32_t n0,n1,n2,n3,n4,n5;
    uint8_t b0;
} Floppy144DataRecord;

uint32_t Floppy144GameDataRecordCount(void);
const Floppy144DataRecord *Floppy144GameDataRecordAt(uint32_t uIndex);
const Floppy144DataRecord *Floppy144GameDataFind(Floppy144DataRecordKind eKind,const char *pszId);
Floppy144TriggerId Floppy144GameDataTriggerId(const char *pszId);
Floppy144InteractionId Floppy144GameDataInteractionId(const char *pszId);
Floppy144EvidenceId Floppy144GameDataEvidenceId(const char *pszId);
Floppy144CollectionId Floppy144GameDataCollectionId(const char *pszId);
Floppy144RoomId Floppy144GameDataRoomId(const char *pszId);
Floppy144CapabilityId Floppy144GameDataCapabilityId(const char *pszId);
bool Floppy144GameDataConnectionUnlocked(const Floppy144RunState *pState,const char *pszConnectionId);
bool Floppy144GameDataCollectionEnabled(const Floppy144RunState *pState,const char *pszCollectionId);
bool Floppy144GameDataPhysicalItemRevealed(const Floppy144RunState *pState,const char *pszPhysicalItemId);
bool Floppy144GameDataFactRecorded(const Floppy144RunState *pState,const char *pszFactId);
bool Floppy144GameDataWorkstreamAvailable(const Floppy144RunState *pState,const char *pszWorkstream);
bool Floppy144GameDataRoomAccessible(const Floppy144RunState *pState,const char *pszRoomId);
bool Floppy144GameDataRoomTransitionAllowed(const Floppy144RunState *pState,Floppy144RoomId eFromRoom,Floppy144RoomId eToRoom);
bool Floppy144GameDataConditionSatisfied(const Floppy144RunState *pState,const char *pszKind,const char *pszTarget);
bool Floppy144GameDataExecuteEffect(Floppy144WorldState *pWorld,Floppy144RunState *pState,const char *pszOperation,const char *pszTarget);
bool Floppy144GameDataTriggerCanFire(const Floppy144RunState *pState,Floppy144TriggerId eTrigger);

/*
 * Query whether an authored trigger document is currently readable.
 *
 * Most trigger documents remain readable whenever their collection has been
 * restored, even if the trigger itself is not yet actionable. Only explicit
 * workstream-availability conditions defer document access.
 */
bool Floppy144GameDataTriggerDocumentAccessible(
    const Floppy144RunState *pState,
    Floppy144TriggerId eTrigger
);
bool Floppy144GameDataTriggerTryFire(Floppy144WorldState *pWorld,Floppy144RunState *pState,Floppy144TriggerId eTrigger);
bool Floppy144GameDataInteractionCanRun(const Floppy144RunState *pState,Floppy144InteractionId eInteraction);
bool Floppy144GameDataInteractionTryRun(Floppy144WorldState *pWorld,Floppy144RunState *pState,Floppy144InteractionId eInteraction);
void Floppy144GameDataResolveEvidence(Floppy144RunState *pState);
const Floppy144DataRecord *Floppy144GameDataAmbientForDate(const char *pszTargetId,uint32_t uMonth,uint32_t uDay);

/* Data-driven Notebook view over persistent recovered knowledge. */
uint32_t Floppy144GameDataNotebookEntryCount(const Floppy144RunState *pState);
const Floppy144DataRecord *Floppy144GameDataNotebookEntryAt(
    const Floppy144RunState *pState,
    uint32_t uVisibleIndex
);
