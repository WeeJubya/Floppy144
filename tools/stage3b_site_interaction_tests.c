/*
 * Floppy//144 Stage 3B.2 physical Site interaction regression.
 *
 * Proves that Site targeting is derived from generated furniture, fixtures,
 * physical items and interactions rather than a story-specific position table.
 */

#include "floppy144_game_data.h"
#include "floppy144_interaction_engine.h"
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

static void Floppy144TestSetPosition(
    Floppy144RunState *pState,
    int32_t nX,
    int32_t nY
)
{
    Floppy144RunStateSetPlayerSitePosition(
        pState,
        nX * FLOPPY144_SITE_FIXED_ONE,
        nY * FLOPPY144_SITE_FIXED_ONE
    );
}

static void Floppy144TestReset(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState
)
{
    Floppy144WorldReset(pWorld);
    Floppy144RunStateBegin(pState, 144U);
}

/*
 * Every canonical physical item must terminate at a generated Site parent.
 * This is the data contract which replaces the small hand-authored location
 * table used by the technical slice.
 */
static void Floppy144TestPhysicalParentCoverage(void)
{
    uint32_t uRecordIndex;
    uint32_t uPhysicalCount = 0U;
    uint32_t uPhysicalInteractionCount = 0U;

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        const Floppy144DataRecord *pParent;

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_PHYSICAL_ITEM
        )
        {
            continue;
        }

        ++uPhysicalCount;

        pParent =
            Floppy144GameDataFind(
                FLOPPY144_DATA_FURNITURE,
                pRecord->pszC
            );

        if(pParent == NULL)
        {
            pParent =
                Floppy144GameDataFind(
                    FLOPPY144_DATA_FIXTURE,
                    pRecord->pszC
                );
        }

        F144_CHECK(
            pParent != NULL,
            "physical item resolves to furniture/fixture parent"
        );

        if(pParent != NULL)
        {
            Floppy144RoomId eItemRoom =
                Floppy144GameDataRoomId(pRecord->pszB);

            Floppy144RoomId eParentRoom =
                Floppy144GameDataRoomId(pParent->pszA);

            bool bBoundaryFixture =
                pParent->eKind == FLOPPY144_DATA_FIXTURE &&
                pParent->pszB != NULL &&
                (
                    strcmp(pParent->pszB, "DOOR") == 0 ||
                    strcmp(pParent->pszB, "WINDOW") == 0
                );

            /*
             * Ordinary furniture/fixtures must live in the same room as their
             * physical child. Boundary fixtures are different: the generated
             * fixture has one owning room while a notice/item attached to that
             * boundary may legitimately belong to the room on the other side.
             * COR_OFF / P-011 is the current canonical example.
             */
            F144_CHECK(
                eItemRoom == eParentRoom || bBoundaryFixture,
                "physical item parent has compatible room ownership"
            );
        }

        if(
            Floppy144InteractionForPhysicalSource(
                pRecord->pszId
            ) != FLOPPY144_INTERACTION_COUNT
        )
        {
            ++uPhysicalInteractionCount;
        }
    }

    F144_CHECK(
        uPhysicalCount == 161U,
        "all 161 physical items participate in Site parent audit"
    );

    F144_CHECK(
        uPhysicalInteractionCount == 34U,
        "34 physical items own canonical gameplay interactions"
    );
}

/*
 * The runtime geometry emitter must preserve centre-authored furniture too.
 */
static void Floppy144TestCentredGeometryMetadata(void)
{
    const Floppy144DataRecord *pDirectorDesk =
        Floppy144GameDataFind(
            FLOPPY144_DATA_FURNITURE,
            "DIRECTOR_OFFICE_DESK"
        );

    const Floppy144DataRecord *pSecretaryDesk =
        Floppy144GameDataFind(
            FLOPPY144_DATA_FURNITURE,
            "SECRETARY_OFFICE_DESK"
        );

    F144_CHECK(
        pDirectorDesk != NULL &&
        pDirectorDesk->b0 != 0U &&
        pDirectorDesk->n4 == 48 &&
        pDirectorDesk->n5 == 9,
        "Director desk retains centre-authored interaction geometry"
    );

    F144_CHECK(
        pSecretaryDesk != NULL &&
        pSecretaryDesk->b0 != 0U &&
        pSecretaryDesk->n4 == 49 &&
        pSecretaryDesk->n5 == 40,
        "Secretary desk retains centre-authored interaction geometry"
    );
}

