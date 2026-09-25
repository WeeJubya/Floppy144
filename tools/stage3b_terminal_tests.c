/*
 * Floppy//144 Stage 3B.1 terminal/archive regression.
 *
 * Exercises the reusable command shell rather than story-specific shortcuts:
 * collection restoration, catalogue lookup, contextual OPEN, paged LIST and
 * terminal-session command history.
 */
#include "floppy144_catalogue.h"
#include "floppy144_collection_registry.h"
#include "floppy144_document.h"
#include "floppy144_draw.h"
#include "floppy144_game_data.h"
#include "floppy144_run_state.h"
#include "floppy144_terminal.h"
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
}

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

/*
 * Reach the first post-Prologue archive state exclusively through public
 * terminal/document APIs. T-001 makes DR-02, DR-03 and HR-01 available.
 */
static void Floppy144TestReachOpeningCollections(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pRunState,
    Floppy144TerminalState *pTerminal
)
{
    Floppy144WorldReset(pWorld);
    Floppy144RunStateBegin(pRunState, 144U);
    Floppy144TerminalReset(pTerminal, pWorld);

    Floppy144TestSubmitCommand(
        pTerminal,
        pWorld,
        pRunState,
        "INITIATE"
    );

    Floppy144TestSubmitCommand(
        pTerminal,
        pWorld,
        pRunState,
        "RESTORE DR-01"
    );

    Floppy144TestSubmitCommand(
        pTerminal,
        pWorld,
        pRunState,
        "OPEN DR-01-RS-0001"
    );

    F144_CHECK(
        pTerminal->open_record_requested,
        "opening fixture requests DR-01 trigger document"
    );

    if(pTerminal->open_record_requested)
    {
        F144_CHECK(
            Floppy144DocumentApplyEffects(
                pWorld,
                pRunState,
                pTerminal->requested_collection,
                pTerminal->requested_record_index
            ),
            "opening fixture applies DR-01 document effects"
        );
    }

    pTerminal->open_record_requested = false;
}

static void Floppy144TestExtendedGlyphs(void)
{
    uint32_t auPixels[64U * 16U] = {0U};
    Floppy144Surface sSurface =
    {
        auPixels,
        64U,
        16U
    };
    uint32_t uPixel;
    uint32_t uLit = 0U;

    F144_CHECK(
        Floppy144DrawTextWidth("A|B#C", 1U) == 29U,
        "extended ASCII interface glyphs retain one-cell text width"
    );
    F144_CHECK(
        Floppy144DrawTextWidth("\xE2\x80\xA2", 1U) == 5U,
        "Unicode Notebook bullet normalises to one glyph"
    );

    Floppy144DrawText(
        &sSurface,
        0U,
        0U,
        "\xE2\x80\xA2|#'",
        1U,
        UINT32_MAX
    );

    for(uPixel = 0U; uPixel < 64U * 16U; ++uPixel)
    {
        if(auPixels[uPixel] != 0U)
        {
            ++uLit;
        }
    }

    F144_CHECK(
        uLit > 0U,
        "Notebook bullet and interface punctuation render visible glyph pixels"
    );
}

static bool Floppy144TestCatalogueRecordId(
    Floppy144CollectionId eCollection,
    uint32_t uRecordIndex,
    char *pszFullId,
    size_t uFullCapacity,
    char *pszShortId,
    size_t uShortCapacity
)
{
    char szTitle[48];
    const char *pszShort;

    if(
        pszFullId == NULL ||
        uFullCapacity == 0U
    )
    {
        return false;
    }

    Floppy144CatalogueBuildRecord(
        eCollection,
        uRecordIndex,
        pszFullId,
        uFullCapacity,
        szTitle,
        sizeof(szTitle)
    );

    pszShort = strstr(pszFullId, "-RS-");

    if(
        pszShortId != NULL &&
        uShortCapacity > 0U
    )
    {
        if(pszShort == NULL)
        {
            return false;
        }

        (void)snprintf(
            pszShortId,
            uShortCapacity,
            "%s",
            pszShort + 1
        );
    }

    return true;
}

