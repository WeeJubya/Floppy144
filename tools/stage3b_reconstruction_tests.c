/*
 * Floppy//144 Stage 3B.3 progressive Site reconstruction regression.
 *
 * Proves that room reconstruction is driven by canonical trigger/interaction
 * data, that collection availability follows ready trigger records, that Site
 * geometry obeys one reconstruction visibility contract, and that room state
 * survives the versioned persistence payload.
 */

#include "floppy144_collection_registry.h"
#include "floppy144_game_data.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_persistence.h"
#include "floppy144_run_state.h"
#include "floppy144_site.h"
#include "floppy144_site_object.h"
#include "floppy144_trigger_engine.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_nFailures = 0;

#define F144_CHECK(bCondition, pszMessage)                         \
    do                                                            \
    {                                                             \
        if(!(bCondition))                                         \
        {                                                         \
            fprintf(                                              \
                stderr,                                           \
                "FAIL: %s (line %d)\n",                          \
                (pszMessage),                                     \
                __LINE__                                          \
            );                                                    \
            ++g_nFailures;                                        \
        }                                                         \
    }                                                             \
    while(0)

static void Floppy144TestReset(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState
)
{
    Floppy144WorldReset(pWorld);
    Floppy144RunStateBegin(pState, 144U);
}

static bool Floppy144TestRestoreCollection(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState,
    const char *pszCollectionId
)
{
    Floppy144CollectionId eCollection =
        Floppy144GameDataCollectionId(pszCollectionId);

    if(
        pWorld == NULL ||
        pState == NULL ||
        eCollection == FLOPPY144_COLLECTION_COUNT ||
        !Floppy144RunStateCollectionAvailable(
            pState,
            eCollection
        ) ||
        !Floppy144RunStateRestoreCollection(
            pState,
            eCollection
        )
    )
    {
        return false;
    }

    (void)Floppy144WorldRestoreCollection(
        pWorld,
        eCollection
    );

    return true;
}

static bool Floppy144TestFireTrigger(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState,
    const char *pszTriggerId
)
{
    Floppy144TriggerId eTrigger =
        Floppy144GameDataTriggerId(pszTriggerId);

    return
        eTrigger != FLOPPY144_TRIGGER_COUNT &&
        Floppy144TriggerTryFire(
            pWorld,
            pState,
            eTrigger
        );
}

static bool Floppy144TestRunInteraction(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState,
    const char *pszInteractionId
)
{
    Floppy144InteractionId eInteraction =
        Floppy144GameDataInteractionId(pszInteractionId);

    return
        eInteraction != FLOPPY144_INTERACTION_COUNT &&
        Floppy144InteractionTryRun(
            pWorld,
            pState,
            eInteraction
        );
}

/*
 * Every room must have at least one canonical RECONSTRUCT_ROOM source.
 *
 * Most rooms are reconstructed by trigger documents. Director's Office is
 * deliberately reconstructed by I-036 when its conventional door is opened.
 */
static void Floppy144TestReconstructionSourceCoverage(void)
{
    bool abRoomHasSource[FLOPPY144_ROOM_COUNT] = { false };
    uint32_t uRecordIndex;
    uint32_t uEffectCount = 0U;
    uint32_t uRoomIndex;

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        Floppy144RoomId eRoom;

        if(
            pRecord == NULL ||
            (
                pRecord->eKind != FLOPPY144_DATA_TRIGGER_EFFECT &&
                pRecord->eKind != FLOPPY144_DATA_INTERACTION_EFFECT
            ) ||
            pRecord->pszA == NULL ||
            strcmp(pRecord->pszA, "RECONSTRUCT_ROOM") != 0
        )
        {
            continue;
        }

        eRoom =
            Floppy144GameDataRoomId(
                pRecord->pszB
            );

        F144_CHECK(
            eRoom < FLOPPY144_ROOM_COUNT,
            "RECONSTRUCT_ROOM target resolves to a registered room"
        );

        if(eRoom < FLOPPY144_ROOM_COUNT)
        {
            abRoomHasSource[(uint32_t)eRoom] = true;
            ++uEffectCount;
        }
    }

    F144_CHECK(
        uEffectCount >= (uint32_t)FLOPPY144_ROOM_COUNT,
        "canonical data contains enough room reconstruction effects"
    );

    for(
        uRoomIndex = 0U;
        uRoomIndex < (uint32_t)FLOPPY144_ROOM_COUNT;
        ++uRoomIndex
    )
    {
        F144_CHECK(
            abRoomHasSource[uRoomIndex],
            "every registered room has a canonical reconstruction source"
        );
    }
}

/*
 * RECONSTRUCT_ROOM is a reusable engine verb, not a switch over story rooms.
 * Exercise it against every generated room record.
 */
static void Floppy144TestGenericReconstructionVerb(void)
{
    uint32_t uRecordIndex;
    uint32_t uRoomRecordCount = 0U;

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        Floppy144WorldState sWorld;
        Floppy144RunState sState;
        Floppy144RoomId eRoom;

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_ROOM
        )
        {
            continue;
        }

        ++uRoomRecordCount;

        eRoom =
            Floppy144GameDataRoomId(
                pRecord->pszId
            );

        Floppy144TestReset(
            &sWorld,
            &sState
        );

        F144_CHECK(
            eRoom < FLOPPY144_ROOM_COUNT,
            "generated room record resolves"
        );

        F144_CHECK(
            Floppy144GameDataExecuteEffect(
                &sWorld,
                &sState,
                "RECONSTRUCT_ROOM",
                pRecord->pszId
            ),
            "generic RECONSTRUCT_ROOM effect executes"
        );

        F144_CHECK(
            eRoom < FLOPPY144_ROOM_COUNT &&
            Floppy144RunStateRoomReconstructed(
                &sState,
                eRoom
            ),
            "generic reconstruction effect persists room bit"
        );
    }

    F144_CHECK(
        uRoomRecordCount == (uint32_t)FLOPPY144_ROOM_COUNT,
        "all generated room records exercise reconstruction verb"
    );
}

/*
 * Collection availability follows actionable trigger records.
 *
 * This is the progression bridge that lets a newly valid reconstruction
 * document appear in LIST without a collection-specific C function.
 */
