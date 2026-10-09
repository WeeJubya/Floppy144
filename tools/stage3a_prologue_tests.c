/*
 * Floppy//144 Stage 3A Prologue vertical-slice regression.
 *
 * This test deliberately drives the same public terminal, document, trigger,
 * RunState and persistence APIs used by the Win32 game. It proves the player
 * route rather than setting progression bits directly as test fixtures.
 */
#include "floppy144_catalogue.h"
#include "floppy144_collection_registry.h"
#include "floppy144_document.h"
#include "floppy144_game_data.h"
#include "floppy144_persistence.h"
#include "floppy144_run_state.h"
#include "floppy144_terminal.h"
#include "floppy144_world.h"

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

/* Enter one command through the public character-input path. */
static void Floppy144TestSubmitCommand(
    Floppy144TerminalState *pTerminal,
    Floppy144WorldState *pWorld,
    Floppy144RunState *pRunState,
    const char *pszCommand
)
{
    const char *pszCharacter;

    for(
        pszCharacter = pszCommand;
        pszCharacter != NULL && *pszCharacter != '\0';
        ++pszCharacter
    )
    {
        Floppy144TerminalInputCharacter(
            pTerminal,
            *pszCharacter
        );
    }

    Floppy144TerminalSubmitInput(
        pTerminal,
        pWorld,
        pRunState
    );

    /*
     * RESTORE is now visibly asynchronous in the game. Stage 3A's command
     * helper advances that UI phase to completion so these older semantic
     * regressions remain deterministic and do not depend on a Win32 timer.
     */
    if(Floppy144TerminalRestoreInProgress(pTerminal))
    {
        Floppy144TerminalAdvanceRestore(
            pTerminal,
            pWorld,
            pRunState,
            pTerminal->restoration_duration_ms
        );
    }
}

/* Search the fixed terminal transcript without depending on line position. */
static bool Floppy144TestTerminalContains(
    const Floppy144TerminalState *pTerminal,
    const char *pszText
)
{
    uint32_t uLine;

    if(pTerminal == NULL || pszText == NULL)
    {
        return false;
    }

    for(uLine = 0U; uLine < pTerminal->output_count; ++uLine)
    {
        if(strstr(pTerminal->output[uLine], pszText) != NULL)
        {
            return true;
        }
    }

    return false;
}

