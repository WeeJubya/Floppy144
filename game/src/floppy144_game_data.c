/*
 * Floppy//144 - generic data-driven Stage 2 runtime.
 *
 * This module contains engine vocabulary, not story IDs. Trigger, interaction,
 * evidence, connection and collection facts come from generated JSON records.
 */
#include "floppy144_game_data.h"
#include "floppy144_collection_registry.h"
#include "floppy144_object_registry.h"
#include <stddef.h>
#include <string.h>

#define F144_COUNT(a) ((uint32_t)(sizeof(a)/sizeof((a)[0])))
#define FLOPPY144_DATA_RECORD(kind,id,a,b,c,d,e,f,n0,n1,n2,n3,n4,n5,b0) {kind,id,a,b,c,d,e,f,n0,n1,n2,n3,n4,n5,(uint8_t)(b0)},
static const Floppy144DataRecord g_asGameData[]={
#include "floppy144_game_data.generated.inc"
};
#undef FLOPPY144_DATA_RECORD

static const char *const g_apszRoomIds[FLOPPY144_ROOM_COUNT]={
#define FLOPPY144_ROOM(symbol,name) #symbol,
#include "floppy144_rooms.def"
#undef FLOPPY144_ROOM
};
static const char *const g_apszCapabilityIds[FLOPPY144_CAPABILITY_COUNT]={
#define FLOPPY144_CAPABILITY(symbol) #symbol,
#include "floppy144_capabilities.def"
#undef FLOPPY144_CAPABILITY
};
typedef struct Floppy144LegacyObjectMap{Floppy144ObjectId eObject;const char *pszSymbol;}Floppy144LegacyObjectMap;
static const Floppy144LegacyObjectMap g_asLegacyObjects[]={
#define FLOPPY144_OBJECT(symbol,...) {FLOPPY144_OBJECT_##symbol,#symbol},
#include "floppy144_objects.def"
#undef FLOPPY144_OBJECT
};

static bool Floppy144StringEqual(const char *pszA,const char *pszB){return pszA&&pszB&&strcmp(pszA,pszB)==0;}
uint32_t Floppy144GameDataRecordCount(void){return F144_COUNT(g_asGameData);}
const Floppy144DataRecord *Floppy144GameDataRecordAt(uint32_t uIndex){return uIndex<F144_COUNT(g_asGameData)?&g_asGameData[uIndex]:NULL;}
const Floppy144DataRecord *Floppy144GameDataFind(Floppy144DataRecordKind eKind,const char *pszId){uint32_t uIndex;for(uIndex=0;uIndex<F144_COUNT(g_asGameData);++uIndex)if(g_asGameData[uIndex].eKind==eKind&&Floppy144StringEqual(g_asGameData[uIndex].pszId,pszId))return&g_asGameData[uIndex];return NULL;}
static int32_t Floppy144Ordinal(Floppy144DataRecordKind eKind,const char *pszId){uint32_t uIndex;int32_t nOrdinal=0;for(uIndex=0;uIndex<F144_COUNT(g_asGameData);++uIndex){if(g_asGameData[uIndex].eKind!=eKind)continue;if(Floppy144StringEqual(g_asGameData[uIndex].pszId,pszId))return nOrdinal;++nOrdinal;}return-1;}
Floppy144TriggerId Floppy144GameDataTriggerId(const char *pszId){int32_t n=Floppy144Ordinal(FLOPPY144_DATA_TRIGGER,pszId);return(n>=0&&n<(int32_t)FLOPPY144_TRIGGER_COUNT)?(Floppy144TriggerId)n:FLOPPY144_TRIGGER_COUNT;}
Floppy144InteractionId Floppy144GameDataInteractionId(const char *pszId){int32_t n=Floppy144Ordinal(FLOPPY144_DATA_INTERACTION,pszId);return(n>=0&&n<(int32_t)FLOPPY144_INTERACTION_COUNT)?(Floppy144InteractionId)n:FLOPPY144_INTERACTION_COUNT;}
Floppy144EvidenceId Floppy144GameDataEvidenceId(const char *pszId){int32_t n=Floppy144Ordinal(FLOPPY144_DATA_EVIDENCE,pszId);return(n>=0&&n<(int32_t)FLOPPY144_EVIDENCE_COUNT)?(Floppy144EvidenceId)n:FLOPPY144_EVIDENCE_COUNT;}
Floppy144CollectionId Floppy144GameDataCollectionId(const char *pszId){uint32_t u;for(u=0;u<(uint32_t)FLOPPY144_COLLECTION_COUNT;++u){const Floppy144CollectionDefinition*p=Floppy144CollectionGet((Floppy144CollectionId)u);if(p&&Floppy144StringEqual(p->code,pszId))return(Floppy144CollectionId)u;}return FLOPPY144_COLLECTION_COUNT;}
Floppy144RoomId Floppy144GameDataRoomId(const char *pszId){uint32_t u;for(u=0;u<(uint32_t)FLOPPY144_ROOM_COUNT;++u)if(Floppy144StringEqual(g_apszRoomIds[u],pszId))return(Floppy144RoomId)u;return FLOPPY144_ROOM_COUNT;}
Floppy144CapabilityId Floppy144GameDataCapabilityId(const char *pszId){uint32_t u;for(u=0;u<(uint32_t)FLOPPY144_CAPABILITY_COUNT;++u)if(Floppy144StringEqual(g_apszCapabilityIds[u],pszId))return(Floppy144CapabilityId)u;return FLOPPY144_CAPABILITY_COUNT;}