static void Floppy144TestProgressionAvailability(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;

    Floppy144CollectionId eFm04;
    Floppy144CollectionId eFm07;
    Floppy144CollectionId eDr04;
    Floppy144CollectionId eHr05;

    Floppy144TestReset(
        &sWorld,
        &sState
    );

    F144_CHECK(
        Floppy144WorldInitialiseArchiveServices(&sWorld) &&
        Floppy144RunStateInitialiseArchiveServices(&sState),
        "progression fixture initialises archive services"
    );

    eFm04 = Floppy144GameDataCollectionId("FM-04");
    eFm07 = Floppy144GameDataCollectionId("FM-07");
    eDr04 = Floppy144GameDataCollectionId("DR-04");
    eHr05 = Floppy144GameDataCollectionId("HR-05");

    F144_CHECK(
        eFm04 < FLOPPY144_COLLECTION_COUNT &&
        eFm07 < FLOPPY144_COLLECTION_COUNT &&
        eDr04 < FLOPPY144_COLLECTION_COUNT &&
        eHr05 < FLOPPY144_COLLECTION_COUNT,
        "progression collection fixtures resolve"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionAvailable(&sState, eFm04),
        "FM-04 is unavailable before Main Office reconstruction"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "DR-01"
        ),
        "opening collection DR-01 restores normally"
    );

    F144_CHECK(
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-001"
        ),
        "T-001 reconstructs opening Site"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "HR-01"
        ),
        "T-001 enables HR-01"
    );

    F144_CHECK(
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-002"
        ),
        "T-002 reconstructs Main Office"
    );

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sState,
            eFm04
        ),
        "FM-04 becomes available when T-004 is actionable"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "FM-04"
        ) &&
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-004"
        ),
        "FM-04 restores and reconstructs Facilities"
    );

    F144_CHECK(
        Floppy144GameDataConnectionUnlocked(
            &sState,
            "COR_FAC"
        ),
        "T-004 satisfies canonical COR_FAC unlock condition"
    );

    F144_CHECK(
        Floppy144GameDataRoomTransitionAllowed(
            &sState,
            FLOPPY144_ROOM_CORRIDOR,
            FLOPPY144_ROOM_FACILITIES
        ),
        "reconstructed Facilities is reachable from Corridor after T-004"
    );

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sState,
            eFm07
        ),
        "FM-07 becomes available when Facilities exists"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "FM-07"
        ) &&
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-008"
        ) &&
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-009"
        ),
        "FM-07 restoration reaches physical closure material"
    );

    F144_CHECK(
        Floppy144TestRunInteraction(
            &sWorld,
            &sState,
            "I-002"
        ) &&
        Floppy144TestRunInteraction(
            &sWorld,
            &sState,
            "I-003"
        ),
        "physical closure chain establishes E-003"
    );

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sState,
            eDr04
        ),
        "DR-04 becomes available after closure evidence"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "DR-04"
        ) &&
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-010"
        ),
        "DR-04 begins Records-first workstream"
    );

    F144_CHECK(
        Floppy144RunStateRoomReconstructed(
            &sState,
            FLOPPY144_ROOM_RECORDS_OFFICE
        ),
        "Records Office reconstructed by canonical trigger"
    );

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sState,
            eHr05
        ),
        "HR-05 becomes available when its Act-II trigger is actionable"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "HR-05"
        ) &&
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-012"
        ),
        "HR-05 reconstructs Staff Room"
    );

    F144_CHECK(
        Floppy144RunStateRoomReconstructed(
            &sState,
            FLOPPY144_ROOM_STAFF_ROOM
        ) &&
        Floppy144GameDataConnectionUnlocked(
            &sState,
            "COR_STAFF"
        ),
        "HR-05 T-012 restores Staff Room and unlocks its Corridor door together"
    );
}

/*
 * Exercise the canonical sources which eventually cover every room.
 *
 * The middle of the Records/Technology workstreams has its own detailed Stage
 * 2/interaction regressions. To reach the late reconstruction gate without
 * duplicating every narrative evidence test here, mark the pre-T-026 branch
 * trigger ledger as completed. T-026/T-027/T-028, T-045 and I-035/I-036 then
 * execute through their real generic trigger/interaction paths.
 */
static void Floppy144TestCanonicalRoomProgression(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    uint32_t uRecordIndex;
    uint32_t uRoomIndex;

    Floppy144TestReset(
        &sWorld,
        &sState
    );

    (void)Floppy144WorldInitialiseArchiveServices(&sWorld);
    (void)Floppy144RunStateInitialiseArchiveServices(&sState);

    F144_CHECK(
        Floppy144TestRestoreCollection(&sWorld, &sState, "DR-01") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-001") &&
        Floppy144TestRestoreCollection(&sWorld, &sState, "HR-01") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-002") &&
        Floppy144TestRestoreCollection(&sWorld, &sState, "FM-04") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-004") &&
        Floppy144TestRestoreCollection(&sWorld, &sState, "FM-07") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-008") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-009") &&
        Floppy144TestRunInteraction(&sWorld, &sState, "I-002") &&
        Floppy144TestRunInteraction(&sWorld, &sState, "I-003") &&
        Floppy144TestRestoreCollection(&sWorld, &sState, "DR-04") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-010") &&
        Floppy144TestRestoreCollection(&sWorld, &sState, "HR-05") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-012"),
        "canonical progression reaches the main Stage 3B reconstruction state"
    );

    /*
     * Represent completion of the already-tested middle branch. Source-order
     * 13..25 is exactly the pre-T-026 core window consumed by the semantic
     * ACT_II_CORE_COMPLETE condition.
     */
    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        if(
            pRecord != NULL &&
            pRecord->eKind == FLOPPY144_DATA_TRIGGER &&
            pRecord->n2 >= 13 &&
            pRecord->n2 < 26
        )
        {
            Floppy144TriggerId eTrigger =
                Floppy144GameDataTriggerId(
                    pRecord->pszId
                );

            if(
                eTrigger != FLOPPY144_TRIGGER_COUNT &&
                !Floppy144RunStateTriggerFired(
                    &sState,
                    eTrigger
                )
            )
            {
                (void)Floppy144RunStateFireTrigger(
                    &sState,
                    eTrigger
                );
            }
        }
    }

    F144_CHECK(
        Floppy144TestRunInteraction(
            &sWorld,
            &sState,
            "I-008"
        ),
        "completed Records branch establishes its final evidence"
    );

    F144_CHECK(
        Floppy144GameDataConditionSatisfied(
            &sState,
            "act_at_least",
            "ACT_II_CORE_COMPLETE"
        ),
        "completed branch evidence satisfies late reconstruction gate"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "DR-47"
        ),
        "DR-47 becomes available from its ready late-stage trigger"
    );

    F144_CHECK(
        Floppy144TestFireTrigger(&sWorld, &sState, "T-026") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-027") &&
        Floppy144TestFireTrigger(&sWorld, &sState, "T-028"),
        "late closure triggers reconstruct Security/Secretary and release alternate workstream"
    );

    F144_CHECK(
        Floppy144RunStateRoomReconstructed(
            &sState,
            FLOPPY144_ROOM_SECURITY
        ) &&
        Floppy144RunStateRoomReconstructed(
            &sState,
            FLOPPY144_ROOM_SECRETARY_OFFICE
        ),
        "Security and Secretary Office reconstructed by T-027"
    );

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "TS-14"
        ) &&
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-045"
        ),
        "TS-14 reconstructs Server Room after technology workstream release"
    );

    F144_CHECK(
        Floppy144TestRunInteraction(
            &sWorld,
            &sState,
            "I-035"
        ) &&
        Floppy144TestRunInteraction(
            &sWorld,
            &sState,
            "I-036"
        ),
        "Site keys and Secretary door reconstruct Director Office"
    );

    /*
     * T-028 has released the alternate workstream. The other DR-04 trigger can
     * now reconstruct IT Support while the original branch identity remains
     * persistent.
     */
    F144_CHECK(
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-011"
        ),
        "released alternate workstream reconstructs IT Support"
    );

    for(
        uRoomIndex = 0U;
        uRoomIndex < (uint32_t)FLOPPY144_ROOM_COUNT;
        ++uRoomIndex
    )
    {
        F144_CHECK(
            Floppy144RunStateRoomReconstructed(
                &sState,
                (Floppy144RoomId)uRoomIndex
            ),
            "canonical reconstruction route covers every Site room"
        );
    }
}