static void Floppy144TestPrologueVerticalSlice(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;
    Floppy144CollectionId eDiskIndex;
    Floppy144TriggerId eDiskIndexTrigger;
    const Floppy144DocumentDefinition *pTriggerDocument;
    Floppy144CollectionId eExpectedRecordCollection;
    uint32_t uExpectedRecordIndex;

    Floppy144WorldReset(&sWorld);
    Floppy144RunStateBegin(&sRunState, 144U);
    Floppy144TerminalReset(&sTerminal, &sWorld);

    eDiskIndex = Floppy144GameDataCollectionId("DR-01");
    eDiskIndexTrigger = Floppy144GameDataTriggerId("T-001");

    F144_CHECK(
        eDiskIndex < FLOPPY144_COLLECTION_COUNT,
        "DR-01 resolves"
    );
    F144_CHECK(
        eDiskIndexTrigger < FLOPPY144_TRIGGER_COUNT,
        "T-001 resolves"
    );
    F144_CHECK(
        Floppy144RunStateAct(&sRunState) ==
            FLOPPY144_RUN_ACT_PROLOGUE,
        "new recovery starts in Prologue"
    );
    F144_CHECK(
        !Floppy144RunStateArchiveServicesInitialised(&sRunState),
        "archive services start offline"
    );
    F144_CHECK(
        !Floppy144RunStateRoomReconstructed(
            &sRunState,
            FLOPPY144_ROOM_RECEPTION
        ),
        "Reception does not exist before the recovery index"
    );

    /*
     * Stage 3C moved INITIATE guidance out of transcript history and into the
     * dedicated prompt/guidance region. PrintNextAction must therefore leave
     * the transcript untouched until archive services have been initialised.
     */
    Floppy144TerminalPrintNextAction(&sTerminal, &sRunState);
    F144_CHECK(
        !Floppy144TestTerminalContains(&sTerminal, "INITIATE"),
        "INITIATE guidance is not written into terminal transcript history"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "INITIATE"
    );
    F144_CHECK(
        Floppy144RunStateArchiveServicesInitialised(&sRunState),
        "INITIATE persists archive-service state"
    );
    F144_CHECK(
        Floppy144WorldArchiveServicesInitialised(&sWorld),
        "INITIATE hydrates live world state"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal, "NEXT RECOVERY ACTION: LIST"),
        "INITIATE recommends LIST"
    );
    Floppy144TestSubmitCommand(&sTerminal, &sWorld, &sRunState, "LIST");
    F144_CHECK(
        Floppy144TerminalRecordPagerActive(&sTerminal) &&
        Floppy144TestTerminalContains(&sTerminal, "DR-01") &&
        Floppy144TestTerminalContains(&sTerminal, "AVAILABLE"),
        "plain LIST shows available DR-01 before restoration"
    );
    Floppy144TerminalCloseRecordPager(&sTerminal);
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal, "NEXT RECOVERY ACTION: RESTORE DR-01"),
        "LIST recommends RESTORE DR-01"
    );
    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE DR-01"
    );
    F144_CHECK(
        Floppy144RunStateCollectionRestored(
            &sRunState,
            eDiskIndex
        ),
        "RESTORE DR-01 changes authoritative RunState"
    );
    F144_CHECK(
        Floppy144RunStateReconstructionPercent(&sRunState) == 4U,
        "DR-01 consumes its four-percent reconstruction quota"
    );

    pTriggerDocument = Floppy144DocumentFirstPendingTrigger(
        &sRunState,
        eDiskIndex
    );
    F144_CHECK(
        pTriggerDocument != NULL,
        "restored DR-01 exposes an eligible trigger document"
    );
    F144_CHECK(
        pTriggerDocument != NULL &&
        pTriggerDocument->record_id_override != NULL &&
        strcmp(
            pTriggerDocument->record_id_override,
            "DR-01-RS-0001"
        ) == 0,
        "Disk Recovery Index is the pending Prologue document"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal, "NEXT RECOVERY ACTION: LIST DR-01"),
        "restoration recommends LIST DR-01"
    );
    Floppy144TestSubmitCommand(
        &sTerminal, &sWorld, &sRunState, "LIST DR-01");
    F144_CHECK(
        Floppy144TerminalRecordPagerActive(&sTerminal) &&
        Floppy144TestTerminalContains(&sTerminal, "DR-01-RS-0001"),
        "LIST DR-01 displays recovered records"
    );
    Floppy144TerminalCloseRecordPager(&sTerminal);
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal, "NEXT RECOVERY ACTION: OPEN RS-0001"),
        "LIST DR-01 recommends OPEN RS-0001"
    );

    F144_CHECK(
        Floppy144DocumentFindRecordId(
            "DR-01-RS-0001",
            &eExpectedRecordCollection,
            &uExpectedRecordIndex
        ),
        "Disk Recovery Index stable record ID resolves through document registry"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "OPEN RS-0001"
    );
    F144_CHECK(
        sTerminal.open_record_requested,
        "OPEN requests the document viewer"
    );
    F144_CHECK(
        sTerminal.requested_collection == eExpectedRecordCollection &&
        sTerminal.requested_record_index == uExpectedRecordIndex,
        "OPEN resolves the dispersed canonical document location"
    );

    F144_CHECK(
        Floppy144DocumentApplyEffects(
            &sWorld,
            &sRunState,
            sTerminal.requested_collection,
            sTerminal.requested_record_index
        ),
        "opening the authored recovery index applies document effects"
    );
    Floppy144TerminalPrintNextAction(&sTerminal, &sRunState);

    F144_CHECK(
        Floppy144RunStateTriggerFired(
            &sRunState,
            eDiskIndexTrigger
        ),
        "Disk Recovery Index fires T-001"
    );
    F144_CHECK(
        Floppy144RunStateRoomReconstructed(
            &sRunState,
            FLOPPY144_ROOM_RECEPTION
        ) &&
        Floppy144RunStateRoomReconstructed(
            &sRunState,
            FLOPPY144_ROOM_CORRIDOR
        ),
        "T-001 reconstructs Reception and Corridor"
    );
    F144_CHECK(
        Floppy144GameDataCollectionEnabled(&sRunState, "DR-02") &&
        Floppy144GameDataCollectionEnabled(&sRunState, "DR-03") &&
        Floppy144GameDataCollectionEnabled(&sRunState, "HR-01"),
        "T-001 enables all three opening collections"
    );
    F144_CHECK(
        Floppy144GameDataFactRecorded(&sRunState, "DR-01"),
        "T-001 records the Disk Recovery Index notebook fact"
    );
    F144_CHECK(
        Floppy144RunStateAct(&sRunState) ==
            FLOPPY144_RUN_ACT_PROLOGUE,
        "T-001 leaves the narrative in the Prologue"
    );
    F144_CHECK(
        Floppy144DocumentFirstPendingTrigger(
            &sRunState,
            eDiskIndex
        ) == NULL,
        "fire-once trigger document is no longer pending"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(&sTerminal, "EXIT TO SITE"),
        "terminal hands the player to reconstructed Site exploration"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(&sTerminal, "RESTORE DR-02"),
        "Site hand-off also preserves the next archive action"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "EXIT"
    );
    F144_CHECK(
        sTerminal.exit_requested,
        "EXIT requests the Site screen transition"
    );