/*
 * Main Office desks must not expose the two Stage 3A content inconsistencies.
 * Before T-003, ordinary contextual material may already be visible, but the
 * staffing items explicitly revealed by T-003 must remain hidden. After T-003
 * Desk 01 exposes its real label material and Desk 02 exposes the allocation slip.
 */
static void Floppy144TestMainOfficeDeskContent(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144SiteInspectionTarget sTarget;
    Floppy144CollectionId eDr01;
    Floppy144CollectionId eHr01;

    Floppy144TestReset(&sWorld, &sState);

    eDr01 =
        Floppy144GameDataCollectionId("DR-01");

    eHr01 =
        Floppy144GameDataCollectionId("HR-01");

    F144_CHECK(
        eDr01 < FLOPPY144_COLLECTION_COUNT &&
        eHr01 < FLOPPY144_COLLECTION_COUNT,
        "DR-01 and HR-01 resolve for desk fixture"
    );

    /*
     * T-002 is an HR-01 document in the real player route. Restore both
     * collections in the fixture so ordinary HR-01 context such as P-124 is
     * legitimately present while the T-003 staffing reveals remain hidden.
     */
    F144_CHECK(
        Floppy144RunStateBitSet(
            sState.collections,
            (uint32_t)eDr01
        ) &&
        Floppy144RunStateBitSet(
            sState.collections,
            (uint32_t)eHr01
        ),
        "desk fixture restores DR-01 and HR-01"
    );

    (void)Floppy144WorldRestoreCollection(&sWorld, eDr01);
    (void)Floppy144WorldRestoreCollection(&sWorld, eHr01);

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-002")
        ),
        "T-002 reconstructs Main Office"
    );

    Floppy144TestSetPosition(&sState, 89, 90);

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ),
        "Desk 02 parent is inspectable before staffing data appears"
    );

    F144_CHECK(
        sTarget.pszParentId != NULL &&
        strcmp(
            sTarget.pszParentId,
            "MAIN_OFFICE_DESK_02"
        ) == 0 &&
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(
            sTarget.pszPhysicalItemId,
            "P-124"
        ) == 0 &&
        sTarget.pszPhysicalItemName != NULL &&
        strcmp(
            sTarget.pszPhysicalItemName,
            "Course feedback forms"
        ) == 0,
        "Desk 02 exposes safe context but not T-003 staffing material"
    );

    F144_CHECK(
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(sTarget.pszPhysicalItemId, "P-009") != 0 &&
        strcmp(sTarget.pszPhysicalItemId, "P-010") != 0,
        "Desk 02 does not leak T-003 physical items early"
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-003")
        ),
        "T-003 reveals Main Office staffing material"
    );

    Floppy144TestSetPosition(&sState, 89, 79);

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(sTarget.pszPhysicalItemId, "P-008") == 0 &&
        sTarget.pszPhysicalItemName != NULL &&
        strcmp(
            sTarget.pszPhysicalItemName,
            "Main Office desk-number labels"
        ) == 0,
        "Desk 01 resolves canonical label material rather than a mug"
    );

    Floppy144TestSetPosition(&sState, 89, 90);

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(sTarget.pszPhysicalItemId, "P-009") == 0 &&
        sTarget.pszPhysicalItemName != NULL &&
        strcmp(
            sTarget.pszPhysicalItemName,
            "Temporary desk allocation slip"
        ) == 0,
        "Desk 02 resolves staffing material without premature cable text"
    );
}

/*
 * Every canonical terminal desk must become an Access target from its own
 * reconstructed room. This covers all nine Site terminals with one rule.
 */
static void Floppy144TestTerminalCoverage(void)
{
    uint32_t uRecordIndex;
    uint32_t uTerminalCount = 0U;

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
        Floppy144RoomId eResolvedRoom;

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_FURNITURE ||
            pRecord->pszB == NULL ||
            strcmp(pRecord->pszB, "TERMINAL_DESK") != 0
        )
        {
            continue;
        }

        ++uTerminalCount;

        eRoom =
            Floppy144GameDataRoomId(
                pRecord->pszA
            );

        F144_CHECK(
            eRoom < FLOPPY144_ROOM_COUNT,
            "terminal furniture resolves owning room"
        );

        Floppy144TestReset(&sWorld, &sState);
        (void)Floppy144RunStateReconstructRoom(&sState, eRoom);

        Floppy144TestSetPosition(
            &sState,
            pRecord->n0 + pRecord->n2 / 2,
            pRecord->n1 + pRecord->n3 / 2
        );

        F144_CHECK(
            Floppy144SiteAccessTerminalRoom(
                &sState,
                &eResolvedRoom
            ) &&
            eResolvedRoom == eRoom,
            "reconstructed terminal desk resolves through generic access"
        );

        F144_CHECK(
            (
                Floppy144SiteAvailableActions(&sState) &
                FLOPPY144_SITE_ACTION_ACCESS
            ) != 0U,
            "terminal advertises Access action"
        );
    }

    F144_CHECK(
        uTerminalCount == 9U,
        "all nine terminal desks audited"
    );
}