/*
 * Bridge canonical physical-item IDs onto the small legacy object registry
 * while Stage 3 migration is still in progress.
 *
 * Prefer the object's declared canonical physical source. This correctly maps
 * items such as P-012 onto SUPPRESSION_CONTROL_PANEL even though the old
 * internal object symbol does not contain the physical ID. The historical
 * symbol-prefix fallback remains for older migrated objects.
 */
static Floppy144ObjectId Floppy144LegacyObjectFromPhysicalId(
    const char *pszPhysicalId
)
{
    char szPrefix[16];
    uint32_t uRead = 0U;
    uint32_t uWrite = 0U;
    uint32_t uIndex;

    if(pszPhysicalId == NULL)
    {
        return FLOPPY144_OBJECT_NONE;
    }

    for(
        uIndex = 0U;
        uIndex < (uint32_t)FLOPPY144_OBJECT_COUNT;
        ++uIndex
    )
    {
        const Floppy144ObjectDefinition *pDefinition =
            Floppy144ObjectGet(
                (Floppy144ObjectId)uIndex
            );

        if(
            pDefinition != NULL &&
            pDefinition->interaction != NULL &&
            Floppy144StringEqual(
                pDefinition->interaction->pszPhysicalSourceId,
                pszPhysicalId
            )
        )
        {
            return (Floppy144ObjectId)uIndex;
        }
    }

    while(
        pszPhysicalId[uRead] != '\0' &&
        uWrite + 1U < sizeof(szPrefix)
    )
    {
        char c = pszPhysicalId[uRead++];

        if(c != '-')
        {
            szPrefix[uWrite++] = c;
        }
    }

    szPrefix[uWrite] = '\0';

    for(
        uIndex = 0U;
        uIndex < F144_COUNT(g_asLegacyObjects);
        ++uIndex
    )
    {
        size_t uPrefixLength =
            strlen(szPrefix);

        if(
            strncmp(
                g_asLegacyObjects[uIndex].pszSymbol,
                szPrefix,
                uPrefixLength
            ) == 0 &&
            (
                g_asLegacyObjects[uIndex].pszSymbol[uPrefixLength] == '\0' ||
                g_asLegacyObjects[uIndex].pszSymbol[uPrefixLength] == '_'
            )
        )
        {
            return g_asLegacyObjects[uIndex].eObject;
        }
    }

    return FLOPPY144_OBJECT_NONE;
}

static bool Floppy144PersistentEffectPresent(const Floppy144RunState *pState,const char *pszOperation,const char *pszTarget){uint32_t uIndex;if(!pState||!pszOperation)return false;for(uIndex=0;uIndex<F144_COUNT(g_asGameData);++uIndex){const Floppy144DataRecord*p=&g_asGameData[uIndex];bool bOwner=false;if(p->eKind!=FLOPPY144_DATA_TRIGGER_EFFECT&&p->eKind!=FLOPPY144_DATA_INTERACTION_EFFECT)continue;if(!Floppy144StringEqual(p->pszA,pszOperation))continue;if(pszTarget&&!Floppy144StringEqual(p->pszB,pszTarget))continue;if(p->eKind==FLOPPY144_DATA_TRIGGER_EFFECT){Floppy144TriggerId e=Floppy144GameDataTriggerId(p->pszId);bOwner=e!=FLOPPY144_TRIGGER_COUNT&&Floppy144RunStateTriggerFired(pState,e);}else{Floppy144InteractionId e=Floppy144GameDataInteractionId(p->pszId);bOwner=e!=FLOPPY144_INTERACTION_COUNT&&Floppy144RunStateInteractionCompleted(pState,e);}if(bOwner)return true;}return false;}

/*
 * Resolve one stable progression ID used by a connection unlock condition.
 *
 * Current canonical connection data names either a trigger (T-xxx) or an
 * interaction (I-xxx). Keep the resolver generic by asking the generated
 * registries rather than switching on specific story IDs.
 */
static bool Floppy144ConnectionConditionAtomSatisfied(
    const Floppy144RunState *pState,
    const char *pszId
)
{
    Floppy144TriggerId eTrigger;
    Floppy144InteractionId eInteraction;

    if(pState == NULL || pszId == NULL || pszId[0] == '\0')
    {
        return false;
    }

    eTrigger = Floppy144GameDataTriggerId(pszId);

    if(eTrigger != FLOPPY144_TRIGGER_COUNT)
    {
        return Floppy144RunStateTriggerFired(pState, eTrigger);
    }

    eInteraction = Floppy144GameDataInteractionId(pszId);

    if(eInteraction != FLOPPY144_INTERACTION_COUNT)
    {
        return Floppy144RunStateInteractionCompleted(
            pState,
            eInteraction
        );
    }

    return false;
}

/*
 * Evaluate the compact OR form used by canonical connection data, e.g.
 *
 *     T-010_OR_T-028
 *
 * A connection may contain more than one CONNECTION_CONDITION record. Those
 * records are treated as AND requirements, while atoms inside one record are
 * OR alternatives. This mirrors the authored structure without introducing a
 * second story-specific door table in C.
 */
static bool Floppy144ConnectionConditionSatisfied(
    const Floppy144RunState *pState,
    const char *pszCondition
)
{
    static const char szOrToken[] = "_OR_";
    const char *pszStart;

    if(pState == NULL || pszCondition == NULL || pszCondition[0] == '\0')
    {
        return false;
    }

    pszStart = pszCondition;

    for(;;)
    {
        const char *pszSeparator = strstr(pszStart, szOrToken);
        size_t uLength = pszSeparator != NULL
            ? (size_t)(pszSeparator - pszStart)
            : strlen(pszStart);
        char szAtom[32];

        if(uLength == 0U || uLength >= sizeof(szAtom))
        {
            return false;
        }

        memcpy(szAtom, pszStart, uLength);
        szAtom[uLength] = '\0';

        if(Floppy144ConnectionConditionAtomSatisfied(pState, szAtom))
        {
            return true;
        }

        if(pszSeparator == NULL)
        {
            break;
        }

        pszStart = pszSeparator + (sizeof(szOrToken) - 1U);
    }

    return false;
}

