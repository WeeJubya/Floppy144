/*
 * Floppy//144 Stage 3B.5 secure-cabinet / Cabinet Interior regression.
 *
 * Proves the reusable access layer against generated furniture and physical
 * item records. No story-specific cabinet coordinates are duplicated here.
 */

#include "floppy144_cabinet.h"
#include "floppy144_crossword.h"
#include "floppy144_game_data.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_takeaway.h"
#include "floppy144_variation.h"
#include "floppy144_persistence.h"
#include "floppy144_paperback.h"
#include "floppy144_run_state.h"
#include "floppy144_grey_door.h"
#include "floppy144_site_2d.h"
#include "floppy144_site_directory.h"
#include "floppy144_site.h"
#include "floppy144_site_object.h"
#include "floppy144_trigger_engine.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

static const Floppy144DataRecord *Floppy144TestCabinetRecord(
    const char *pszCabinetId
)
{
    return Floppy144GameDataFind(
        FLOPPY144_DATA_FURNITURE,
        pszCabinetId
    );
}

static void Floppy144TestPlaceAtCabinet(
    Floppy144RunState *pState,
    const char *pszCabinetId
)
{
    const Floppy144DataRecord *pCabinet =
        Floppy144TestCabinetRecord(pszCabinetId);

    Floppy144RoomId eRoom;

    F144_CHECK(
        pCabinet != NULL &&
        pCabinet->pszA != NULL &&
        pCabinet->pszC != NULL &&
        strcmp(pCabinet->pszC, "SECURE_CABINET") == 0,
        "secure-cabinet fixture resolves from generated furniture"
    );

    if(pCabinet == NULL)
        return;

    eRoom = Floppy144GameDataRoomId(pCabinet->pszA);

    F144_CHECK(
        eRoom < FLOPPY144_ROOM_COUNT,
        "secure-cabinet fixture room resolves"
    );

    if(eRoom >= FLOPPY144_ROOM_COUNT)
        return;

    (void)Floppy144RunStateReconstructRoom(pState, eRoom);

    /*
     * The interaction resolver measures point-to-rectangle distance. The centre
     * is therefore a compact geometry-independent fixture position.
     */
    Floppy144RunStateSetPlayerSitePosition(
        pState,
        (pCabinet->n0 * FLOPPY144_SITE_FIXED_ONE) +
            (pCabinet->n2 * FLOPPY144_SITE_FIXED_ONE) / 2,
        (pCabinet->n1 * FLOPPY144_SITE_FIXED_ONE) +
            (pCabinet->n3 * FLOPPY144_SITE_FIXED_ONE) / 2
    );
}

static void Floppy144TestReset(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState,
    Floppy144CabinetState *pCabinet
)
{
    Floppy144WorldReset(pWorld);
    Floppy144RunStateBegin(pState, 144U);
    Floppy144CabinetReset(pCabinet);
}

static void Floppy144TestDigitEntry(
    Floppy144CabinetState *pCabinet,
    const char *pszDigits
)
{
    const char *p = pszDigits;

    while(*p != '\0')
    {
        F144_CHECK(
            Floppy144CabinetInputDigit(pCabinet, *p),
            "keypad accepts digit within configured capacity"
        );
        ++p;
    }
}

static uint32_t Floppy144TestVisibleContentIndex(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pState,
    const char *pszPhysicalItemId
)
{
    uint32_t uIndex;
    uint32_t uCount;

    if(
        pCabinet == NULL ||
        pState == NULL ||
        pszPhysicalItemId == NULL
    )
    {
        return UINT32_MAX;
    }

    uCount =
        Floppy144CabinetVisibleContentCount(
            pCabinet,
            pState
        );

    for(uIndex = 0U; uIndex < uCount; ++uIndex)
    {
        const Floppy144DataRecord *pItem =
            Floppy144CabinetVisibleContentAt(
                pCabinet,
                pState,
                uIndex
            );

        if(
            pItem != NULL &&
            pItem->pszId != NULL &&
            strcmp(
                pItem->pszId,
                pszPhysicalItemId
            ) == 0
        )
        {
            return uIndex;
        }
    }

    return UINT32_MAX;
}

static void Floppy144TestGeneratedCabinetDiscovery(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);
    Floppy144TestPlaceAtCabinet(&sState, "MAIN_OFFICE_SECURE_CABINET");

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sState),
        "A-access resolver finds nearby generated secure cabinet"
    );

    F144_CHECK(
        strcmp(
            Floppy144CabinetId(&sCabinet),
            "MAIN_OFFICE_SECURE_CABINET"
        ) == 0,
        "cabinet screen retains canonical generated cabinet ID"
    );

    F144_CHECK(
        Floppy144CabinetRequiredDigits(&sCabinet) == 6U,
        "Main Office cabinet uses authored six-digit code metadata"
    );

    F144_CHECK(
        !Floppy144CabinetCodeKnown(&sCabinet, &sState),
        "cabinet code is unavailable before master-code capability"
    );
}

static void Floppy144TestSecurityCabinetUnlockAndContents(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    Floppy144TriggerId eT037;
    const Floppy144DataRecord *pItem;
    uint32_t uP092Index;
    char szExpected[FLOPPY144_CABINET_CODE_CAPACITY + 1U];
    char szWrong[FLOPPY144_CABINET_CODE_CAPACITY + 1U];

    Floppy144TestReset(&sWorld, &sState, &sCabinet);
    Floppy144TestPlaceAtCabinet(&sState, "SECURITY_SECURE_CABINET");

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sState),
        "Security secure cabinet opens keypad screen"
    );

    F144_CHECK(
        Floppy144CabinetRequiredDigits(&sCabinet) == 8U,
        "Security secure cabinet requires eight digits"
    );

    Floppy144CabinetExpectedCode(
        &sCabinet,
        sState.recovery_seed,
        szExpected,
        (uint32_t)sizeof(szExpected)
    );
    (void)snprintf(szWrong, sizeof(szWrong), "%s", szExpected);
    szWrong[0] = szWrong[0] == '9' ? '0' : (char)(szWrong[0] + 1);
    Floppy144TestDigitEntry(&sCabinet, szWrong);

    F144_CHECK(
        !Floppy144CabinetSubmitCode(
            &sCabinet,
            &sWorld,
            &sState
        ) &&
        !Floppy144CabinetInteriorOpen(&sCabinet),
        "incorrect Security code cannot unlock cabinet"
    );

    eT037 = Floppy144GameDataTriggerId("T-037");
    F144_CHECK(
        eT037 < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(&sState, eT037),
        "Security-code fixture records T-037"
    );

    F144_CHECK(
        Floppy144CabinetCodeKnown(&sCabinet, &sState),
        "T-037 notebook fact exposes Security cabinet code"
    );

    Floppy144CabinetClearInput(&sCabinet);
    Floppy144TestDigitEntry(&sCabinet, szExpected);

    F144_CHECK(
        Floppy144CabinetSubmitCode(
            &sCabinet,
            &sWorld,
            &sState
        ),
        "exact generated eight-digit Security code unlocks selected cabinet"
    );

    F144_CHECK(
        Floppy144CabinetInteriorOpen(&sCabinet) &&
        Floppy144CabinetUnlocked(&sCabinet, &sState),
        "successful code entry enters persistent Cabinet Interior"
    );

    F144_CHECK(
        Floppy144CabinetVisibleContentCount(
            &sCabinet,
            &sState
        ) >= 1U,
        "Cabinet Interior exposes recovered contents including P-092"
    );

    uP092Index =
        Floppy144TestVisibleContentIndex(
            &sCabinet,
            &sState,
            "P-092"
        );

    F144_CHECK(
        uP092Index != UINT32_MAX,
        "Cabinet Interior contains recovered Master Access Register"
    );

    if(uP092Index != UINT32_MAX)
    {
        Floppy144CabinetMoveSelection(
            &sCabinet,
            &sState,
            (int32_t)uP092Index
        );
    }

    pItem =
        uP092Index != UINT32_MAX
            ? Floppy144CabinetVisibleContentAt(
                &sCabinet,
                &sState,
                uP092Index
            )
            : NULL;

    F144_CHECK(
        pItem != NULL &&
        pItem->pszId != NULL &&
        strcmp(pItem->pszId, "P-092") == 0,
        "Cabinet Interior resolves recovered Master Access Register by ID"
    );

    F144_CHECK(
        Floppy144CabinetInspectSelected(
            &sCabinet,
            &sWorld,
            &sState
        ),
        "I inspects selected Cabinet Interior item"
    );

    F144_CHECK(
        Floppy144RunStateHasCapability(
            &sState,
            FLOPPY144_CAPABILITY_MASTER_SECURE_CABINET_CODES
        ),
        "inspecting P-092 grants master secure-cabinet codes"
    );

    F144_CHECK(
        sCabinet.pszStatus != NULL &&
        strstr(
            sCabinet.pszStatus,
            "NOTEBOOK UPDATED"
        ) != NULL,
        "gameplay physical-item inspection reports Notebook update"
    );

    F144_CHECK(
        Floppy144CabinetDetailOpen(&sCabinet),
        "inspection opens in-screen Cabinet Interior detail"
    );

    F144_CHECK(
        Floppy144CabinetBackspace(&sCabinet) &&
        !Floppy144CabinetDetailOpen(&sCabinet) &&
        Floppy144CabinetInteriorOpen(&sCabinet),
        "Backspace returns item detail to Cabinet Interior"
    );

    F144_CHECK(
        !Floppy144CabinetBackspace(&sCabinet),
        "Backspace from Cabinet Interior delegates return to Site coordinator"
    );
}

