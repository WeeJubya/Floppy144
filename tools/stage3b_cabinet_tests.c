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
        ) == 1U,
        "Cabinet Interior initially exposes only recovered P-092 content"
    );

    pItem = Floppy144CabinetVisibleContentAt(
        &sCabinet,
        &sState,
        0U
    );

    F144_CHECK(
        pItem != NULL &&
        strcmp(pItem->pszId, "P-092") == 0,
        "Cabinet Interior resolves recovered Master Access Register"
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
