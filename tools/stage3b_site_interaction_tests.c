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
#include "floppy144_site_rooms.h"
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
 * Test fixture helper for a collection which is already restored in the
 * player route.  Some Site objects retain a legacy required_collection gate,
 * so firing their trigger without restoring the owning collection produces a
 * state the real game cannot reach.
 */
static bool Floppy144TestRestoreCollection(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState,
    const char *pszCollectionId
)
{
    Floppy144CollectionId eCollection;

    if(
        pWorld == NULL ||
        pState == NULL ||
        pszCollectionId == NULL
    )
    {
        return false;
    }

    eCollection =
        Floppy144GameDataCollectionId(
            pszCollectionId
        );

    if(eCollection >= FLOPPY144_COLLECTION_COUNT)
    {
        return false;
    }

    if(
        !Floppy144RunStateBitSet(
            pState->collections,
            (uint32_t)eCollection
        )
    )
    {
        return false;
    }

    return
        Floppy144WorldRestoreCollection(
            pWorld,
            eCollection
        );
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

        F144_CHECK(
            pRecord->pszF != NULL &&
            pRecord->pszF[0] != '\0',
            "physical item carries a player-facing short description"
        );

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
        uPhysicalCount == 633U,
        "all 633 physical items participate in Site parent audit"
    );

    F144_CHECK(
        uPhysicalInteractionCount == 35U,
        "35 physical items own canonical gameplay interactions"
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
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "FM-13"
        ),
        "physical interaction fixture restores FM-13"
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

    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            Floppy144GameDataTriggerId("T-029")
        ),
        "rotated-parent fixture reveals one Director desk item"
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

/*
 * Furniture now carries small physical context even when it has no gameplay
 * role. A waiting chair may therefore advertise Inspect, but its incidental
 * find must remain ordinary worldbuilding rather than a runnable interaction.
 */
static void Floppy144TestSceneryDoesNotBecomeInteraction(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144SiteInspectionTarget sTarget;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );

    Floppy144TestSetPosition(&sState, 80, 61);

    F144_CHECK(
        Floppy144SiteAvailableActions(&sState) != 0U,
        "waiting-chair physical context advertises Inspect"
    );

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(
            &sState,
            &sTarget
        ) &&
        sTarget.pszParentId != NULL &&
        strcmp(
            sTarget.pszParentId,
            "RECEPTION_CHAIR_13"
        ) == 0 &&
        sTarget.pszPhysicalItemId != NULL &&
        strcmp(
            sTarget.pszPhysicalItemId,
            "P-518"
        ) == 0 &&
        sTarget.eInteraction == FLOPPY144_INTERACTION_COUNT &&
        !sTarget.bInteractionAvailable,
        "waiting-chair find remains contextual rather than gameplay evidence"
    );
}


/*
 * Site inspection reach is measured from the player's collision footprint.
 * The resolver may legitimately find a different nearby object, so the
 * out-of-range assertions below are target-specific rather than assuming that
 * the surrounding Site contains no other inspectable physical material.
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
        !Floppy144SiteResolveInspectionTarget(&sState, &sTarget) ||
        sTarget.pszParentId == NULL ||
        strcmp(
            sTarget.pszParentId,
            "MAIN_OFFICE_DESK_05"
        ) != 0,
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
        !Floppy144SiteResolveInspectionTarget(&sState, &sTarget) ||
        sTarget.pszParentId == NULL ||
        strcmp(
            sTarget.pszParentId,
            "MAIN_OFFICE_DESK_06"
        ) != 0,
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
    const Floppy144DataRecord *pPanelContext;
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

    F144_CHECK(
        Floppy144TestRestoreCollection(
            &sWorld,
            &sState,
            "FM-13"
        ),
        "Notebook fixture restores FM-13 before panel service note"
    );

    pPanelContext =
        Floppy144GameDataFind(
            FLOPPY144_DATA_PHYSICAL_ITEM,
            "P-200"
        );

    F144_CHECK(
        pPanelContext != NULL &&
        !Floppy144SitePhysicalItemVisible(
            &sState,
            pPanelContext
        ),
        "contextual child of hidden panel remains hidden"
    );

    Floppy144TestSetPosition(&sState, 68, 90);
    F144_CHECK(
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_INSPECT
        ) == 0U,
        "hidden physical parent does not advertise Inspect"
    );

    (void)Floppy144TriggerTryFire(
        &sWorld,
        &sState,
        Floppy144GameDataTriggerId("T-006")
    );

    F144_CHECK(
        pPanelContext != NULL &&
        Floppy144SitePhysicalItemVisible(
            &sState,
            pPanelContext
        ),
        "contextual child appears when hidden parent is revealed"
    );

    F144_CHECK(
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_INSPECT
        ) != 0U,
        "revealed physical item advertises Inspect"
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
        Floppy144GameDataNotebookEntryCount(&sState) == uBefore + 1U,
        "evidence inspection adds one Notebook finding without duplicate title note"
    );

    {
        const Floppy144DataRecord *pLatest =
            Floppy144GameDataNotebookEntryAt(
                &sState,
                Floppy144GameDataNotebookEntryCount(&sState) - 1U
            );

        F144_CHECK(
            pLatest != NULL &&
            pLatest->pszId != NULL &&
            strcmp(pLatest->pszId, "E-001") == 0,
            "suppression-panel Notebook finding uses canonical evidence entry"
        );
    }
}

/*
 * Passive labels are intentionally low priority. Furniture names appear only
 * when immediately adjacent. Corridor-facing doors retain their room plaque
 * through lock/unlock state, while non-Corridor locked doors keep the generic
 * fallback. Explicit inspection notices still overwrite passive labels.
 */