static void Floppy144TestChairContentsContract(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    uint32_t uCount;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sState,
            FLOPPY144_ROOM_MAIN_OFFICE
        ),
        "chair contents fixture reconstructs Main Office"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &sCabinet,
            &sState,
            "MAIN_OFFICE_CHAIR_01"
        ),
        "chair opens the reusable Contents screen"
    );

    F144_CHECK(
        Floppy144CabinetInteriorOpen(&sCabinet) &&
        strcmp(
            sCabinet.szContainerType,
            "CHAIR"
        ) == 0,
        "chair Contents state retains a dedicated chair presentation type"
    );

    uCount =
        Floppy144CabinetVisibleContentCount(
            &sCabinet,
            &sState
        );

    F144_CHECK(
        uCount >= 1U &&
        uCount <= 2U,
        "chair exposes only one or two plausible physical items"
    );

    F144_CHECK(
        Floppy144CabinetVisibleContentAt(
            &sCabinet,
            &sState,
            2U
        ) == NULL,
        "chair runtime contents are hard-capped at two items"
    );
}

static void Floppy144TestGenericParentContents(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    const Floppy144DataRecord *pItem;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sState,
            FLOPPY144_ROOM_MAIN_OFFICE
        ),
        "generic contents fixture reconstructs Main Office"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &sCabinet,
            &sState,
            "MAIN_OFFICE_DESK_01"
        ),
        "I-style parent access opens Main Office Desk 01 contents"
    );

    F144_CHECK(
        Floppy144CabinetInteriorOpen(&sCabinet) &&
        strcmp(
            sCabinet.szContainerType,
            "STANDARD_DESK"
        ) == 0,
        "generic contents state retains desk display type"
    );

    F144_CHECK(
        Floppy144CabinetVisibleContentCount(
            &sCabinet,
            &sState
        ) >= 2U,
        "desk contents view exposes multiple contextual physical items"
    );

    pItem =
        Floppy144CabinetVisibleContentAt(
            &sCabinet,
            &sState,
            0U
        );

    F144_CHECK(
        pItem != NULL &&
        pItem->pszId != NULL &&
        pItem->pszA != NULL &&
        pItem->pszF != NULL,
        "generic contents view resolves a described physical item"
    );

    F144_CHECK(
        Floppy144CabinetInspectSelected(
            &sCabinet,
            &sWorld,
            &sState
        ) &&
        Floppy144CabinetDetailOpen(&sCabinet),
        "I inspects the selected desk item inside the contents screen"
    );
}

static void Floppy144TestSeededContainerOrdering(void)
{
    Floppy144WorldState sWorldA,sWorldB,sWorldC;
    Floppy144RunState sStateA,sStateB,sStateC;
    Floppy144CabinetState sCabinetA,sCabinetB,sCabinetC;
    uint32_t uCount,uIndex;
    bool bDifferent=false;

    Floppy144WorldReset(&sWorldA);
    Floppy144WorldReset(&sWorldB);
    Floppy144WorldReset(&sWorldC);
    Floppy144RunStateBegin(&sStateA,144U);
    Floppy144RunStateBegin(&sStateB,144U);
    Floppy144RunStateBegin(&sStateC,145U);
    Floppy144CabinetReset(&sCabinetA);
    Floppy144CabinetReset(&sCabinetB);
    Floppy144CabinetReset(&sCabinetC);

    (void)Floppy144RunStateReconstructRoom(&sStateA,FLOPPY144_ROOM_MAIN_OFFICE);
    (void)Floppy144RunStateReconstructRoom(&sStateB,FLOPPY144_ROOM_MAIN_OFFICE);
    (void)Floppy144RunStateReconstructRoom(&sStateC,FLOPPY144_ROOM_MAIN_OFFICE);

    F144_CHECK(
        Floppy144CabinetOpenParent(&sCabinetA,&sStateA,"MAIN_OFFICE_DESK_01") &&
        Floppy144CabinetOpenParent(&sCabinetB,&sStateB,"MAIN_OFFICE_DESK_01") &&
        Floppy144CabinetOpenParent(&sCabinetC,&sStateC,"MAIN_OFFICE_DESK_01"),
        "seeded-order fixture opens the same multi-item desk in three runs"
    );

    uCount=Floppy144CabinetVisibleContentCount(&sCabinetA,&sStateA);

    F144_CHECK(
        uCount>=2U &&
        Floppy144CabinetVisibleContentCount(&sCabinetB,&sStateB)==uCount &&
        Floppy144CabinetVisibleContentCount(&sCabinetC,&sStateC)==uCount,
        "seeded-order fixture has matching visible content sets"
    );

    for(uIndex=0U;uIndex<uCount;++uIndex)
    {
        const Floppy144DataRecord *pA=
            Floppy144CabinetVisibleContentAt(&sCabinetA,&sStateA,uIndex);
        const Floppy144DataRecord *pB=
            Floppy144CabinetVisibleContentAt(&sCabinetB,&sStateB,uIndex);
        const Floppy144DataRecord *pC=
            Floppy144CabinetVisibleContentAt(&sCabinetC,&sStateC,uIndex);

        F144_CHECK(
            pA!=NULL&&pB!=NULL&&pA->pszId!=NULL&&pB->pszId!=NULL&&
            strcmp(pA->pszId,pB->pszId)==0,
            "identical recovery seeds preserve identical container ordering"
        );

        if(
            pA!=NULL&&pC!=NULL&&pA->pszId!=NULL&&pC->pszId!=NULL&&
            strcmp(pA->pszId,pC->pszId)!=0
        )
        {
            bDifferent=true;
        }
    }

    F144_CHECK(
        uCount<2U || bDifferent,
        "different recovery seeds vary the order of a multi-item container"
    );
}

static void Floppy144TestShelvingPresentationAndRecoveredOrder(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    const Floppy144DataRecord *pBeforeFirst;
    const Floppy144DataRecord *pAfterFirst;
    const Floppy144DataRecord *pAfterLast;
    uint32_t uBeforeCount;
    uint32_t uAfterCount;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sState,
            FLOPPY144_ROOM_FACILITIES
        ),
        "shelving-order fixture reconstructs Facilities"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &sCabinet,
            &sState,
            "FACILITIES_SHELVING_03"
        ),
        "Facilities Shelving 03 opens reusable Contents screen"
    );

    F144_CHECK(
        strcmp(
            sCabinet.szContainerType,
            "SHELVING_FULL"
        ) == 0,
        "shelving retains a distinct SHELVING_FULL presentation type"
    );

    uBeforeCount =
        Floppy144CabinetVisibleContentCount(
            &sCabinet,
            &sState
        );

    pBeforeFirst =
        Floppy144CabinetVisibleContentAt(
            &sCabinet,
            &sState,
            0U
        );

    F144_CHECK(
        uBeforeCount >= 1U &&
        pBeforeFirst != NULL &&
        pBeforeFirst->pszId != NULL &&
        strcmp(
            pBeforeFirst->pszId,
            "P-035"
        ) != 0,
        "unrecovered Facilities evidence is not already first in shelving contents"
    );

    F144_CHECK(
        Floppy144TriggerTryFire(
            &sWorld,
            &sState,
            Floppy144GameDataTriggerId("T-008")
        ),
        "T-008 reveals Facilities closure material"
    );

    uAfterCount =
        Floppy144CabinetVisibleContentCount(
            &sCabinet,
            &sState
        );

    pAfterFirst =
        Floppy144CabinetVisibleContentAt(
            &sCabinet,
            &sState,
            0U
        );

    pAfterLast =
        uAfterCount > 0U
            ? Floppy144CabinetVisibleContentAt(
                &sCabinet,
                &sState,
                uAfterCount - 1U
            )
            : NULL;

    F144_CHECK(
        uAfterCount == uBeforeCount + 1U,
        "revealing P-035 adds one item to Shelving 03"
    );

    F144_CHECK(
        pBeforeFirst != NULL &&
        pAfterFirst != NULL &&
        pBeforeFirst->pszId != NULL &&
        pAfterFirst->pszId != NULL &&
        strcmp(
            pBeforeFirst->pszId,
            pAfterFirst->pszId
        ) == 0,
        "existing shelving order stays stable after a recovered item appears"
    );

    F144_CHECK(
        pAfterLast != NULL &&
        pAfterLast->pszId != NULL &&
        strcmp(
            pAfterLast->pszId,
            "P-035"
        ) == 0,
        "newly recovered shelving evidence is appended after existing clutter"
    );
}

