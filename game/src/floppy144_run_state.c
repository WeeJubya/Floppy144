#include "floppy144_run_state.h"
#include "floppy144_collection_registry.h"
#include "floppy144_object_registry.h"
#include "floppy144_site.h"
#include "floppy144_site_rooms.h"
#include "floppy144_site_object.h"
#include "floppy144_game_data.h"

#include <stdio.h>
#include <string.h>

/**************/
/* Validators */
/**************/

static bool Floppy144RunStateRoomValid
(
    Floppy144RoomId room
){
    return
    (uint32_t)room <
    (uint32_t)FLOPPY144_ROOM_COUNT;
}

static bool Floppy144RunStateCollectionValid
(
    Floppy144CollectionId collection
){
    return
    (uint32_t)collection <
    (uint32_t)FLOPPY144_COLLECTION_COUNT;
}

static bool Floppy144RunStateInteractionValid
(
    Floppy144InteractionId interaction
){
    return
    (uint32_t)interaction <
    (uint32_t)FLOPPY144_INTERACTION_COUNT;
}

static bool Floppy144RunStateEvidenceValid
(
    Floppy144EvidenceId evidence
){
    return
    (uint32_t)evidence <
    (uint32_t)FLOPPY144_EVIDENCE_COUNT;
}

static bool Floppy144RunStateCapabilityValid
(
    Floppy144CapabilityId capability
){
    return
    (uint32_t)capability <
    (uint32_t)FLOPPY144_CAPABILITY_COUNT;
}

static bool Floppy144RunStateTriggerValid
(
    Floppy144TriggerId trigger
){
    return
    (uint32_t)trigger <
    (uint32_t)FLOPPY144_TRIGGER_COUNT;
}

static bool Floppy144RunStateNotebookEntryValid
(
    Floppy144NotebookId entry
){
    return
    (uint32_t)entry <
    (uint32_t)FLOPPY144_NOTEBOOK_COUNT;
}

static bool Floppy144RunStateObjectValid
(
    Floppy144ObjectId object
){
    return
    object >= 0 &&
    (uint32_t)object <
    (uint32_t)FLOPPY144_OBJECT_COUNT;
}

/*****************/
/* API Functions */
/*****************/

bool Floppy144RunStateRoomReconstructed
(
    const Floppy144RunState *state,
 Floppy144RoomId room
){
    if(
        state == NULL ||
        !Floppy144RunStateRoomValid(room)
    )
    {
        return false;
    }

    if(
        Floppy144RunStateBitGet(
            state->rooms,
            (uint32_t)room
        )
    )
    {
        return true;
    }

    return false;
}