bool Floppy144GameDataConnectionUnlocked(
    const Floppy144RunState *pState,
    const char *pszId
)
{
    const Floppy144DataRecord *pConnection;
    uint32_t uIndex;
    bool bHasCondition = false;

    if(pState == NULL || pszId == NULL)
    {
        return false;
    }

    pConnection =
        Floppy144GameDataFind(
            FLOPPY144_DATA_CONNECTION,
            pszId
        );

    if(pConnection == NULL)
    {
        return false;
    }

    if(Floppy144StringEqual(pConnection->pszC, "UNLOCKED"))
    {
        return true;
    }

    /*
     * Preserve explicit UNLOCK_CONNECTION effects. Some authored interactions
     * deliberately use the effect even when the connection also names the
     * completing interaction as an unlock condition.
     */
    if(
        Floppy144PersistentEffectPresent(
            pState,
            "UNLOCK_CONNECTION",
            pszId
        )
    )
    {
        return true;
    }

    /*
     * Honour the canonical connection.unlock_conditions array emitted as
     * CONNECTION_CONDITION records. Every record must be satisfied; a record
     * may itself contain OR alternatives such as T-010_OR_T-028.
     */
    for(uIndex = 0U; uIndex < F144_COUNT(g_asGameData); ++uIndex)
    {
        const Floppy144DataRecord *pRecord = &g_asGameData[uIndex];

        if(
            pRecord->eKind != FLOPPY144_DATA_CONNECTION_CONDITION ||
            !Floppy144StringEqual(pRecord->pszId, pszId)
        )
        {
            continue;
        }

        bHasCondition = true;

        if(
            !Floppy144ConnectionConditionSatisfied(
                pState,
                pRecord->pszA
            )
        )
        {
            return false;
        }
    }

    return bHasCondition;
}
/*
 * A collection may enter the available pool in two generic ways:
 *
 * 1. an earlier persistent effect explicitly ENABLE_COLLECTIONs it; or
 * 2. one of that collection's trigger records has become actionable.
 *
 * The second rule bridges narrative progression and the terminal catalogue.
 * Room-reconstruction collections therefore appear when their authored
 * conditions become true, without hard-coding collection IDs or player-facing
 * Act labels in the engine.
 */
static bool Floppy144CollectionHasReadyTrigger(
    const Floppy144RunState *pState,
    const char *pszCollectionId
)
{
    uint32_t uIndex;

    if(pState == NULL || pszCollectionId == NULL)
    {
        return false;
    }

    for(uIndex = 0U; uIndex < F144_COUNT(g_asGameData); ++uIndex)
    {
        const Floppy144DataRecord *pRecord = &g_asGameData[uIndex];
        Floppy144TriggerId eTrigger;

        if(
            pRecord->eKind != FLOPPY144_DATA_TRIGGER ||
            !Floppy144StringEqual(pRecord->pszA, pszCollectionId)
        )
        {
            continue;
        }

        eTrigger = Floppy144GameDataTriggerId(pRecord->pszId);

        if(
            eTrigger != FLOPPY144_TRIGGER_COUNT &&
            Floppy144GameDataTriggerCanFire(pState, eTrigger)
        )
        {
            return true;
        }
    }

    return false;
}

bool Floppy144GameDataCollectionEnabled(
    const Floppy144RunState *pState,
    const char *pszId
)
{
    Floppy144CollectionId eCollection =
        Floppy144GameDataCollectionId(pszId);

    if(
        pState == NULL ||
        eCollection == FLOPPY144_COLLECTION_COUNT
    )
    {
        return false;
    }

    if(eCollection == FLOPPY144_COLLECTION_DR01)
    {
        return true;
    }

    if(
        Floppy144RunStateCollectionRestored(
            pState,
            eCollection
        )
    )
    {
        return true;
    }

    if(
        Floppy144PersistentEffectPresent(
            pState,
            "ENABLE_COLLECTION",
            pszId
        )
    )
    {
        return true;
    }

    return
        Floppy144CollectionHasReadyTrigger(
            pState,
            pszId
        );
}
bool Floppy144GameDataPhysicalItemRevealed(const Floppy144RunState*pState,const char*pszId){return Floppy144PersistentEffectPresent(pState,"REVEAL_PHYSICAL_ITEM",pszId);}
bool Floppy144GameDataFactRecorded(const Floppy144RunState*pState,const char*pszId){return Floppy144PersistentEffectPresent(pState,"RECORD_NOTEBOOK_FACT",pszId);}
/*
 * Notebook visibility is derived from already-persisted gameplay state.
 * Generated Notebook records carry prose only; they do not add save fields.
 */