static void Floppy144TestAllCorridorDoorContainers(void)
{
    typedef struct Floppy144DoorContainerFixture
    {
        const char *pszDoorId;
        const char *pszPlateId;
    }
    Floppy144DoorContainerFixture;

    static const Floppy144DoorContainerFixture asDoors[] =
    {
        { "COR_REC",   "P-152" },
        { "COR_RECO",  "P-158" },
        { "COR_OFF",   "P-153" },
        { "COR_SEC",   "P-156" },
        { "COR_IT",    "P-155" },
        { "COR_SECR",  "P-030" },
        { "COR_STAFF", "P-157" },
        { "COR_FAC",   "P-154" }
    };

    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    uint32_t uDoorIndex;

    Floppy144TestReset(
        &sWorld,
        &sState,
        &sCabinet
    );

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    /*
     * Room plates are permanent door furniture, not recovery effects. They
     * must already be present before T-005 unlocks COR_REC/COR_OFF.
     */
    F144_CHECK(
        !Floppy144RunStateTriggerFired(
            &sState,
            Floppy144GameDataTriggerId("T-005")
        ),
        "Corridor Door container fixture starts before T-005"
    );

    for(
        uDoorIndex = 0U;
        uDoorIndex < (uint32_t)(sizeof(asDoors) / sizeof(asDoors[0]));
        ++uDoorIndex
    )
    {
        const Floppy144DoorContainerFixture *pDoor =
            &asDoors[uDoorIndex];

        uint32_t uPlateIndex;

        Floppy144CabinetReset(
            &sCabinet
        );

        F144_CHECK(
            Floppy144CabinetOpenParent(
                &sCabinet,
                &sState,
                pDoor->pszDoorId
            ),
            "every internal Corridor door opens the reusable Door container"
        );

        F144_CHECK(
            strcmp(
                sCabinet.szContainerType,
                "DOOR"
            ) == 0,
            "Corridor door container identifies itself as DOOR"
        );

        uPlateIndex =
            Floppy144TestVisibleContentIndex(
                &sCabinet,
                &sState,
                pDoor->pszPlateId
            );

        F144_CHECK(
            uPlateIndex != UINT32_MAX,
            "Corridor Door container contains its office room plate"
        );
    }
}

static void Floppy144TestDoorParentDisplayName(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    Floppy144TriggerId eT003;
    Floppy144TriggerId eT005;
    const Floppy144DataRecord *pNotice;
    uint32_t uNoticeIndex;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sState,
            FLOPPY144_ROOM_MAIN_OFFICE
        ),
        "door-title fixture reconstructs Main Office"
    );

    eT003 = Floppy144GameDataTriggerId("T-003");
    eT005 = Floppy144GameDataTriggerId("T-005");

    F144_CHECK(
        eT003 < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(
            &sState,
            eT003
        ),
        "door-title fixture reveals Main Office corridor notice"
    );

    F144_CHECK(
        eT005 < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(
            &sState,
            eT005
        ),
        "door-title fixture reveals Corridor room plate"
    );

    Floppy144RunStateSetPlayerSitePosition(
        &sState,
        70 * FLOPPY144_SITE_FIXED_ONE,
        82 * FLOPPY144_SITE_FIXED_ONE
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &sCabinet,
            &sState,
            "COR_OFF"
        ),
        "door fixture opens reusable Contents screen"
    );

    F144_CHECK(
        strcmp(
            sCabinet.szDisplayName,
            "Door between Main Office and Corridor"
        ) == 0,
        "door Contents title uses room names instead of COR_OFF"
    );

    F144_CHECK(
        strstr(
            sCabinet.szDisplayName,
            "COR_OFF"
        ) == NULL,
        "door Contents title never exposes internal connection ID"
    );

    uNoticeIndex =
        Floppy144TestVisibleContentIndex(
            &sCabinet,
            &sState,
            "P-011"
        );

    pNotice =
        uNoticeIndex != UINT32_MAX
            ? Floppy144CabinetVisibleContentAt(
                &sCabinet,
                &sState,
                uNoticeIndex
            )
            : NULL;

    F144_CHECK(
        pNotice != NULL &&
        pNotice->pszF != NULL &&
        strstr(
            pNotice->pszF,
            "Corridor access"
        ) != NULL &&
        strstr(
            pNotice->pszF,
            "closure works"
        ) != NULL,
        "door notice exposes useful in-world text rather than a generic label"
    );
}

static void Floppy144TestPerCabinetUnlockPersistence(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144RunState sDecoded;
    Floppy144CabinetState sCabinet;
    char szExpected[FLOPPY144_CABINET_CODE_CAPACITY + 1U];
    uint8_t auPayload[FLOPPY144_SAVE_PAYLOAD_V2_SIZE];

    Floppy144TestReset(&sWorld, &sState, &sCabinet);

    F144_CHECK(
        Floppy144RunStateGrantCapability(
            &sState,
            FLOPPY144_CAPABILITY_MASTER_SECURE_CABINET_CODES
        ),
        "master-code fixture grants secure-cabinet capability"
    );

    Floppy144TestPlaceAtCabinet(&sState, "MAIN_OFFICE_SECURE_CABINET");

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sState),
        "Main Office cabinet opens keypad"
    );

    Floppy144CabinetExpectedCode(
        &sCabinet,
        sState.recovery_seed,
        szExpected,
        (uint32_t)sizeof(szExpected)
    );
    Floppy144TestDigitEntry(&sCabinet, szExpected);

    F144_CHECK(
        Floppy144CabinetSubmitCode(
            &sCabinet,
            &sWorld,
            &sState
        ),
        "exact generated Main Office cabinet code unlocks cabinet"
    );

    /*
     * Persistence rejects player coordinates that are not legal standing
     * positions. The cabinet targeting fixture deliberately places the player
     * at the centre of the generated cabinet rectangle, which is solid
     * furniture. Move back to the canonical Site spawn before exercising the
     * save payload so this test isolates cabinet-unlock persistence rather
     * than failing on unrelated collision validation.
     */
    {
        int32_t nSpawnX;
        int32_t nSpawnY;

        Floppy144SiteSpawnPosition(
            &nSpawnX,
            &nSpawnY
        );

        F144_CHECK(
            !Floppy144SitePositionBlocked(
                nSpawnX,
                nSpawnY
            ),
            "persistence fixture returns player to legal Site spawn"
        );

        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            nSpawnX,
            nSpawnY
        );
    }

    F144_CHECK(
        Floppy144PersistenceEncodeRunState(
            &sState,
            auPayload,
            (uint32_t)sizeof(auPayload)
        ),
        "cabinet unlock serialises into run-state payload"
    );

    F144_CHECK(
        Floppy144PersistenceDecodeRunState(
            &sDecoded,
            auPayload,
            (uint32_t)sizeof(auPayload)
        ),
        "cabinet unlock run-state payload decodes"
    );

    Floppy144CabinetReset(&sCabinet);
    Floppy144TestPlaceAtCabinet(&sDecoded, "MAIN_OFFICE_SECURE_CABINET");

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sDecoded) &&
        Floppy144CabinetInteriorOpen(&sCabinet) &&
        Floppy144CabinetUnlocked(&sCabinet, &sDecoded),
        "reinstated run returns previously unlocked cabinet directly to Interior"
    );
}

static void Floppy144TestRecoveredChildRevealsCabinetCode(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    Floppy144TriggerId eT014;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);

    eT014 = Floppy144GameDataTriggerId("T-014");
    F144_CHECK(
        eT014 < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(&sState, eT014),
        "Records reconciliation fixture reveals Cabinet 05 marker"
    );

    Floppy144TestPlaceAtCabinet(
        &sState,
        "RECORDS_OFFICE_SECURE_CABINET_05"
    );

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sState),
        "Records Office Cabinet 05 opens keypad"
    );

    F144_CHECK(
        Floppy144CabinetCodeKnown(&sCabinet, &sState),
        "revealed Cabinet 05 child recovers only that cabinet code"
    );

    Floppy144CabinetReset(&sCabinet);
    Floppy144TestPlaceAtCabinet(
        &sState,
        "RECORDS_OFFICE_SECURE_CABINET_06"
    );

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sState) &&
        !Floppy144CabinetCodeKnown(&sCabinet, &sState),
        "unrevealed neighbouring cabinet code remains unavailable"
    );
}

static void Floppy144TestSiteKeySetInteraction(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    Floppy144InteractionId eI035;
    Floppy144CapabilityId eSiteKeys;
    uint32_t uP093Index;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_SECURITY
    );

    /*
     * The real route reaches Security after T-027, which also unlocks COR_SEC.
     * Mirror that persistent state so ROOM_SECURITY means accessible, not only
     * reconstructed, when I-035 is evaluated from P-093.
     */
    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            Floppy144GameDataTriggerId("T-027")
        ),
        "Site-key fixture makes reconstructed Security accessible"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &sCabinet,
            &sState,
            "SECURITY_KEY_CABINET"
        ),
        "Security key cabinet opens reusable Contents view"
    );

    uP093Index = Floppy144TestVisibleContentIndex(
        &sCabinet,
        &sState,
        "P-093"
    );

    F144_CHECK(
        uP093Index != UINT32_MAX,
        "Complete Site key set is visible in Security key cabinet"
    );

    if(uP093Index == UINT32_MAX)
    {
        return;
    }

    Floppy144CabinetMoveSelection(
        &sCabinet,
        &sState,
        (int32_t)uP093Index
    );

    F144_CHECK(
        Floppy144CabinetInspectSelected(
            &sCabinet,
            &sWorld,
            &sState
        ),
        "inspecting Complete Site key set executes its interaction"
    );

    eI035 = Floppy144GameDataInteractionId("I-035");
    eSiteKeys = Floppy144GameDataCapabilityId("SITE_KEYS");

    F144_CHECK(
        eI035 < FLOPPY144_INTERACTION_COUNT &&
        Floppy144RunStateInteractionCompleted(
            &sState,
            eI035
        ) &&
        eSiteKeys < FLOPPY144_CAPABILITY_COUNT &&
        Floppy144RunStateHasCapability(
            &sState,
            eSiteKeys
        ),
        "P-093 inspection completes I-035 and grants Site keys"
    );
}