/*
 * Wall hangings retain their authored Site rectangle for interaction targeting
 * but do not consume walkable floor. Furniture and structural boundaries still
 * participate in collision normally.
 */
static void Floppy144TestWallHangingCollisionContract(void)
{
    F144_CHECK(
        !Floppy144SiteElementBlocksMovement(
            FLOPPY144_SITE_WALL_MOUNTED_ITEM
        ),
        "wall-mounted items do not create a 1U floor collision box"
    );

    F144_CHECK(
        Floppy144SiteElementBlocksMovement(
            FLOPPY144_SITE_STANDARD_DESK
        ),
        "ordinary furniture still blocks player movement"
    );

    F144_CHECK(
        Floppy144SiteElementBlocksMovement(
            FLOPPY144_SITE_PARTITION_WALL
        ),
        "partition walls remain structural collision"
    );
}

/*
 * Collision support must obey the same runtime visibility contract as drawing.
 *
 * Door cells are authored as walkable ground so an active doorway can bridge
 * two room floors. Before the destination room exists, however, that same
 * compiled door must not carve an invisible walkable slot into the wall.
 */
static bool Floppy144TestRuntimeCollisionVisible(
    const Floppy144SiteRect *pRect,
    void *pContext
)
{
    return
        Floppy144SiteRectRuntimeVisible(
            (const Floppy144RunState *)pContext,
            pRect
        );
}

static const Floppy144SiteRect *Floppy144TestFindDoorBetweenRooms(
    Floppy144RoomId eRoomA,
    Floppy144RoomId eRoomB
)
{
    uint32_t uRectIndex;

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);

        if(
            pRect != NULL &&
            pRect->type == (uint8_t)FLOPPY144_SITE_DOOR &&
            (
                (
                    pRect->from_room == (uint8_t)eRoomA &&
                    pRect->to_room == (uint8_t)eRoomB
                ) ||
                (
                    pRect->from_room == (uint8_t)eRoomB &&
                    pRect->to_room == (uint8_t)eRoomA
                )
            )
        )
        {
            return pRect;
        }
    }

    return NULL;
}

static bool Floppy144TestPlayerFootprintOverlapsRect(
    int32_t nCentreX16,
    int32_t nCentreY16,
    const Floppy144SiteRect *pRect
)
{
    int32_t nPlayerX0;
    int32_t nPlayerX1;
    int32_t nPlayerY0;
    int32_t nPlayerY1;
    int32_t nRectX0;
    int32_t nRectX1;
    int32_t nRectY0;
    int32_t nRectY1;

    if(pRect == NULL)
    {
        return false;
    }

    nPlayerX0 =
        nCentreX16 -
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 / 2;
    nPlayerX1 =
        nCentreX16 +
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 / 2;
    nPlayerY0 =
        nCentreY16 -
        FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16;
    nPlayerY1 =
        nCentreY16;

    nRectX0 =
        (int32_t)pRect->x *
        FLOPPY144_SITE_FIXED_ONE;
    nRectX1 =
        ((int32_t)pRect->x + (int32_t)pRect->width) *
        FLOPPY144_SITE_FIXED_ONE;
    nRectY0 =
        (int32_t)pRect->y *
        FLOPPY144_SITE_FIXED_ONE;
    nRectY1 =
        ((int32_t)pRect->y + (int32_t)pRect->height) *
        FLOPPY144_SITE_FIXED_ONE;

    return
        nPlayerX1 > nRectX0 &&
        nPlayerX0 < nRectX1 &&
        nPlayerY1 > nRectY0 &&
        nPlayerY0 < nRectY1;
}