static void Floppy144TestRecordsTrolleyNotebookGuidance(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144InteractionId eI004;
    Floppy144TriggerId eT013;
    uint32_t uBefore;

    Floppy144TestReset(&sWorld, &sState);

    eT013 = Floppy144GameDataTriggerId("T-013");
    eI004 = Floppy144GameDataInteractionId("I-004");

    F144_CHECK(
        eT013 < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(&sState, eT013),
        "Records trolley Notebook fixture records T-013"
    );

    uBefore = Floppy144GameDataNotebookEntryCount(&sState);

    F144_CHECK(
        eI004 < FLOPPY144_INTERACTION_COUNT &&
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eI004
        ),
        "Records trolley interaction completes"
    );

    F144_CHECK(
        Floppy144GameDataNotebookEntryCount(&sState) == uBefore + 1U,
        "Records trolley inspection immediately adds a Notebook breadcrumb"
    );

    {
        const Floppy144DataRecord *pLatest =
            Floppy144GameDataNotebookEntryAt(
                &sState,
                Floppy144GameDataNotebookEntryCount(&sState) - 1U
            );

        F144_CHECK(
            pLatest != NULL &&
            pLatest->pszId != NULL &&
            strcmp(pLatest->pszId, "I-004") == 0 &&
            strstr(pLatest->pszA, "Secure Cabinet 05") != NULL,
            "trolley Notebook breadcrumb points to the next comparison location"
        );
    }
}

static void Floppy144TestLockedDoorInspectActions(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;

    /*
     * Fresh-run COR_REC fixture: both endpoint rooms exist, but the authored
     * connection is still locked. The door must advertise Inspect without
     * becoming traversable.
     */
    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(
            &sState,
            "COR_REC"
        ),
        "fresh COR_REC fixture remains locked"
    );

    Floppy144TestSetPosition(&sState, 68, 56);

    F144_CHECK(
        Floppy144SiteLockedDoorNearby(&sState),
        "locked COR_REC resolves as a nearby Inspect target"
    );

    F144_CHECK(
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_INSPECT
        ) != 0U,
        "locked COR_REC advertises Inspect"
    );

    /*
     * REC_OFF uses the same rule. Reconstructing Main Office makes the shared
     * boundary visible, but must not silently remove Inspect while the
     * connection itself is still locked.
     */
    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(
            &sState,
            "REC_OFF"
        ),
        "fresh REC_OFF fixture remains locked"
    );

    Floppy144TestSetPosition(&sState, 71, 65);

    F144_CHECK(
        Floppy144SiteLockedDoorNearby(&sState),
        "locked REC_OFF resolves as a nearby Inspect target"
    );

    F144_CHECK(
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_INSPECT
        ) != 0U,
        "locked REC_OFF advertises Inspect"
    );
}