static void Floppy144TestFocusedCabinetAccess(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    const char *pszFocusedParentId;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECORDS_OFFICE
    );

    /*
     * Reproduce the reported crossover: trolley focus while Cabinet 06 was
     * still inside the former two-unit secure-cabinet Access radius.
     */
    Floppy144RunStateSetPlayerSitePosition(
        &sState,
        85 * FLOPPY144_SITE_FIXED_ONE,
        19 * FLOPPY144_SITE_FIXED_ONE
    );

    pszFocusedParentId = Floppy144SiteFocusedParentId(&sState);

    F144_CHECK(
        pszFocusedParentId != NULL &&
        strcmp(pszFocusedParentId, "RECORDS_OFFICE_TROLLEY") == 0 &&
        !Floppy144CabinetOpenNearby(&sCabinet, &sState),
        "Cabinet 06 cannot leak Access while trolley owns Site focus"
    );

    Floppy144TestPlaceAtCabinet(
        &sState,
        "RECORDS_OFFICE_SECURE_CABINET_06"
    );

    pszFocusedParentId = Floppy144SiteFocusedParentId(&sState);

    F144_CHECK(
        pszFocusedParentId != NULL &&
        strcmp(
            pszFocusedParentId,
            "RECORDS_OFFICE_SECURE_CABINET_06"
        ) == 0 &&
        Floppy144CabinetOpenNearby(&sCabinet, &sState) &&
        strcmp(
            Floppy144CabinetId(&sCabinet),
            "RECORDS_OFFICE_SECURE_CABINET_06"
        ) == 0,
        "Cabinet 06 Access returns when Cabinet 06 owns Site focus"
    );
}

static uint32_t Floppy144TestSurfaceHash(
    const uint32_t *pPixels,
    uint32_t uCount
)
{
    uint32_t uHash=2166136261U;
    uint32_t uIndex;

    for(uIndex=0U;uIndex<uCount;++uIndex)
    {
        uHash^=pPixels[uIndex];
        uHash*=16777619U;
    }

    return uHash;
}

static void Floppy144TestInspectionPresentationProgression(void)
{
    enum
    {
        TEST_WIDTH=640,
        TEST_HEIGHT=360
    };

    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;
    Floppy144Surface sSurface;
    const Floppy144SiteRect *pSecretaryDeskRect;
    uint32_t *pPixels;
    uint32_t uBasicHash;
    uint32_t uEnhancedHash;
    uint32_t uCountBefore;
    uint32_t uSelectedBefore;

    Floppy144TestReset(&sWorld,&sState,&sCabinet);

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sState,
            FLOPPY144_ROOM_RECEPTION
        ),
        "Inspection presentation fixture reconstructs Reception"
    );

    F144_CHECK(
        !Floppy144CabinetEnhancedPresentationUnlocked(
            &sState
        ),
        "2.5D Inspection presentation is locked before Main Office restoration"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &sCabinet,
            &sState,
            "RECEPTION_DESK"
        ),
        "pre-unlock Reception desk opens the established Contents framework"
    );

    uCountBefore=
        Floppy144CabinetVisibleContentCount(
            &sCabinet,
            &sState
        );

    if(uCountBefore>1U)
    {
        Floppy144CabinetMoveSelection(
            &sCabinet,
            &sState,
            1
        );
    }

    uSelectedBefore=
        sCabinet.uSelectedContent;

    pPixels=
        (uint32_t *)calloc(
            (size_t)TEST_WIDTH*(size_t)TEST_HEIGHT,
            sizeof(uint32_t)
        );

    F144_CHECK(
        pPixels!=NULL,
        "Inspection presentation framebuffer allocates"
    );

    if(pPixels==NULL)
    {
        return;
    }

    sSurface.pixels=pPixels;
    sSurface.width=TEST_WIDTH;
    sSurface.height=TEST_HEIGHT;

    Floppy144CabinetDraw(
        &sSurface,
        &sCabinet,
        &sState
    );

    uBasicHash=
        Floppy144TestSurfaceHash(
            pPixels,
            TEST_WIDTH*TEST_HEIGHT
        );

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sState,
            FLOPPY144_ROOM_MAIN_OFFICE
        ),
        "Main Office restoration succeeds for Inspection presentation unlock"
    );

    F144_CHECK(
        Floppy144CabinetEnhancedPresentationUnlocked(
            &sState
        ),
        "Main Office restoration unlocks 2.5D Inspection presentation"
    );

    memset(
        pPixels,
        0,
        (size_t)TEST_WIDTH*(size_t)TEST_HEIGHT*sizeof(uint32_t)
    );

    Floppy144CabinetDraw(
        &sSurface,
        &sCabinet,
        &sState
    );

    uEnhancedHash=
        Floppy144TestSurfaceHash(
            pPixels,
            TEST_WIDTH*TEST_HEIGHT
        );

    F144_CHECK(
        uBasicHash!=uEnhancedHash,
        "Main Office restoration changes only the parent presentation layer"
    );

    F144_CHECK(
        Floppy144CabinetVisibleContentCount(
            &sCabinet,
            &sState
        )==uCountBefore &&
        sCabinet.uSelectedContent==uSelectedBefore,
        "presentation unlock preserves contents count and current selection"
    );

    pSecretaryDeskRect=
        Floppy144SiteRectForParentId(
            "SECRETARY_OFFICE_DESK"
        );

    F144_CHECK(
        pSecretaryDeskRect!=NULL &&
        pSecretaryDeskRect->rotation==135 &&
        pSecretaryDeskRect->authored_width16==
            6U*FLOPPY144_SITE_FIXED_ONE &&
        pSecretaryDeskRect->authored_height16==
            4U*FLOPPY144_SITE_FIXED_ONE,
        "Inspection renderer can recover authored diagonal Secretary desk geometry"
    );

    free(pPixels);
}

static void Floppy144TestActLengthContract(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144CabinetState sCabinet;

    Floppy144TestReset(&sWorld, &sState, &sCabinet);
    Floppy144TestPlaceAtCabinet(&sState, "RECORDS_OFFICE_SECURE_CABINET_01");

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sState) &&
        Floppy144CabinetRequiredDigits(&sCabinet) == 6U,
        "mid-recovery Records cabinet requires six digits"
    );

    Floppy144CabinetReset(&sCabinet);
    Floppy144TestPlaceAtCabinet(&sState, "DIRECTOR_OFFICE_SECURE_CABINET");

    F144_CHECK(
        Floppy144CabinetOpenNearby(&sCabinet, &sState) &&
        Floppy144CabinetRequiredDigits(&sCabinet) == 6U,
        "Director cabinet uses authored six-digit code metadata"
    );
}

/*
 * S4E-03: exercise the real canonical noticeboard, date-window data and the
 * same transient Cabinet Interior path as the player. Dates are injected,
 * never read from the CI/developer machine.
 */