static void Floppy144TestHiddenDoorGroundPair(
    Floppy144RoomId eVisibleRoom,
    Floppy144RoomId eHiddenRoom,
    const char *pszDoorMessage,
    const char *pszVisibleMessage,
    const char *pszHiddenMessage
)
{
    Floppy144WorldState sHiddenWorld;
    Floppy144RunState sHiddenState;
    Floppy144WorldState sVisibleWorld;
    Floppy144RunState sVisibleState;
    const Floppy144SiteRect *pDoor;
    int32_t nMinX16;
    int32_t nMaxX16;
    int32_t nMinY16;
    int32_t nMaxY16;
    int32_t nX16;
    int32_t nY16;
    bool bVisibleThreshold = false;
    bool bGhostThreshold = false;

    Floppy144TestReset(
        &sHiddenWorld,
        &sHiddenState
    );
    Floppy144TestReset(
        &sVisibleWorld,
        &sVisibleState
    );

    (void)Floppy144RunStateReconstructRoom(
        &sHiddenState,
        eVisibleRoom
    );
    (void)Floppy144RunStateReconstructRoom(
        &sVisibleState,
        eVisibleRoom
    );
    (void)Floppy144RunStateReconstructRoom(
        &sVisibleState,
        eHiddenRoom
    );

    pDoor =
        Floppy144TestFindDoorBetweenRooms(
            eVisibleRoom,
            eHiddenRoom
        );

    F144_CHECK(
        pDoor != NULL,
        pszDoorMessage
    );

    if(pDoor == NULL)
    {
        return;
    }

    nMinX16 =
        ((int32_t)pDoor->x - 3) *
        FLOPPY144_SITE_FIXED_ONE;
    nMaxX16 =
        ((int32_t)pDoor->x + (int32_t)pDoor->width + 3) *
        FLOPPY144_SITE_FIXED_ONE;
    nMinY16 =
        ((int32_t)pDoor->y - 3) *
        FLOPPY144_SITE_FIXED_ONE;
    nMaxY16 =
        ((int32_t)pDoor->y + (int32_t)pDoor->height + 3) *
        FLOPPY144_SITE_FIXED_ONE;

    if(nMinX16 < 0) nMinX16 = 0;
    if(nMinY16 < 0) nMinY16 = 0;
    if(
        nMaxX16 >
        FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE
    )
    {
        nMaxX16 =
            FLOPPY144_SITE_SIZE_UNITS *
            FLOPPY144_SITE_FIXED_ONE;
    }
    if(
        nMaxY16 >
        FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE
    )
    {
        nMaxY16 =
            FLOPPY144_SITE_SIZE_UNITS *
            FLOPPY144_SITE_FIXED_ONE;
    }

    for(
        nY16 = nMinY16;
        nY16 <= nMaxY16;
        nY16 += FLOPPY144_SITE_MOVE_STEP_X16
    )
    {
        for(
            nX16 = nMinX16;
            nX16 <= nMaxX16;
            nX16 += FLOPPY144_SITE_MOVE_STEP_X16
        )
        {
            if(
                !Floppy144TestPlayerFootprintOverlapsRect(
                    nX16,
                    nY16,
                    pDoor
                )
            )
            {
                continue;
            }

            if(
                !Floppy144SitePositionBlockedFiltered(
                    nX16,
                    nY16,
                    Floppy144TestRuntimeCollisionVisible,
                    &sVisibleState
                )
            )
            {
                bVisibleThreshold = true;
            }

            if(
                !Floppy144SitePositionBlockedFiltered(
                    nX16,
                    nY16,
                    Floppy144TestRuntimeCollisionVisible,
                    &sHiddenState
                )
            )
            {
                bGhostThreshold = true;
            }
        }
    }

    F144_CHECK(
        bVisibleThreshold,
        pszVisibleMessage
    );

    F144_CHECK(
        !bGhostThreshold,
        pszHiddenMessage
    );
}

static void Floppy144TestHiddenDoorsDoNotCreateGhostThresholds(void)
{
    Floppy144TestHiddenDoorGroundPair(
        FLOPPY144_ROOM_IT_SUPPORT,
        FLOPPY144_ROOM_SERVER_ROOM,
        "IT Support / Server Room door fixture resolves",
        "reconstructed IT Support / Server Room door has a walkable threshold",
        "hidden Server Room door leaves no ghost threshold in IT Support"
    );

    Floppy144TestHiddenDoorGroundPair(
        FLOPPY144_ROOM_CORRIDOR,
        FLOPPY144_ROOM_RECORDS_OFFICE,
        "Corridor / Records Office door fixture resolves",
        "reconstructed Records Office door has a walkable threshold",
        "hidden Records Office door leaves no ghost threshold in Corridor"
    );

    Floppy144TestHiddenDoorGroundPair(
        FLOPPY144_ROOM_SECRETARY_OFFICE,
        FLOPPY144_ROOM_DIRECTOR_OFFICE,
        "Secretary / Director Office door fixture resolves",
        "reconstructed Secretary / Director door has a walkable threshold",
        "hidden Director Office door leaves no ghost threshold in Secretary Office"
    );
}

/*
 * Rendering, collision and passive labels share one runtime geometry contract.
 */
static void Floppy144TestRuntimeGeometryVisibility(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;

    const Floppy144SiteRect *pReceptionFloor = NULL;
    const Floppy144SiteRect *pMainDesk = NULL;
    const Floppy144SiteRect *pReceptionMainDoor = NULL;
    const Floppy144SiteRect *pReceptionExteriorDoor = NULL;
    const Floppy144SiteRect *pSuppressionPanel = NULL;

    uint32_t uRectIndex;

    Floppy144TestReset(
        &sWorld,
        &sState
    );

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);

        if(pRect == NULL)
        {
            continue;
        }

        if(
            pReceptionFloor == NULL &&
            pRect->room == (uint8_t)FLOPPY144_ROOM_RECEPTION &&
            pRect->type <= (uint8_t)FLOPPY144_SITE_FLOOR_D
        )
        {
            pReceptionFloor = pRect;
        }

        if(
            pMainDesk == NULL &&
            pRect->room == (uint8_t)FLOPPY144_ROOM_MAIN_OFFICE &&
            pRect->type == (uint8_t)FLOPPY144_SITE_STANDARD_DESK
        )
        {
            pMainDesk = pRect;
        }

        if(
            pReceptionMainDoor == NULL &&
            pRect->type == (uint8_t)FLOPPY144_SITE_DOOR &&
            (
                (
                    pRect->from_room == (uint8_t)FLOPPY144_ROOM_RECEPTION &&
                    pRect->to_room == (uint8_t)FLOPPY144_ROOM_MAIN_OFFICE
                ) ||
                (
                    pRect->to_room == (uint8_t)FLOPPY144_ROOM_RECEPTION &&
                    pRect->from_room == (uint8_t)FLOPPY144_ROOM_MAIN_OFFICE
                )
            )
        )
        {
            pReceptionMainDoor = pRect;
        }

        if(
            pReceptionExteriorDoor == NULL &&
            pRect->type == (uint8_t)FLOPPY144_SITE_DOOR &&
            (
                (
                    pRect->from_room == FLOPPY144_SITE_ROOM_OUTSIDE &&
                    pRect->to_room == (uint8_t)FLOPPY144_ROOM_RECEPTION
                ) ||
                (
                    pRect->to_room == FLOPPY144_SITE_ROOM_OUTSIDE &&
                    pRect->from_room == (uint8_t)FLOPPY144_ROOM_RECEPTION
                )
            )
        )
        {
            pReceptionExteriorDoor = pRect;
        }

        if(
            pSuppressionPanel == NULL &&
            pRect->room == (uint8_t)FLOPPY144_ROOM_MAIN_OFFICE &&
            pRect->type == (uint8_t)FLOPPY144_SITE_WALL_MOUNTED_ITEM &&
            pRect->x == 67U &&
            pRect->y == 90U
        )
        {
            pSuppressionPanel = pRect;
        }
    }

    F144_CHECK(
        pReceptionFloor != NULL &&
        pMainDesk != NULL &&
        pReceptionMainDoor != NULL &&
        pReceptionExteriorDoor != NULL &&
        pSuppressionPanel != NULL,
        "runtime visibility geometry fixtures resolve"
    );

    F144_CHECK(
        pReceptionFloor != NULL &&
        !Floppy144SiteRectRuntimeVisible(
            &sState,
            pReceptionFloor
        ),
        "unreconstructed Reception floor is absent"
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );

    F144_CHECK(
        pReceptionFloor != NULL &&
        Floppy144SiteRectRuntimeVisible(
            &sState,
            pReceptionFloor
        ),
        "Reception geometry appears after reconstruction"
    );

    F144_CHECK(
        pMainDesk != NULL &&
        !Floppy144SiteRectRuntimeVisible(
            &sState,
            pMainDesk
        ),
        "unreconstructed Main Office furniture remains absent"
    );

    F144_CHECK(
        pReceptionMainDoor != NULL &&
        !Floppy144SiteRectRuntimeVisible(
            &sState,
            pReceptionMainDoor
        ),
        "internal boundary stays absent until both endpoint rooms exist"
    );

    F144_CHECK(
        pReceptionExteriorDoor != NULL &&
        Floppy144SiteRectRuntimeVisible(
            &sState,
            pReceptionExteriorDoor
        ),
        "exterior boundary appears with its reconstructed interior room"
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    F144_CHECK(
        pMainDesk != NULL &&
        Floppy144SiteRectRuntimeVisible(
            &sState,
            pMainDesk
        ),
        "Main Office furniture appears after reconstruction"
    );

    F144_CHECK(
        pReceptionMainDoor != NULL &&
        Floppy144SiteRectRuntimeVisible(
            &sState,
            pReceptionMainDoor
        ),
        "internal boundary appears when both endpoint rooms exist"
    );

    F144_CHECK(
        pSuppressionPanel != NULL &&
        !Floppy144SiteRectRuntimeVisible(
            &sState,
            pSuppressionPanel
        ),
        "reconstructed room does not reveal progression-controlled fixture early"
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_FACILITIES
    );

    /*
     * T-006 belongs to FM-13. In the real archive path the collection first
     * becomes available because its trigger is ready, is then restored, and
     * only then can the player open the trigger record which reveals P-012.
     */
    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "FM-13"
        ),
        "FM-13 becomes restorable when Facilities makes T-006 actionable"
    );

    F144_CHECK(
        Floppy144TestFireTrigger(
            &sWorld,
            &sState,
            "T-006"
        ),
        "T-006 reveals suppression-panel fixture"
    );

    F144_CHECK(
        pSuppressionPanel != NULL &&
        Floppy144SiteRectRuntimeVisible(
            &sState,
            pSuppressionPanel
        ),
        "revealed fixture joins reconstructed room geometry"
    );
}