/*
 * A data-defined physical interaction must run through the generic engine,
 * establish its evidence and remain completed when targeted again.
 */
static void Floppy144TestPhysicalInteractionExecution(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144SiteInspectionTarget sTarget;
    Floppy144InteractionId eInteraction;
    Floppy144EvidenceId eEvidence;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_FACILITIES
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-006")
        ),
        "T-006 reveals suppression-panel physical data"
    );

    Floppy144TestSetPosition(&sState, 69, 92);

    eInteraction =
        Floppy144GameDataInteractionId("I-001");

    eEvidence =
        Floppy144GameDataEvidenceId("E-001");

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(sTarget.pszPhysicalItemId, "P-012") == 0 &&
        sTarget.eInteraction == eInteraction &&
        sTarget.bInteractionAvailable,
        "P-012 resolves to runnable I-001"
    );

    F144_CHECK(
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            sTarget.eInteraction
        ),
        "generic Site inspection runs I-001"
    );

    F144_CHECK(
        Floppy144RunStateInteractionCompleted(
            &sState,
            eInteraction
        ),
        "I-001 persists as completed"
    );

    F144_CHECK(
        Floppy144RunStateEvidenceEstablished(
            &sState,
            eEvidence
        ),
        "I-001 resolves E-001 through generic evidence synthesis"
    );

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.eInteraction == eInteraction &&
        sTarget.bInteractionCompleted &&
        !sTarget.bInteractionAvailable,
        "completed physical interaction remains readable but not repeatable"
    );
}

/*
 * Interaction effects can reveal later physical items on entirely different
 * furniture. P-033 -> I-002 -> P-014 is the compact Stage 3B.2 proof.
 */
static void Floppy144TestFollowOnReveal(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144SiteInspectionTarget sTarget;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_FACILITIES
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-008")
        ),
        "T-008 prepares Facilities closure material"
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-009")
        ),
        "T-009 reveals P-033"
    );

    Floppy144TestSetPosition(&sState, 14, 54);

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(sTarget.pszPhysicalItemId, "P-033") == 0 &&
        sTarget.bInteractionAvailable,
        "Facilities shelf resolves Final Isolation Register"
    );

    F144_CHECK(
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            sTarget.eInteraction
        ),
        "I-002 runs through generic physical interaction"
    );

    F144_CHECK(
        Floppy144GameDataPhysicalItemRevealed(
            &sState,
            "P-014"
        ),
        "I-002 persistently reveals P-014"
    );

    Floppy144TestSetPosition(&sState, 75, 90);

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(sTarget.pszPhysicalItemId, "P-014") == 0 &&
        sTarget.eInteraction ==
            Floppy144GameDataInteractionId("I-003") &&
        sTarget.bInteractionAvailable,
        "revealed handover folder becomes generic Main Office target"
    );
}

/*
 * Centre-authored rotated furniture must be targetable without hard-coded
 * Director/Secretary coordinates.
 */
static void Floppy144TestRotatedParentTargeting(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144SiteInspectionTarget sTarget;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_DIRECTOR_OFFICE
    );

    Floppy144TestSetPosition(&sState, 48, 9);

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.pszParentId != NULL &&
        strcmp(
            sTarget.pszParentId,
            "DIRECTOR_OFFICE_DESK"
        ) == 0,
        "centre-authored Director desk participates in generic targeting"
    );
}

/* Pure visual chairs should not make the interaction prompt noisy. */
static void Floppy144TestSceneryDoesNotBecomeInteraction(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );

    Floppy144TestSetPosition(&sState, 80, 61);

    F144_CHECK(
        Floppy144SiteAvailableActions(&sState) == 0U,
        "pure waiting-chair scenery does not advertise an action"
    );
}


/*
 * Site inspection reach is measured from the player's collision footprint.
 * Four half-unit movement steps away from the Main Office desks is too far;
 * moving to within one Site unit enables inspection.
 */