#if defined(_WIN32)
    {
        const char szSavePath[] = "floppy144_stage3a_test.sav";
        Floppy144RunState sReinstated;
        Floppy144WorldState sReinstatedWorld;

        Floppy144RunStateReset(&sReinstated);
        F144_CHECK(
            Floppy144PersistenceSaveRunState(
                szSavePath,
                &sRunState
            ),
            "Prologue state saves"
        );
        F144_CHECK(
            Floppy144PersistenceLoadRunState(
                szSavePath,
                &sReinstated
            ),
            "Prologue state reinstates"
        );
        (void)remove(szSavePath);

        Floppy144WorldReset(&sReinstatedWorld);
        Floppy144WorldHydrateFromRunState(
            &sReinstatedWorld,
            &sReinstated
        );

        F144_CHECK(
            Floppy144RunStateCollectionRestored(
                &sReinstated,
                eDiskIndex
            ) &&
            Floppy144RunStateTriggerFired(
                &sReinstated,
                eDiskIndexTrigger
            ),
            "reinstated state retains collection and trigger"
        );
        F144_CHECK(
            Floppy144RunStateRoomReconstructed(
                &sReinstated,
                FLOPPY144_ROOM_RECEPTION
            ) &&
            Floppy144GameDataFactRecorded(
                &sReinstated,
                "DR-01"
            ),
            "reinstated state retains Site and notebook progression"
        );
        F144_CHECK(
            Floppy144WorldArchiveServicesInitialised(
                &sReinstatedWorld
            ),
            "reinstated RunState rehydrates archive services"
        );
    }
#endif
}