/*
 * Connection unlock_conditions are canonical progression data, not comments.
 * Verify direct trigger, OR-trigger and interaction conditions independently
 * of explicit UNLOCK_CONNECTION effects.
 */
static void Floppy144TestConnectionUnlockConditions(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;

    Floppy144TestReset(&sWorld, &sState);

    F144_CHECK(
        Floppy144GameDataConnectionUnlocked(&sState, "ENTRY_01"),
        "initially-unlocked exterior connection stays unlocked"
    );

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(&sState, "COR_FAC"),
        "COR_FAC starts locked before T-004"
    );

    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            Floppy144GameDataTriggerId("T-004")
        ) &&
        Floppy144GameDataConnectionUnlocked(&sState, "COR_FAC"),
        "single-trigger connection condition unlocks COR_FAC"
    );

    Floppy144TestReset(&sWorld, &sState);

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(&sState, "COR_RECO"),
        "OR-conditioned Records connection starts locked"
    );

    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            Floppy144GameDataTriggerId("T-028")
        ) &&
        Floppy144GameDataConnectionUnlocked(&sState, "COR_RECO"),
        "T-010_OR_T-028 condition accepts either trigger"
    );

    Floppy144TestReset(&sWorld, &sState);

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(&sState, "SECR_DIR"),
        "interaction-conditioned Director connection starts locked"
    );

    F144_CHECK(
        Floppy144RunStateCompleteInteraction(
            &sState,
            Floppy144GameDataInteractionId("I-036")
        ) &&
        Floppy144GameDataConnectionUnlocked(&sState, "SECR_DIR"),
        "interaction condition unlocks SECR_DIR"
    );

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(&sState, "EMER_01"),
        "locked connection with no unlock conditions remains locked"
    );
}


/*
 * The two DR-04 workstream records form the Act II branch choice.
 *
 * Both are legal before the choice. Firing one commits the branch and defers
 * the other until the authored T-028 RELEASE_ALTERNATE_WORKSTREAM effect.
 */
static void Floppy144TestActIiBranchChoiceGate(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144EvidenceId eBranchEvidence;
    Floppy144TriggerId eRecords;
    Floppy144TriggerId eTechnology;
    Floppy144TriggerId eActIiComplete;
    Floppy144TriggerId eReleaseAlternate;

    Floppy144TestReset(
        &sWorld,
        &sState
    );

    eBranchEvidence =
        Floppy144GameDataEvidenceId(
            "E-003"
        );

    eRecords =
        Floppy144GameDataTriggerId(
            "T-010"
        );

    eTechnology =
        Floppy144GameDataTriggerId(
            "T-011"
        );

    eActIiComplete =
        Floppy144GameDataTriggerId(
            "T-026"
        );

    eReleaseAlternate =
        Floppy144GameDataTriggerId(
            "T-028"
        );

    F144_CHECK(
        eBranchEvidence < FLOPPY144_EVIDENCE_COUNT &&
        eRecords < FLOPPY144_TRIGGER_COUNT &&
        eTechnology < FLOPPY144_TRIGGER_COUNT &&
        eActIiComplete < FLOPPY144_TRIGGER_COUNT &&
        eReleaseAlternate < FLOPPY144_TRIGGER_COUNT,
        "Act II branch fixture IDs resolve"
    );

    F144_CHECK(
        Floppy144RunStateEstablishEvidence(
            &sState,
            eBranchEvidence
        ),
        "E-003 establishes branch choice"
    );

    F144_CHECK(
        Floppy144TriggerCanFire(
            &sState,
            eRecords
        ) &&
        Floppy144TriggerCanFire(
            &sState,
            eTechnology
        ),
        "both DR-04 workstreams are selectable before branch commitment"
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            eRecords
        ),
        "Records-first workstream fires"
    );

    F144_CHECK(
        sState.branch ==
            (uint8_t)FLOPPY144_RUN_BRANCH_RECORDS_FIRST,
        "first workstream commits Records branch"
    );

    F144_CHECK(
        !Floppy144TriggerCanFire(
            &sState,
            eTechnology
        ),
        "unchosen Technology workstream is deferred during Act II"
    );

    /*
     * T-028 is authored to follow T-026. Mark the core-complete trigger as
     * already fired, then exercise the real release trigger/effect.
     */
    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            eActIiComplete
        ),
        "Act II core-complete trigger can be represented in fixture"
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            eReleaseAlternate
        ),
        "T-028 releases alternate workstream"
    );

    F144_CHECK(
        Floppy144TriggerCanFire(
            &sState,
            eTechnology
        ),
        "unchosen Technology workstream becomes available after Act II"
    );
}