static const char *Floppy144TestCorridorDoorExpectedLabel(
    const Floppy144SiteRect *pDoor
)
{
    uint8_t uOtherRoom;

    if(
        pDoor == NULL ||
        pDoor->type != (uint8_t)FLOPPY144_SITE_DOOR ||
        (
            pDoor->from_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
            pDoor->to_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR
        )
    )
    {
        return NULL;
    }

    uOtherRoom =
        pDoor->from_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR
        ? pDoor->to_room
        : pDoor->from_room;

    if(uOtherRoom == FLOPPY144_SITE_ROOM_OUTSIDE)
        return "EMERGENCY EXIT";

    switch((Floppy144RoomId)uOtherRoom)
    {
        case FLOPPY144_ROOM_RECEPTION:         return "RECEPTION";
        case FLOPPY144_ROOM_RECORDS_OFFICE:    return "RECORDS OFFICE";
        case FLOPPY144_ROOM_MAIN_OFFICE:       return "MAIN OFFICE";
        case FLOPPY144_ROOM_SECURITY:          return "SECURITY";
        case FLOPPY144_ROOM_IT_SUPPORT:        return "IT SUPPORT";
        case FLOPPY144_ROOM_SECRETARY_OFFICE:  return "DIRECTOR";
        case FLOPPY144_ROOM_STAFF_ROOM:        return "STAFF ROOM";
        case FLOPPY144_ROOM_FACILITIES:        return "FACILITIES";
        default:                               return NULL;
    }
}

static const char *Floppy144TestCorridorDoorExpectedParent(
    const Floppy144SiteRect *pDoor
)
{
    uint8_t uOtherRoom;

    if(
        pDoor == NULL ||
        pDoor->type != (uint8_t)FLOPPY144_SITE_DOOR ||
        (
            pDoor->from_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
            pDoor->to_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR
        )
    )
    {
        return NULL;
    }

    uOtherRoom =
        pDoor->from_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR
        ? pDoor->to_room
        : pDoor->from_room;

    switch((Floppy144RoomId)uOtherRoom)
    {
        case FLOPPY144_ROOM_RECEPTION:         return "COR_REC";
        case FLOPPY144_ROOM_RECORDS_OFFICE:    return "COR_RECO";
        case FLOPPY144_ROOM_MAIN_OFFICE:       return "COR_OFF";
        case FLOPPY144_ROOM_SECURITY:          return "COR_SEC";
        case FLOPPY144_ROOM_IT_SUPPORT:        return "COR_IT";
        case FLOPPY144_ROOM_SECRETARY_OFFICE:  return "COR_SECR";
        case FLOPPY144_ROOM_STAFF_ROOM:        return "COR_STAFF";
        case FLOPPY144_ROOM_FACILITIES:        return "COR_FAC";
        default:                               return NULL;
    }
}

static bool Floppy144TestCorridorDoorApproach(
    const Floppy144SiteRect *pDoor,
    int32_t *pX16,
    int32_t *pY16
)
{
    int32_t nAlong;
    int32_t nCandidateAX;
    int32_t nCandidateAY;
    int32_t nCandidateBX;
    int32_t nCandidateBY;
    int32_t nChosenX;
    int32_t nChosenY;

    if(
        pDoor == NULL ||
        pX16 == NULL ||
        pY16 == NULL ||
        pDoor->type != (uint8_t)FLOPPY144_SITE_DOOR
    )
    {
        return false;
    }

    if(pDoor->width >= pDoor->height)
    {
        nAlong =
            (int32_t)pDoor->x +
            (int32_t)pDoor->width / 2;

        nCandidateAX = nAlong;
        nCandidateAY = (int32_t)pDoor->y - 1;
        nCandidateBX = nAlong;
        nCandidateBY =
            (int32_t)pDoor->y +
            (int32_t)pDoor->height;
    }
    else
    {
        nAlong =
            (int32_t)pDoor->y +
            (int32_t)pDoor->height / 2;

        nCandidateAX = (int32_t)pDoor->x - 1;
        nCandidateAY = nAlong;
        nCandidateBX =
            (int32_t)pDoor->x +
            (int32_t)pDoor->width;
        nCandidateBY = nAlong;
    }

    if(
        nCandidateAX >= 0 &&
        nCandidateAY >= 0 &&
        nCandidateAX < FLOPPY144_SITE_SIZE_UNITS &&
        nCandidateAY < FLOPPY144_SITE_SIZE_UNITS &&
        Floppy144SiteRoomAtCell(
            (uint8_t)nCandidateAX,
            (uint8_t)nCandidateAY
        ) == FLOPPY144_ROOM_CORRIDOR
    )
    {
        nChosenX = nCandidateAX;
        nChosenY = nCandidateAY;
    }
    else if(
        nCandidateBX >= 0 &&
        nCandidateBY >= 0 &&
        nCandidateBX < FLOPPY144_SITE_SIZE_UNITS &&
        nCandidateBY < FLOPPY144_SITE_SIZE_UNITS &&
        Floppy144SiteRoomAtCell(
            (uint8_t)nCandidateBX,
            (uint8_t)nCandidateBY
        ) == FLOPPY144_ROOM_CORRIDOR
    )
    {
        nChosenX = nCandidateBX;
        nChosenY = nCandidateBY;
    }
    else
    {
        return false;
    }

    *pX16 =
        nChosenX * FLOPPY144_SITE_FIXED_ONE +
        FLOPPY144_SITE_FIXED_ONE / 2;

    *pY16 =
        nChosenY * FLOPPY144_SITE_FIXED_ONE +
        FLOPPY144_SITE_FIXED_ONE / 2;

    return true;
}