/* Save/reload the tutorial itself, independent of terminal-local history. */
static void Floppy144TestOpeningTutorialResume(void)
{
    Floppy144WorldState world, loaded_world;
    Floppy144RunState run, loaded;
    Floppy144TerminalState terminal;
    uint8_t payload[FLOPPY144_SAVE_PAYLOAD_V4_SIZE];

    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run, 146U);
    Floppy144TerminalReset(&terminal, &world);
    Floppy144TestSubmitCommand(&terminal, &world, &run, "INITIATE");
    Floppy144TestSubmitCommand(&terminal, &world, &run, "LIST");
    Floppy144TerminalCloseRecordPager(&terminal);
    F144_CHECK(
        run.opening_recovery_step ==
            (uint8_t)FLOPPY144_OPENING_COLLECTIONS_LISTED,
        "first LIST records a persistent milestone"
    );
    F144_CHECK(
        Floppy144PersistenceEncodeRunState(&run, payload, sizeof(payload)) &&
        Floppy144PersistenceDecodeRunState(&loaded, payload, sizeof(payload)) &&
        loaded.opening_recovery_step ==
            (uint8_t)FLOPPY144_OPENING_COLLECTIONS_LISTED,
        "LIST milestone survives V4 save payload"
    );
    Floppy144WorldReset(&loaded_world);
    Floppy144WorldHydrateFromRunState(&loaded_world, &loaded);
    Floppy144TerminalReset(&terminal, &loaded_world);
    Floppy144TerminalPrintNextAction(&terminal, &loaded);
    F144_CHECK(
        Floppy144TestTerminalContains(
            &terminal, "NEXT RECOVERY ACTION: RESTORE DR-01"),
        "reinstated tutorial resumes with RESTORE"
    );

    Floppy144TestSubmitCommand(
        &terminal, &loaded_world, &loaded, "RESTORE DR-01");
    Floppy144TestSubmitCommand(
        &terminal, &loaded_world, &loaded, "LIST DR-01");
    Floppy144TerminalCloseRecordPager(&terminal);
    F144_CHECK(
        Floppy144PersistenceEncodeRunState(&loaded, payload, sizeof(payload)) &&
        Floppy144PersistenceDecodeRunState(&run, payload, sizeof(payload)) &&
        run.opening_recovery_step ==
            (uint8_t)FLOPPY144_OPENING_DR01_RECORDS_LISTED,
        "DR-01 index milestone survives V4 save payload"
    );
    Floppy144WorldReset(&world);
    Floppy144WorldHydrateFromRunState(&world, &run);
    Floppy144TerminalReset(&terminal, &world);
    Floppy144TerminalPrintNextAction(&terminal, &run);
    F144_CHECK(
        Floppy144TestTerminalContains(
            &terminal, "NEXT RECOVERY ACTION: OPEN RS-0001"),
        "reopened terminal recommends OPEN RS-0001"
    );
    Floppy144TestSubmitCommand(&terminal, &world, &run, "OPEN RS-0001");
    F144_CHECK(
        terminal.open_record_requested &&
        terminal.requested_collection == FLOPPY144_COLLECTION_DR01 &&
        terminal.requested_record_index == 0U,
        "restarted terminal resolves tutorial shorthand safely"
    );
    F144_CHECK(
        run.opening_recovery_step ==
            (uint8_t)FLOPPY144_OPENING_FIRST_RECORD_OPENED,
        "valid OPEN completes the command tutorial"
    );

    /* V3 legacy saves have no tutorial byte and migrate to a safe default. */
    {
        uint8_t legacy[FLOPPY144_SAVE_PAYLOAD_V3_SIZE];
        Floppy144RunState older;
        F144_CHECK(
            Floppy144PersistenceEncodeRunState(
                &run, legacy, sizeof(legacy)) &&
            Floppy144PersistenceDecodeRunState(
                &older, legacy, sizeof(legacy)) &&
            older.opening_recovery_step ==
                (uint8_t)FLOPPY144_OPENING_NEEDS_LIST &&
            Floppy144RunStateCollectionRestored(
                &older, FLOPPY144_COLLECTION_DR01),
            "V3 recovery migration retains restoration without bogus tutorial state"
        );
    }

    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run, 147U);
    Floppy144TerminalReset(&terminal, &world);
    Floppy144TestSubmitCommand(&terminal, &world, &run, "INITIATE");
    Floppy144TestSubmitCommand(&terminal, &world, &run, "RESTORE DR-01");
    F144_CHECK(
        Floppy144RunStateCollectionRestored(
            &run, FLOPPY144_COLLECTION_DR01) &&
        Floppy144TestTerminalContains(
            &terminal, "NEXT RECOVERY ACTION: LIST DR-01"),
        "manual RESTORE bypasses LIST without bypassing recovery checks"
    );
    Floppy144TestSubmitCommand(&terminal, &world, &run, "OPEN RS-0001");
    F144_CHECK(
        terminal.open_record_requested &&
        terminal.requested_collection == FLOPPY144_COLLECTION_DR01,
        "manual OPEN remains available without following LIST hints"
    );
}