static const Floppy144DocumentDefinition *
Floppy144TestDocumentForTrigger(
    Floppy144CollectionId eCollection,
    Floppy144TriggerId eTrigger
)
{
    const Floppy144CollectionDefinition *pCollection =
        Floppy144CollectionGet(eCollection);

    uint32_t uRecordIndex;

    if(pCollection == NULL)
    {
        return NULL;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < pCollection->catalogue.record_count;
        ++uRecordIndex
    )
    {
        const Floppy144DocumentDefinition *pDocument =
            Floppy144DocumentGet(
                eCollection,
                uRecordIndex
            );

        if(
            pDocument != NULL &&
            pDocument->trigger == eTrigger
        )
        {
            return pDocument;
        }
    }

    return NULL;
}

static bool Floppy144TestRecordNumber(
    const char *pszRecordId,
    uint32_t *pNumber
)
{
    size_t uLength;
    uint32_t uNumber = 0U;
    uint32_t uDigit;

    if(pszRecordId == NULL || pNumber == NULL)
    {
        return false;
    }

    uLength = strlen(pszRecordId);
    if(uLength < 4U)
    {
        return false;
    }

    for(uDigit = 0U; uDigit < 4U; ++uDigit)
    {
        char ch = pszRecordId[uLength - 4U + uDigit];
        if(ch < '0' || ch > '9')
        {
            return false;
        }
        uNumber = uNumber * 10U + (uint32_t)(ch - '0');
    }

    *pNumber = uNumber;
    return true;
}

/*
 * Every catalogue ID must round-trip through the shared resolver. This covers
 * generated index entries and authored ID overrides with one generic test.
 */