static void Floppy144TestCorridorDoorLabels(void)
{
    uint32_t uRectIndex;
    uint32_t uCorridorDoorCount = 0U;
    uint32_t uAuditedDoorCount = 0U;

    /*
     * Audit the compiled topology itself rather than maintaining a second set
     * of hand-written approach coordinates. This catches new Corridor doors
     * automatically and derives the player position from adjacent floor
     * ownership, so horizontal/vertical boundaries and either endpoint order
     * are handled identically.
     */
    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pDoor =
            Floppy144SiteRectAt(uRectIndex);

        Floppy144WorldState sWorld;
        Floppy144RunState sState;
        const char *pszExpected;
        const char *pszActual;
        uint8_t uOtherRoom;
        int32_t nPlayerX16;
        int32_t nPlayerY16;
        bool bLocked = false;

        if(
            pDoor == NULL ||
            pDoor->type != (uint8_t)FLOPPY144_SITE_DOOR ||
            (
                pDoor->from_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
                pDoor->to_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR
            )
        )
        {
            continue;
        }

        ++uCorridorDoorCount;

        pszExpected =
            Floppy144TestCorridorDoorExpectedLabel(
                pDoor
            );

        F144_CHECK(
            pszExpected != NULL,
            "Corridor-facing door has a player-facing label mapping"
        );

        if(pszExpected == NULL)
        {
            continue;
        }

        F144_CHECK(
            Floppy144TestCorridorDoorApproach(
                pDoor,
                &nPlayerX16,
                &nPlayerY16
            ),
            "Corridor-facing door has a real Corridor-side floor approach"
        );

        if(
            !Floppy144TestCorridorDoorApproach(
                pDoor,
                &nPlayerX16,
                &nPlayerY16
            )
        )
        {
            continue;
        }

        Floppy144TestReset(
            &sWorld,
            &sState
        );

        (void)Floppy144RunStateReconstructRoom(
            &sState,
            FLOPPY144_ROOM_CORRIDOR
        );

        uOtherRoom =
            pDoor->from_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR
            ? pDoor->to_room
            : pDoor->from_room;

        if(uOtherRoom < (uint8_t)FLOPPY144_ROOM_COUNT)
        {
            (void)Floppy144RunStateReconstructRoom(
                &sState,
                (Floppy144RoomId)uOtherRoom
            );

            /*
             * FM-04 restores the physical room plates. Once T-005 has fired,
             * Inspect must resolve the door itself as the parent rather than
             * falling back to a one-line proximity notice.
             */
            (void)Floppy144RunStateFireTrigger(
                &sState,
                Floppy144GameDataTriggerId("T-005")
            );
        }

        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            nPlayerX16,
            nPlayerY16
        );

        F144_CHECK(
            Floppy144SiteRoomAtPosition(
                sState.player_site_x,
                sState.player_site_y
            ) == FLOPPY144_ROOM_CORRIDOR,
            "derived door approach is actually inside Corridor"
        );

        pszActual =
            Floppy144SiteCorridorDoorLabelNearby(
                &sState,
                &bLocked
            );

        if(
            pszActual == NULL ||
            strcmp(
                pszActual,
                pszExpected
            ) != 0
        )
        {
            fprintf(
                stderr,
                "CORRIDOR DOOR LABEL DIAGNOSTIC: rect=(%u,%u %ux%u) endpoints=%u/%u expected=%s actual=%s\n",
                (unsigned)pDoor->x,
                (unsigned)pDoor->y,
                (unsigned)pDoor->width,
                (unsigned)pDoor->height,
                (unsigned)pDoor->from_room,
                (unsigned)pDoor->to_room,
                pszExpected,
                pszActual != NULL ? pszActual : "<none>"
            );
        }

        F144_CHECK(
            pszActual != NULL &&
            strcmp(
                pszActual,
                pszExpected
            ) == 0,
            "Corridor-facing door exposes its correct room plaque"
        );

        F144_CHECK(
            (
                Floppy144SiteAvailableActions(
                    &sState
                ) &
                FLOPPY144_SITE_ACTION_INSPECT
            ) != 0U,
            "Corridor-facing door advertises Inspect"
        );

        if(uOtherRoom < (uint8_t)FLOPPY144_ROOM_COUNT)
        {
            Floppy144SiteInspectionTarget sTarget;
            const char *pszExpectedParent =
                Floppy144TestCorridorDoorExpectedParent(
                    pDoor
                );

            F144_CHECK(
                pszExpectedParent != NULL &&
                Floppy144SiteResolveInspectionTarget(
                    &sState,
                    &sTarget
                ) &&
                sTarget.pszParentId != NULL &&
                strcmp(
                    sTarget.pszParentId,
                    pszExpectedParent
                ) == 0,
                "Corridor office door Inspect resolves the DOOR parent"
            );
        }

        ++uAuditedDoorCount;
    }

    F144_CHECK(
        uCorridorDoorCount == 10U,
        "compiled Site still contains ten Corridor-facing door rectangles"
    );

    F144_CHECK(
        uAuditedDoorCount == uCorridorDoorCount,
        "every Corridor-facing door was audited from a real Corridor approach"
    );
}