static bool Floppy144GameDataNotebookRecordVisible(
    const Floppy144RunState *pState,
    const Floppy144DataRecord *pRecord
)
{
    if(!pState||!pRecord||pRecord->eKind!=FLOPPY144_DATA_NOTEBOOK||!pRecord->pszId||!pRecord->pszA||!pRecord->pszB)return false;
    if(Floppy144StringEqual(pRecord->pszB,"FACT"))return Floppy144GameDataFactRecorded(pState,pRecord->pszId);
    if(Floppy144StringEqual(pRecord->pszB,"EVIDENCE")){Floppy144EvidenceId e=Floppy144GameDataEvidenceId(pRecord->pszId);return e!=FLOPPY144_EVIDENCE_COUNT&&Floppy144RunStateEvidenceEstablished(pState,e);}
    if(Floppy144StringEqual(pRecord->pszB,"INTERACTION")){Floppy144InteractionId e=Floppy144GameDataInteractionId(pRecord->pszId);return e!=FLOPPY144_INTERACTION_COUNT&&Floppy144RunStateInteractionCompleted(pState,e);}
    return false;
}
static const Floppy144DataRecord *Floppy144GameDataNotebookRecordAtOrdinal(uint32_t uOrdinal)
{
    uint32_t u,n=0U;
    for(u=0U;u<F144_COUNT(g_asGameData);++u){if(g_asGameData[u].eKind!=FLOPPY144_DATA_NOTEBOOK)continue;if(n==uOrdinal)return &g_asGameData[u];++n;}
    return NULL;
}

void Floppy144GameDataCaptureNewNotebookEntries(Floppy144RunState *pState)
{
    uint32_t u,n=0U;
    if(pState==NULL)return;
    for(u=0U;u<F144_COUNT(g_asGameData);++u)
    {
        const Floppy144DataRecord *p=&g_asGameData[u];
        if(p->eKind!=FLOPPY144_DATA_NOTEBOOK)continue;
        if(Floppy144GameDataNotebookRecordVisible(pState,p))(void)Floppy144RunStateAppendNotebookEntry(pState,n);
        ++n;
    }
}

uint32_t Floppy144GameDataNotebookOrderedCount(const Floppy144RunState *pState)
{
    return pState!=NULL?(uint32_t)pState->notebook_order_count:0U;
}

const Floppy144DataRecord *Floppy144GameDataNotebookOrderedEntryAt(const Floppy144RunState *pState,uint32_t uIndex)
{
    uint32_t uOrdinal;
    if(pState==NULL||uIndex>=(uint32_t)pState->notebook_order_count)return NULL;
    uOrdinal=(uint32_t)pState->notebook_order[uIndex];
    return Floppy144GameDataNotebookRecordAtOrdinal(uOrdinal);
}

/* Compatibility wrappers now present the same chronological order. */
uint32_t Floppy144GameDataNotebookEntryCount(const Floppy144RunState*pState){return Floppy144GameDataNotebookOrderedCount(pState);}
const Floppy144DataRecord *Floppy144GameDataNotebookEntryAt(const Floppy144RunState*pState,uint32_t uVisibleIndex){return Floppy144GameDataNotebookOrderedEntryAt(pState,uVisibleIndex);}

bool Floppy144GameDataWorkstreamAvailable(
    const Floppy144RunState *pState,
    const char *pszWorkstream
)
{
    bool bKnownWorkstream;

    if(pState == NULL || pszWorkstream == NULL)
    {
        return false;
    }

    bKnownWorkstream =
        Floppy144StringEqual(pszWorkstream, "TECHNOLOGY") ||
        Floppy144StringEqual(pszWorkstream, "RECORDS");

    if(!bKnownWorkstream)
    {
        return false;
    }

    /*
     * Before the Act II branch choice, both workstream documents are valid
     * choices. Selecting either one commits RunState.branch, after which the
     * unchosen workstream is deferred until the authored
     * RELEASE_ALTERNATE_WORKSTREAM effect fires at the end of Act II.
     */
    if(
        pState->branch ==
        (uint8_t)FLOPPY144_RUN_BRANCH_NONE
    )
    {
        return true;
    }

    if(
        Floppy144StringEqual(pszWorkstream, "TECHNOLOGY") &&
        pState->branch ==
            (uint8_t)FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST
    )
    {
        return true;
    }

    if(
        Floppy144StringEqual(pszWorkstream, "RECORDS") &&
        pState->branch ==
            (uint8_t)FLOPPY144_RUN_BRANCH_RECORDS_FIRST
    )
    {
        return true;
    }

    return
        Floppy144PersistentEffectPresent(
            pState,
            "RELEASE_ALTERNATE_WORKSTREAM",
            NULL
        );
}

bool Floppy144GameDataRoomAccessible(const Floppy144RunState*pState,const char*pszRoomId){Floppy144RoomId eRoom=Floppy144GameDataRoomId(pszRoomId);uint32_t u;if(!pState||eRoom==FLOPPY144_ROOM_COUNT||!Floppy144RunStateRoomReconstructed(pState,eRoom))return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];const char*pszOther=NULL;if(p->eKind!=FLOPPY144_DATA_CONNECTION)continue;if(Floppy144StringEqual(p->pszA,pszRoomId))pszOther=p->pszB;else if(Floppy144StringEqual(p->pszB,pszRoomId))pszOther=p->pszA;else continue;if(!Floppy144GameDataConnectionUnlocked(pState,p->pszId))continue;if(Floppy144StringEqual(pszOther,"OUTSIDE"))return true;{Floppy144RoomId eOther=Floppy144GameDataRoomId(pszOther);if(eOther!=FLOPPY144_ROOM_COUNT&&Floppy144RunStateRoomReconstructed(pState,eOther))return true;}}return false;}
bool Floppy144GameDataRoomTransitionAllowed(const Floppy144RunState*pState,Floppy144RoomId eFrom,Floppy144RoomId eTo){uint32_t u;if(!pState)return false;if(eFrom==eTo)return true;if((uint32_t)eFrom>=(uint32_t)FLOPPY144_ROOM_COUNT||(uint32_t)eTo>=(uint32_t)FLOPPY144_ROOM_COUNT)return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];Floppy144RoomId a,b;if(p->eKind!=FLOPPY144_DATA_CONNECTION)continue;a=Floppy144GameDataRoomId(p->pszA);b=Floppy144GameDataRoomId(p->pszB);if(((a==eFrom&&b==eTo)||(a==eTo&&b==eFrom))&&Floppy144GameDataConnectionUnlocked(pState,p->pszId))return true;}return false;}
static bool Floppy144TerminalAvailable(const Floppy144RunState*pState,const char*pszTerminalId){uint32_t u;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if((p->eKind==FLOPPY144_DATA_FURNITURE||p->eKind==FLOPPY144_DATA_FIXTURE)&&Floppy144StringEqual(p->pszD,pszTerminalId)){Floppy144RoomId e=Floppy144GameDataRoomId(p->pszA);return Floppy144GameDataRoomAccessible(pState,p->pszA)||(e!=FLOPPY144_ROOM_COUNT&&Floppy144RunStateRoomReconstructed(pState,e));}}return false;}
static Floppy144RunAct Floppy144ActFromText(const char*psz){if(Floppy144StringEqual(psz,"ACT_I"))return FLOPPY144_RUN_ACT_I;if(Floppy144StringEqual(psz,"ACT_II"))return FLOPPY144_RUN_ACT_II;if(Floppy144StringEqual(psz,"ACT_III"))return FLOPPY144_RUN_ACT_III;if(Floppy144StringEqual(psz,"COMPLETE"))return FLOPPY144_RUN_ACT_COMPLETE;return FLOPPY144_RUN_ACT_PROLOGUE;}