/*
 * The Full Site spreadsheet is authoritative for furniture facing as well as
 * placement. The orientation migration previously moved the room coordinates
 * correctly while leaving back-edge rotations one quarter-turn behind.
 *
 * Reception provides a compact regression because its single desk chair and
 * waiting-room chair banks face opposite directions.
 */
static void Floppy144TestReceptionFurnitureFacing(void)
{
    uint32_t uRectIndex;
    bool bDeskChairFound = false;
    bool bWaitingChairFound = false;

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);

        if(
            pRect == NULL ||
            pRect->room != (uint8_t)FLOPPY144_ROOM_RECEPTION ||
            pRect->type != (uint8_t)FLOPPY144_SITE_CHAIR
        )
        {
            continue;
        }

        if(
            pRect->x == 90U &&
            pRect->y == 39U &&
            pRect->width == 2U &&
            pRect->height == 2U
        )
        {
            bDeskChairFound = true;

            F144_CHECK(
                pRect->rotation == 0U,
                "Reception desk chair keeps correct player-facing rotation"
            );
        }

        if(
            pRect->x == 94U &&
            pRect->y == 56U &&
            pRect->width == 2U &&
            pRect->height == 2U
        )
        {
            bWaitingChairFound = true;

            F144_CHECK(
                pRect->rotation == 180U,
                "Reception waiting chair keeps correct player-facing rotation"
            );
        }
    }

    F144_CHECK(
        bDeskChairFound,
        "Reception desk chair geometry exists"
    );

    F144_CHECK(
        bWaitingChairFound,
        "Reception waiting chair geometry exists"
    );
}


/*
 * Full Site is the dimensional authority for the late Stage 3B geometry
 * corrections which were previously special-cased during the orientation
 * migration.
 */
static void Floppy144TestFullSiteFurnitureGeometry(void)
{
    uint32_t uRectIndex;

    bool bStaffTable = false;
    bool bStaffChair135 = false;
    bool bStaffChair45 = false;
    bool bStaffChair225 = false;
    bool bStaffChair315 = false;

    bool bItDesk = false;
    bool bItBookcase = false;
    bool bItPatchPanel = false;
    bool bServerDesk = false;
    bool bSecurityDeskLeft = false;
    bool bSecurityDeskRight = false;

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(
                uRectIndex
            );

        if(pRect == NULL)
        {
            continue;
        }

        if(
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_STAFF_ROOM &&
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_TABLE &&
            pRect->x == 7U &&
            pRect->y == 34U &&
            pRect->width == 6U &&
            pRect->height == 6U
        )
        {
            bStaffTable = true;

            F144_CHECK(
                pRect->rotation == 45U,
                "Staff dining table retains 45-degree rotation"
            );
        }

        /*
         * The 6x6 table and 2x2 chairs are rendered as diamonds inside their
         * conservative bounds. A two-unit centre offset places each chair
         * visually against the table edge; the former three-unit offset left
         * a conspicuous ring of empty floor around the dining set.
         */
        if(
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_STAFF_ROOM &&
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_CHAIR
        )
        {
            if(pRect->x == 11U && pRect->y == 38U)
            {
                bStaffChair135 = true;
                F144_CHECK(
                    pRect->rotation == 135U,
                    "Staff dining 135-degree chair follows the table diagonal"
                );
            }
            else if(pRect->x == 7U && pRect->y == 38U)
            {
                bStaffChair45 = true;
                F144_CHECK(
                    pRect->rotation == 45U,
                    "Staff dining 45-degree chair follows the table diagonal"
                );
            }
            else if(pRect->x == 11U && pRect->y == 34U)
            {
                bStaffChair225 = true;
                F144_CHECK(
                    pRect->rotation == 225U,
                    "Staff dining 225-degree chair follows the table diagonal"
                );
            }
            else if(pRect->x == 7U && pRect->y == 34U)
            {
                bStaffChair315 = true;
                F144_CHECK(
                    pRect->rotation == 315U,
                    "Staff dining 315-degree chair follows the table diagonal"
                );
            }
        }

        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_STANDARD_DESK &&
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_IT_SUPPORT &&
            pRect->x == 50U &&
            pRect->y == 95U &&
            pRect->width == 6U &&
            pRect->height == 4U &&
            pRect->rotation == 90U
        )
        {
            bItDesk = true;
        }

        /*
         * Furniture and wall-hangings occupy independent visual Z-levels.
         * The IT Support bookcase therefore sits directly against the x=44
         * wall plane even though the patch panel is mounted on that same wall.
         */
        if(
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_IT_SUPPORT &&
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_BOOKCASE &&
            pRect->x == 44U &&
            pRect->y == 79U &&
            pRect->width == 2U &&
            pRect->height == 6U
        )
        {
            bItBookcase = true;
        }

        if(
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_IT_SUPPORT &&
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_WALL_MOUNTED_ITEM &&
            pRect->x == 44U &&
            pRect->y == 79U &&
            pRect->width == 1U &&
            pRect->height == 8U
        )
        {
            bItPatchPanel = true;
        }

        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_STANDARD_DESK &&
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_SERVER_ROOM &&
            pRect->x == 35U &&
            pRect->y == 59U &&
            pRect->width == 6U &&
            pRect->height == 2U &&
            pRect->rotation == 90U
        )
        {
            bServerDesk = true;
        }

        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_STANDARD_DESK &&
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_SECURITY &&
            pRect->y == 59U &&
            pRect->width == 6U &&
            pRect->height == 4U &&
            pRect->rotation == 90U
        )
        {
            if(pRect->x == 44U)
            {
                bSecurityDeskLeft = true;
            }
            else if(pRect->x == 50U)
            {
                bSecurityDeskRight = true;
            }
        }
    }

    F144_CHECK(
        bStaffTable &&
        bStaffChair135 &&
        bStaffChair45 &&
        bStaffChair225 &&
        bStaffChair315,
        "Staff dining chairs sit immediately adjacent to the 45-degree table"
    );

    F144_CHECK(
        bItDesk,
        "IT Support standard desk uses locked-plan 6x4 footprint"
    );

    F144_CHECK(
        bItBookcase &&
        bItPatchPanel,
        "IT Support furniture and wall-hanging share the wall plane on separate Z-levels"
    );

    F144_CHECK(
        bServerDesk,
        "Server Room standard desk uses Full Site 6x2 footprint"
    );

    F144_CHECK(
        bSecurityDeskLeft &&
        bSecurityDeskRight,
        "Security Office keeps two adjacent locked-plan 6x4 standard desks"
    );
}