/* Prove that catalogue navigation reaches the same data-driven document path. */
static void Floppy144TestCatalogueProloguePath(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144CatalogueState sCatalogue;
    Floppy144CollectionId eDiskIndex;
    Floppy144TriggerId eDiskIndexTrigger;
    Floppy144CollectionId eDocumentCollection;
    uint32_t uDocumentRecordIndex;

    Floppy144WorldReset(&sWorld);
    Floppy144RunStateBegin(&sRunState, 145U);
    F144_CHECK(
        Floppy144WorldInitialiseArchiveServices(&sWorld),
        "catalogue fixture initialises live archive services"
    );
    F144_CHECK(
        Floppy144RunStateInitialiseArchiveServices(&sRunState),
        "catalogue fixture initialises persistent archive services"
    );

    eDiskIndex = Floppy144GameDataCollectionId("DR-01");
    eDiskIndexTrigger = Floppy144GameDataTriggerId("T-001");

    F144_CHECK(
        Floppy144RunStateRestoreCollection(
            &sRunState,
            eDiskIndex
        ),
        "catalogue fixture restores DR-01"
    );
    F144_CHECK(
        Floppy144WorldRestoreCollection(
            &sWorld,
            eDiskIndex
        ),
        "catalogue fixture mirrors DR-01 into live world"
    );

    Floppy144CatalogueReset(&sCatalogue, eDiskIndex);
    F144_CHECK(
        Floppy144DocumentFindRecordId(
            "DR-01-RS-0001",
            &eDocumentCollection,
            &uDocumentRecordIndex
        ),
        "catalogue path resolves dispersed Disk Recovery Index"
    );
    F144_CHECK(
        Floppy144CatalogueOpenRecord(
            &sCatalogue,
            eDocumentCollection,
            uDocumentRecordIndex
        ),
        "catalogue opens the dispersed Disk Recovery Index"
    );
    F144_CHECK(
        Floppy144DocumentApplyEffects(
            &sWorld,
            &sRunState,
            sCatalogue.collection,
            sCatalogue.selected_index
        ),
        "catalogue document routes through authored effects"
    );
    F144_CHECK(
        Floppy144RunStateTriggerFired(
            &sRunState,
            eDiskIndexTrigger
        ) &&
        Floppy144RunStateRoomReconstructed(
            &sRunState,
            FLOPPY144_ROOM_RECEPTION
        ),
        "catalogue route completes the same T-001 Site reconstruction"
    );
}

int main(void)
{
    Floppy144TestPrologueVerticalSlice();
    Floppy144TestOpeningTutorialResume();
    Floppy144TestCatalogueProloguePath();

    if(g_nFailures != 0)
    {
        fprintf(
            stderr,
            "\nSTAGE 3A PROLOGUE TESTS: FAIL (%d failure%s)\n",
            g_nFailures,
            g_nFailures == 1 ? "" : "s"
        );

        return 1;
    }

    puts("\nSTAGE 3A PROLOGUE TESTS: PASS");
    return 0;
}