/* Generic graph walk used by the semantic ACT_II_CORE_COMPLETE condition. */
static bool Floppy144TriggerDependsOnBranch(const char*,const char*,uint32_t);
static bool Floppy144InteractionDependsOnBranch(const char*pszId,const char*pszBranch,uint32_t uDepth){uint32_t u;if(!pszId||!pszBranch||uDepth>64U)return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind!=FLOPPY144_DATA_INTERACTION_PREREQUISITE||!Floppy144StringEqual(p->pszId,pszId)||!p->pszA)continue;if(Floppy144GameDataTriggerId(p->pszA)!=FLOPPY144_TRIGGER_COUNT&&Floppy144TriggerDependsOnBranch(p->pszA,pszBranch,uDepth+1U))return true;if(Floppy144GameDataInteractionId(p->pszA)!=FLOPPY144_INTERACTION_COUNT&&Floppy144InteractionDependsOnBranch(p->pszA,pszBranch,uDepth+1U))return true;}return false;}
static bool Floppy144EvidenceDependsOnBranch(const char*pszId,const char*pszBranch,uint32_t uDepth){uint32_t u;if(!pszId||!pszBranch||uDepth>64U)return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_EVIDENCE_REQUIREMENT&&Floppy144StringEqual(p->pszId,pszId)&&p->pszA&&Floppy144InteractionDependsOnBranch(p->pszA,pszBranch,uDepth+1U))return true;}return false;}
static bool Floppy144TriggerDependsOnBranch(const char*pszId,const char*pszBranch,uint32_t uDepth){uint32_t u;if(!pszId||!pszBranch||uDepth>64U)return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind!=FLOPPY144_DATA_TRIGGER_CONDITION||!Floppy144StringEqual(p->pszId,pszId))continue;if(Floppy144StringEqual(p->pszA,"branch")&&Floppy144StringEqual(p->pszB,pszBranch))return true;if(Floppy144StringEqual(p->pszA,"trigger")&&Floppy144TriggerDependsOnBranch(p->pszB,pszBranch,uDepth+1U))return true;if(Floppy144StringEqual(p->pszA,"evidence")&&Floppy144EvidenceDependsOnBranch(p->pszB,pszBranch,uDepth+1U))return true;}return false;}
static bool Floppy144ActIiCoreComplete(const Floppy144RunState*pState){const char*pszBranch;int32_t nGate=0x7fffffff,nLast=-1;Floppy144TriggerId eLast=FLOPPY144_TRIGGER_COUNT;uint32_t u;if(!pState)return false;if(pState->branch==(uint8_t)FLOPPY144_RUN_BRANCH_RECORDS_FIRST)pszBranch="RECORDS_FIRST";else if(pState->branch==(uint8_t)FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST)pszBranch="TECHNOLOGY_FIRST";else return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_TRIGGER_CONDITION&&Floppy144StringEqual(p->pszA,"act_at_least")&&Floppy144StringEqual(p->pszB,"ACT_II_CORE_COMPLETE")){const Floppy144DataRecord*t=Floppy144GameDataFind(FLOPPY144_DATA_TRIGGER,p->pszId);if(t&&t->n2<nGate)nGate=t->n2;}}for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_TRIGGER&&p->n2<nGate&&p->n2>nLast&&Floppy144TriggerDependsOnBranch(p->pszId,pszBranch,0U)){nLast=p->n2;eLast=Floppy144GameDataTriggerId(p->pszId);}}return eLast!=FLOPPY144_TRIGGER_COUNT&&Floppy144RunStateTriggerFired(pState,eLast);}