static void Floppy144TestNoticeboardCalendar(void)
{
    static const struct
    {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        const char *ambient_id;
    } dates[] =
    {
        { 2026U, 1U, 1U, "AMB-NB-01" },   /* New Year */
        { 2026U, 1U, 15U, "AMB-NB-08" },  /* ordinary winter */
        { 2026U, 2U, 14U, "AMB-NB-02" },  /* Valentine */
        { 2026U, 3U, 31U, "AMB-NB-03" },  /* spring/Easter window */
        { 2026U, 4U, 21U, "AMB-NB-08" },  /* ordinary spring */
        { 2026U, 7U, 15U, "AMB-NB-04" },  /* summer */
        { 2026U, 9U, 15U, "AMB-NB-08" },  /* ordinary autumn */
        { 2026U, 10U, 31U, "AMB-NB-05" }, /* Halloween */
        { 2026U, 11U, 5U, "AMB-NB-06" },  /* Bonfire */
        { 2026U, 12U, 20U, "AMB-NB-07" }, /* Christmas */
        { 2026U, 12U, 31U, "AMB-NB-01" }  /* year-end wrap */
    };
    static const char *permanent[] =
    {
        "P-075", "P-078", "P-138",
        "P-329", "P-330", "P-331", "P-332", "P-333", "P-334"
    };
    Floppy144WorldState world;
    Floppy144RunState run;
    Floppy144RunState before;
    Floppy144CabinetState cabinet;
    Floppy144CabinetState alternate;
    F144CalendarDate date;
    uint32_t i;
    uint32_t j;
    uint32_t original_count;
    uint32_t guarded[640U * 360U + 2U];
    Floppy144Surface surface;
    const Floppy144DataRecord *original_social =
        Floppy144GameDataFind(FLOPPY144_DATA_PHYSICAL_ITEM, "P-138");

    F144_CHECK(
        original_social != NULL && original_social->pszF != NULL &&
        strstr(original_social->pszF, "Final-week staff tea") != NULL,
        "permanent authored P-138 still contains its original final-week text"
    );

    Floppy144TestReset(&world, &run, &cabinet);
    (void)Floppy144RunStateReconstructRoom(&run, FLOPPY144_ROOM_STAFF_ROOM);
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet, &run, "STAFF_ROOM_NOTICEBOARD"
        ),
        "canonical Staff Room Noticeboard opens normally"
    );
    /*
     * Stage 3 progression hides P-075, P-078 and P-138 until T-012.
     * Seasonality must not reveal those canonical physical items early.
     */
    original_count = Floppy144CabinetVisibleContentCount(&cabinet, &run);
    F144_CHECK(
        original_count == 6U &&
        Floppy144TestVisibleContentIndex(&cabinet, &run, "P-138") == UINT32_MAX,
        "before T-012 the three story-controlled notices remain hidden"
    );

    date.year = 2026U;
    date.month = 12U;
    date.day = 20U;
    Floppy144CabinetSetNoticeboardDate(&cabinet, &run, &date);
    F144_CHECK(
        Floppy144CabinetVisibleContentCount(&cabinet, &run) == 7U &&
        Floppy144TestVisibleContentIndex(&cabinet, &run, "P-138") == UINT32_MAX &&
        Floppy144CabinetVisibleContentAt(&cabinet, &run, 6U) != NULL,
        "contextual flyer appears independently without revealing T-012 children"
    );

    {
        Floppy144TriggerId trigger = Floppy144GameDataTriggerId("T-012");
        F144_CHECK(
            trigger != FLOPPY144_TRIGGER_COUNT &&
            Floppy144RunStateFireTrigger(&run, trigger),
            "test fixture completes the existing T-012 reveal condition"
        );
    }
    original_count = Floppy144CabinetVisibleContentCount(&cabinet, &run) - 1U;
    F144_CHECK(
        original_count == (uint32_t)(sizeof(permanent) / sizeof(permanent[0])),
        "all nine authored physical notices become visible after T-012"
    );
    for(j = 0U; j < (uint32_t)(sizeof(permanent) / sizeof(permanent[0])); ++j)
    {
        F144_CHECK(
            Floppy144TestVisibleContentIndex(&cabinet, &run, permanent[j]) != UINT32_MAX,
            "all canonical permanent physical notices retain their IDs"
        );
    }

    for(i = 0U; i < (uint32_t)(sizeof(dates) / sizeof(dates[0])); ++i)
    {
        const Floppy144DataRecord *flyer;
        const char *first_annotation;
        uint32_t original_indices[9];

        for(j = 0U; j < 9U; ++j)
        {
            original_indices[j] =
                Floppy144TestVisibleContentIndex(&cabinet, &run, permanent[j]);
        }

        date.year = dates[i].year;
        date.month = dates[i].month;
        date.day = dates[i].day;

        Floppy144CabinetSetNoticeboardDate(&cabinet, &run, &date);
        F144_CHECK(
            Floppy144CabinetVisibleContentCount(&cabinet, &run) ==
                original_count + 1U,
            "seasonal flyer appends without suppressing permanent entries"
        );

        flyer = Floppy144CabinetVisibleContentAt(&cabinet, &run, original_count);
        F144_CHECK(
            flyer != NULL && flyer->pszId != NULL &&
            strcmp(flyer->pszId, dates[i].ambient_id) == 0 &&
            flyer->pszF != NULL &&
            cabinet.pszContextualAnnotation != NULL,
            "calendar date chooses the expected authored contextual flyer"
        );

        first_annotation = cabinet.pszContextualAnnotation;
        Floppy144CabinetSetNoticeboardDate(&cabinet, &run, &date);
        F144_CHECK(
            cabinet.pszContextualAnnotation == first_annotation &&
            Floppy144CabinetVisibleContentAt(&cabinet, &run, original_count) != NULL,
            "same seed and date selects identical stable flyer and marginalia"
        );

        for(j = 0U; j < 9U; ++j)
        {
            F144_CHECK(
                Floppy144TestVisibleContentIndex(&cabinet, &run, permanent[j]) ==
                    original_indices[j],
                "seasonal flyer never reorders an authored permanent notice"
            );
        }

        cabinet.uSelectedContent = original_count;
        before = run;
        F144_CHECK(
            Floppy144CabinetInspectSelected(&cabinet, &world, &run) &&
            Floppy144CabinetDetailOpen(&cabinet) &&
            memcmp(&run, &before, sizeof(run)) == 0,
            "inspecting contextual flyer does not change gameplay RunState"
        );

        /* The original detail/list renderer must not exceed the framebuffer. */
        memset(guarded, 0, sizeof(guarded));
        guarded[0] = 0x13579BDFU;
        guarded[640U * 360U + 1U] = 0x2468ACE0U;
        surface.pixels = &guarded[1];
        surface.width = 640U;
        surface.height = 360U;
        Floppy144CabinetDraw(&surface, &cabinet, &run);
        F144_CHECK(
            guarded[0] == 0x13579BDFU &&
            guarded[640U * 360U + 1U] == 0x2468ACE0U,
            "seasonal detail renderer stays within 640x360 surface"
        );
        F144_CHECK(
            Floppy144CabinetBackspace(&cabinet) &&
            !Floppy144CabinetDetailOpen(&cabinet),
            "Backspace returns from seasonal flyer to the scrolling contents"
        );
        cabinet.uSelectedContent = 0U;
    }

    /* Different seeds can select different marginalia without changing flyer. */
    date.year = 2026U;
    date.month = 12U;
    date.day = 20U;
    Floppy144CabinetSetNoticeboardDate(&cabinet, &run, &date);
    Floppy144TestReset(&world, &before, &alternate);
    Floppy144RunStateBegin(&before, 145U);
    (void)Floppy144RunStateReconstructRoom(&before, FLOPPY144_ROOM_STAFF_ROOM);
    F144_CHECK(
        Floppy144CabinetOpenParent(&alternate, &before, "STAFF_ROOM_NOTICEBOARD"),
        "second seeded run opens the same noticeboard"
    );
    Floppy144CabinetSetNoticeboardDate(&alternate, &before, &date);
    F144_CHECK(
        cabinet.sContextualFlyer.pszId != NULL &&
        alternate.sContextualFlyer.pszId != NULL &&
        strcmp(cabinet.sContextualFlyer.pszId, alternate.sContextualFlyer.pszId) == 0 &&
        strcmp(cabinet.pszContextualAnnotation, alternate.pszContextualAnnotation) != 0,
        "different recovery seeds alter only atmospheric marginalia"
    );

    date.month = 0U;
    Floppy144CabinetSetNoticeboardDate(&cabinet, &run, &date);
    F144_CHECK(
        Floppy144CabinetVisibleContentCount(&cabinet, &run) == original_count,
        "invalid date falls back to all permanent physical notices"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(&cabinet, &run, "STAFF_ROOM_WORKTOP"),
        "unrelated parent opens normally"
    );
    original_count = Floppy144CabinetVisibleContentCount(&cabinet, &run);
    date.month = 12U;
    Floppy144CabinetSetNoticeboardDate(&cabinet, &run, &date);
    F144_CHECK(
        Floppy144CabinetVisibleContentCount(&cabinet, &run) == original_count &&
        cabinet.sContextualFlyer.pszId == NULL,
        "date-aware additions never affect non-noticeboard contents"
    );
}

/*
 * Stage 4E-04: actual Cabinet Interior P-330, canonical generated data,
 * interaction-free inspection and the frozen V2 on-disk RunState codec.
 */