bool Floppy144RunStateReconstructRoom
(
    Floppy144RunState *state,
 Floppy144RoomId room
){
    if(
        state == NULL ||
        !Floppy144RunStateRoomValid(room)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->rooms,
            (uint32_t)room
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateObjectVisible
(
    const Floppy144RunState *state,
 Floppy144ObjectId object
){
    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->objects_visible,
        (uint32_t)object
    );
}

bool Floppy144RunStateRevealObject
(
    Floppy144RunState *state,
 Floppy144ObjectId object
){
    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->objects_visible,
            (uint32_t)object
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

/* STAGE 3B.5 SECURE CABINET STATE
 *
 * Cabinet ordinals are derived from canonical generated secure-cabinet
 * furniture order. One 32-bit word is sufficient for the current Site.
 */
bool Floppy144RunStateSecureCabinetUnlocked(
    const Floppy144RunState *state,
    uint32_t uCabinetIndex
)
{
    uint32_t uMask;

    if(state == NULL || uCabinetIndex >= FLOPPY144_SECURE_CABINET_MAX)
    {
        return false;
    }

    uMask = 1U << uCabinetIndex;
    return (state->secure_cabinets_unlocked & uMask) != 0U;
}

bool Floppy144RunStateUnlockSecureCabinet(
    Floppy144RunState *state,
    uint32_t uCabinetIndex
)
{
    uint32_t uMask;

    if(state == NULL || uCabinetIndex >= FLOPPY144_SECURE_CABINET_MAX)
    {
        return false;
    }

    uMask = 1U << uCabinetIndex;

    if((state->secure_cabinets_unlocked & uMask) != 0U)
    {
        return false;
    }

    state->secure_cabinets_unlocked |= uMask;
    state->dirty = 1U;
    return true;
}

bool Floppy144RunStateBitGet
(
    const uint32_t *words,
    uint32_t bit
){
    uint32_t word;
    uint32_t mask;

    if(!words)
    {
        return false;
    }

    word =
    bit / FLOPPY144_RUN_WORD_BITS;

    mask =
    1U <<
    (bit % FLOPPY144_RUN_WORD_BITS);

    return
    (words[word] & mask) != 0U;
}

bool Floppy144RunStateBitSet
(
    uint32_t *words,
    uint32_t bit
){
    uint32_t word;
    uint32_t mask;

    if(!words)
    {
        return false;
    }

    word =
    bit / FLOPPY144_RUN_WORD_BITS;

    mask =
    1U <<
    (bit % FLOPPY144_RUN_WORD_BITS);

    if(words[word] & mask)
    {
        return false;
    }

    words[word] |=
    mask;

    return true;
}

bool Floppy144RunStateBitClear
(
    uint32_t *words,
    uint32_t bit
){
    uint32_t word;
    uint32_t mask;

    if(!words)
    {
        return false;
    }

    word =
    bit / FLOPPY144_RUN_WORD_BITS;

    mask =
    1U <<
    (bit % FLOPPY144_RUN_WORD_BITS);

    if(!(words[word] & mask))
    {
        return false;
    }

    words[word] &=
    ~mask;

    return true;
}

bool Floppy144RunStateCollectionAvailable
(
    const Floppy144RunState *pState,
    Floppy144CollectionId eCollection
)
{
    const Floppy144CollectionDefinition *pDefinition;
    if(pState == NULL || !Floppy144RunStateCollectionValid(eCollection)) return false;
    pDefinition = Floppy144CollectionGet(eCollection);
    return pDefinition != NULL && Floppy144GameDataCollectionEnabled(pState, pDefinition->code);
}

bool Floppy144RunStateCollectionRestored
(
    const Floppy144RunState *state,
 Floppy144CollectionId collection
){
    if(
        state == NULL ||
        !Floppy144RunStateCollectionValid(collection)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->collections,
        (uint32_t)collection
    );
}

uint32_t Floppy144RunStateRecoveredKb(const Floppy144RunState *pState)
{
    uint32_t uIndex,uUsed=0U;
    if(pState==NULL)return 0U;
    for(uIndex=0U;uIndex<(uint32_t)FLOPPY144_COLLECTION_COUNT;++uIndex)
    {
        if(Floppy144RunStateCollectionRestored(pState,(Floppy144CollectionId)uIndex))
        {
            const Floppy144CollectionDefinition *pDefinition=Floppy144CollectionGet((Floppy144CollectionId)uIndex);
            if(pDefinition!=NULL)uUsed+=pDefinition->size_kb;
        }
    }
    return uUsed;
}

uint32_t Floppy144RunStateFreeKb(const Floppy144RunState *pState)
{
    uint32_t uUsed=Floppy144RunStateRecoveredKb(pState);
    return uUsed>=FLOPPY144_RECOVERY_CAPACITY_KB?0U:FLOPPY144_RECOVERY_CAPACITY_KB-uUsed;
}

uint32_t Floppy144RunStateRecoveredPercent(const Floppy144RunState *pState)
{
    return (Floppy144RunStateRecoveredKb(pState)*100U)/FLOPPY144_RECOVERY_CAPACITY_KB;
}

void Floppy144RunStateFormatCapacity(
    const Floppy144RunState *pState,
    char *pszBuffer,
    uint32_t uBufferCapacity
)
{
    if(pszBuffer == NULL || uBufferCapacity == 0U)
        return;

    if(pState == NULL)
    {
        (void)snprintf(
            pszBuffer,
            uBufferCapacity,
            "RECOVERED 0 / %u KB  0%%",
            (unsigned)FLOPPY144_RECOVERY_CAPACITY_KB
        );
        return;
    }

    (void)snprintf(
        pszBuffer,
        uBufferCapacity,
        "RECOVERED %u / %u KB  %u%%",
        (unsigned)Floppy144RunStateRecoveredKb(pState),
        (unsigned)FLOPPY144_RECOVERY_CAPACITY_KB,
        (unsigned)Floppy144RunStateRecoveredPercent(pState)
    );
}

uint32_t Floppy144RunStateRequiredTotalKb(void)
{
    uint32_t uIndex,uTotal=0U;
    for(uIndex=0U;uIndex<(uint32_t)FLOPPY144_COLLECTION_COUNT;++uIndex)
    {
        const Floppy144CollectionDefinition *pDefinition=Floppy144CollectionGet((Floppy144CollectionId)uIndex);
        if(pDefinition!=NULL&&pDefinition->required_for_completion)uTotal+=pDefinition->size_kb;
    }
    return uTotal;
}

uint32_t Floppy144RunStateRequiredRecoveredKb(const Floppy144RunState *pState)
{
    uint32_t uIndex,uTotal=0U;
    if(pState==NULL)return 0U;
    for(uIndex=0U;uIndex<(uint32_t)FLOPPY144_COLLECTION_COUNT;++uIndex)
    {
        const Floppy144CollectionDefinition *pDefinition=Floppy144CollectionGet((Floppy144CollectionId)uIndex);
        if(pDefinition!=NULL&&pDefinition->required_for_completion&&Floppy144RunStateCollectionRestored(pState,(Floppy144CollectionId)uIndex))uTotal+=pDefinition->size_kb;
    }
    return uTotal;
}

uint32_t Floppy144RunStateRequiredCoveragePercent(const Floppy144RunState *pState)
{
    uint32_t uTotal=Floppy144RunStateRequiredTotalKb();
    return uTotal==0U?0U:(Floppy144RunStateRequiredRecoveredKb(pState)*100U)/uTotal;
}

bool Floppy144RunStateCanRestoreCollection(const Floppy144RunState *pState,Floppy144CollectionId eCollection)
{
    const Floppy144CollectionDefinition *pDefinition;
    if(pState==NULL||!Floppy144RunStateCollectionValid(eCollection)||Floppy144RunStateCollectionRestored(pState,eCollection))return false;
    pDefinition=Floppy144CollectionGet(eCollection);
    return pDefinition!=NULL&&pDefinition->size_kb<=Floppy144RunStateFreeKb(pState);
}

bool Floppy144RunStateRestoreCollection(Floppy144RunState *pState,Floppy144CollectionId eCollection)
{
    if(pState==NULL||!Floppy144RunStateCollectionAvailable(pState,eCollection)||!Floppy144RunStateCanRestoreCollection(pState,eCollection))return false;
    if(!Floppy144RunStateBitSet(pState->collections,(uint32_t)eCollection))return false;
    pState->dirty=1U;
    return true;
}

bool Floppy144RunStateAnyUnrestoredCollectionFits(const Floppy144RunState *pState)
{
    uint32_t uIndex;
    if(pState==NULL)return false;
    for(uIndex=0U;uIndex<(uint32_t)FLOPPY144_COLLECTION_COUNT;++uIndex)
    {
        Floppy144CollectionId e=(Floppy144CollectionId)uIndex;
        if(!Floppy144RunStateCollectionRestored(pState,e)&&Floppy144RunStateCanRestoreCollection(pState,e))return true;
    }
    return false;
}

bool Floppy144RunStateRecoveryExhausted(const Floppy144RunState *pState)
{
    return pState!=NULL&&!Floppy144RunStateAnyUnrestoredCollectionFits(pState);
}

bool Floppy144RunStateTriggerFired
(
    const Floppy144RunState *state,
 Floppy144TriggerId trigger
){
    if(
        state == NULL ||
        !Floppy144RunStateTriggerValid(trigger)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->triggers,
        (uint32_t)trigger
    );
}

bool Floppy144RunStateFireTrigger
(
    Floppy144RunState *state,
 Floppy144TriggerId trigger
){
    if(
        state == NULL ||
        !Floppy144RunStateTriggerValid(trigger)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->triggers,
            (uint32_t)trigger
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateInteractionCompleted
(
    const Floppy144RunState *state,
 Floppy144InteractionId interaction
){
    if(
        state == NULL ||
        !Floppy144RunStateInteractionValid(interaction)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->interactions,
        (uint32_t)interaction
    );
}

bool Floppy144RunStateCompleteInteraction
(
    Floppy144RunState *state,
 Floppy144InteractionId interaction
){
    if(
        state == NULL ||
        !Floppy144RunStateInteractionValid(interaction)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->interactions,
            (uint32_t)interaction
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateEvidenceEstablished
(
    const Floppy144RunState *state,
 Floppy144EvidenceId evidence
){
    if(
        state == NULL ||
        !Floppy144RunStateEvidenceValid(evidence)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->evidence,
        (uint32_t)evidence
    );
}

bool Floppy144RunStateEstablishEvidence
(
    Floppy144RunState *state,
 Floppy144EvidenceId evidence
){
    if(
        state == NULL ||
        !Floppy144RunStateEvidenceValid(evidence)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->evidence,
            (uint32_t)evidence
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

bool Floppy144RunStateNotebookEntryRecorded
(
    const Floppy144RunState *state,
 Floppy144NotebookId entry
){
    if(
        state == NULL ||
        !Floppy144RunStateNotebookEntryValid(entry)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->notebook,
        (uint32_t)entry
    );
}

bool Floppy144RunStateAppendNotebookEntry(Floppy144RunState *pState,uint32_t uNotebookOrdinal)
{
    uint16_t u;
    if(pState==NULL||uNotebookOrdinal>=FLOPPY144_NOTEBOOK_ORDER_MAX||pState->notebook_order_count>=FLOPPY144_NOTEBOOK_ORDER_MAX)return false;
    for(u=0U;u<pState->notebook_order_count;++u)if((uint32_t)pState->notebook_order[u]==uNotebookOrdinal)return false;
    pState->notebook_order[pState->notebook_order_count++]=(uint16_t)uNotebookOrdinal;
    pState->dirty=1U;
    return true;
}

bool Floppy144RunStateRecordNotebookEntry(Floppy144RunState *pState,Floppy144NotebookId eEntry)
{
    if(pState==NULL||!Floppy144RunStateNotebookEntryValid(eEntry))return false;
    if(!Floppy144RunStateBitSet(pState->notebook,(uint32_t)eEntry))return false;
    (void)Floppy144RunStateAppendNotebookEntry(pState,(uint32_t)eEntry);
    pState->dirty=1U;
    return true;
}

bool Floppy144RunStateHasCapability
(
    const Floppy144RunState *state,
 Floppy144CapabilityId capability
){
    if(
        state == NULL ||
        !Floppy144RunStateCapabilityValid(capability)
    )
    {
        return false;
    }

    return
    Floppy144RunStateBitGet(
        state->capabilities,
        (uint32_t)capability
    );
}

bool Floppy144RunStateGrantCapability
(
    Floppy144RunState *state,
 Floppy144CapabilityId capability
){
    if(
        state == NULL ||
        !Floppy144RunStateCapabilityValid(capability)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            state->capabilities,
            (uint32_t)capability
        )
    )
    {
        return false;
    }

    state->dirty = 1;

    return true;
}

void Floppy144RunStateSetPlayerSitePosition
(
    Floppy144RunState *state,
 int32_t x,
 int32_t y
){
    if(state == NULL)
    {
        return;
    }

    if(
        state->player_site_x == x &&
        state->player_site_y == y
    )
    {
        return;
    }

    state->player_site_x =
    x;

    state->player_site_y =
    y;

    state->dirty = 1;
}

/*
 * Determine whether progression state permits movement between two
 * reconstructed rooms.
 *
 * Physical door geometry remains authoritative in the Site model.
 * This function supplies only the recovery/progression access layer.
 *
 * Current early-Site rules:
 *
 * T-003:
 *   Reception <-> Main Office
 *
 * T-005:
 *   Reception  <-> Corridor
 *   Main Office <-> Corridor
 *
 * Other room-access rules are added here as their authored trigger routes
 * are migrated.
 */
static bool Floppy144RunStateRoomTransitionAllowed
(
    const Floppy144RunState *pState,
    Floppy144RoomId eFromRoom,
    Floppy144RoomId eToRoom
)
{
    return Floppy144GameDataRoomTransitionAllowed(pState, eFromRoom, eToRoom);
}

/*
 * Run-aware collision filter.
 *
 * Compiled Site geometry may contain a progression-controlled fixture before
 * that fixture has been reconstructed into the current run. Rendering already
 * suppresses such geometry through Floppy144SiteObjectGeometryVisible(); use
 * the same decision for collision so an invisible item cannot block movement.
 */
static bool Floppy144RunStateCollisionGeometryVisible
(
    const Floppy144SiteRect *pRect,
    void *pContext
)
{
    const Floppy144RunState *pState =
        (const Floppy144RunState *)pContext;

    if(pState == NULL || pRect == NULL)
    {
        return false;
    }

    return Floppy144SiteRectRuntimeVisible(
        pState,
        pRect
    );
}

/*
 * Convert one generated connection endpoint to the compact room code carried
 * by Site boundary rectangles.
 */
static uint8_t Floppy144RunStateConnectionEndpoint
(
    const char *pszRoomId
)
{
    Floppy144RoomId eRoom;

    if(pszRoomId == NULL)
    {
        return FLOPPY144_SITE_ROOM_SHARED;
    }

    if(strcmp(pszRoomId, "OUTSIDE") == 0)
    {
        return FLOPPY144_SITE_ROOM_OUTSIDE;
    }

    eRoom = Floppy144GameDataRoomId(pszRoomId);

    if((uint32_t)eRoom >= (uint32_t)FLOPPY144_ROOM_COUNT)
    {
        return FLOPPY144_SITE_ROOM_SHARED;
    }

    return (uint8_t)eRoom;
}

/*
 * Determine whether a generated exterior door is currently unlocked.
 *
 * Boundary rectangles intentionally remain compact and do not carry string
 * connection IDs. Their endpoints identify the corresponding canonical
 * connection record. Parallel exterior doors in the current Site share the
 * same state, so endpoint matching is sufficient.
 */
static bool Floppy144RunStateExteriorDoorUnlocked
(
    const Floppy144RunState *pState,
    const Floppy144SiteRect *pDoor
)
{
    uint32_t uRecordIndex;

    if(
        pState == NULL ||
        pDoor == NULL ||
        pDoor->type != (uint8_t)FLOPPY144_SITE_DOOR ||
        (
            pDoor->from_room != FLOPPY144_SITE_ROOM_OUTSIDE &&
            pDoor->to_room != FLOPPY144_SITE_ROOM_OUTSIDE
        )
    )
    {
        return false;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pConnection =
            Floppy144GameDataRecordAt(uRecordIndex);

        uint8_t uFrom;
        uint8_t uTo;

        if(
            pConnection == NULL ||
            pConnection->eKind != FLOPPY144_DATA_CONNECTION
        )
        {
            continue;
        }

        uFrom =
            Floppy144RunStateConnectionEndpoint(
                pConnection->pszA
            );

        uTo =
            Floppy144RunStateConnectionEndpoint(
                pConnection->pszB
            );

        if(
            !(
                (
                    uFrom == pDoor->from_room &&
                    uTo == pDoor->to_room
                ) ||
                (
                    uFrom == pDoor->to_room &&
                    uTo == pDoor->from_room
                )
            )
        )
        {
            continue;
        }

        if(
            pConnection->pszId != NULL &&
            Floppy144GameDataConnectionUnlocked(
                pState,
                pConnection->pszId
            )
        )
        {
            return true;
        }
    }

    return false;
}

bool Floppy144RunStateWouldExitSite
(
    const Floppy144RunState *state,
    int32_t delta_x16,
    int32_t delta_y16
)
{
    const Floppy144SiteRect *pDoor;
    Floppy144RoomId eCurrentRoom;
    uint8_t uInteriorRoom;

    if(state == NULL)
    {
        return false;
    }

    pDoor =
        Floppy144SiteExteriorDoorForMove(
            state->player_site_x,
            state->player_site_y,
            delta_x16,
            delta_y16
        );

    if(pDoor == NULL)
    {
        return false;
    }

    eCurrentRoom =
        Floppy144SiteRoomAtPosition(
            state->player_site_x,
            state->player_site_y
        );

    if(
        (uint32_t)eCurrentRoom >= (uint32_t)FLOPPY144_ROOM_COUNT ||
        !Floppy144RunStateRoomReconstructed(
            state,
            eCurrentRoom
        )
    )
    {
        return false;
    }

    uInteriorRoom =
        pDoor->from_room == FLOPPY144_SITE_ROOM_OUTSIDE
            ? pDoor->to_room
            : pDoor->from_room;

    if(uInteriorRoom != (uint8_t)eCurrentRoom)
    {
        return false;
    }

    return
        Floppy144RunStateExteriorDoorUnlocked(
            state,
            pDoor
        );
}

bool Floppy144RunStateMovePlayerSite
(
    Floppy144RunState *state,
    int32_t delta_x16,
    int32_t delta_y16
)
{
    int32_t x;
    int32_t y;
    Floppy144RoomId current_room;

    if(state == NULL)
    {
        return false;
    }

    current_room =
        Floppy144SiteRoomAtPosition(
            state->player_site_x,
            state->player_site_y
        );

    if(current_room == FLOPPY144_ROOM_COUNT)
    {
        return false;
    }

    x =
    state->player_site_x;

    y =
    state->player_site_y;

    if(
        !Floppy144SiteMovePositionFiltered(
            &x,
            &y,
            delta_x16,
            delta_y16,
            Floppy144RunStateCollisionGeometryVisible,
            state
        )
    )
    {
        return false;
    }

    /*
     * Physical collision decides whether the candidate position is walkable.
     * Reconstruction state decides whether the room at that position exists
     * in this recovery. Door-threshold classification in site_rooms.c makes
     * this a clean room-transition gate without a second door-lock map.
     */
    {
        Floppy144RoomId destination_room =
            Floppy144SiteRoomAtPosition(
                x,
                y
            );

        if(
            destination_room == FLOPPY144_ROOM_COUNT ||
            !Floppy144RunStateRoomReconstructed(
                state,
                destination_room
            )
        )
        {
            return false;
        }

        /*
         * A room may be reconstructed without yet being accessible.
         *
         * Door geometry decides whether a physical crossing exists.
         * Trigger state decides whether recovery has unlocked that crossing.
         */
        if(
            !Floppy144RunStateRoomTransitionAllowed(
                state,
                current_room,
                destination_room
            )
        )
        {
            return false;
        }
    }

    state->player_site_x =
    x;

    state->player_site_y =
    y;

    state->dirty =
    1;

    return true;
}

Floppy144Projection Floppy144RunStateProjection
(
    const Floppy144RunState *state
){
    if(
        state == NULL ||
        state->projection >=
        (uint8_t)FLOPPY144_PROJECTION_COUNT
    )
    {
        return FLOPPY144_PROJECTION_2D;
    }

    return
    (Floppy144Projection)state->projection;
}

bool Floppy144RunStateSetProjection
(
    Floppy144RunState *state,
 Floppy144Projection projection
){
    if(
        state == NULL ||
        (uint32_t)projection >=
        (uint32_t)FLOPPY144_PROJECTION_COUNT
    )
    {
        return false;
    }

    if(
        state->projection ==
        (uint8_t)projection
    )
    {
        return false;
    }

    state->projection =
    (uint8_t)projection;

    state->dirty = 1;

    return true;
}

Floppy144RunAct Floppy144RunStateAct
(
    const Floppy144RunState *pState
)
{
    if(pState == NULL || pState->act > (uint8_t)FLOPPY144_RUN_ACT_COMPLETE) return FLOPPY144_RUN_ACT_PROLOGUE;
    return (Floppy144RunAct)pState->act;
}

bool Floppy144RunStateSetAct
(
    Floppy144RunState *pState,
    Floppy144RunAct eAct
)
{
    if(pState == NULL || (uint32_t)eAct > (uint32_t)FLOPPY144_RUN_ACT_COMPLETE || (uint8_t)eAct <= pState->act) return false;
    pState->act=(uint8_t)eAct; pState->dirty=1U; return true;
}

bool Floppy144RunStateIsIsometric
(
    const Floppy144RunState *pState
)
{
    return Floppy144RunStateProjection(pState) == FLOPPY144_PROJECTION_ISOMETRIC;
}

bool Floppy144RunStateSetBranch
(
    Floppy144RunState *state,
 Floppy144RunBranch branch
)
{
    if(
        state == NULL ||
        (
            branch !=
            FLOPPY144_RUN_BRANCH_RECORDS_FIRST &&
            branch !=
            FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST
        )
    )
    {
        return false;
    }

    /*
     * Branch selection is a commitment. Once one route has been selected,
     * another trigger cannot silently replace it.
     */
    if(
        state->branch !=
        (uint8_t)FLOPPY144_RUN_BRANCH_NONE
    )
    {
        return false;
    }

    state->branch =
    (uint8_t)branch;

    state->dirty =
    1U;

    return true;
}

void Floppy144RunStateReset
(
    Floppy144RunState *state
){
    if(!state)
    {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );

    state->act =
        FLOPPY144_RUN_ACT_PROLOGUE;

    state->branch =
        FLOPPY144_RUN_BRANCH_NONE;
}

Floppy144ObjectAccessState Floppy144RunStateObjectAccessState
(
    const Floppy144RunState *state,
 Floppy144ObjectId object
){
    const Floppy144ObjectDefinition *definition;

    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return FLOPPY144_OBJECT_ACCESS_NONE;
    }

    definition =
    Floppy144ObjectGet(object);

    if(
        definition == NULL ||
        !(definition->flags & FLOPPY144_OBJECT_FLAG_OPENABLE)
    )
    {
        return FLOPPY144_OBJECT_ACCESS_NONE;
    }

    if(
        Floppy144RunStateBitGet(
            state->objects_open,
            (uint32_t)object
        )
    )
    {
        return FLOPPY144_OBJECT_ACCESS_OPEN;
    }

    if(
        Floppy144RunStateBitGet(
            state->objects_unlocked,
            (uint32_t)object
        )
    )
    {
        return FLOPPY144_OBJECT_ACCESS_UNLOCKED;
    }

    return FLOPPY144_OBJECT_ACCESS_LOCKED;
}

bool Floppy144RunStateSetObjectAccessState
(
    Floppy144RunState *state,
 Floppy144ObjectId object,
 Floppy144ObjectAccessState access_state
){
    const Floppy144ObjectDefinition *definition;

    bool unlocked;
    bool open;

    bool changed =
    false;

    if(
        state == NULL ||
        !Floppy144RunStateObjectValid(object)
    )
    {
        return false;
    }

    definition =
    Floppy144ObjectGet(object);

    if(
        definition == NULL ||
        !(definition->flags & FLOPPY144_OBJECT_FLAG_OPENABLE)
    )
    {
        return false;
    }

    switch(access_state)
    {
        case FLOPPY144_OBJECT_ACCESS_LOCKED:
        {
            unlocked = false;
            open = false;
            break;
        }

        case FLOPPY144_OBJECT_ACCESS_UNLOCKED:
        {
            unlocked = true;
            open = false;
            break;
        }

        case FLOPPY144_OBJECT_ACCESS_OPEN:
        {
            unlocked = true;
            open = true;
            break;
        }

        case FLOPPY144_OBJECT_ACCESS_NONE:
        default:
        {
            return false;
        }
    }

    if(unlocked)
    {
        changed |=
        Floppy144RunStateBitSet(
            state->objects_unlocked,
            (uint32_t)object
        );
    }
    else
    {
        changed |=
        Floppy144RunStateBitClear(
            state->objects_unlocked,
            (uint32_t)object
        );
    }

    if(open)
    {
        changed |=
        Floppy144RunStateBitSet(
            state->objects_open,
            (uint32_t)object
        );
    }
    else
    {
        changed |=
        Floppy144RunStateBitClear(
            state->objects_open,
            (uint32_t)object
        );
    }

    if(changed)
    {
        state->dirty = 1;
    }

    return changed;
}

void Floppy144RunStateBegin
(
    Floppy144RunState *state,
    uint32_t recovery_seed
)
{
    uint32_t object_index;

    Floppy144RunStateReset(
        state
    );

    if(!state)
    {
        return;
    }

    state->recovery_seed =
    recovery_seed;

    Floppy144SiteSpawnPosition(
        &state->player_site_x,
        &state->player_site_y
    );

    for(
        object_index = 0U;
    object_index <
    (uint32_t)FLOPPY144_OBJECT_COUNT;
    ++object_index
    )
    {
        const Floppy144ObjectDefinition *definition =
        Floppy144ObjectGet(
            (Floppy144ObjectId)object_index
        );

        if(
            definition != NULL &&
            definition->initially_visible
        )
        {
            Floppy144RunStateBitSet(
                state->objects_visible,
                object_index
            );
        }
        if(
            definition != NULL &&
            (definition->flags & FLOPPY144_OBJECT_FLAG_OPENABLE) &&
            (definition->flags & FLOPPY144_OBJECT_FLAG_INITIALLY_UNLOCKED)
        )
        {
            Floppy144RunStateBitSet(
                state->objects_unlocked,
                object_index
            );
        }
    }

    state->dirty = 1;
}

uint32_t Floppy144RunStateReconstructionPercent(const Floppy144RunState *pState)
{
    return Floppy144RunStateRecoveredPercent(pState);
}

bool Floppy144RunStateArchiveServicesInitialised
(
    const Floppy144RunState *state
){
    if(state == NULL)
    {
        return false;
    }

    return
    state->archive_services_initialised != 0U;
}

bool Floppy144RunStateInitialiseArchiveServices
(
    Floppy144RunState *state
){
    if(
        state == NULL ||
        state->archive_services_initialised != 0U
    )
    {
        return false;
    }

    state->archive_services_initialised =
    1U;

    state->dirty =
    1U;

    return true;
}