static void Floppy144TestCatalogueRecordResolution(void)
{
    uint32_t uCollection;
    uint32_t uChecked = 0U;
    uint32_t uComparedGaps = 0U;
    uint32_t uIrregularGaps = 0U;

    for(
        uCollection = 0U;
        uCollection < (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++uCollection
    )
    {
        Floppy144CollectionId eCollection =
            (Floppy144CollectionId)uCollection;

        const Floppy144CollectionDefinition *pDefinition =
            Floppy144CollectionGet(eCollection);

        uint32_t uRecord;
        uint32_t uPreviousRecordNumber = 0U;

        for(
            uRecord = 0U;
            uRecord < pDefinition->catalogue.record_count;
            ++uRecord
        )
        {
            char szRecordId[24];
            char szTitle[48];
            Floppy144CollectionId eResolvedCollection =
                FLOPPY144_COLLECTION_COUNT;
            uint32_t uResolvedRecord = UINT32_MAX;

            Floppy144CatalogueBuildRecord(
                eCollection,
                uRecord,
                szRecordId,
                sizeof(szRecordId),
                szTitle,
                sizeof(szTitle)
            );

            {
                uint32_t uCurrentRecordNumber = 0U;
                bool bNumberValid =
                    Floppy144TestRecordNumber(
                        szRecordId,
                        &uCurrentRecordNumber
                    );

                F144_CHECK(
                    bNumberValid &&
                    (
                        uRecord == 0U ||
                        uCurrentRecordNumber > uPreviousRecordNumber
                    ),
                    "catalogue rows remain in numerical record-ID order"
                );

                if(bNumberValid)
                {
                    if(uRecord > 0U)
                    {
                        ++uComparedGaps;

                        if(
                            uCurrentRecordNumber -
                            uPreviousRecordNumber != 10U
                        )
                        {
                            ++uIrregularGaps;
                        }
                    }

                    uPreviousRecordNumber = uCurrentRecordNumber;
                }
            }

            F144_CHECK(
                Floppy144CatalogueFindRecord(
                    szRecordId,
                    &eResolvedCollection,
                    &uResolvedRecord
                ),
                "catalogue record ID resolves"
            );

            F144_CHECK(
                eResolvedCollection == eCollection &&
                uResolvedRecord == uRecord,
                "catalogue record ID round-trips to original location"
            );

            ++uChecked;
        }
    }

    F144_CHECK(
        uChecked > 0U,
        "catalogue resolver exercised registered records"
    );

    F144_CHECK(
        uComparedGaps > 0U &&
        uIrregularGaps * 4U > uComparedGaps * 3U,
        "catalogue record IDs use irregular Technical-Slice-style spacing"
    );

    {
        Floppy144CollectionId eCollection;
        uint32_t uRecord;

        F144_CHECK(
            !Floppy144CatalogueFindRecord(
                "NOT-A-RECORD",
                &eCollection,
                &uRecord
            ),
            "unknown record ID is rejected"
        );
    }
}

/*
 * Long recovered documents use a 13-line viewport. FM-04-RS-0035 is a compact
 * permanent fixture for scroll behaviour because its in-universe body wraps
 * beyond that window.
 */
static void Floppy144TestDocumentBodyScrolling(void)
{
    Floppy144CatalogueState sCatalogue;
    Floppy144CollectionId eCollection =
        FLOPPY144_COLLECTION_COUNT;
    uint32_t uRecordIndex = UINT32_MAX;
    uint32_t uMaximumScroll;
    uint32_t uStep;

    F144_CHECK(
        Floppy144CatalogueFindRecord(
            "FM-04-RS-0035",
            &eCollection,
            &uRecordIndex
        ),
        "FM-04-RS-0035 resolves for document-scroll regression"
    );

    if(eCollection == FLOPPY144_COLLECTION_COUNT)
    {
        return;
    }

    Floppy144CatalogueReset(
        &sCatalogue,
        eCollection
    );

    F144_CHECK(
        Floppy144CatalogueOpenRecord(
            &sCatalogue,
            eCollection,
            uRecordIndex
        ) &&
        Floppy144CatalogueDocumentOpen(
            &sCatalogue
        ) &&
        sCatalogue.document_scroll_line == 0U,
        "long document opens at first wrapped body line"
    );

    Floppy144CatalogueScrollDocument(
        &sCatalogue,
        1
    );

    F144_CHECK(
        sCatalogue.document_scroll_line == 1U,
        "Down scrolls long document by one wrapped line"
    );

    for(uStep = 0U; uStep < 100U; ++uStep)
    {
        Floppy144CatalogueScrollDocument(
            &sCatalogue,
            1
        );
    }

    uMaximumScroll =
        sCatalogue.document_scroll_line;

    F144_CHECK(
        uMaximumScroll > 1U &&
        uMaximumScroll < 100U,
        "document scrolling clamps at final full viewport"
    );

    Floppy144CatalogueScrollDocument(
        &sCatalogue,
        1
    );

    F144_CHECK(
        sCatalogue.document_scroll_line == uMaximumScroll,
        "Down at document end remains clamped"
    );

    for(uStep = 0U; uStep < 100U; ++uStep)
    {
        Floppy144CatalogueScrollDocument(
            &sCatalogue,
            -1
        );
    }

    F144_CHECK(
        sCatalogue.document_scroll_line == 0U,
        "Up clamps at first document line"
    );

    Floppy144CatalogueCloseDocument(
        &sCatalogue
    );

    F144_CHECK(
        !Floppy144CatalogueDocumentOpen(
            &sCatalogue
        ) &&
        sCatalogue.document_scroll_line == 0U,
        "closing document clears scroll position"
    );
}

/*
 * Stage 3C collection LIST presentation owns a fixed-width table. The header
 * and rows must share separator columns, and unavailable identities must remain
 * opaque rather than looking like plausible collection names.
 */
static void Floppy144TestCollectionListPresentation(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;
    uint32_t auHeaderBars[3] = {0U,0U,0U};
    uint32_t auRowBars[3] = {0U,0U,0U};
    uint32_t uHeaderBarCount = 0U;
    uint32_t uRowBarCount = 0U;
    uint32_t uIndex;

    Floppy144TestReachOpeningCollections(
        &sWorld,
        &sRunState,
        &sTerminal
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "LIST"
    );

    F144_CHECK(
        Floppy144TerminalRecordPagerActive(&sTerminal) &&
        sTerminal.record_pager_collection == FLOPPY144_COLLECTION_COUNT,
        "bare LIST opens the full collection pager"
    );

    F144_CHECK(
        sTerminal.output_count >= 3U,
        "collection LIST renders heading, page label and rows"
    );

    F144_CHECK(
        sTerminal.output_count >= 2U &&
        strstr(sTerminal.output[1], "Q: EXIT") != NULL,
        "collection LIST page visibly advertises Q: EXIT"
    );

    if(sTerminal.output_count >= 3U)
    {
        for(
            uIndex = 0U;
            sTerminal.output[0][uIndex] != '\0';
            ++uIndex
        )
        {
            if(
                sTerminal.output[0][uIndex] == '|' &&
                uHeaderBarCount < 3U
            )
            {
                auHeaderBars[uHeaderBarCount++] = uIndex;
            }
        }

        for(
            uIndex = 0U;
            sTerminal.output[2][uIndex] != '\0';
            ++uIndex
        )
        {
            if(
                sTerminal.output[2][uIndex] == '|' &&
                uRowBarCount < 3U
            )
            {
                auRowBars[uRowBarCount++] = uIndex;
            }
        }

        F144_CHECK(
            uHeaderBarCount == 3U &&
            uRowBarCount == 3U &&
            auHeaderBars[0] == auRowBars[0] &&
            auHeaderBars[1] == auRowBars[1] &&
            auHeaderBars[2] == auRowBars[2],
            "collection LIST headings align with row columns"
        );
    }

    F144_CHECK(
        Floppy144TestTerminalContains(&sTerminal, "N/A") &&
        !Floppy144TestTerminalContains(&sTerminal, "UX-") &&
        !Floppy144TestTerminalContains(&sTerminal, "BLANK PACKET") &&
        !Floppy144TestTerminalContains(&sTerminal, "DR-04"),
        "N/A collection identity is opaque and does not leak real ID"
    );

    if(uHeaderBarCount == 3U)
    {
        F144_CHECK(
            auHeaderBars[1] > 43U,
            "collection LIST expands the name column beyond the old 27-character width"
        );
    }

    for(uIndex = 0U; uIndex < sTerminal.output_count; ++uIndex)
    {
        F144_CHECK(
            strlen(sTerminal.output[uIndex]) <
                FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY,
            "collection LIST rows remain within terminal line capacity"
        );
    }

    F144_CHECK(
        sTerminal.cursor_visible,
        "terminal command cursor begins in its visible blink phase"
    );
}

/*
 * Restore and browse several collections without embedding special-case
 * terminal behaviour for any one of them.
 */
static void Floppy144TestMultipleCollectionCommands(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;

    Floppy144CollectionId eDr02;
    Floppy144CollectionId eDr03;
    Floppy144CollectionId eHr01;
    Floppy144CollectionId eFm04;

    Floppy144TestReachOpeningCollections(
        &sWorld,
        &sRunState,
        &sTerminal
    );

    eDr02 = Floppy144GameDataCollectionId("DR-02");
    eDr03 = Floppy144GameDataCollectionId("DR-03");
    eHr01 = Floppy144GameDataCollectionId("HR-01");
    eFm04 = Floppy144GameDataCollectionId("FM-04");

    F144_CHECK(
        eDr02 < FLOPPY144_COLLECTION_COUNT &&
        eDr03 < FLOPPY144_COLLECTION_COUNT &&
        eHr01 < FLOPPY144_COLLECTION_COUNT &&
        eFm04 < FLOPPY144_COLLECTION_COUNT,
        "3B.1 collection fixtures resolve"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE FM-04"
    );
    F144_CHECK(
        !Floppy144RunStateCollectionRestored(&sRunState, eFm04),
        "unavailable collection cannot be restored"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "NOT YET AVAILABLE"
        ),
        "unavailable collection reports progression gate"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE DR-02"
    );
    F144_CHECK(
        Floppy144RunStateCollectionRestored(&sRunState, eDr02) &&
        Floppy144WorldCollectionRestored(&sWorld, eDr02),
        "RESTORE works for second generic collection"
    );
    F144_CHECK(
        sTerminal.default_record_collection_valid &&
        sTerminal.default_record_collection == eDr02,
        "most recently restored collection becomes short-ID context"
    );

    {
        char szFullId[24];
        char szShortId[16];
        char szCommand[40];

        F144_CHECK(
            Floppy144TestCatalogueRecordId(
                eDr02,
                0U,
                szFullId,
                sizeof(szFullId),
                szShortId,
                sizeof(szShortId)
            ),
            "DR-02 first catalogue ID is generated"
        );

        (void)snprintf(
            szCommand,
            sizeof(szCommand),
            "OPEN %s",
            szShortId
        );

        Floppy144TestSubmitCommand(
            &sTerminal,
            &sWorld,
            &sRunState,
            szCommand
        );

        F144_CHECK(
            sTerminal.open_record_requested &&
            sTerminal.requested_collection == eDr02 &&
            sTerminal.requested_record_index == 0U,
            "short OPEN resolves against most recently restored collection"
        );
    }
    sTerminal.open_record_requested = false;

    {
        const Floppy144DocumentDefinition *pHrOpening =
            Floppy144TestDocumentForTrigger(
                eHr01,
                Floppy144GameDataTriggerId("T-002")
            );

        char szCommand[48];

        F144_CHECK(
            pHrOpening != NULL &&
            pHrOpening->record_id_override != NULL,
            "HR-01 opening authored record resolves from trigger registry"
        );

        (void)snprintf(
            szCommand,
            sizeof(szCommand),
            "OPEN %s",
            pHrOpening != NULL &&
            pHrOpening->record_id_override != NULL
                ? pHrOpening->record_id_override
                : "HR-01-RS-0000"
        );

        Floppy144TestSubmitCommand(
            &sTerminal,
            &sWorld,
            &sRunState,
            szCommand
        );
    }
    F144_CHECK(
        !sTerminal.open_record_requested,
        "full OPEN cannot retrieve an unrestored collection"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "COLLECTION HR-01 HAS NOT BEEN RESTORED"
        ),
        "unrestored full OPEN explains collection state"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "LIST HR-01"
    );
    F144_CHECK(
        !Floppy144TerminalRecordPagerActive(&sTerminal),
        "LIST does not open unrestored collection index"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE HR-01"
    );
    F144_CHECK(
        Floppy144RunStateCollectionRestored(&sRunState, eHr01),
        "RESTORE works across archive domains"
    );
    F144_CHECK(
        sTerminal.default_record_collection == eHr01,
        "HR-01 becomes current short-ID context"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "LIST HR-01 2"
    );
    F144_CHECK(
        Floppy144TerminalRecordPagerActive(&sTerminal) &&
        sTerminal.record_pager_collection == eHr01 &&
        sTerminal.record_pager_page == 2U,
        "LIST <CODE> <PAGE> opens requested restored index page"
    );

    Floppy144TerminalMoveRecordPager(&sTerminal, 1);
    F144_CHECK(
        sTerminal.record_pager_page == 2U,
        "record pager clamps at final page"
    );

    Floppy144TerminalMoveRecordPager(&sTerminal, -1);
    F144_CHECK(
        sTerminal.record_pager_page == 1U,
        "record pager moves backward"
    );
    Floppy144TerminalCloseRecordPager(&sTerminal);

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "OPEN RS-0107"
    );
    {
        Floppy144CollectionId eExpectedCollection;
        uint32_t uExpectedRecord;
        bool bResolved = Floppy144DocumentFindRecordId(
            "HR-01-RS-0107",
            &eExpectedCollection,
            &uExpectedRecord
        );
        F144_CHECK(
            bResolved &&
            sTerminal.open_record_requested &&
            sTerminal.requested_collection == eExpectedCollection &&
            sTerminal.requested_record_index == uExpectedRecord,
            "short OPEN resolves authored HR-01 override"
        );
    }
    sTerminal.open_record_requested = false;

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE DR-03"
    );
    F144_CHECK(
        Floppy144RunStateCollectionRestored(&sRunState, eDr03),
        "third enabled collection restores generically"
    );
    F144_CHECK(
        sTerminal.default_record_collection == eDr03,
        "latest restore replaces short-ID context"
    );

    {
        char szFullId[24];
        char szShortId[16];
        char szCommand[40];

        F144_CHECK(
            Floppy144TestCatalogueRecordId(
                eDr03,
                0U,
                szFullId,
                sizeof(szFullId),
                szShortId,
                sizeof(szShortId)
            ),
            "DR-03 first catalogue ID is generated"
        );

        (void)snprintf(
            szCommand,
            sizeof(szCommand),
            "OPEN %s",
            szShortId
        );

        Floppy144TestSubmitCommand(
            &sTerminal,
            &sWorld,
            &sRunState,
            szCommand
        );

        F144_CHECK(
            sTerminal.open_record_requested &&
            sTerminal.requested_collection == eDr03 &&
            sTerminal.requested_record_index == 0U,
            "short OPEN follows updated restore context"
        );
    }
    sTerminal.open_record_requested = false;

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE DR-03"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "COLLECTION DR-03 ALREADY RESTORED"
        ),
        "repeat RESTORE is idempotent at terminal layer"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE XX-99"
    );
    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "COLLECTION XX-99 NOT FOUND"
        ),
        "unknown collection is rejected generically"
    );
}