static void Floppy144TestTakeawayMenuPresentation(void)
{
    Floppy144WorldState world;
    Floppy144RunState run;
    Floppy144RunState snapshot;
    Floppy144RunState loaded;
    Floppy144RunState second_run;
    Floppy144CabinetState cabinet;
    Floppy144CabinetState reloaded_cabinet;
    Floppy144CabinetState different_cabinet;
    const Floppy144DataRecord *original;
    const Floppy144DataRecord *menu;
    const Floppy144DataRecord *after_reload;
    uint8_t save_payload[FLOPPY144_SAVE_PAYLOAD_V2_SIZE];
    static uint32_t guarded[640U * 360U + 2U];
    Floppy144Surface screen;
    uint32_t index;
    uint32_t count;
    char first_menu[FLOPPY144_TAKEAWAY_MENU_CAPACITY];

    Floppy144TestReset(&world, &run, &cabinet);
    original = Floppy144GameDataFind(FLOPPY144_DATA_PHYSICAL_ITEM, "P-330");

    F144_CHECK(
        original != NULL && original->pszA != NULL &&
        strcmp(original->pszA, "Takeaway menu") == 0 &&
        original->pszF != NULL &&
        strcmp(original->pszF,
            "Takeaway menu left with this noticeboard in Staff Room.") == 0 &&
        original->pszC != NULL &&
        strcmp(original->pszC, "STAFF_ROOM_NOTICEBOARD") == 0 &&
        Floppy144InteractionForPhysicalSource("P-330") ==
            FLOPPY144_INTERACTION_COUNT,
        "P-330 remains its original flavour-only generated PI and has no interaction"
    );

    F144_CHECK(
        Floppy144RunStateReconstructRoom(&run, FLOPPY144_ROOM_STAFF_ROOM) &&
        Floppy144CabinetOpenParent(&cabinet, &run, "STAFF_ROOM_NOTICEBOARD"),
        "the real Staff Room noticeboard is accessible"
    );
    count = Floppy144CabinetVisibleContentCount(&cabinet, &run);
    index = Floppy144TestVisibleContentIndex(&cabinet, &run, "P-330");
    menu = index < count
        ? Floppy144CabinetVisibleContentAt(&cabinet, &run, index)
        : NULL;

    F144_CHECK(
        menu != NULL &&
        menu == &cabinet.sGeneratedTakeaway &&
        menu->pszId != NULL && strcmp(menu->pszId, "P-330") == 0 &&
        menu->pszA != NULL && strcmp(menu->pszA, "Takeaway menu") == 0 &&
        menu->pszF == cabinet.szTakeawayText &&
        strchr(menu->pszF, '\n') != NULL,
        "original P-330 list item shows generated text only on inspection"
    );
    if(menu == NULL || menu->pszF == NULL)
        return;

    (void)snprintf(first_menu, sizeof(first_menu), "%s", menu->pszF);

    /* Interactions and story flags are completely untouched by menu viewing. */
    snapshot = run;
    cabinet.uSelectedContent = index;
    F144_CHECK(
        Floppy144CabinetInspectSelected(&cabinet, &world, &run) &&
        Floppy144CabinetDetailOpen(&cabinet) &&
        memcmp(&snapshot, &run, sizeof(run)) == 0,
        "takeaway-menu inspection never changes gameplay RunState"
    );

    memset(guarded, 0, sizeof(guarded));
    guarded[0] = 0x13579BDFU;
    guarded[640U * 360U + 1U] = 0x2468ACE0U;
    screen.pixels = &guarded[1];
    screen.width = 640U;
    screen.height = 360U;
    Floppy144CabinetDraw(&screen, &cabinet, &run);
    F144_CHECK(
        guarded[0] == 0x13579BDFU &&
        guarded[640U * 360U + 1U] == 0x2468ACE0U,
        "menu inspection stays inside the 640x360 framebuffer"
    );
    F144_CHECK(
        Floppy144CabinetBackspace(&cabinet) &&
        !Floppy144CabinetDetailOpen(&cabinet),
        "menu Backspace returns to the regular scrolling contents"
    );

    (void)Floppy144VariationValue(
        run.recovery_seed, "staff.noticeboard.annotation.v1", "AMB-NB-07"
    );
    (void)Floppy144VariationValue(
        run.recovery_seed, "takeaway.unrelated.future.v1", "OTHER_ITEM"
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(&cabinet, &run, "STAFF_ROOM_NOTICEBOARD") &&
        strcmp(first_menu, cabinet.szTakeawayText) == 0 &&
        Floppy144CabinetVisibleContentCount(&cabinet, &run) == count,
        "reopening after other variation calls preserves menu and PI count"
    );

    F144_CHECK(
        Floppy144PersistenceEncodeRunState(
            &run, save_payload, (uint32_t)sizeof(save_payload)
        ) &&
        Floppy144PersistenceDecodeRunState(
            &loaded, save_payload, (uint32_t)sizeof(save_payload)
        ),
        "V2 save/reload codec round-trips the existing recovery seed"
    );
    F144_CHECK(
        loaded.recovery_seed == run.recovery_seed &&
        Floppy144CabinetOpenParent(
            &reloaded_cabinet, &loaded, "STAFF_ROOM_NOTICEBOARD"
        ),
        "decoded saved run reopens the identical noticeboard"
    );
    index = Floppy144TestVisibleContentIndex(
        &reloaded_cabinet, &loaded, "P-330"
    );
    after_reload = index != UINT32_MAX
        ? Floppy144CabinetVisibleContentAt(
            &reloaded_cabinet, &loaded, index
        )
        : NULL;
    F144_CHECK(
        after_reload != NULL &&
        after_reload->pszF != NULL &&
        strcmp(first_menu, after_reload->pszF) == 0,
        "save/reload produces exactly the same menu wording"
    );

    Floppy144RunStateBegin(&second_run, 145U);
    (void)Floppy144RunStateReconstructRoom(
        &second_run, FLOPPY144_ROOM_STAFF_ROOM
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &different_cabinet, &second_run, "STAFF_ROOM_NOTICEBOARD"
        ) &&
        strcmp(first_menu, different_cabinet.szTakeawayText) != 0,
        "different recovery seed changes the takeaway menu"
    );

    F144_CHECK(
        original != NULL &&
        strcmp(original->pszF,
            "Takeaway menu left with this noticeboard in Staff Room.") == 0,
        "generated menu never modifies canonical authored source data"
    );
}


/*
 * S4E-05 tests the actual 3B.5 Cabinet/PI reveal, not a simulated UI.
 * The canonical P-074/HR-05/T-012 game data remains the source of truth.
 */
static void Floppy144TestCrosswordPresentation(void)
{
    Floppy144WorldState world;
    Floppy144RunState run;
    Floppy144RunState before;
    Floppy144RunState loaded;
    Floppy144RunState alternate_run;
    Floppy144CabinetState cabinet;
    Floppy144CabinetState reloaded_cabinet;
    Floppy144CabinetState alternate_cabinet;
    const Floppy144DataRecord *original =
        Floppy144GameDataFind(FLOPPY144_DATA_PHYSICAL_ITEM, "P-074");
    const Floppy144DataRecord *crossword;
    Floppy144TriggerId trigger =
        Floppy144GameDataTriggerId("T-012");
    static uint32_t guarded[640U * 360U + 2U];
    Floppy144Surface surface;
    uint8_t save_payload[FLOPPY144_SAVE_PAYLOAD_V2_SIZE];
    Floppy144CrosswordView first_view;
    uint32_t before_count;
    uint32_t after_count;
    uint32_t selected;

    F144_CHECK(
        original != NULL &&
        original->pszId != NULL &&
        strcmp(original->pszId, "P-074") == 0 &&
        original->pszA != NULL &&
        strcmp(original->pszA, "Half-finished crossword") == 0 &&
        original->pszC != NULL &&
        strcmp(original->pszC, "STAFF_ROOM_COFFEE_TABLE_01") == 0 &&
        original->pszF != NULL &&
        strcmp(original->pszF,
            "Half-finished crossword. Recovered in Staff Room.") == 0 &&
        Floppy144InteractionForPhysicalSource("P-074") ==
            FLOPPY144_INTERACTION_COUNT,
        "canonical P-074 original ID/title/parent/text and no interaction"
    );
    F144_CHECK(
        trigger != FLOPPY144_TRIGGER_COUNT,
        "existing T-012 reveal trigger is present"
    );

    Floppy144TestReset(&world, &run, &cabinet);
    (void)Floppy144RunStateReconstructRoom(
        &run, FLOPPY144_ROOM_STAFF_ROOM
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet, &run, "STAFF_ROOM_COFFEE_TABLE_01"
        ),
        "Coffee Table 01 uses the existing inspection screen"
    );
    before_count = Floppy144CabinetVisibleContentCount(
        &cabinet, &run
    );
    F144_CHECK(
        Floppy144TestVisibleContentIndex(&cabinet, &run, "P-074") ==
            UINT32_MAX &&
        cabinet.sGeneratedCrossword.pszId == NULL,
        "before T-012 the hidden crossword is neither generated nor visible"
    );

    F144_CHECK(
        trigger < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(&run, trigger),
        "original story progression reveals P-074"
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet, &run, "STAFF_ROOM_COFFEE_TABLE_01"
        ),
        "Coffee Table 01 is inspectable after original reveal"
    );
    after_count = Floppy144CabinetVisibleContentCount(&cabinet, &run);
    selected = Floppy144TestVisibleContentIndex(
        &cabinet, &run, "P-074"
    );
    crossword = selected != UINT32_MAX
        ? Floppy144CabinetVisibleContentAt(&cabinet, &run, selected)
        : NULL;
    F144_CHECK(
        after_count == before_count + 1U &&
        crossword == &cabinet.sGeneratedCrossword &&
        crossword->pszId != NULL &&
        strcmp(crossword->pszId, "P-074") == 0 &&
        crossword->pszA != NULL &&
        strcmp(crossword->pszA, "Half-finished crossword") == 0 &&
        cabinet.sCrosswordView.variant == 4U &&
        strcmp(cabinet.sCrosswordView.across_answer, "ORDER") == 0 &&
        strcmp(cabinet.sCrosswordView.down_answer, "INDEX") == 0,
        "after T-012 the canonical slot shows seed-144 crossing"
    );

    first_view = cabinet.sCrosswordView;
    before = run;
    cabinet.uSelectedContent = selected;
    F144_CHECK(
        Floppy144CabinetInspectSelected(&cabinet, &world, &run) &&
        Floppy144CabinetDetailOpen(&cabinet) &&
        memcmp(&before, &run, sizeof(run)) == 0,
        "inspecting P-074 changes no RunState flags, clues or counters"
    );

    memset(guarded, 0, sizeof(guarded));
    guarded[0] = 0x13579BDFU;
    guarded[640U * 360U + 1U] = 0x2468ACE0U;
    surface.pixels = &guarded[1];
    surface.width = 640U;
    surface.height = 360U;
    Floppy144CabinetDraw(&surface, &cabinet, &run);
    F144_CHECK(
        guarded[0] == 0x13579BDFU &&
        guarded[640U * 360U + 1U] == 0x2468ACE0U,
        "five-by-five display keeps the 640x360 framebuffer canaries intact"
    );

    F144_CHECK(
        Floppy144CabinetBackspace(&cabinet) &&
        !Floppy144CabinetDetailOpen(&cabinet),
        "Backspace returns from the crossword to Contents"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet, &run, "STAFF_ROOM_COFFEE_TABLE_01"
        ) &&
        memcmp(&cabinet.sCrosswordView, &first_view, sizeof(first_view)) == 0 &&
        Floppy144CabinetVisibleContentCount(&cabinet, &run) == after_count,
        "reopening the same run gives an identical crossword and PI count"
    );

    F144_CHECK(
        Floppy144PersistenceEncodeRunState(
            &run, save_payload, (uint32_t)sizeof(save_payload)
        ) &&
        Floppy144PersistenceDecodeRunState(
            &loaded, save_payload, (uint32_t)sizeof(save_payload)
        ),
        "the unchanged V2 save codec round-trips the crossword recovery seed"
    );
    F144_CHECK(
        loaded.recovery_seed == run.recovery_seed &&
        Floppy144CabinetOpenParent(
            &reloaded_cabinet, &loaded, "STAFF_ROOM_COFFEE_TABLE_01"
        ) &&
        memcmp(
            &reloaded_cabinet.sCrosswordView,
            &first_view,
            sizeof(first_view)
        ) == 0,
        "save/reload regenerates an identical crossing, pencil fills and note"
    );

    Floppy144RunStateBegin(&alternate_run, 145U);
    (void)Floppy144RunStateReconstructRoom(
        &alternate_run, FLOPPY144_ROOM_STAFF_ROOM
    );
    F144_CHECK(
        trigger < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(&alternate_run, trigger) &&
        Floppy144CabinetOpenParent(
            &alternate_cabinet,
            &alternate_run,
            "STAFF_ROOM_COFFEE_TABLE_01"
        ) &&
        alternate_cabinet.sCrosswordView.variant == 3U &&
        strcmp(alternate_cabinet.sCrosswordView.across_answer, "QUEUE") == 0 &&
        strcmp(alternate_cabinet.sCrosswordView.down_answer, "SHELF") == 0,
        "seed 145 gives the different QUEUE/SHELF crossing"
    );

    F144_CHECK(
        original != NULL &&
        strcmp(original->pszF,
            "Half-finished crossword. Recovered in Staff Room.") == 0,
        "the original physical-item record is still unmodified"
    );

    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet, &run, "STAFF_ROOM_COFFEE_TABLE_02"
        ) &&
        cabinet.sGeneratedCrossword.pszId == NULL,
        "Coffee Table 02's completed P-137 is never replaced"
    );
}