static void Floppy144TestInspectionRange(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144SiteInspectionTarget sTarget;

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    /*
     * In the JSON-oriented Site the player footprint is centred on X and
     * extends upward from the foot point. Approach each desk from its left so
     * the range test is independent of the neighbouring paired desk.
     */
    Floppy144TestSetPosition(&sState, 69, 80);
    F144_CHECK(
        !Floppy144SiteResolveInspectionTarget(&sState, &sTarget),
        "Desk 05 does not trigger inspection from outside one-unit reach"
    );

    Floppy144TestSetPosition(&sState, 70, 80);
    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(&sState, &sTarget) &&
        sTarget.pszParentId != NULL &&
        strcmp(sTarget.pszParentId, "MAIN_OFFICE_DESK_05") == 0,
        "Desk 05 becomes inspectable only when adjacent"
    );

    Floppy144TestSetPosition(&sState, 69, 86);
    F144_CHECK(
        !Floppy144SiteResolveInspectionTarget(&sState, &sTarget),
        "Desk 06 does not trigger inspection from outside one-unit reach"
    );

    Floppy144TestSetPosition(&sState, 70, 86);
    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(&sState, &sTarget) &&
        sTarget.pszParentId != NULL &&
        strcmp(sTarget.pszParentId, "MAIN_OFFICE_DESK_06") == 0,
        "Desk 06 becomes inspectable only when adjacent"
    );
}

/*
 * A progression-hidden Site fixture must not become an invisible wall. The
 * suppression panel is the compact regression case: it exists in compiled
 * geometry but is not visible at the start of a run.
 */
static void Floppy144TestHiddenGeometryDoesNotCollide(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    uint32_t uRectIndex;
    const Floppy144SiteRect *pPanelRect = NULL;

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

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
            pRect->type == (uint8_t)FLOPPY144_SITE_WALL_MOUNTED_ITEM &&
            pRect->x == 67U &&
            pRect->y == 90U &&
            pRect->width == 1U &&
            pRect->height == 4U
        )
        {
            pPanelRect = pRect;
            break;
        }
    }

    F144_CHECK(
        pPanelRect != NULL,
        "suppression panel Site geometry resolves"
    );

    F144_CHECK(
        pPanelRect != NULL &&
        !Floppy144SiteObjectGeometryVisible(&sState, pPanelRect),
        "suppression panel starts progression-hidden"
    );

    Floppy144TestSetPosition(&sState, 68, 90);

    F144_CHECK(
        Floppy144RunStateMovePlayerSite(
            &sState,
            0,
            FLOPPY144_SITE_MOVE_STEP_X16
        ),
        "progression-hidden fixture does not block movement"
    );
}

/*
 * Notebook prose is emitted by the game-data compiler and visibility is driven
 * by already-persisted trigger/interaction/evidence state. This catches a
 * compiler regression where NOTEBOOK flat records were accidentally omitted.
 */
static void Floppy144TestNotebookPopulation(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CollectionId eDr01;
    Floppy144InteractionId eI001;
    uint32_t uBefore;

    Floppy144TestReset(&sWorld, &sState);

    F144_CHECK(
        Floppy144WorldInitialiseArchiveServices(&sWorld) &&
        Floppy144RunStateInitialiseArchiveServices(&sState),
        "Notebook fixture initialises archive services"
    );

    eDr01 = Floppy144GameDataCollectionId("DR-01");
    F144_CHECK(
        eDr01 < FLOPPY144_COLLECTION_COUNT,
        "DR-01 resolves for Notebook fixture"
    );

    (void)Floppy144RunStateBitSet(
        sState.collections,
        (uint32_t)eDr01
    );
    (void)Floppy144WorldRestoreCollection(&sWorld, eDr01);

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-001")
        ),
        "T-001 records opening Notebook fact"
    );

    uBefore = Floppy144GameDataNotebookEntryCount(&sState);
    F144_CHECK(
        uBefore > 0U,
        "Notebook exposes recovered DR-01 fact"
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_FACILITIES
    );

    (void)Floppy144TriggerTryFire(
        &sWorld,
        &sState,
        Floppy144GameDataTriggerId("T-006")
    );

    eI001 = Floppy144GameDataInteractionId("I-001");
    F144_CHECK(
        eI001 < FLOPPY144_INTERACTION_COUNT &&
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eI001
        ),
        "I-001 runs for Notebook fixture"
    );

    F144_CHECK(
        Floppy144GameDataNotebookEntryCount(&sState) > uBefore,
        "Notebook gains interaction/evidence notes after physical inspection"
    );
}