/*
 * Both authored Site Directory fixtures must expose the same Inspect action.
 * Reception used to be inert because only the Corridor directory owned a
 * contextual physical child.
 */
static void Floppy144TestSiteDirectoryActions(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );

    sState.player_site_x =
        79 * FLOPPY144_SITE_FIXED_ONE;
    sState.player_site_y =
        43 * FLOPPY144_SITE_FIXED_ONE;

    F144_CHECK(
        Floppy144SiteDirectoryNearby(&sState) &&
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_INSPECT
        ) != 0U,
        "Reception Site Directory exposes the restored-room map action"
    );

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    sState.player_site_x =
        38 * FLOPPY144_SITE_FIXED_ONE;
    sState.player_site_y =
        54 * FLOPPY144_SITE_FIXED_ONE;

    F144_CHECK(
        Floppy144SiteDirectoryNearby(&sState) &&
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_INSPECT
        ) != 0U,
        "Corridor Site Directory exposes the restored-room map action"
    );
}

/*
 * Staff Room orientation regression.
 *
 * The room is not horizontally mirrored. Keep the original furniture side,
 * exterior window bank and Corridor door while preserving later footprint
 * corrections and the compact 45-degree dining cluster.
 */
static void Floppy144TestStaffRoomSpreadsheetGeometry(void)
{
    uint32_t uRectIndex;
    uint32_t uCompactSofas = 0U;
    uint32_t uExteriorWindowSegments = 0U;
    bool bUtilityChair = false;
    bool bNoticeboard = false;
    bool bFridge = false;
    bool bWorktop = false;
    bool bSink = false;
    bool bCoffeeMaker = false;
    bool bStaffDoor = false;
    bool bStaffDoorBridgesRooms = false;

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);

        if(pRect == NULL)
        {
            continue;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_DOOR &&
            pRect->x == 18U &&
            pRect->y == 46U &&
            pRect->width == 5U &&
            pRect->height == 1U &&
            (
                (
                    pRect->from_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
                    pRect->to_room == (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
                ) ||
                (
                    pRect->to_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
                    pRect->from_room == (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
                )
            )
        )
        {
            uint8_t uDoorCentreX =
                (uint8_t)(
                    pRect->x +
                    pRect->width / 2U
                );

            bStaffDoor = true;
            bStaffDoorBridgesRooms =
                pRect->y > 0U &&
                (
                    (uint16_t)pRect->y +
                    (uint16_t)pRect->height
                ) < FLOPPY144_SITE_SIZE_UNITS &&
                Floppy144SiteRoomAtCell(
                    uDoorCentreX,
                    (uint8_t)(pRect->y - 1U)
                ) == FLOPPY144_ROOM_STAFF_ROOM &&
                Floppy144SiteRoomAtCell(
                    uDoorCentreX,
                    (uint8_t)(pRect->y + pRect->height)
                ) == FLOPPY144_ROOM_CORRIDOR;
        }

        if(
            pRect->room != (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
        )
        {
            continue;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_CHAIR &&
            pRect->x == 22U &&
            pRect->y == 10U &&
            pRect->width == 2U &&
            pRect->height == 2U &&
            pRect->rotation == 90U
        )
        {
            bUtilityChair = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_WALL_MOUNTED_ITEM &&
            pRect->x == 4U &&
            pRect->y == 45U &&
            pRect->width == 10U &&
            pRect->height == 1U
        )
        {
            bNoticeboard = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_FRIDGE &&
            pRect->x == 19U &&
            pRect->y == 1U &&
            pRect->width == 5U &&
            pRect->height == 5U
        )
        {
            bFridge = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_WORKTOP &&
            pRect->x == 1U &&
            pRect->y == 1U &&
            pRect->width == 18U &&
            pRect->height == 5U
        )
        {
            bWorktop = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_SINK &&
            pRect->x == 14U &&
            pRect->y == 2U &&
            pRect->width == 4U &&
            pRect->height == 3U
        )
        {
            bSink = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_COFFEE_MAKER &&
            pRect->x == 8U &&
            pRect->y == 2U &&
            pRect->width == 4U &&
            pRect->height == 2U
        )
        {
            bCoffeeMaker = true;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_SOFA &&
            pRect->width == 3U &&
            pRect->height == 6U &&
            (
                (pRect->x == 21U && pRect->y == 13U) ||
                (pRect->x == 21U && pRect->y == 23U) ||
                (pRect->x == 1U && pRect->y == 13U) ||
                (pRect->x == 1U && pRect->y == 23U)
            )
        )
        {
            ++uCompactSofas;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_WINDOW &&
            pRect->x == 0U &&
            pRect->width == 1U &&
            pRect->height == 9U &&
            (
                pRect->y == 35U ||
                pRect->y == 24U ||
                pRect->y == 13U ||
                pRect->y == 2U
            )
        )
        {
            ++uExteriorWindowSegments;

            F144_CHECK(
                (
                    pRect->from_room ==
                        (uint8_t)FLOPPY144_SITE_ROOM_OUTSIDE &&
                    pRect->to_room ==
                        (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
                ) ||
                (
                    pRect->to_room ==
                        (uint8_t)FLOPPY144_SITE_ROOM_OUTSIDE &&
                    pRect->from_room ==
                        (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
                ),
                "Staff Room left-wall windows remain external after unmirroring"
            );
        }
    }

    F144_CHECK(
        bUtilityChair &&
        bNoticeboard &&
        bFridge &&
        bWorktop &&
        bSink &&
        bCoffeeMaker,
        "Staff Room utility furniture remains in the unmirrored layout"
    );

    F144_CHECK(
        uCompactSofas == 4U,
        "Staff Room sofa footprints retain the compact size correction"
    );

    F144_CHECK(
        uExteriorWindowSegments == 4U,
        "Staff Room four-window bank remains on the unmirrored exterior wall"
    );

    F144_CHECK(
        bStaffDoor,
        "Staff Room Corridor door remains at the original x=18 position"
    );

    F144_CHECK(
        bNoticeboard && bStaffDoor,
        "Staff Room noticeboard remains clear of the Corridor doorway"
    );

    F144_CHECK(
        bStaffDoorBridgesRooms,
        "Staff Room Corridor door actually bridges the two authored floor areas"
    );
}

/*
 * Records Office furniture must begin inside the floor, not on the one-unit
 * wall cell immediately to its left. The narrow Records Office leg has floor
 * beginning at x=57, so x=56 is wall space and must remain clear of furniture.
 */
static void Floppy144TestRecordsOfficeLeftWallClear(void)
{
    uint32_t uRectIndex;
    bool bDesk04Found = false;
    bool bTopLeftChairCentred = false;
    bool bRecordsDoorAligned = false;
    bool bRecordsMainFloorFound = false;
    bool bRecordsNarrowFloorFound = false;
    uint32_t uLeftCabinetsFound = 0U;
    uint32_t uRightWallCabinetsFound = 0U;

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);

        if(pRect == NULL)
        {
            continue;
        }

        /*
         * The corrected narrow floor spans x=56..65. COR_RECO therefore
         * starts at x=58; x=59 was the stale pre-floor-shift position.
         */
        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_DOOR &&
            pRect->x == 58U &&
            pRect->y == 46U &&
            pRect->width == 5U &&
            pRect->height == 1U &&
            (
                (
                    pRect->from_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
                    pRect->to_room == (uint8_t)FLOPPY144_ROOM_RECORDS_OFFICE
                ) ||
                (
                    pRect->to_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
                    pRect->from_room == (uint8_t)FLOPPY144_ROOM_RECORDS_OFFICE
                )
            )
        )
        {
            bRecordsDoorAligned = true;
        }

        if(
            pRect->room !=
                (uint8_t)FLOPPY144_ROOM_RECORDS_OFFICE
        )
        {
            continue;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_FLOOR_C &&
            pRect->x == 66U &&
            pRect->y == 1U &&
            pRect->width == 33U &&
            pRect->height == 32U
        )
        {
            bRecordsMainFloorFound = true;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_FLOOR_C &&
            pRect->x == 56U &&
            pRect->y == 1U &&
            pRect->width == 10U &&
            pRect->height == 45U
        )
        {
            bRecordsNarrowFloorFound = true;
        }

        /*
         * These placements are checked relative to the corrected floor,
         * rather than preserving the stale coordinates that pre-dated the
         * 27 September one-unit floor shift.
         */
        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_STANDARD_DESK &&
            pRect->x == 56U &&
            pRect->y == 1U &&
            pRect->width == 6U &&
            pRect->height == 4U
        )
        {
            bDesk04Found = true;
        }

        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_SECURE_CABINET_FULL &&
            pRect->x == 56U &&
            pRect->width == 2U &&
            pRect->height == 6U &&
            (
                pRect->y == 12U ||
                pRect->y == 19U ||
                pRect->y == 26U ||
                pRect->y == 33U ||
                pRect->y == 40U
            )
        )
        {
            ++uLeftCabinetsFound;
        }

        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_SECURE_CABINET_FULL &&
            pRect->x == 64U &&
            pRect->width == 2U &&
            pRect->height == 6U &&
            (
                pRect->y == 33U ||
                pRect->y == 40U
            )
        )
        {
            ++uRightWallCabinetsFound;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_CHAIR &&
            pRect->x == 58U &&
            pRect->y == 5U &&
            pRect->width == 2U &&
            pRect->height == 2U &&
            pRect->rotation == 180U
        )
        {
            bTopLeftChairCentred = true;
        }
    }

    F144_CHECK(
        bRecordsMainFloorFound,
        "Records Office main floor keeps corrected 33-column footprint"
    );

    F144_CHECK(
        bRecordsNarrowFloorFound,
        "Records Office narrow floor leg keeps corrected x=56 footprint"
    );

    F144_CHECK(
        bDesk04Found,
        "Records Office Desk 04 stays flush with corrected narrow-floor edge"
    );

    F144_CHECK(
        uLeftCabinetsFound == 5U,
        "Records Office left cabinet bank stays flush with corrected floor edge"
    );

    F144_CHECK(
        uRightWallCabinetsFound == 2U,
        "Records Office door-side cabinets stay inside the x=66 wall"
    );

    F144_CHECK(
        bTopLeftChairCentred,
        "Records Office top-left chair stays centred on relocated Desk 04"
    );

    F144_CHECK(
        bRecordsDoorAligned,
        "Records Office corridor door follows corrected narrow-floor position"
    );
}

