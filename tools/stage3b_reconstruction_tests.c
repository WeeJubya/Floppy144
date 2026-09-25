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
        Floppy144GameDataConditionSatisfied(
            &sState,
            "act_at_least",
            "ACT_II_CORE_COMPLETE"
        ),
        "completed branch ledger satisfies late reconstruction gate"
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
    bool bStaffChairNorth = false;
    bool bStaffChairWest = false;
    bool bStaffChairEast = false;
    bool bStaffChairSouth = false;

    bool bItDesk = false;
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
            pRect->x == 12U &&
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

        if(
            pRect->room ==
                (uint8_t)FLOPPY144_ROOM_STAFF_ROOM &&
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_CHAIR
        )
        {
            if(pRect->x == 18U && pRect->y == 36U)
            {
                bStaffChairNorth = true;
                F144_CHECK(
                    pRect->rotation == 135U,
                    "Staff dining north chair uses border facing plus 45 degrees"
                );
            }
            else if(pRect->x == 14U && pRect->y == 32U)
            {
                bStaffChairWest = true;
                F144_CHECK(
                    pRect->rotation == 45U,
                    "Staff dining west chair uses border facing plus 45 degrees"
                );
            }
            else if(pRect->x == 14U && pRect->y == 40U)
            {
                bStaffChairEast = true;
                F144_CHECK(
                    pRect->rotation == 225U,
                    "Staff dining east chair uses border facing plus 45 degrees"
                );
            }
            else if(pRect->x == 10U && pRect->y == 36U)
            {
                bStaffChairSouth = true;
                F144_CHECK(
                    pRect->rotation == 315U,
                    "Staff dining south chair uses border facing plus 45 degrees"
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
            pRect->height == 2U &&
            pRect->rotation == 90U
        )
        {
            bItDesk = true;
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
            pRect->height == 2U &&
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
        bStaffChairNorth &&
        bStaffChairWest &&
        bStaffChairEast &&
        bStaffChairSouth,
        "Staff dining table/chair geometry is emitted from Full Site"
    );

    F144_CHECK(
        bItDesk,
        "IT Support standard desk uses Full Site 6x2 footprint"
    );

    F144_CHECK(
        bServerDesk,
        "Server Room standard desk uses Full Site 6x2 footprint"
    );

    F144_CHECK(
        bSecurityDeskLeft &&
        bSecurityDeskRight,
        "Security Office keeps two adjacent 6x2 standard desks"
    );
}

/*
 * Locked Staff Room spreadsheet regression.
 *
 * The dining cluster was only one visible symptom of a stale room import.
 * Keep the rest of the final spreadsheet geometry under test too: utility
 * fixtures, sofas, the moved exterior window bank and the Corridor door.
 */
static void Floppy144TestStaffRoomSpreadsheetGeometry(void)
{
    uint32_t uRectIndex;
    uint32_t uCompactSofas = 0U;
    uint32_t uRightWallWindows = 0U;
    bool bUtilityChair = false;
    bool bNoticeboard = false;
    bool bFridge = false;
    bool bWorktop = false;
    bool bSink = false;
    bool bCoffeeMaker = false;
    bool bStaffDoor = false;

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
            pRect->x == 2U &&
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
            bStaffDoor = true;
        }

        if(
            pRect->room != (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
        )
        {
            continue;
        }

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_CHAIR &&
            pRect->x == 1U &&
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
            pRect->x == 11U &&
            pRect->y == 45U &&
            pRect->width == 10U &&
            pRect->height == 1U
        )
        {
            bNoticeboard = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_FRIDGE &&
            pRect->x == 1U &&
            pRect->y == 1U &&
            pRect->width == 5U &&
            pRect->height == 5U
        )
        {
            bFridge = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_WORKTOP &&
            pRect->x == 6U &&
            pRect->y == 1U &&
            pRect->width == 18U &&
            pRect->height == 5U
        )
        {
            bWorktop = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_SINK &&
            pRect->x == 7U &&
            pRect->y == 2U &&
            pRect->width == 4U &&
            pRect->height == 3U
        )
        {
            bSink = true;
        }
        else if(
            pRect->type == (uint8_t)FLOPPY144_SITE_COFFEE_MAKER &&
            pRect->x == 13U &&
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
            pRect->x == 24U &&
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
            ++uRightWallWindows;
        }
    }

    F144_CHECK(
        bUtilityChair &&
        bNoticeboard &&
        bFridge &&
        bWorktop &&
        bSink &&
        bCoffeeMaker,
        "Staff Room utility furniture matches locked spreadsheet"
    );

    F144_CHECK(
        uCompactSofas == 4U,
        "Staff Room sofa footprints match locked spreadsheet"
    );

    F144_CHECK(
        uRightWallWindows == 4U,
        "Staff Room four-window bank is on spreadsheet right-hand wall"
    );

    F144_CHECK(
        bStaffDoor,
        "Staff Room Corridor door matches locked spreadsheet position"
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
    bool bRecordsDoorCentred = false;
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

        if(
            pRect->type == (uint8_t)FLOPPY144_SITE_DOOR &&
            pRect->x == 59U &&
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
            bRecordsDoorCentred = true;
        }

        if(
            pRect->room !=
                (uint8_t)FLOPPY144_ROOM_RECORDS_OFFICE
        )
        {
            continue;
        }

        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_STANDARD_DESK ||
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_SECURE_CABINET_FULL
        )
        {
            F144_CHECK(
                pRect->x != 56U,
                "Records Office furniture does not occupy left wall cell x=56"
            );
        }

        if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_STANDARD_DESK &&
            pRect->x == 57U &&
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
            pRect->x == 57U &&
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
            pRect->x == 65U &&
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
            pRect->x == 59U &&
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
        bDesk04Found,
        "Records Office Desk 04 is shifted one unit inside left wall"
    );

    F144_CHECK(
        uLeftCabinetsFound == 5U,
        "Records Office left cabinet bank is shifted one unit inside wall"
    );

    F144_CHECK(
        uRightWallCabinetsFound == 2U,
        "Records Office door-side cabinets sit flush against right-hand wall"
    );

    F144_CHECK(
        bTopLeftChairCentred,
        "Records Office top-left desk chair is centred on Desk 04"
    );

    F144_CHECK(
        bRecordsDoorCentred,
        "Records Office corridor door is centred on the narrow room leg"
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
    Floppy144TestRuntimeGeometryVisibility();
    Floppy144TestConnectionUnlockConditions();
    Floppy144TestActIiBranchChoiceGate();
    Floppy144TestReceptionFurnitureFacing();
    Floppy144TestFullSiteFurnitureGeometry();
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