bool Floppy144GameDataConditionSatisfied(const Floppy144RunState*pState,const char*pszKind,const char*pszTarget){if(!pState||!pszKind)return false;if(Floppy144StringEqual(pszKind,"archive_initialised"))return Floppy144RunStateArchiveServicesInitialised(pState);if(Floppy144StringEqual(pszKind,"trigger")){Floppy144TriggerId e=Floppy144GameDataTriggerId(pszTarget);return e!=FLOPPY144_TRIGGER_COUNT&&Floppy144RunStateTriggerFired(pState,e);}if(Floppy144StringEqual(pszKind,"evidence")){Floppy144EvidenceId e=Floppy144GameDataEvidenceId(pszTarget);return e!=FLOPPY144_EVIDENCE_COUNT&&Floppy144RunStateEvidenceEstablished(pState,e);}if(Floppy144StringEqual(pszKind,"collection_restored")){Floppy144CollectionId e=Floppy144GameDataCollectionId(pszTarget);return e!=FLOPPY144_COLLECTION_COUNT&&Floppy144RunStateCollectionRestored(pState,e);}if(Floppy144StringEqual(pszKind,"collection_restorable")){Floppy144CollectionId e=Floppy144GameDataCollectionId(pszTarget);return e!=FLOPPY144_COLLECTION_COUNT&&Floppy144RunStateCanRestoreCollection(pState,e);}if(Floppy144StringEqual(pszKind,"room_reconstructed")){Floppy144RoomId e=Floppy144GameDataRoomId(pszTarget);return e!=FLOPPY144_ROOM_COUNT&&Floppy144RunStateRoomReconstructed(pState,e);}if(Floppy144StringEqual(pszKind,"room_accessible"))return Floppy144GameDataRoomAccessible(pState,pszTarget);if(Floppy144StringEqual(pszKind,"terminal_available"))return Floppy144TerminalAvailable(pState,pszTarget);if(Floppy144StringEqual(pszKind,"branch")){if(Floppy144StringEqual(pszTarget,"RECORDS_FIRST"))return pState->branch==(uint8_t)FLOPPY144_RUN_BRANCH_RECORDS_FIRST;if(Floppy144StringEqual(pszTarget,"TECHNOLOGY_FIRST"))return pState->branch==(uint8_t)FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST;return false;}if(Floppy144StringEqual(pszKind,"act_at_least")){if(Floppy144StringEqual(pszTarget,"ACT_II_CORE_COMPLETE"))return Floppy144ActIiCoreComplete(pState);return pState->act>=(uint8_t)Floppy144ActFromText(pszTarget);}if(Floppy144StringEqual(pszKind,"workstream_available"))return Floppy144GameDataWorkstreamAvailable(pState,pszTarget);return false;}

bool Floppy144GameDataExecuteEffect(Floppy144WorldState*pWorld,Floppy144RunState*pState,const char*pszOp,const char*pszTarget){if(!pState||!pszOp)return false;if(Floppy144StringEqual(pszOp,"RECONSTRUCT_ROOM")){Floppy144RoomId e=Floppy144GameDataRoomId(pszTarget);return e!=FLOPPY144_ROOM_COUNT&&Floppy144RunStateReconstructRoom(pState,e);}if(Floppy144StringEqual(pszOp,"SET_ACT"))return Floppy144RunStateSetAct(pState,Floppy144ActFromText(pszTarget));if(Floppy144StringEqual(pszOp,"SET_BRANCH")){if(Floppy144StringEqual(pszTarget,"RECORDS_FIRST"))return Floppy144RunStateSetBranch(pState,FLOPPY144_RUN_BRANCH_RECORDS_FIRST);if(Floppy144StringEqual(pszTarget,"TECHNOLOGY_FIRST"))return Floppy144RunStateSetBranch(pState,FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST);return false;}if(Floppy144StringEqual(pszOp,"SET_PROJECTION"))return Floppy144RunStateSetProjection(pState,Floppy144StringEqual(pszTarget,"ISOMETRIC")?FLOPPY144_PROJECTION_ISOMETRIC:FLOPPY144_PROJECTION_2D);if(Floppy144StringEqual(pszOp,"GRANT_CAPABILITY")){Floppy144CapabilityId e=Floppy144GameDataCapabilityId(pszTarget);return e!=FLOPPY144_CAPABILITY_COUNT&&Floppy144RunStateGrantCapability(pState,e);}if(Floppy144StringEqual(pszOp,"COMPLETE_INTERACTION")){Floppy144InteractionId e=Floppy144GameDataInteractionId(pszTarget);return e!=FLOPPY144_INTERACTION_COUNT&&Floppy144RunStateCompleteInteraction(pState,e);}if(Floppy144StringEqual(pszOp,"REVEAL_PHYSICAL_ITEM")){Floppy144ObjectId e=Floppy144LegacyObjectFromPhysicalId(pszTarget);if(e!=FLOPPY144_OBJECT_NONE){(void)Floppy144RunStateRevealObject(pState,e);if(pWorld)(void)Floppy144WorldRevealObject(pWorld,e);}return true;}if(Floppy144StringEqual(pszOp,"ENABLE_COLLECTION")||Floppy144StringEqual(pszOp,"UNLOCK_CONNECTION")||Floppy144StringEqual(pszOp,"RECORD_NOTEBOOK_FACT")||Floppy144StringEqual(pszOp,"RELEASE_ALTERNATE_WORKSTREAM")){pState->dirty=1U;return true;}return false;}

static const Floppy144DataRecord *Floppy144Nth(Floppy144DataRecordKind eKind,int32_t nOrdinal){uint32_t u;int32_t n=0;for(u=0;u<F144_COUNT(g_asGameData);++u)if(g_asGameData[u].eKind==eKind){if(n==nOrdinal)return&g_asGameData[u];++n;}return NULL;}
bool Floppy144GameDataTriggerCanFire(const Floppy144RunState*pState,Floppy144TriggerId eTrigger){const Floppy144DataRecord*pTrigger;uint32_t u;if(!pState||(uint32_t)eTrigger>=(uint32_t)FLOPPY144_TRIGGER_COUNT||Floppy144RunStateTriggerFired(pState,eTrigger))return false;pTrigger=Floppy144Nth(FLOPPY144_DATA_TRIGGER,(int32_t)eTrigger);if(!pTrigger)return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_TRIGGER_CONDITION&&Floppy144StringEqual(p->pszId,pTrigger->pszId)&&!Floppy144GameDataConditionSatisfied(pState,p->pszA,p->pszB))return false;}return true;}