/*
 * History is deliberately UI-only. It remembers meaningful submissions in
 * the current terminal session, ignores blank/duplicate commands and supports
 * conventional Up/Down recall with restoration of an unfinished draft.
 */
static void Floppy144TestCommandHistory(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;
    uint32_t uOutputBeforeBlank;

    Floppy144WorldReset(&sWorld);
    Floppy144RunStateBegin(&sRunState, 144U);
    Floppy144TerminalReset(&sTerminal, &sWorld);

    uOutputBeforeBlank = sTerminal.output_count;

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "     "
    );
    F144_CHECK(
        sTerminal.history_count == 0U &&
        sTerminal.output_count == uOutputBeforeBlank,
        "blank command is ignored and not stored"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "   help   "
    );
    F144_CHECK(
        sTerminal.history_count == 1U &&
        strcmp(sTerminal.history[0], "HELP") == 0,
        "history stores normalised uppercase command"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "HELP"
    );
    F144_CHECK(
        sTerminal.history_count == 1U,
        "consecutive duplicate command is not stored twice"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "INITIATE"
    );
    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "LIST"
    );

    F144_CHECK(
        sTerminal.history_count == 3U,
        "distinct submitted commands populate history"
    );

    {
        const char *pszDraft = "RESTORE D";
        const char *pszCharacter;

        for(pszCharacter = pszDraft; *pszCharacter != '\0'; ++pszCharacter)
        {
            Floppy144TerminalInputCharacter(
                &sTerminal,
                *pszCharacter
            );
        }
    }

    Floppy144TerminalMoveHistory(&sTerminal, -1);
    F144_CHECK(
        strcmp(sTerminal.input, "LIST") == 0,
        "Up recalls newest command"
    );

    Floppy144TerminalMoveHistory(&sTerminal, -1);
    F144_CHECK(
        strcmp(sTerminal.input, "INITIATE") == 0,
        "second Up recalls older command"
    );

    Floppy144TerminalMoveHistory(&sTerminal, -1);
    Floppy144TerminalMoveHistory(&sTerminal, -1);
    F144_CHECK(
        strcmp(sTerminal.input, "HELP") == 0,
        "history clamps at oldest command"
    );

    Floppy144TerminalMoveHistory(&sTerminal, 1);
    F144_CHECK(
        strcmp(sTerminal.input, "INITIATE") == 0,
        "Down recalls newer command"
    );

    Floppy144TerminalMoveHistory(&sTerminal, 1);
    F144_CHECK(
        strcmp(sTerminal.input, "LIST") == 0,
        "Down reaches newest stored command"
    );

    Floppy144TerminalMoveHistory(&sTerminal, 1);
    F144_CHECK(
        strcmp(sTerminal.input, "RESTORE D") == 0 &&
        sTerminal.history_cursor == -1,
        "Down past newest restores unfinished draft"
    );

    Floppy144TerminalMoveHistory(&sTerminal, -1);
    Floppy144TerminalBackspace(&sTerminal);
    F144_CHECK(
        strcmp(sTerminal.input, "LIS") == 0 &&
        sTerminal.history_cursor == -1,
        "editing a recalled command detaches history navigation"
    );

    Floppy144TerminalReset(&sTerminal, &sWorld);
    F144_CHECK(
        sTerminal.history_count == 0U &&
        sTerminal.history_cursor == -1,
        "terminal reset starts a fresh non-persistent command history"
    );
}