static void Floppy144TestConventionalDoorAccess(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    const Floppy144DataRecord *pDoor;
    Floppy144InteractionId eI035;
    Floppy144InteractionId eI037;
    Floppy144InteractionId eResolved;

    Floppy144TestReset(&sWorld, &sState);

    /*
     * Mirror the real late-game route closely enough that I-035 runs through
     * its own prerequisites/effects rather than faking only its completion bit.
     * Security is accessible after T-027; P-093 inspection then grants
     * SITE_KEYS before the player approaches the Server Room door.
     */
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_SECURITY
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_IT_SUPPORT
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_SERVER_ROOM
    );

    eI035 = Floppy144GameDataInteractionId("I-035");
    eI037 = Floppy144GameDataInteractionId("I-037");

    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            Floppy144GameDataTriggerId("T-027")
        ),
        "Server Room access fixture makes Security reachable"
    );

    F144_CHECK(
        eI035 < FLOPPY144_INTERACTION_COUNT &&
        eI037 < FLOPPY144_INTERACTION_COUNT &&
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eI035
        ) &&
        Floppy144RunStateHasCapability(
            &sState,
            Floppy144GameDataCapabilityId("SITE_KEYS")
        ),
        "Server Room access fixture acquires the complete Site key set"
    );

    pDoor = Floppy144GameDataFind(
        FLOPPY144_DATA_FIXTURE,
        "IT_SERV_01"
    );

    F144_CHECK(
        pDoor != NULL,
        "Server Room conventional door fixture resolves"
    );

    if(pDoor == NULL)
    {
        return;
    }

    Floppy144TestSetPosition(
        &sState,
        pDoor->n0 + pDoor->n2 / 2,
        pDoor->n1 + pDoor->n3 / 2
    );

    F144_CHECK(
        Floppy144SiteFocusedParentId(&sState) != NULL &&
        strcmp(
            Floppy144SiteFocusedParentId(&sState),
            "IT_SERV_01"
        ) == 0,
        "Server Room door owns Site focus at its threshold"
    );

    F144_CHECK(
        Floppy144SiteAccessInteractionNearby(
            &sState,
            &eResolved
        ) &&
        eResolved == eI037 &&
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_ACCESS
        ) != 0U,
        "Server Room door advertises its authored Access interaction"
    );

    F144_CHECK(
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eResolved
        ) &&
        Floppy144GameDataConnectionUnlocked(
            &sState,
            "IT_SERV_01"
        ),
        "Server Room Access unlocks the conventional door"
    );
}