/*
 * S4E-06 tests canonical P-073 in the actual Stage 3B.5 Bookcase UI.
 * No new item/reveal state is introduced by cover generation.
 */
static void Floppy144TestPaperbackPresentation(void)
{
    Floppy144WorldState world;
    Floppy144RunState run;
    Floppy144RunState before;
    Floppy144RunState loaded;
    Floppy144RunState alternate_run;
    Floppy144CabinetState cabinet;
    Floppy144CabinetState loaded_cabinet;
    Floppy144CabinetState alternate_cabinet;
    const Floppy144DataRecord *original =
        Floppy144GameDataFind(FLOPPY144_DATA_PHYSICAL_ITEM, "P-073");
    const Floppy144DataRecord *paperback;
    Floppy144TriggerId trigger = Floppy144GameDataTriggerId("T-012");
    uint8_t encoded[FLOPPY144_SAVE_PAYLOAD_V2_SIZE];
    static uint32_t guard[640U * 360U + 2U];
    Floppy144Surface screen;
    char original_cover[FLOPPY144_PAPERBACK_TEXT_CAPACITY];
    uint32_t previous_count;
    uint32_t selected;
    uint32_t after_count;

    F144_CHECK(
        original != NULL &&
        original->pszId != NULL &&
        strcmp(original->pszId, "P-073") == 0 &&
        original->pszA != NULL &&
        strcmp(original->pszA, "Dog-eared paperback") == 0 &&
        original->pszC != NULL &&
        strcmp(original->pszC, "STAFF_ROOM_BOOKCASE_02") == 0 &&
        original->pszF != NULL &&
        strcmp(original->pszF,
            "Dog-eared paperback. Recovered in Staff Room.") == 0 &&
        Floppy144InteractionForPhysicalSource("P-073") ==
            FLOPPY144_INTERACTION_COUNT,
        "canonical P-073 remains a non-interactive dog-eared paperback"
    );

    Floppy144TestReset(&world, &run, &cabinet);
    (void)Floppy144RunStateReconstructRoom(
        &run, FLOPPY144_ROOM_STAFF_ROOM
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet, &run, "STAFF_ROOM_BOOKCASE_02"
        ),
        "Coffee-room Bookcase 02 uses original Cabinet Interior screen"
    );
    previous_count = Floppy144CabinetVisibleContentCount(
        &cabinet, &run
    );
    F144_CHECK(
        trigger != FLOPPY144_TRIGGER_COUNT &&
        Floppy144TestVisibleContentIndex(&cabinet, &run, "P-073") ==
            UINT32_MAX &&
        cabinet.sGeneratedPaperback.pszId == NULL,
        "before T-012 paperback cover is not generated and P-073 is hidden"
    );

    F144_CHECK(
        trigger < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(&run, trigger),
        "original T-012 progression reveals the paperback"
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet, &run, "STAFF_ROOM_BOOKCASE_02"
        ),
        "reopening Bookcase 02 after reveal uses existing Cabinet routing"
    );
    after_count = Floppy144CabinetVisibleContentCount(
        &cabinet, &run
    );
    selected = Floppy144TestVisibleContentIndex(
        &cabinet, &run, "P-073"
    );
    paperback = selected != UINT32_MAX
        ? Floppy144CabinetVisibleContentAt(&cabinet, &run, selected)
        : NULL;
    F144_CHECK(
        after_count == previous_count + 1U &&
        paperback == &cabinet.sGeneratedPaperback &&
        paperback->pszId != NULL &&
        strcmp(paperback->pszId, "P-073") == 0 &&
        paperback->pszA != NULL &&
        strcmp(paperback->pszA, "Dog-eared paperback") == 0 &&
        paperback->pszF == cabinet.szPaperbackText &&
        strcmp(paperback->pszF,
            "TITLE: SENIOR LINE MANAGER\n"
            "BY: CRISPIN QUIBBLE\n"
            "THE LAST PAGE IS A LEAVE REQUEST") == 0,
        "original P-073 list identity and slot have seed-144 cover on Inspect"
    );
    if(paperback == NULL || paperback->pszF == NULL)
        return;

    (void)snprintf(original_cover,sizeof(original_cover),"%s",paperback->pszF);
    before = run;
    cabinet.uSelectedContent = selected;
    F144_CHECK(
        Floppy144CabinetInspectSelected(&cabinet,&world,&run) &&
        Floppy144CabinetDetailOpen(&cabinet) &&
        memcmp(&before,&run,sizeof(run)) == 0,
        "reading paperback cover causes no gameplay RunState mutation"
    );

    memset(guard,0,sizeof(guard));
    guard[0]=0x13579BDFU;
    guard[640U * 360U + 1U]=0x2468ACE0U;
    screen.pixels=&guard[1];
    screen.width=640U;
    screen.height=360U;
    Floppy144CabinetDraw(&screen,&cabinet,&run);
    F144_CHECK(
        guard[0]==0x13579BDFU &&
        guard[640U * 360U + 1U]==0x2468ACE0U,
        "paperback detail render stays inside 640x360 bitmap"
    );
    F144_CHECK(
        Floppy144CabinetBackspace(&cabinet) &&
        !Floppy144CabinetDetailOpen(&cabinet),
        "paperback detail exits to normal Bookcase Contents"
    );

    (void)Floppy144VariationValue(
        run.recovery_seed, "takeaway.name.business.v1", "P-330"
    );
    (void)Floppy144VariationValue(
        run.recovery_seed, "staff.crossword.pattern.v1", "P-074"
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet,&run,"STAFF_ROOM_BOOKCASE_02"
        ) &&
        strcmp(cabinet.szPaperbackText,original_cover)==0 &&
        Floppy144CabinetVisibleContentCount(&cabinet,&run)==after_count,
        "reopening after other flavour services keeps same title and item count"
    );

    F144_CHECK(
        Floppy144PersistenceEncodeRunState(
            &run,encoded,(uint32_t)sizeof(encoded)
        ) &&
        Floppy144PersistenceDecodeRunState(
            &loaded,encoded,(uint32_t)sizeof(encoded)
        ),
        "existing V2 save codec round-trips without paperback data fields"
    );
    F144_CHECK(
        loaded.recovery_seed == run.recovery_seed &&
        Floppy144CabinetOpenParent(
            &loaded_cabinet,&loaded,"STAFF_ROOM_BOOKCASE_02"
        ) &&
        strcmp(original_cover,loaded_cabinet.szPaperbackText)==0,
        "save/reload regenerates identical cover and byline from saved seed"
    );

    Floppy144RunStateBegin(&alternate_run,145U);
    (void)Floppy144RunStateReconstructRoom(
        &alternate_run,FLOPPY144_ROOM_STAFF_ROOM
    );
    F144_CHECK(
        trigger < FLOPPY144_TRIGGER_COUNT &&
        Floppy144RunStateFireTrigger(&alternate_run,trigger) &&
        Floppy144CabinetOpenParent(
            &alternate_cabinet,&alternate_run,"STAFF_ROOM_BOOKCASE_02"
        ) &&
        strcmp(alternate_cabinet.szPaperbackText,
            "TITLE: HEARTBROKEN ARCHIVE CLERK\n"
            "BY: MILLICENT MUDDLE\n"
            "THE LAST PAGE IS A LEAVE REQUEST")==0 &&
        strcmp(original_cover,alternate_cabinet.szPaperbackText)!=0,
        "known seed 145 regenerates different cover and author"
    );
    F144_CHECK(
        original != NULL &&
        strcmp(original->pszF,
            "Dog-eared paperback. Recovered in Staff Room.") == 0,
        "original generated P-073 description is never overwritten"
    );
    F144_CHECK(
        Floppy144CabinetOpenParent(
            &cabinet,&run,"STAFF_ROOM_BOOKCASE_01"
        ) &&
        cabinet.sGeneratedPaperback.pszId==NULL,
        "other Bookcase contents never receive a generated paperback"
    );
}