/*
 * DR-04 is the player-facing Act II branch choice.
 *
 * Both records are initially readable. Opening one commits the branch, and a
 * direct OPEN of the other must then be deferred until T-028 releases the
 * alternate workstream.
 */
static void Floppy144TestBranchDocumentAccessGate(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;

    Floppy144CollectionId eDr04;
    Floppy144EvidenceId eE003;
    Floppy144TriggerId eT026;
    Floppy144TriggerId eT028;
    const Floppy144DocumentDefinition *pRecordsBranchDocument;
    const Floppy144DocumentDefinition *pTechnologyBranchDocument;
    char szBranchCommand[48];

    Floppy144WorldReset(
        &sWorld
    );

    Floppy144RunStateBegin(
        &sRunState,
        144U
    );

    F144_CHECK(
        Floppy144WorldInitialiseArchiveServices(
            &sWorld
        ) &&
        Floppy144RunStateInitialiseArchiveServices(
            &sRunState
        ),
        "branch fixture initialises archive services"
    );

    eDr04 =
        Floppy144GameDataCollectionId(
            "DR-04"
        );

    eE003 =
        Floppy144GameDataEvidenceId(
            "E-003"
        );

    eT026 =
        Floppy144GameDataTriggerId(
            "T-026"
        );

    eT028 =
        Floppy144GameDataTriggerId(
            "T-028"
        );

    F144_CHECK(
        eDr04 < FLOPPY144_COLLECTION_COUNT &&
        eE003 < FLOPPY144_EVIDENCE_COUNT &&
        eT026 < FLOPPY144_TRIGGER_COUNT &&
        eT028 < FLOPPY144_TRIGGER_COUNT,
        "branch fixture IDs resolve"
    );

    pRecordsBranchDocument =
        Floppy144TestDocumentForTrigger(
            eDr04,
            Floppy144GameDataTriggerId("T-010")
        );

    pTechnologyBranchDocument =
        Floppy144TestDocumentForTrigger(
            eDr04,
            Floppy144GameDataTriggerId("T-011")
        );

    F144_CHECK(
        pRecordsBranchDocument != NULL &&
        pRecordsBranchDocument->record_id_override != NULL &&
        pTechnologyBranchDocument != NULL &&
        pTechnologyBranchDocument->record_id_override != NULL,
        "both DR-04 branch documents resolve from trigger registry"
    );

    /*
     * This fixture starts at the already-recovered DR-04 branch point. Set the
     * restored bit directly so the test is independent of reconstruction
     * budget consumed by the earlier narrative route.
     */
    F144_CHECK(
        Floppy144RunStateBitSet(
            sRunState.collections,
            (uint32_t)eDr04
        ) &&
        Floppy144WorldRestoreCollection(
            &sWorld,
            eDr04
        ),
        "DR-04 is restored for branch fixture"
    );

    /*
     * Initialise the terminal after establishing the fixture's recovered
     * collection state. TerminalReset derives OPEN availability from the
     * collections currently restored in the World.
     */
    Floppy144TerminalReset(
        &sTerminal,
        &sWorld
    );

    F144_CHECK(
        Floppy144RunStateEstablishEvidence(
            &sRunState,
            eE003
        ),
        "E-003 exposes both branch records initially"
    );

    (void)snprintf(
        szBranchCommand,
        sizeof(szBranchCommand),
        "OPEN %s",
        pRecordsBranchDocument != NULL &&
        pRecordsBranchDocument->record_id_override != NULL
            ? pRecordsBranchDocument->record_id_override
            : "DR-04-RS-0000"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        szBranchCommand
    );

    F144_CHECK(
        sTerminal.open_record_requested,
        "Records-first branch record is initially readable"
    );

    if(sTerminal.open_record_requested)
    {
        F144_CHECK(
            Floppy144DocumentApplyEffects(
                &sWorld,
                &sRunState,
                sTerminal.requested_collection,
                sTerminal.requested_record_index
            ),
            "Records-first branch document applies effects"
        );
    }

    sTerminal.open_record_requested = false;

    F144_CHECK(
        sRunState.branch ==
            (uint8_t)FLOPPY144_RUN_BRANCH_RECORDS_FIRST,
        "opening first workstream commits Records branch"
    );

    (void)snprintf(
        szBranchCommand,
        sizeof(szBranchCommand),
        "OPEN %s",
        pTechnologyBranchDocument != NULL &&
        pTechnologyBranchDocument->record_id_override != NULL
            ? pTechnologyBranchDocument->record_id_override
            : "DR-04-RS-0000"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        szBranchCommand
    );

    F144_CHECK(
        !sTerminal.open_record_requested &&
        Floppy144TestTerminalContains(
            &sTerminal,
            "RECORD ACCESS DEFERRED BY RECOVERY SEQUENCE."
        ),
        "unchosen branch record is deferred during Act II"
    );

    F144_CHECK(
        Floppy144RunStateFireTrigger(
            &sRunState,
            eT026
        ),
        "branch fixture marks Act II core complete"
    );

    F144_CHECK(
        Floppy144GameDataTriggerTryFire(
            &sWorld,
            &sRunState,
            eT028
        ),
        "T-028 releases the alternate workstream"
    );

    (void)snprintf(
        szBranchCommand,
        sizeof(szBranchCommand),
        "OPEN %s",
        pTechnologyBranchDocument != NULL &&
        pTechnologyBranchDocument->record_id_override != NULL
            ? pTechnologyBranchDocument->record_id_override
            : "DR-04-RS-0000"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        szBranchCommand
    );

    F144_CHECK(
        sTerminal.open_record_requested,
        "alternate branch record becomes readable after Act II"
    );
}

int main(void)
{
    Floppy144TestExtendedGlyphs();
    Floppy144TestCatalogueRecordResolution();
    Floppy144TestDocumentBodyScrolling();
    Floppy144TestCollectionListPresentation();
    Floppy144TestMultipleCollectionCommands();
    Floppy144TestCommandHistory();
    Floppy144TestBranchDocumentAccessGate();

    if(g_nFailures != 0)
    {
        fprintf(
            stderr,
            "\nSTAGE 3B.1 TERMINAL TESTS: FAIL (%d failure%s)\n",
            g_nFailures,
            g_nFailures == 1 ? "" : "s"
        );

        return 1;
    }

    puts("\nSTAGE 3B.1 TERMINAL TESTS: PASS");
    return 0;
}