/* Room reconstruction bits are part of the versioned save payload. */
static void Floppy144TestReconstructionPersistence(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144RunState sLoadedState;

    uint8_t auPayload[FLOPPY144_SAVE_PAYLOAD_V1_SIZE];
    uint32_t uRoomIndex;

    Floppy144TestReset(
        &sWorld,
        &sState
    );

    for(
        uRoomIndex = 0U;
        uRoomIndex < (uint32_t)FLOPPY144_ROOM_COUNT;
        ++uRoomIndex
    )
    {
        (void)Floppy144RunStateReconstructRoom(
            &sState,
            (Floppy144RoomId)uRoomIndex
        );
    }

    F144_CHECK(
        Floppy144PersistenceEncodeRunState(
            &sState,
            auPayload,
            (uint32_t)sizeof(auPayload)
        ),
        "reconstructed Site encodes into versioned save payload"
    );

    Floppy144RunStateReset(
        &sLoadedState
    );

    F144_CHECK(
        Floppy144PersistenceDecodeRunState(
            &sLoadedState,
            auPayload,
            (uint32_t)sizeof(auPayload)
        ),
        "versioned save payload decodes reconstructed Site"
    );

    for(
        uRoomIndex = 0U;
        uRoomIndex < (uint32_t)FLOPPY144_ROOM_COUNT;
        ++uRoomIndex
    )
    {
        F144_CHECK(
            Floppy144RunStateRoomReconstructed(
                &sLoadedState,
                (Floppy144RoomId)uRoomIndex
            ),
            "reconstructed room survives persistence round-trip"
        );
    }
}

int main(void)
{
    Floppy144TestReconstructionSourceCoverage();
    Floppy144TestGenericReconstructionVerb();
    Floppy144TestProgressionAvailability();
    Floppy144TestCanonicalRoomProgression();
    Floppy144TestWallHangingCollisionContract();
    Floppy144TestRuntimeGeometryVisibility();
    Floppy144TestHiddenDoorsDoNotCreateGhostThresholds();
    Floppy144TestConnectionUnlockConditions();
    Floppy144TestActIiBranchChoiceGate();
    Floppy144TestReceptionFurnitureFacing();
    Floppy144TestFullSiteFurnitureGeometry();
    Floppy144TestSiteDirectoryActions();
    Floppy144TestStaffRoomSpreadsheetGeometry();
    Floppy144TestRecordsOfficeLeftWallClear();
    Floppy144TestReconstructionPersistence();

    if(g_nFailures != 0)
    {
        fprintf(
            stderr,
            "\nSTAGE 3B.3 RECONSTRUCTION TESTS: FAIL (%d failure%s)\n",
            g_nFailures,
            g_nFailures == 1 ? "" : "s"
        );

        return 1;
    }

    puts("\nSTAGE 3B.3 RECONSTRUCTION TESTS: PASS");
    return 0;
}
