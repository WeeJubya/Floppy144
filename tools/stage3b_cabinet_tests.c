/*
 * Floppy//144 Stage 3B.5 secure-cabinet / Cabinet Interior regression.
 *
 * Proves the reusable access layer against generated furniture and physical
 * item records. No story-specific cabinet coordinates are duplicated here.
 */

#include "floppy144_cabinet.h"
#include "floppy144_game_data.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_persistence.h"
#include "floppy144_run_state.h"
#include "floppy144_site.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>
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

    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sState,
            Floppy144GameDataTriggerId("T-005")
        ),
        "corridor-door fixture reveals FM-04 room plates"
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

int main(void)
{
    Floppy144TestGeneratedCabinetDiscovery();
    Floppy144TestChairContentsContract();
    Floppy144TestGenericParentContents();
    Floppy144TestShelvingPresentationAndRecoveredOrder();
    Floppy144TestAllCorridorDoorContainers();
    Floppy144TestDoorParentDisplayName();
    Floppy144TestRecoveredChildRevealsCabinetCode();
    Floppy144TestSecurityCabinetUnlockAndContents();
    Floppy144TestPerCabinetUnlockPersistence();
    Floppy144TestActLengthContract();

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