bool Floppy144GameDataTriggerDocumentAccessible(
    const Floppy144RunState *pState,
    Floppy144TriggerId eTrigger
)
{
    const Floppy144DataRecord *pTrigger;
    uint32_t uIndex;

    if(
        pState == NULL ||
        (uint32_t)eTrigger >=
            (uint32_t)FLOPPY144_TRIGGER_COUNT
    )
    {
        return false;
    }

    pTrigger =
        Floppy144Nth(
            FLOPPY144_DATA_TRIGGER,
            (int32_t)eTrigger
        );

    if(pTrigger == NULL)
    {
        return false;
    }

    /*
     * Recovery-sequence conditions normally control effects, not whether the
     * recovered prose itself can be read. Workstream availability is the one
     * deliberate exception: it represents an authored branch-access gate.
     */
    for(
        uIndex = 0U;
        uIndex < F144_COUNT(g_asGameData);
        ++uIndex
    )
    {
        const Floppy144DataRecord *pCondition =
            &g_asGameData[uIndex];

        if(
            pCondition->eKind !=
                FLOPPY144_DATA_TRIGGER_CONDITION ||
            !Floppy144StringEqual(
                pCondition->pszId,
                pTrigger->pszId
            ) ||
            !Floppy144StringEqual(
                pCondition->pszA,
                "workstream_available"
            )
        )
        {
            continue;
        }

        if(
            !Floppy144GameDataConditionSatisfied(
                pState,
                pCondition->pszA,
                pCondition->pszB
            )
        )
        {
            return false;
        }
    }

    return true;
}
bool Floppy144GameDataTriggerTryFire(Floppy144WorldState*pWorld,Floppy144RunState*pState,Floppy144TriggerId eTrigger){const Floppy144DataRecord*pTrigger;uint32_t u;if(!Floppy144GameDataTriggerCanFire(pState,eTrigger))return false;pTrigger=Floppy144Nth(FLOPPY144_DATA_TRIGGER,(int32_t)eTrigger);if(!pTrigger||!Floppy144RunStateFireTrigger(pState,eTrigger))return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_TRIGGER_EFFECT&&Floppy144StringEqual(p->pszId,pTrigger->pszId))(void)Floppy144GameDataExecuteEffect(pWorld,pState,p->pszA,p->pszB);}Floppy144GameDataCaptureNewNotebookEntries(pState);return true;}