/*
 * Passive labels are intentionally low priority. Furniture names appear only
 * when immediately adjacent; locked doors identify themselves; unlocked doors
 * remain silent. Any explicit inspection notice is supplied separately by the
 * renderer and therefore overwrites these labels.
 */
static void Floppy144TestContextLabels(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    const char *pszLabel;

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    /* Reception waiting chair at x79 y60 w2 h2. */
    Floppy144TestSetPosition(&sState, 80, 61);

    pszLabel = Floppy144SiteContextLabel(&sState);
    F144_CHECK(
        pszLabel != NULL && strcmp(pszLabel, "CHAIR") == 0,
        "adjacent furniture exposes generic CHAIR label"
    );

    /* COR_REC is the vertical x66 Corridor/Reception boundary. */
    Floppy144TestSetPosition(&sState, 68, 56);
    pszLabel = Floppy144SiteContextLabel(&sState);
    F144_CHECK(
        pszLabel != NULL && strcmp(pszLabel, "LOCKED DOOR") == 0,
        "locked reconstructed door exposes LOCKED DOOR label"
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-004")
        ),
        "T-004 prepares Corridor unlock fixture"
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-005")
        ),
        "T-005 unlocks Corridor connections for label fixture"
    );

    pszLabel = Floppy144SiteContextLabel(&sState);
    F144_CHECK(
        pszLabel == NULL || strcmp(pszLabel, "LOCKED DOOR") != 0,
        "unlocked door no longer exposes a door label"
    );
}

/*
 * OUTSIDE is a connection endpoint rather than a reconstructable room.
 * Entry doors are initially unlocked and should request a Site exit; emergency
 * doors remain locked and must not. The query must not move the player.
 */
static void Floppy144TestExteriorExitBehaviour(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    int32_t nOriginalX;
    int32_t nOriginalY;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );

    /*
     * ENTRY_01/02 now occupy the x=99 east edge. From x=99.5, one half-unit
     * movement to the right crosses the exterior threshold.
     */
    Floppy144RunStateSetPlayerSitePosition(
        &sState,
        99 * FLOPPY144_SITE_FIXED_ONE +
            FLOPPY144_SITE_MOVE_STEP_X16,
        50 * FLOPPY144_SITE_FIXED_ONE
    );

    nOriginalX = sState.player_site_x;
    nOriginalY = sState.player_site_y;

    F144_CHECK(
        Floppy144RunStateWouldExitSite(
            &sState,
            FLOPPY144_SITE_MOVE_STEP_X16,
            0
        ),
        "unlocked Reception entrance requests exterior Site exit"
    );

    F144_CHECK(
        sState.player_site_x == nOriginalX &&
        sState.player_site_y == nOriginalY,
        "exterior exit query leaves player just inside threshold"
    );

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    /* EMER_01/02 now occupy the y=99 south edge and remain locked. */
    Floppy144RunStateSetPlayerSitePosition(
        &sState,
        61 * FLOPPY144_SITE_FIXED_ONE,
        99 * FLOPPY144_SITE_FIXED_ONE +
            FLOPPY144_SITE_MOVE_STEP_X16
    );

    F144_CHECK(
        !Floppy144RunStateWouldExitSite(
            &sState,
            0,
            FLOPPY144_SITE_MOVE_STEP_X16
        ),
        "locked Corridor emergency exit does not leave Site"
    );
}

int main(void)
{
    Floppy144TestPhysicalParentCoverage();
    Floppy144TestCentredGeometryMetadata();
    Floppy144TestMainOfficeDeskContent();
    Floppy144TestTerminalCoverage();
    Floppy144TestPhysicalInteractionExecution();
    Floppy144TestFollowOnReveal();
    Floppy144TestRotatedParentTargeting();
    Floppy144TestSceneryDoesNotBecomeInteraction();
    Floppy144TestInspectionRange();
    Floppy144TestHiddenGeometryDoesNotCollide();
    Floppy144TestNotebookPopulation();
    Floppy144TestContextLabels();
    Floppy144TestExteriorExitBehaviour();

    if(g_nFailures != 0)
    {
        fprintf(
            stderr,
            "\nSTAGE 3B.2 SITE INTERACTION TESTS: FAIL (%d failure%s)\n",
            g_nFailures,
            g_nFailures == 1 ? "" : "s"
        );

        return 1;
    }

    puts("\nSTAGE 3B.2 SITE INTERACTION TESTS: PASS");
    return 0;
}