static void Floppy144TestSingleFocusedParentActions(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144SiteInspectionTarget sTarget;
    const char *pszLabel;
    const char *pszParentId;
    uint32_t uActions;

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECORDS_OFFICE
    );

    /*
     * At x85/y19 the player's collision footprint touches the Records trolley,
     * while the former secure-cabinet point radius also reached Cabinet 06 at
     * x79/y21 w6/h2. The trolley must own the complete contextual prompt.
     */
    Floppy144TestSetPosition(&sState, 85, 19);

    pszLabel = Floppy144SiteContextLabel(&sState);
    pszParentId = Floppy144SiteFocusedParentId(&sState);
    uActions = Floppy144SiteAvailableActions(&sState);

    F144_CHECK(
        pszLabel != NULL &&
        strcmp(pszLabel, "TROLLEY") == 0 &&
        pszParentId != NULL &&
        strcmp(pszParentId, "RECORDS_OFFICE_TROLLEY") == 0,
        "Records trolley owns the canonical proximity focus"
    );

    F144_CHECK(
        (uActions & FLOPPY144_SITE_ACTION_INSPECT) != 0U &&
        (uActions & FLOPPY144_SITE_ACTION_ACCESS) == 0U,
        "trolley focus advertises Inspect but not neighbouring Access"
    );

    F144_CHECK(
        Floppy144SiteResolveInspectionTarget(&sState, &sTarget) &&
        sTarget.pszParentId != NULL &&
        strcmp(sTarget.pszParentId, "RECORDS_OFFICE_TROLLEY") == 0,
        "Inspect resolves the same parent named by the Site focus"
    );
}

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

    /*
     * COR_REC is the vertical x66 Corridor/Reception boundary. The topology
     * audit above proves all doors; this is only the focused lock/unlock label
     * behavior check from the Corridor side.
     */
    Floppy144TestSetPosition(&sState, 65, 56);
    pszLabel = Floppy144SiteContextLabel(&sState);
    F144_CHECK(
        pszLabel != NULL && strcmp(pszLabel, "RECEPTION") == 0,
        "locked Corridor side of COR_REC exposes its RECEPTION plaque"
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
        pszLabel != NULL && strcmp(pszLabel, "RECEPTION") == 0,
        "unlocked Corridor side of COR_REC retains its RECEPTION plaque"
    );

    F144_CHECK(
        Floppy144SiteCorridorDoorLabelNearby(
            &sState,
            NULL
        ) != NULL &&
        (
            Floppy144SiteAvailableActions(&sState) &
            FLOPPY144_SITE_ACTION_INSPECT
        ) != 0U,
        "unlocked Corridor door remains inspectable"
    );

    /*
     * The same physical threshold viewed from Reception is not a Corridor
     * plaque. This protects the side-specific behavior from accidentally
     * becoming a generic label on every door face.
     */
    Floppy144TestSetPosition(&sState, 68, 56);
    F144_CHECK(
        Floppy144SiteCorridorDoorLabelNearby(
            &sState,
            NULL
        ) == NULL,
        "Reception side of COR_REC does not masquerade as Corridor signage"
    );

    /*
     * Wall-mounted fixtures must participate in the same proximity-label
     * system as freestanding furniture.  Security's key cabinet occupies
     * x44 y72 w1 h3.
     */
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_SECURITY
    );
    /*
     * Use y74 rather than y73. The player's 2U-deep collision footprint at
     * y73 also touches the terminal desk's y71 edge, making both fixtures
     * equally near and legitimately allowing the earlier terminal label to
     * win the passive-label tie. At y74 only the key cabinet is adjacent.
     */
    Floppy144TestSetPosition(&sState, 45, 74);
    pszLabel = Floppy144SiteContextLabel(&sState);
    F144_CHECK(
        pszLabel != NULL &&
        strcmp(pszLabel, "KEY CABINET") == 0,
        "wall-mounted Security fixture exposes its authored context label"
    );

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );
    /*
     * Stand on the authored fixture footprint so the assertion tests the
     * Directory's label rather than whichever nearby object wins proximity.
     * RECEPTION_SITE_DIRECTORY: x77 y40 w1 h6.
     */
    Floppy144TestSetPosition(&sState, 77, 43);
    pszLabel = Floppy144SiteContextLabel(&sState);
    F144_CHECK(
        pszLabel != NULL &&
        strcmp(pszLabel, "SITE DIRECTORY") == 0,
        "Reception wall hanging is labelled Site Directory"
    );

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );
    /*
     * CORRIDOR_SITE_DIRECTORY: x35 y55 w6 h1. Use its centre line for an
     * unambiguous zero-distance context-label lookup.
     */
    Floppy144TestSetPosition(&sState, 38, 55);
    pszLabel = Floppy144SiteContextLabel(&sState);
    F144_CHECK(
        pszLabel != NULL &&
        strcmp(pszLabel, "SITE DIRECTORY") == 0,
        "Corridor wall hanging is labelled Site Directory"
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


/*
 * Order-independent evidence may be inspected in either order, but a synthesis
 * interaction must not expose its follow-on collection until the evidence pair
 * is actually complete. This reproduces the Records-route DR-31 dead-end where
 * P-047 could previously unlock DR-31 before P-046/E-005 had been established.
 *
 * Act II must also wait for the final physical evidence item itself, rather
 * than ending as soon as the trigger which reveals that item fires.
 */
static void Floppy144TestEvidenceGatedProgression(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;

    Floppy144InteractionId eI006;
    Floppy144InteractionId eI007;
    Floppy144InteractionId eI008;
    Floppy144InteractionId eI011;
    Floppy144EvidenceId eE005;
    Floppy144EvidenceId eE006;
    Floppy144EvidenceId eE009;
    Floppy144CollectionId eDr31;
    Floppy144TriggerId eT015;
    Floppy144TriggerId eT016;
    Floppy144TriggerId eT017;
    Floppy144TriggerId eT018;
    Floppy144TriggerId eT023;

    Floppy144TestReset(
        &sWorld,
        &sState
    );

    eI006 = Floppy144GameDataInteractionId("I-006");
    eI007 = Floppy144GameDataInteractionId("I-007");
    eI008 = Floppy144GameDataInteractionId("I-008");
    eI011 = Floppy144GameDataInteractionId("I-011");

    eE005 = Floppy144GameDataEvidenceId("E-005");
    eE006 = Floppy144GameDataEvidenceId("E-006");
    eE009 = Floppy144GameDataEvidenceId("E-009");

    eDr31 = Floppy144GameDataCollectionId("DR-31");

    eT015 = Floppy144GameDataTriggerId("T-015");
    eT016 = Floppy144GameDataTriggerId("T-016");
    eT017 = Floppy144GameDataTriggerId("T-017");
    eT018 = Floppy144GameDataTriggerId("T-018");
    eT023 = Floppy144GameDataTriggerId("T-023");

    F144_CHECK(
        eI006 < FLOPPY144_INTERACTION_COUNT &&
        eI007 < FLOPPY144_INTERACTION_COUNT &&
        eI008 < FLOPPY144_INTERACTION_COUNT &&
        eI011 < FLOPPY144_INTERACTION_COUNT &&
        eE005 < FLOPPY144_EVIDENCE_COUNT &&
        eE006 < FLOPPY144_EVIDENCE_COUNT &&
        eE009 < FLOPPY144_EVIDENCE_COUNT &&
        eDr31 < FLOPPY144_COLLECTION_COUNT &&
        eT015 < FLOPPY144_TRIGGER_COUNT &&
        eT016 < FLOPPY144_TRIGGER_COUNT &&
        eT017 < FLOPPY144_TRIGGER_COUNT &&
        eT018 < FLOPPY144_TRIGGER_COUNT &&
        eT023 < FLOPPY144_TRIGGER_COUNT,
        "evidence-gated progression fixture IDs resolve"
    );

    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            eT015
        ) &&
        Floppy144RunStateFireTrigger(
            &sState,
            eT016
        ),
        "Records evidence fixture arms DR-29 interactions"
    );

    /*
     * Inspect the synthesis half first. It is deliberately allowed to complete
     * because E-005 is order-independent, but DR-31 must remain unavailable.
     */
    F144_CHECK(
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eI007
        ),
        "P-047 may be inspected before P-046"
    );

    F144_CHECK(
        Floppy144RunStateInteractionCompleted(
            &sState,
            eI007
        ) &&
        !Floppy144RunStateEvidenceEstablished(
            &sState,
            eE005
        ),
        "early synthesis inspection does not fabricate E-005"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionAvailable(
            &sState,
            eDr31
        ),
        "DR-31 remains unavailable until E-005 is established"
    );

    F144_CHECK(
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eI006
        ),
        "P-046 completes the remaining half of E-005"
    );

    F144_CHECK(
        Floppy144RunStateEvidenceEstablished(
            &sState,
            eE005
        ) &&
        Floppy144RunStateCollectionAvailable(
            &sState,
            eDr31
        ),
        "establishing E-005 automatically authorises DR-31"
    );

    F144_CHECK(
        Floppy144GameDataTriggerTryFire(
            &sWorld,
            &sState,
            eT017
        ) &&
        Floppy144GameDataTriggerTryFire(
            &sWorld,
            &sState,
            eT018
        ),
        "eligible DR-31 trigger records fire in sequence"
    );

    F144_CHECK(
        Floppy144GameDataPhysicalItemRevealed(
            &sState,
            "P-052"
        ),
        "DR-31 reveals the Signed Custody Sheet on Records Office Desk 01"
    );

    /*
     * Records-first Act II now terminates on E-006, not T-018 alone.
     */
    Floppy144TestReset(
        &sWorld,
        &sState
    );

    F144_CHECK(
        Floppy144RunStateSetBranch(
            &sState,
            FLOPPY144_RUN_BRANCH_RECORDS_FIRST
        ) &&
        Floppy144RunStateFireTrigger(
            &sState,
            eT018
        ),
        "Records Act II fixture reaches the DR-31 reveal trigger"
    );

    F144_CHECK(
        !Floppy144GameDataConditionSatisfied(
            &sState,
            "act_at_least",
            "ACT_II_CORE_COMPLETE"
        ),
        "T-018 alone does not complete Records Act II"
    );

    F144_CHECK(
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eI008
        ) &&
        Floppy144RunStateEvidenceEstablished(
            &sState,
            eE006
        ),
        "Signed Custody Sheet inspection establishes E-006"
    );

    F144_CHECK(
        Floppy144GameDataConditionSatisfied(
            &sState,
            "act_at_least",
            "ACT_II_CORE_COMPLETE"
        ),
        "Records Act II completes only after E-006"
    );

    /*
     * Mirror the contract on the Technology branch so the semantic rule is
     * proven generic rather than Records-specific.
     */
    Floppy144TestReset(
        &sWorld,
        &sState
    );

    F144_CHECK(
        Floppy144RunStateSetBranch(
            &sState,
            FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST
        ) &&
        Floppy144RunStateFireTrigger(
            &sState,
            eT023
        ),
        "Technology Act II fixture reaches its final reveal trigger"
    );

    F144_CHECK(
        !Floppy144GameDataConditionSatisfied(
            &sState,
            "act_at_least",
            "ACT_II_CORE_COMPLETE"
        ),
        "T-023 alone does not complete Technology Act II"
    );

    F144_CHECK(
        Floppy144InteractionTryRun(
            &sWorld,
            &sState,
            eI011
        ) &&
        Floppy144RunStateEvidenceEstablished(
            &sState,
            eE009
        ),
        "Technology final evidence inspection establishes E-009"
    );

    F144_CHECK(
        Floppy144GameDataConditionSatisfied(
            &sState,
            "act_at_least",
            "ACT_II_CORE_COMPLETE"
        ),
        "Technology Act II completes only after E-009"
    );
}

int main(void)
{
    Floppy144TestPhysicalParentCoverage();
    Floppy144TestEvidenceGatedProgression();
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
    Floppy144TestRecordsTrolleyNotebookGuidance();
    Floppy144TestLockedDoorInspectActions();
    Floppy144TestCorridorDoorLabels();
    Floppy144TestConventionalDoorAccess();
    Floppy144TestSingleFocusedParentActions();
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