static bool Floppy144EvidenceOwnsInteraction(const char*pszEvidenceId,const char*pszInteractionId){const Floppy144DataRecord*pEvidence=Floppy144GameDataFind(FLOPPY144_DATA_EVIDENCE,pszEvidenceId);uint32_t u;if(!pEvidence||!pEvidence->b0)return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_EVIDENCE_REQUIREMENT&&Floppy144StringEqual(p->pszId,pszEvidenceId)&&Floppy144StringEqual(p->pszA,pszInteractionId))return true;}return false;}
static bool Floppy144AnySecureCabinetUnlocked(const Floppy144RunState*pState){uint32_t u;int32_t n=0;if(!pState)return false;if(Floppy144RunStateHasCapability(pState,FLOPPY144_CAPABILITY_MASTER_SECURE_CABINET_CODES))return true;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind!=FLOPPY144_DATA_INTERACTION)continue;if(p->pszB&&strstr(p->pszB,"SECURE_CABINET")&&Floppy144RunStateInteractionCompleted(pState,(Floppy144InteractionId)n))return true;++n;}return false;}
static bool Floppy144InteractionPrerequisiteSatisfied(const Floppy144RunState*pState,const Floppy144DataRecord*pInteraction,const char*psz){if(!psz||!*psz)return true;if(psz[0]=='T'&&psz[1]=='-'){Floppy144TriggerId e=Floppy144GameDataTriggerId(psz);return e!=FLOPPY144_TRIGGER_COUNT&&Floppy144RunStateTriggerFired(pState,e);}if(psz[0]=='E'&&psz[1]=='-'){Floppy144EvidenceId e=Floppy144GameDataEvidenceId(psz);return e!=FLOPPY144_EVIDENCE_COUNT&&Floppy144RunStateEvidenceEstablished(pState,e);}if(psz[0]=='I'&&psz[1]=='-'){Floppy144InteractionId e=Floppy144GameDataInteractionId(psz);if(pInteraction&&pInteraction->pszE&&Floppy144EvidenceOwnsInteraction(pInteraction->pszE,psz))return true;return e!=FLOPPY144_INTERACTION_COUNT&&Floppy144RunStateInteractionCompleted(pState,e);}if(Floppy144StringEqual(psz,"ROOM_SECURITY"))return Floppy144GameDataRoomAccessible(pState,"SECURITY");if(Floppy144StringEqual(psz,"ROOM_SERVER_ROOM"))return Floppy144RunStateRoomReconstructed(pState,FLOPPY144_ROOM_SERVER_ROOM);if(Floppy144StringEqual(psz,"CABINET_UNLOCKED"))return Floppy144AnySecureCabinetUnlocked(pState);if(Floppy144StringEqual(psz,"SECURITY_CODE_KNOWN")||Floppy144StringEqual(psz,"CODE_KNOWN"))return Floppy144GameDataFactRecorded(pState,"SECURITY_CABINET_CODE")||Floppy144RunStateHasCapability(pState,FLOPPY144_CAPABILITY_MASTER_SECURE_CABINET_CODES);if(Floppy144StringEqual(psz,"CABINET_CONSTRUCTED"))return true;{Floppy144CapabilityId e=Floppy144GameDataCapabilityId(psz);return e!=FLOPPY144_CAPABILITY_COUNT&&Floppy144RunStateHasCapability(pState,e);}}
bool Floppy144GameDataInteractionCanRun(const Floppy144RunState*pState,Floppy144InteractionId eInteraction){const Floppy144DataRecord*pInteraction;uint32_t u;if(!pState||(uint32_t)eInteraction>=(uint32_t)FLOPPY144_INTERACTION_COUNT||Floppy144RunStateInteractionCompleted(pState,eInteraction))return false;pInteraction=Floppy144Nth(FLOPPY144_DATA_INTERACTION,(int32_t)eInteraction);if(!pInteraction)return false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_INTERACTION_PREREQUISITE&&Floppy144StringEqual(p->pszId,pInteraction->pszId)&&!Floppy144InteractionPrerequisiteSatisfied(pState,pInteraction,p->pszA))return false;}return true;}
bool Floppy144GameDataInteractionTryRun(Floppy144WorldState*pWorld,Floppy144RunState*pState,Floppy144InteractionId eInteraction){const Floppy144DataRecord*pInteraction;uint32_t u;if(!Floppy144GameDataInteractionCanRun(pState,eInteraction))return false;pInteraction=Floppy144Nth(FLOPPY144_DATA_INTERACTION,(int32_t)eInteraction);if(!pInteraction)return false;(void)Floppy144RunStateCompleteInteraction(pState,eInteraction);for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*p=&g_asGameData[u];if(p->eKind==FLOPPY144_DATA_INTERACTION_EFFECT&&Floppy144StringEqual(p->pszId,pInteraction->pszId))(void)Floppy144GameDataExecuteEffect(pWorld,pState,p->pszA,p->pszB);}Floppy144GameDataResolveEvidence(pState);Floppy144GameDataCaptureNewNotebookEntries(pState);return true;}
void Floppy144GameDataResolveEvidence(Floppy144RunState*pState){uint32_t uPass;if(!pState)return;for(uPass=0;uPass<(uint32_t)FLOPPY144_EVIDENCE_COUNT;++uPass){uint32_t u;bool bChanged=false;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*pE=&g_asGameData[u];Floppy144EvidenceId e;uint32_t k;bool ok=true;if(pE->eKind!=FLOPPY144_DATA_EVIDENCE)continue;e=Floppy144GameDataEvidenceId(pE->pszId);if(e==FLOPPY144_EVIDENCE_COUNT||Floppy144RunStateEvidenceEstablished(pState,e))continue;for(k=0;k<F144_COUNT(g_asGameData);++k){const Floppy144DataRecord*p=&g_asGameData[k];if(p->eKind==FLOPPY144_DATA_EVIDENCE_REQUIREMENT&&Floppy144StringEqual(p->pszId,pE->pszId)){Floppy144InteractionId r=Floppy144GameDataInteractionId(p->pszA);if(r==FLOPPY144_INTERACTION_COUNT||!Floppy144RunStateInteractionCompleted(pState,r)){ok=false;break;}}}if(!ok)continue;for(k=0;k<F144_COUNT(g_asGameData);++k){const Floppy144DataRecord*p=&g_asGameData[k];if(p->eKind==FLOPPY144_DATA_EVIDENCE_CONDITION&&Floppy144StringEqual(p->pszId,pE->pszId)&&!Floppy144GameDataConditionSatisfied(pState,p->pszA,p->pszB)){ok=false;break;}}if(ok&&Floppy144RunStateEstablishEvidence(pState,e))bChanged=true;}if(!bChanged)break;}Floppy144GameDataCaptureNewNotebookEntries(pState);}

static uint32_t Floppy144MonthDay(uint32_t m,uint32_t d){return m*100U+d;}
static uint32_t Floppy144ParseMonthDay(const char*s){if(!s||strlen(s)!=5U||s[2]!='-')return 0U;return(uint32_t)(s[0]-'0')*1000U+(uint32_t)(s[1]-'0')*100U+(uint32_t)(s[3]-'0')*10U+(uint32_t)(s[4]-'0');}
const Floppy144DataRecord *Floppy144GameDataAmbientForDate(const char*pszTargetId,uint32_t uMonth,uint32_t uDay){const Floppy144DataRecord*pBest=NULL;uint32_t uBest=0,uDate=Floppy144MonthDay(uMonth,uDay),u;for(u=0;u<F144_COUNT(g_asGameData);++u){const Floppy144DataRecord*pA=&g_asGameData[u];uint32_t r;bool match=false,has=false;if(pA->eKind!=FLOPPY144_DATA_AMBIENT||!Floppy144StringEqual(pA->pszB,pszTargetId))continue;for(r=0;r<F144_COUNT(g_asGameData);++r){const Floppy144DataRecord*pR=&g_asGameData[r];uint32_t a,b;if(pR->eKind!=FLOPPY144_DATA_AMBIENT_RANGE||!Floppy144StringEqual(pR->pszId,pA->pszId))continue;has=true;a=Floppy144ParseMonthDay(pR->pszA);b=Floppy144ParseMonthDay(pR->pszB);if((a<=b&&uDate>=a&&uDate<=b)||(a>b&&(uDate>=a||uDate<=b))){match=true;break;}}if(!has)match=true;if(match&&(!pBest||(uint32_t)pA->n0>uBest)){pBest=pA;uBest=(uint32_t)pA->n0;}}return pBest;}