/*
 * S4G-04: pixel-level no-scar proof.
 *
 * Render at the *same* corridor stance in all three valid states, with the
 * same seed, camera, HUD and player. UNAVAILABLE and COMPLETED must produce
 * byte-for-byte identical 640x360 frames; AVAILABLE must differ. The Site
 * Directory must remain identical through the full lifecycle.
 */
static void Floppy144TestGreyDoorNoScar(void)
{
    uint32_t *before_pixels=(uint32_t *)malloc(640U*360U*sizeof(uint32_t));
    uint32_t *available_pixels=(uint32_t *)malloc(640U*360U*sizeof(uint32_t));
    uint32_t *after_pixels=(uint32_t *)malloc(640U*360U*sizeof(uint32_t));
    Floppy144Surface before_surface,available_surface,after_surface;
    Floppy144RunState run;
    Floppy144GreyDoorCandidate candidate;
    uint32_t original_rect_count=Floppy144SiteRectCount();
    uint32_t i;
    static const uint32_t seeds[]={144U,146U,2026U,0x12345678U};

    F144_CHECK(before_pixels!=NULL && available_pixels!=NULL &&
        after_pixels!=NULL,"S4G no-scar frame buffers can be allocated");
    if(before_pixels==NULL || available_pixels==NULL || after_pixels==NULL)
    {
        free(before_pixels);free(available_pixels);free(after_pixels);
        return;
    }
    memset(&before_surface,0,sizeof(before_surface));
    before_surface.pixels=before_pixels;
    before_surface.width=640U;
    before_surface.height=360U;
    available_surface=before_surface;
    available_surface.pixels=available_pixels;
    after_surface=before_surface;
    after_surface.pixels=after_pixels;

    for(i=0U;i<(uint32_t)(sizeof(seeds)/sizeof(seeds[0]));++i)
    {
        Floppy144RunStateBegin(&run,seeds[i]);
        F144_CHECK(Floppy144RunStateReconstructRoom(
            &run,FLOPPY144_ROOM_CORRIDOR),
            "S4G room restoration initialises the corridor without the door");
        F144_CHECK(Floppy144GreyDoorCandidateAt(
            Floppy144RunStateGreyDoorPlacementSlot(
                &run,Floppy144GreyDoorCandidateCount()),&candidate),
            "S4G seeded candidate for pixel-level wall audit");
        Floppy144RunStateSetPlayerSitePosition(
            &run,candidate.stand_x16,candidate.stand_y16);
        F144_CHECK(!Floppy144GreyDoorForRun(&run,&candidate) &&
            !Floppy144GreyDoorNearby(&run),
            "S4G unseen Door creates no visible or inspectable hook");

        Floppy144Site2DDrawForPlayerState(
            &before_surface,&run,NULL,FLOPPY144_OPERATOR_BODY_STYLE_A,NULL);
        Floppy144SiteDirectoryDraw(&before_surface,&run);
        F144_CHECK(Floppy144RunStateGreyDoorDiscover(&run),
            "S4G first view advances the one-shot state");
        F144_CHECK(Floppy144GreyDoorForRun(&run,&candidate) &&
            Floppy144GreyDoorNearby(&run),
            "S4G discovered Door provides exactly one overlay and proximity");

        Floppy144SiteDirectoryDraw(&available_surface,&run);
        F144_CHECK(memcmp(before_pixels,available_pixels,
            640U*360U*sizeof(uint32_t))==0,
            "S4G Directory contains no pre/post/unlocked Easter egg marker");
        Floppy144Site2DDrawForPlayerState(
            &before_surface,&run,NULL,FLOPPY144_OPERATOR_BODY_STYLE_A,NULL);
        /* Re-render the baseline using an identical copy with only the
           one-shot state cleared. No other bytes of RunState can change. */
        {
            Floppy144RunState pristine=run;
            pristine.grey_door_state=(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE;
            Floppy144Site2DDrawForPlayerState(
                &before_surface,&pristine,NULL,
                FLOPPY144_OPERATOR_BODY_STYLE_A,NULL);
        }
        Floppy144Site2DDrawForPlayerState(
            &available_surface,&run,NULL,FLOPPY144_OPERATOR_BODY_STYLE_A,NULL);
        F144_CHECK(memcmp(before_pixels,available_pixels,
            640U*360U*sizeof(uint32_t))!=0,
            "S4G available Door visibly changes exactly one corridor view");

        F144_CHECK(Floppy144RunStateGreyDoorComplete(&run),
            "S4G availability is consumed once");
        Floppy144Site2DDrawForPlayerState(
            &after_surface,&run,NULL,FLOPPY144_OPERATOR_BODY_STYLE_A,NULL);
        F144_CHECK(memcmp(before_pixels,after_pixels,
            640U*360U*sizeof(uint32_t))==0,
            "S4G COMPLETED wall and prompts pixel-match UNSEEN with no scar");
        F144_CHECK(!Floppy144GreyDoorForRun(&run,&candidate) &&
            !Floppy144GreyDoorNearby(&run) &&
            Floppy144SiteRectCount()==original_rect_count,
            "S4G consumed Door has no hook, geometry or collision change");
        Floppy144SiteDirectoryDraw(&before_surface,&run);
        Floppy144SiteDirectoryDraw(&after_surface,&run);
        F144_CHECK(memcmp(before_pixels,after_pixels,
            640U*360U*sizeof(uint32_t))==0,
            "S4G post-completion Directory remains byte-identical");
        F144_CHECK(!Floppy144RunStateGreyDoorDiscover(&run) &&
            !Floppy144RunStateGreyDoorComplete(&run),
            "S4G completed lifecycle is irreversible");
    }
    free(before_pixels);
    free(available_pixels);
    free(after_pixels);
    puts("S4G-04 visual lifecycle: UNSEEN == COMPLETED; AVAILABLE differs; directory unchanged");
}

int main(void)
{
    Floppy144TestGeneratedCabinetDiscovery();
    Floppy144TestChairContentsContract();
    Floppy144TestGenericParentContents();
    Floppy144TestSeededContainerOrdering();
    Floppy144TestShelvingPresentationAndRecoveredOrder();
    Floppy144TestAllCorridorDoorContainers();
    Floppy144TestDoorParentDisplayName();
    Floppy144TestInspectionPresentationProgression();
    Floppy144TestRecoveredChildRevealsCabinetCode();
    Floppy144TestSecurityCabinetUnlockAndContents();
    Floppy144TestPerCabinetUnlockPersistence();
    Floppy144TestSiteKeySetInteraction();
    Floppy144TestFocusedCabinetAccess();
    Floppy144TestActLengthContract();
    Floppy144TestNoticeboardCalendar();
    Floppy144TestTakeawayMenuPresentation();
    Floppy144TestCrosswordPresentation();
    Floppy144TestPaperbackPresentation();
    Floppy144TestGreyDoorNoScar();

    if(g_nFailures != 0)
    {
        fprintf(
            stderr,
            "\nSTAGE 3B.5 CABINET TESTS: FAIL (%d failure%s)\n",
            g_nFailures,
            g_nFailures == 1 ? "" : "s"
        );

        return 1;
    }

    puts("\nSTAGE 3B.5 CABINET TESTS: PASS");
    return 0;
}
