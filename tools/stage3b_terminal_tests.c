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
#include "floppy144_persistence.h"
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

    /*
     * Player-facing RESTORE now owns a short size-weighted progress animation.
     * Existing command tests care about the completed command semantics, so
     * advance any pending restore deterministically instead of sleeping.
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

static void Floppy144TestRestoreProgress(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;
    Floppy144CollectionId eDr01;
    const Floppy144CollectionDefinition *pDefinition;
    const char *pszCommand = "RESTORE DR-01";
    const char *pszCharacter;

    Floppy144WorldReset(&sWorld);
    Floppy144RunStateBegin(&sRunState, 144U);
    Floppy144TerminalReset(&sTerminal, &sWorld);

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "INITIATE"
    );

    eDr01 =
        Floppy144GameDataCollectionId(
            "DR-01"
        );
    pDefinition =
        Floppy144CollectionGet(
            eDr01
        );

    for(
        pszCharacter = pszCommand;
        *pszCharacter != '\0';
        ++pszCharacter
    )
    {
        Floppy144TerminalInputCharacter(
            &sTerminal,
            *pszCharacter
        );
    }

    Floppy144TerminalSubmitInput(
        &sTerminal,
        &sWorld,
        &sRunState
    );

    F144_CHECK(
        Floppy144TerminalRestoreInProgress(&sTerminal),
        "RESTORE starts a visible progress phase before collection commit"
    );

    F144_CHECK(
        pDefinition != NULL &&
        sTerminal.restoration_duration_ms ==
            300U + pDefinition->size_kb * 18U,
        "restore duration scales from the collection size"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionRestored(
            &sRunState,
            eDr01
        ),
        "collection remains uncommitted while progress is incomplete"
    );

    Floppy144TerminalAdvanceRestore(
        &sTerminal,
        &sWorld,
        &sRunState,
        sTerminal.restoration_duration_ms / 2U
    );

    F144_CHECK(
        Floppy144TerminalRestoreProgressPercent(
            &sTerminal
        ) >= 49U &&
        Floppy144TerminalRestoreProgressPercent(
            &sTerminal
        ) <= 50U,
        "restore progress tracks elapsed size-weighted duration"
    );

    Floppy144TerminalAdvanceRestore(
        &sTerminal,
        &sWorld,
        &sRunState,
        sTerminal.restoration_duration_ms
    );

    F144_CHECK(
        !Floppy144TerminalRestoreInProgress(&sTerminal) &&
        Floppy144RunStateCollectionRestored(
            &sRunState,
            eDr01
        ) &&
        Floppy144WorldCollectionRestored(
            &sWorld,
            eDr01
        ),
        "collection commits only when progress reaches 100 percent"
    );
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

static const char *Floppy144TestShortAuthoredId(
    const Floppy144DocumentDefinition *document
)
{
    const char *separator;
    if(document == NULL || document->record_id_override == NULL)
        return "RS-UNAVAILABLE";
    separator = strstr(document->record_id_override, "-RS-");
    return separator != NULL ? separator + 1 : document->record_id_override;
}

/* Resolve player-facing shorthand after the seed-scoped DR-04 mapping. */
static const char *Floppy144TestShortSeededAuthoredId(
    const Floppy144DocumentDefinition *document, uint32_t seed
)
{
    const char *id = Floppy144DocumentRecordIdForSeed(document, seed);
    const char *marker = id != NULL ? strstr(id, "-RS-") : NULL;
    return marker != NULL ? marker + 1 : "RS-UNAVAILABLE";
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
        bool bPreviousPinned = false;

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

                const Floppy144DocumentDefinition *pAuthored =
                    Floppy144DocumentGet(eCollection,uRecord);
                bool bPinned =
                    pAuthored != NULL &&
                    pAuthored->record_id_override != NULL &&
                    (
                        strcmp(pAuthored->record_id_override,
                               "DR-01-RS-0001") == 0 ||
                        strcmp(pAuthored->record_id_override,
                               "FM-13-RS-0047") == 0 ||
                        strcmp(pAuthored->record_id_override,
                               "HR-01-RS-0107") == 0
                    );
                F144_CHECK(
                    bNumberValid &&
                    (
                        uRecord == 0U ||
                        uCurrentRecordNumber > uPreviousRecordNumber
                    ),
                    "all generated and pinned story IDs remain strictly ascending"
                );

                if(bNumberValid)
                {
                    if(uRecord > 0U && !bPinned && !bPreviousPinned)
                    {
                        ++uComparedGaps;
                        if(
                            uCurrentRecordNumber -
                            uPreviousRecordNumber != 10U
                        )
                            ++uIrregularGaps;
                    }
                    uPreviousRecordNumber = uCurrentRecordNumber;
                }
                bPreviousPinned=bPinned;
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

    /*
     * BUG FIX 08: move the authored HR-01 RS-0107 ahead of RS-0113
     * without changing a player-facing ID, changing the trigger or
     * displacing the authored RS-0134 hot-desk record.
     */
    {
        static const char *const ordered_ids[] = {
            "HR-01-RS-0097", "HR-01-RS-0107",
            "HR-01-RS-0113", "HR-01-RS-0134"
        };
        uint32_t index;
        char id[24], title[48];
        Floppy144CollectionId resolved = FLOPPY144_COLLECTION_COUNT;
        uint32_t slot = UINT32_MAX;

        for(index = 0U; index < 4U; ++index)
        {
            Floppy144CatalogueBuildRecord(
                FLOPPY144_COLLECTION_HR01,
                8U + index,
                id,sizeof(id),title,sizeof(title)
            );
            F144_CHECK(
                strcmp(id,ordered_ids[index]) == 0,
                "HR-01 preserved record numbering strictly follows LIST order"
            );
        }
        F144_CHECK(
            Floppy144CatalogueFindRecord(
                "HR-01-RS-0107", &resolved, &slot
            ) &&
            resolved == FLOPPY144_COLLECTION_HR01 &&
            slot == 9U,
            "RS-0107 OPEN lookup follows the moved authored record"
        );
        {
            const Floppy144DocumentDefinition *author =
                Floppy144DocumentGet(FLOPPY144_COLLECTION_HR01,9U);
            F144_CHECK(
                author != NULL &&
                author->trigger == FLOPPY144_TRIGGER_T003 &&
                strcmp(author->record_id_override,"HR-01-RS-0107") == 0,
                "HR-01 T-003 remains bound to RS-0107 after reorder"
            );
        }
        F144_CHECK(
            Floppy144CatalogueFindRecord(
                "HR-01-RS-0113",&resolved,&slot
            ) &&
            resolved == FLOPPY144_COLLECTION_HR01 && slot == 10U &&
            Floppy144DocumentGet(FLOPPY144_COLLECTION_HR01,10U)==NULL,
            "original procedural RS-0113 survives and opens at new slot"
        );
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
 * BUG FIX 09: exercise the actual software renderer, not only document flags.
 *
 * The earlier short-document branch rendered a complete authored body, then
 * entered the scrollbar's else and overprinted three index-only status lines.
 * Amber status glyphs in the document text viewport therefore identify the
 * regression without depending on any individual line of prose or font pixel.
 * Scrollbar amber pixels at x=594 are deliberately outside the tested band.
 */
static uint32_t g_document_view_test_pixels[640U * 360U];

static uint32_t Floppy144TestCountViewerPixels(
    uint32_t colour, uint32_t x0, uint32_t y0,
    uint32_t x1, uint32_t y1)
{
    uint32_t count = 0U, x, y;
    for(y=y0;y<y1;++y)
    for(x=x0;x<x1;++x)
    {
        if(g_document_view_test_pixels[y*640U+x]==colour)
            ++count;
    }
    return count;
}

static void Floppy144TestDocumentViewerContentStates(void)
{
    const uint32_t amber = FLOPPY144_RGB(194,153,76);
    const uint32_t body_text = FLOPPY144_RGB(202,211,205);
    const uint32_t muted = FLOPPY144_RGB(118,133,132);
    Floppy144Surface surface = {
        g_document_view_test_pixels,640U,360U
    };
    Floppy144CatalogueState catalogue;
    Floppy144CollectionId collection = FLOPPY144_COLLECTION_COUNT;
    uint32_t slot = UINT32_MAX;
    uint32_t body_amber;

    /* The reported short authored record used to show both body and status. */
    F144_CHECK(
        Floppy144CatalogueFindRecord(
            "HR-01-RS-0171",&collection,&slot) &&
        collection==FLOPPY144_COLLECTION_HR01,
        "HR-01-RS-0171 resolves to its authored catalogue slot"
    );
    if(collection!=FLOPPY144_COLLECTION_HR01) return;

    F144_CHECK(
        Floppy144CatalogueOpenRecord(&catalogue,collection,slot),
        "short authored HR-01 document opens normally"
    );
    Floppy144CatalogueDraw(&surface,&catalogue);
    body_amber=Floppy144TestCountViewerPixels(
        amber,52U,102U,588U,285U);
    F144_CHECK(
        body_amber==0U &&
        Floppy144TestCountViewerPixels(
            muted,52U,102U,588U,285U)==0U &&
        Floppy144TestCountViewerPixels(
            body_text,52U,102U,588U,115U)>0U,
        "short authored content is visible without index-only status overlay"
    );
    Floppy144CatalogueScrollDocument(&catalogue,1);
    F144_CHECK(
        catalogue.document_scroll_line==0U,
        "short authored record needs no scrollbar or scrolling"
    );

    /*
     * A procedural entry has no document-body registry row. It should show
     * the index-only placeholder, never fake recovered text or scroll.
     * HR-01 slot 10 is the generated RS-0113 entry after BUG FIX 08.
     */
    F144_CHECK(
        Floppy144CatalogueOpenRecord(
            &catalogue,FLOPPY144_COLLECTION_HR01,10U) &&
        Floppy144DocumentGet(FLOPPY144_COLLECTION_HR01,10U)==NULL,
        "generated/index-only record is genuinely without authored body"
    );
    Floppy144CatalogueDraw(&surface,&catalogue);
    F144_CHECK(
        Floppy144TestCountViewerPixels(
            amber,52U,130U,588U,155U)>0U &&
        Floppy144TestCountViewerPixels(
            muted,52U,180U,588U,200U)>0U &&
        Floppy144TestCountViewerPixels(
            body_text,52U,102U,588U,115U)==0U,
        "index-only status remains legible in its exclusive placeholder"
    );
    Floppy144CatalogueScrollDocument(&catalogue,1);
    F144_CHECK(
        catalogue.document_scroll_line==0U,
        "index-only document cannot scroll a missing body"
    );

    /* Reusing the same frame buffer must not ghost old status over a body. */
    F144_CHECK(
        Floppy144CatalogueOpenRecord(
            &catalogue,FLOPPY144_COLLECTION_HR01,slot),
        "reopened short authored record replaces generated index page"
    );
    Floppy144CatalogueDraw(&surface,&catalogue);
    F144_CHECK(
        Floppy144TestCountViewerPixels(
            amber,52U,102U,588U,285U)==0U &&
        Floppy144TestCountViewerPixels(
            muted,52U,102U,588U,285U)==0U &&
        Floppy144TestCountViewerPixels(
            body_text,52U,102U,588U,115U)>0U &&
        catalogue.document_scroll_line==0U,
        "reopened authored body has neither stale index status nor stale scroll"
    );

    /* A regular recovered trigger document also owns the entire body band. */
    F144_CHECK(
        Floppy144CatalogueFindRecord(
            "DR-01-RS-0001",&collection,&slot),
        "normal recovered DR-01 document resolves"
    );
    F144_CHECK(
        Floppy144CatalogueOpenRecord(&catalogue,collection,slot),
        "normal recovered record opens"
    );
    Floppy144CatalogueDraw(&surface,&catalogue);
    F144_CHECK(
        Floppy144TestCountViewerPixels(
            amber,52U,102U,588U,285U)==0U &&
        Floppy144TestCountViewerPixels(
            muted,52U,102U,588U,285U)==0U &&
        Floppy144TestCountViewerPixels(
            body_text,52U,102U,588U,115U)>0U,
        "normal recovered record displays its body, not index-only status"
    );

    /* Long, scrollable documents also must never print missing-body status. */
    F144_CHECK(
        Floppy144CatalogueFindRecord(
            "FM-04-RS-0035",&collection,&slot),
        "long FM-04 record resolves for viewer status regression"
    );
    if(collection==FLOPPY144_COLLECTION_COUNT) return;
    F144_CHECK(
        Floppy144CatalogueOpenRecord(&catalogue,collection,slot),
        "long authored document opens normally"
    );
    Floppy144CatalogueDraw(&surface,&catalogue);
    F144_CHECK(
        Floppy144TestCountViewerPixels(
            amber,52U,102U,588U,285U)==0U,
        "long authored first page cannot display index-only status"
    );
    Floppy144CatalogueScrollDocument(&catalogue,1);
    Floppy144CatalogueDraw(&surface,&catalogue);
    F144_CHECK(
        catalogue.document_scroll_line==1U &&
        Floppy144TestCountViewerPixels(
            amber,52U,102U,588U,285U)==0U,
        "scrolled document retains content-only body and live scrollbar"
    );
    /* The header/footer remain outside the exclusive body/status viewport. */
    F144_CHECK(
        Floppy144TestCountViewerPixels(
            muted,10U,5U,260U,15U)>0U &&
        Floppy144TestCountViewerPixels(
            amber,10U,311U,620U,334U)>0U,
        "document viewer header and footer remain visible"
    );
    Floppy144CatalogueCloseDocument(&catalogue);
    Floppy144CatalogueOpenDocument(&catalogue);
    Floppy144CatalogueDraw(&surface,&catalogue);
    F144_CHECK(
        catalogue.document_scroll_line==0U &&
        Floppy144TestCountViewerPixels(
            amber,52U,102U,588U,285U)==0U,
        "reopening long content resets scroll without creating ghosted status"
    );
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
        strstr(sTerminal.output[1], "Q: RETURN") != NULL,
        "collection LIST page visibly advertises Q: RETURN"
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

    for(uIndex = 0U; uIndex < sTerminal.output_count; ++uIndex)
    {
        F144_CHECK(
            strlen(sTerminal.output[uIndex]) <
                FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY,
            "collection LIST rows remain within terminal line capacity"
        );
    }

    /*
     * Re-run the same page with two long collection names visible. The old
     * fixed 27-character column truncated both; the dynamic column must keep
     * them intact while preserving STATUS and SIZE.
     */
    Floppy144TerminalCloseRecordPager(&sTerminal);

    (void)Floppy144RunStateBitSet(
        sRunState.collections,
        (uint32_t)FLOPPY144_COLLECTION_DR04
    );

    (void)Floppy144RunStateBitSet(
        sRunState.collections,
        (uint32_t)FLOPPY144_COLLECTION_DR07
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "LIST"
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "Site Closure & Building Handover"
        ) &&
        Floppy144TestTerminalContains(
            &sTerminal,
            "Archive Holdings & Reconciliation"
        ),
        "collection LIST expands its name column to preserve visible collection names"
    );

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

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "LIST DR-02"
    );
    F144_CHECK(
        Floppy144TerminalRecordPagerActive(&sTerminal) &&
        sTerminal.record_pager_collection == eDr02 &&
        Floppy144TestTerminalContains(
            &sTerminal,
            "PAGE 1 OF 2"
        ),
        "expanded DR-02 index opens a two-page document LIST"
    );
    Floppy144TerminalCloseRecordPager(&sTerminal);

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
        sTerminal.record_pager_page == 3U,
        "record pager advances to third page"
    );

    Floppy144TerminalMoveRecordPager(&sTerminal, -1);
    F144_CHECK(
        sTerminal.record_pager_page == 2U,
        "record pager can navigate back to page two"
    );
    Floppy144TerminalMoveRecordPager(&sTerminal, -1);
    F144_CHECK(
        sTerminal.record_pager_page == 1U,
        "record pager moves backward"
    );

    F144_CHECK(
        sTerminal.output_count == FLOPPY144_TERMINAL_OUTPUT_LINES,
        "full ten-record pager page fits exactly within terminal output"
    );

    F144_CHECK(
        strstr(
            sTerminal.output[0],
            "COLLECTION HR-01:"
        ) != NULL &&
        strstr(
            sTerminal.output[0],
            "            PAGE 1 OF 3"
        ) != NULL,
        "record pager keeps collection and page indicator together on first line"
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
 * Player-facing recovery policy: normal runs receive only the first-profile
 * DR tutorial, while departmental restoration is tied to physical terminals.
 */
static bool Floppy144TestNotebookContains(
    const Floppy144RunState *pRunState,
    const char *pszId,
    const char *pszText
)
{
    uint32_t uIndex;

    for(
        uIndex = 0U;
        uIndex < Floppy144GameDataNotebookEntryCount(pRunState);
        ++uIndex
    )
    {
        const Floppy144DataRecord *pEntry =
            Floppy144GameDataNotebookEntryAt(
                pRunState,
                uIndex
            );

        if(
            pEntry != NULL &&
            pEntry->pszId != NULL &&
            strcmp(pEntry->pszId, pszId) == 0 &&
            (
                pszText == NULL ||
                (
                    pEntry->pszA != NULL &&
                    strstr(pEntry->pszA, pszText) != NULL
                )
            )
        )
        {
            return true;
        }
    }

    return false;
}

static void Floppy144TestPlayerRecoveryPolicy(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;
    Floppy144CollectionId eDr02;
    Floppy144CollectionId eHr01;
    Floppy144CollectionId eFm04;
    Floppy144CollectionId eFm07;
    Floppy144CollectionId eFm23;
    Floppy144CollectionId eTs10;
    Floppy144InteractionId eI042;

    Floppy144TestReachOpeningCollections(
        &sWorld,
        &sRunState,
        &sTerminal
    );

    eDr02 = Floppy144GameDataCollectionId("DR-02");
    eHr01 = Floppy144GameDataCollectionId("HR-01");
    eFm04 = Floppy144GameDataCollectionId("FM-04");
    eFm07 = Floppy144GameDataCollectionId("FM-07");
    eFm23 = Floppy144GameDataCollectionId("FM-23");
    eTs10 = Floppy144GameDataCollectionId("TS-10");
    eI042 = Floppy144GameDataInteractionId("I-042");

    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        false,
        true
    );
    sTerminal.output_count = 0U;
    Floppy144TerminalPrintNextAction(
        &sTerminal,
        &sRunState
    );

    F144_CHECK(
        !Floppy144TestTerminalContains(
            &sTerminal,
            "RESTORE DR-02"
        ) &&
        Floppy144TestTerminalContains(
            &sTerminal,
            "EXIT TO SITE"
        ),
        "later profile runs suppress DR-02/DR-03 tutorial recommendation"
    );

    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        true,
        true
    );
    sTerminal.output_count = 0U;
    Floppy144TerminalPrintNextAction(
        &sTerminal,
        &sRunState
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "RESTORE DR-02"
        ),
        "first profile run retains DR-02/DR-03 tutorial recommendation"
    );

    Floppy144TerminalResetAtRoom(
        &sTerminal,
        &sWorld,
        FLOPPY144_ROOM_SECURITY
    );
    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        true,
        true
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE DR-02"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionRestored(
            &sRunState,
            eDr02
        ) &&
        Floppy144TestTerminalContains(
            &sTerminal,
            "RESTORE REFUSED"
        ),
        "Security terminal refuses DR collection restoration"
    );

    Floppy144TerminalResetAtRoom(
        &sTerminal,
        &sWorld,
        FLOPPY144_ROOM_RECEPTION
    );
    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        true,
        true
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE DR-02"
    );

    F144_CHECK(
        Floppy144RunStateCollectionRestored(
            &sRunState,
            eDr02
        ),
        "Reception terminal permits DR collection restoration"
    );

    Floppy144TerminalResetAtRoom(
        &sTerminal,
        &sWorld,
        FLOPPY144_ROOM_IT_SUPPORT
    );
    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        true,
        true
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE HR-01"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionRestored(
            &sRunState,
            eHr01
        ),
        "IT Support terminal refuses HR collection restoration"
    );

    Floppy144TerminalResetAtRoom(
        &sTerminal,
        &sWorld,
        FLOPPY144_ROOM_MAIN_OFFICE
    );
    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        true,
        true
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE HR-01"
    );

    F144_CHECK(
        Floppy144RunStateCollectionRestored(
            &sRunState,
            eHr01
        ),
        "Main Office terminal permits HR collection restoration"
    );

    F144_CHECK(
        eFm04 < FLOPPY144_COLLECTION_COUNT &&
        eFm07 < FLOPPY144_COLLECTION_COUNT &&
        eFm23 < FLOPPY144_COLLECTION_COUNT &&
        eTs10 < FLOPPY144_COLLECTION_COUNT &&
        eI042 < FLOPPY144_INTERACTION_COUNT,
        "Facilities bootstrap regression IDs resolve"
    );

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sRunState,
            FLOPPY144_ROOM_MAIN_OFFICE
        ),
        "FM-04 bootstrap fixture reconstructs Main Office"
    );

    Floppy144TerminalResetAtRoom(
        &sTerminal,
        &sWorld,
        FLOPPY144_ROOM_MAIN_OFFICE
    );
    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        true,
        true
    );
    Floppy144TerminalRefreshEnvironmentLine(
        &sTerminal,
        &sRunState
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE FM-04"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionRestored(
            &sRunState,
            eFm04
        ) &&
        Floppy144TestTerminalContains(
            &sTerminal,
            "PROJECT MANAGER'S DESK"
        ),
        "Main Office refuses FM-04 until Project Manager authorisation is inspected"
    );

    F144_CHECK(
        Floppy144GameDataInteractionTryRun(
            &sWorld,
            &sRunState,
            eI042
        ) &&
        Floppy144RunStateInteractionCompleted(
            &sRunState,
            eI042
        ) &&
        Floppy144RunStateHasCapability(
            &sRunState,
            FLOPPY144_CAPABILITY_MAIN_OFFICE_FACILITIES_TERMINAL
        ),
        "Project Manager authorisation upgrades Main Office terminal for FM-04"
    );

    F144_CHECK(
        Floppy144TestNotebookContains(
            &sRunState,
            "I-042",
            "Temporary access to Facilities"
        ),
        "Project Manager authorisation writes the temporary Facilities access note"
    );

    /*
     * Acquisition is permanent. Temporarily remove the source completion bit
     * to prove the viewer does not re-evaluate old notes against live state.
     */
    (void)Floppy144RunStateBitClear(
        sRunState.interactions,
        (uint32_t)eI042
    );

    F144_CHECK(
        !Floppy144RunStateInteractionCompleted(
            &sRunState,
            eI042
        ) &&
        Floppy144TestNotebookContains(
            &sRunState,
            "I-042",
            "Temporary access to Facilities"
        ),
        "written Notebook entries remain visible after their source predicate changes"
    );

    (void)Floppy144RunStateBitSet(
        sRunState.interactions,
        (uint32_t)eI042
    );

    Floppy144TerminalResetAtRoom(
        &sTerminal,
        &sWorld,
        FLOPPY144_ROOM_MAIN_OFFICE
    );
    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        true,
        true
    );
    Floppy144TerminalRefreshEnvironmentLine(
        &sTerminal,
        &sRunState
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "MAIN OFFICE / FACILITIES TERMINAL"
        ),
        "authorised Main Office terminal advertises combined Facilities role"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE FM-04"
    );

    F144_CHECK(
        Floppy144RunStateCollectionRestored(
            &sRunState,
            eFm04
        ),
        "authorised Main Office / Facilities terminal permits FM-04 bootstrap restoration"
    );

    F144_CHECK(
        Floppy144TestNotebookContains(
            &sRunState,
            "I-042",
            "Temporary access to Facilities"
        ),
        "temporary Facilities access note remains permanently in the Notebook after FM-04 restoration"
    );

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sRunState,
            FLOPPY144_ROOM_FACILITIES
        ),
        "FM location fixture reconstructs Facilities"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE FM-07"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionRestored(
            &sRunState,
            eFm07
        ) &&
        Floppy144TestTerminalContains(
            &sTerminal,
            "RESTORE REFUSED"
        ),
        "Main Office FM exception remains limited to FM-04"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionRestored(&sRunState,eTs10) &&
        Floppy144RunStateSetAct(&sRunState,FLOPPY144_RUN_ACT_II),
        "FM-23 timing fixture reaches Act II before TS-10"
    );

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sRunState,
            eFm23
        ),
        "FM-23 becomes available during Act II without waiting for TS-10"
    );

    {
        const Floppy144CollectionDefinition *pFm23=
            Floppy144CollectionGet(eFm23);
        const Floppy144DocumentDefinition *pFm23Upgrade =
            Floppy144TestDocumentForTrigger(
                eFm23,Floppy144GameDataTriggerId("T-025")
            );
        uint32_t uRecord;
        uint32_t uAuthored=0U;

        for(
            uRecord=0U;
            pFm23!=NULL &&
            uRecord<pFm23->catalogue.record_count;
            ++uRecord
        )
        {
            if(Floppy144DocumentGet(eFm23,uRecord)!=NULL)
            {
                ++uAuthored;
            }
        }

        F144_CHECK(
            pFm23!=NULL &&
            pFm23->size_kb==101U &&
            pFm23->catalogue.record_count==50U &&
            uAuthored==6U,
            "FM-23 is a full optional collection with 50 records and six authored documents"
        );

        F144_CHECK(
            Floppy144RunStateBitSet(
                sRunState.collections,
                (uint32_t)eFm23
            ) &&
            Floppy144DocumentApplyEffects(
                &sWorld,
                &sRunState,
                eFm23,
                pFm23Upgrade != NULL ? pFm23Upgrade->record_index : 0U
            ) &&
            Floppy144GameDataFactRecorded(
                &sRunState,
                "FM-23_ISOMETRIC_DIRECTORY"
            ) &&
            !Floppy144RunStateIsIsometric(&sRunState),
            "FM-23 trigger upgrades only the Site Directory and leaves live Site projection 2D"
        );
    }
}


static void Floppy144TestServerRoomItTerminal(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144TerminalState sTerminal;
    Floppy144CollectionId eTs14;

    Floppy144TestReachOpeningCollections(
        &sWorld,
        &sRunState,
        &sTerminal
    );

    eTs14 = Floppy144GameDataCollectionId("TS-14");

    F144_CHECK(
        eTs14 < FLOPPY144_COLLECTION_COUNT &&
        Floppy144RunStateSetBranch(
            &sRunState,
            FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST
        ) &&
        Floppy144RunStateSetAct(
            &sRunState,
            FLOPPY144_RUN_ACT_III
        ) &&
        Floppy144RunStateReconstructRoom(
            &sRunState,
            FLOPPY144_ROOM_SERVER_ROOM
        ),
        "Server Room IT-terminal fixture reaches Technology Act III"
    );

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sRunState,
            eTs14
        ),
        "TS-14 is recoverable for the Server Room IT-terminal fixture"
    );

    Floppy144TerminalResetAtRoom(
        &sTerminal,
        &sWorld,
        FLOPPY144_ROOM_SERVER_ROOM
    );
    Floppy144TerminalConfigureSession(
        &sTerminal,
        false,
        false,
        true
    );
    Floppy144TerminalRefreshEnvironmentLine(
        &sTerminal,
        &sRunState
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            "SERVER ROOM IT TERMINAL"
        ),
        "Server Room terminal identifies itself as an IT terminal"
    );

    Floppy144TestSubmitCommand(
        &sTerminal,
        &sWorld,
        &sRunState,
        "RESTORE TS-14"
    );

    F144_CHECK(
        Floppy144RunStateCollectionRestored(
            &sRunState,
            eTs14
        ),
        "Server Room IT terminal permits Technology Services restoration"
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
    const Floppy144DocumentDefinition *pEntryDocument;
    const Floppy144DocumentDefinition *pChoiceBriefing;
    const Floppy144DocumentDefinition *pRecordsBranchDocument;
    const Floppy144DocumentDefinition *pTechnologyBranchDocument;
    char szBranchCommand[48];
    char szNextBriefing[96];
    char szBranchChoices[112];

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

    pEntryDocument =
        Floppy144DocumentRecoveryEntryPoint(
            eDr04
        );

    pChoiceBriefing =
        Floppy144DocumentChoiceBriefing(
            eDr04
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

    (void)snprintf(
        szNextBriefing,sizeof(szNextBriefing),
        "NEXT RECOVERY ACTION: OPEN %s",
        Floppy144TestShortAuthoredId(pChoiceBriefing)
    );
    (void)snprintf(
        szBranchChoices,sizeof(szBranchChoices),
        "NEXT RECOVERY ACTION: OPEN %s OR %s",
        Floppy144TestShortSeededAuthoredId(pRecordsBranchDocument,sRunState.recovery_seed),
        Floppy144TestShortSeededAuthoredId(pTechnologyBranchDocument,sRunState.recovery_seed)
    );

    F144_CHECK(
        pEntryDocument != NULL &&
        pEntryDocument->record_id_override != NULL &&
        strcmp(
            pEntryDocument->title_override,
            "Handover Readiness Checklist"
        ) == 0,
        "DR-04 handover checklist is the first player-facing recovery document"
    );

    F144_CHECK(
        pChoiceBriefing != NULL &&
        pChoiceBriefing->record_id_override != NULL &&
        strcmp(
            pChoiceBriefing->title_override,
            "Outstanding Workstream Summary"
        ) == 0,
        "DR-04 neutral workstream summary follows the handover checklist"
    );

    F144_CHECK(
        pRecordsBranchDocument != NULL &&
        pRecordsBranchDocument->record_id_override != NULL &&
        strcmp(
            pRecordsBranchDocument->title_override,
            "Physical Archive Reconciliation Workstream"
        ) == 0 &&
        pTechnologyBranchDocument != NULL &&
        pTechnologyBranchDocument->record_id_override != NULL &&
        strcmp(
            pTechnologyBranchDocument->title_override,
            "Terminal Network Remediation Workstream"
        ) == 0,
        "both DR-04 branch documents follow the neutral briefing"
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

    /*
     * Simulate the collection-local shorthand established by RESTORE DR-04.
     * The canonical neutral breadcrumb is:
     *
     *   Handover Readiness Checklist
     *       -> Outstanding Workstream Summary
     *       -> whichever current DR-04 records hold the two workstreams.
     */
    sTerminal.default_record_collection =
        eDr04;

    sTerminal.default_record_collection_valid =
        true;

    sTerminal.output_count = 0U;

    Floppy144TerminalPrintPostOpenAction(
        &sTerminal,
        &sRunState,
        eDr04,
        pEntryDocument != NULL
            ? pEntryDocument->record_index
            : 0U
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            szNextBriefing
        ),
        "DR-04 handover checklist points to the neutral workstream summary"
    );

    sTerminal.output_count = 0U;

    Floppy144TerminalPrintPostOpenAction(
        &sTerminal,
        &sRunState,
        eDr04,
        pChoiceBriefing != NULL
            ? pChoiceBriefing->record_index
            : 0U
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            szBranchChoices
        ),
        "DR-04 briefing offers Records or Technology as an equal branch choice"
    );

    (void)snprintf(
        szBranchCommand,
        sizeof(szBranchCommand),
        "OPEN %s",
        pRecordsBranchDocument != NULL &&
        pRecordsBranchDocument->record_id_override != NULL
            ? Floppy144DocumentRecordIdForSeed(pRecordsBranchDocument,sRunState.recovery_seed)
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

    F144_CHECK(
        Floppy144GameDataCollectionEnabled(
            &sRunState,
            "DR-23"
        ),
        "Records Office reconstruction unlocks optional DR-23 context"
    );

    F144_CHECK(
        !Floppy144GameDataCollectionEnabled(
            &sRunState,
            "TS-08"
        ) &&
        !Floppy144GameDataCollectionEnabled(
            &sRunState,
            "TS-27"
        ),
        "Technology context stays locked until IT Support is reconstructed"
    );

    /*
     * Reopening the neutral summary after one workstream has fired must still
     * describe both authored workstreams. This reproduces the player case where
     * other DR-04 documents have already been viewed before RS-0037 is reopened.
     */
    sTerminal.output_count = 0U;

    Floppy144TerminalPrintPostOpenAction(
        &sTerminal,
        &sRunState,
        eDr04,
        pChoiceBriefing != NULL
            ? pChoiceBriefing->record_index
            : 0U
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            szBranchChoices
        ),
        "DR-04 briefing keeps both workstream choices after one branch has fired"
    );

    (void)snprintf(
        szBranchCommand,
        sizeof(szBranchCommand),
        "OPEN %s",
        pTechnologyBranchDocument != NULL &&
        pTechnologyBranchDocument->record_id_override != NULL
            ? Floppy144DocumentRecordIdForSeed(pTechnologyBranchDocument,sRunState.recovery_seed)
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
            ? Floppy144DocumentRecordIdForSeed(pTechnologyBranchDocument,sRunState.recovery_seed)
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

    if(sTerminal.open_record_requested)
    {
        F144_CHECK(
            Floppy144DocumentApplyEffects(
                &sWorld,
                &sRunState,
                sTerminal.requested_collection,
                sTerminal.requested_record_index
            ),
            "alternate branch document applies effects after release"
        );
    }

    F144_CHECK(
        Floppy144GameDataCollectionEnabled(
            &sRunState,
            "TS-08"
        ) &&
        Floppy144GameDataCollectionEnabled(
            &sRunState,
            "TS-27"
        ),
        "IT Support reconstruction unlocks optional TS-08 and TS-27 context"
    );

    sTerminal.output_count = 0U;

    Floppy144TerminalPrintPostOpenAction(
        &sTerminal,
        &sRunState,
        eDr04,
        pChoiceBriefing != NULL
            ? pChoiceBriefing->record_index
            : 0U
    );

    F144_CHECK(
        Floppy144TestTerminalContains(
            &sTerminal,
            szBranchChoices
        ),
        "DR-04 briefing keeps both authored workstreams after both have been viewed"
    );
}


/*
 * Compatibility regression for saves created before evidence-gated Act II.
 *
 * The most pathological reachable old state has DR-31 already restored after
 * I-007 completed, while I-006/E-005 are still absent. The collection is real
 * in the save, but its unique entry trigger T-017 is permanently blocked by the
 * stale upstream evidence prerequisite.
 *
 * Reopening the entry record must heal that restored collection without
 * fabricating E-005. The second trigger must then follow its ordinary internal
 * dependency and reveal the Signed Custody Sheet.
 */
static void Floppy144TestStaleEvidenceSaveRecovery(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;

    Floppy144InteractionId eI007;
    Floppy144EvidenceId eE005;
    Floppy144TriggerId eT010;
    Floppy144TriggerId eT017;
    Floppy144TriggerId eT018;
    Floppy144CollectionId eDr04;
    Floppy144CollectionId eDr31;
    Floppy144CollectionId eCollection;
    uint32_t uRecordIndex;

    Floppy144WorldReset(
        &sWorld
    );

    Floppy144RunStateBegin(
        &sRunState,
        144U
    );

    eI007 = Floppy144GameDataInteractionId("I-007");
    eE005 = Floppy144GameDataEvidenceId("E-005");
    eT010 = Floppy144GameDataTriggerId("T-010");
    eT017 = Floppy144GameDataTriggerId("T-017");
    eT018 = Floppy144GameDataTriggerId("T-018");
    eDr04 = Floppy144GameDataCollectionId("DR-04");
    eDr31 = Floppy144GameDataCollectionId("DR-31");

    F144_CHECK(
        eI007 < FLOPPY144_INTERACTION_COUNT &&
        eE005 < FLOPPY144_EVIDENCE_COUNT &&
        eT010 < FLOPPY144_TRIGGER_COUNT &&
        eT017 < FLOPPY144_TRIGGER_COUNT &&
        eT018 < FLOPPY144_TRIGGER_COUNT &&
        eDr04 < FLOPPY144_COLLECTION_COUNT &&
        eDr31 < FLOPPY144_COLLECTION_COUNT,
        "stale-evidence compatibility fixture IDs resolve"
    );

    /*
     * Reproduce the old broken state: I-007 had enough latitude to complete and
     * expose DR-31, despite the full E-005 synthesis never being established.
     */
    F144_CHECK(
        Floppy144RunStateCompleteInteraction(
            &sRunState,
            eI007
        ) &&
        Floppy144RunStateBitSet(
            sRunState.collections,
            (uint32_t)eDr31
        ) &&
        Floppy144WorldRestoreCollection(
            &sWorld,
            eDr31
        ),
        "legacy DR-31 state is constructed"
    );

    F144_CHECK(
        !Floppy144RunStateEvidenceEstablished(
            &sRunState,
            eE005
        ) &&
        !Floppy144RunStateTriggerFired(
            &sRunState,
            eT017
        ),
        "fixture begins with missing E-005 and unfired T-017"
    );

    F144_CHECK(
        Floppy144DocumentFindRecordId(
            Floppy144TestShortAuthoredId(NULL) != NULL &&
                Floppy144TestDocumentForTrigger(
                    eDr31,eT017
                ) != NULL
                ? Floppy144TestDocumentForTrigger(
                    eDr31,eT017
                )->record_id_override : "MISSING",
            &eCollection,
            &uRecordIndex
        ) &&
        Floppy144DocumentApplyEffects(
            &sWorld,
            &sRunState,
            eCollection,
            uRecordIndex
        ),
        "reopening restored DR-31 entry record applies legacy recovery"
    );

    F144_CHECK(
        !Floppy144RunStateEvidenceEstablished(
            &sRunState,
            eE005
        ) &&
        Floppy144RunStateTriggerFired(
            &sRunState,
            eT017
        ) &&
        Floppy144GameDataPhysicalItemRevealed(
            &sRunState,
            "P-053"
        ),
        "legacy recovery fires DR-31 root without inventing E-005"
    );

    F144_CHECK(
        Floppy144DocumentFindRecordId(
            Floppy144TestDocumentForTrigger(
                eDr31,eT018
            ) != NULL
                ? Floppy144TestDocumentForTrigger(
                    eDr31,eT018
                )->record_id_override : "MISSING",
            &eCollection,
            &uRecordIndex
        ) &&
        Floppy144DocumentApplyEffects(
            &sWorld,
            &sRunState,
            eCollection,
            uRecordIndex
        ),
        "reopening Senior Archivist assignment applies ordinary T-018 dependency"
    );

    F144_CHECK(
        Floppy144RunStateTriggerFired(
            &sRunState,
            eT018
        ) &&
        Floppy144GameDataPhysicalItemRevealed(
            &sRunState,
            "P-052"
        ),
        "legacy recovered save reveals Signed Custody Sheet"
    );

    /*
     * The fallback must never choose a branch. DR-04 owns two root triggers,
     * so restoring that collection alone is not authority to fire T-010.
     */
    Floppy144WorldReset(
        &sWorld
    );

    Floppy144RunStateBegin(
        &sRunState,
        144U
    );

    F144_CHECK(
        Floppy144RunStateBitSet(
            sRunState.collections,
            (uint32_t)eDr04
        ) &&
        Floppy144WorldRestoreCollection(
            &sWorld,
            eDr04
        ),
        "branch-safety fixture restores DR-04"
    );

    F144_CHECK(
        !Floppy144GameDataTriggerTryFire(
            &sWorld,
            &sRunState,
            eT010
        ) &&
        sRunState.branch ==
            (uint8_t)FLOPPY144_RUN_BRANCH_NONE,
        "legacy fallback never chooses one of multiple collection roots"
    );
}

/*
 * FM-18 has an absolute dependency on FM-13. The Server Room requirement on
 * FM-18-RS-0098 is an additional document gate, not an alternate route into
 * the collection. Reconstructing the Server Room alone must therefore never
 * make FM-18 recoverable.
 */
static void Floppy144TestFm18SuppressionRecordRestoresServerPanel(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sRunState;
    Floppy144RunState sBypassState;
    Floppy144CollectionId eFm18;
    Floppy144CollectionId eFm13;
    Floppy144CollectionId eCollection = FLOPPY144_COLLECTION_COUNT;
    Floppy144TriggerId eT041;
    Floppy144TriggerId eT042;
    uint32_t uEnvironmentalRecordIndex = 0U;
    uint32_t uSuppressionRecordIndex = 0U;

    Floppy144WorldReset(
        &sWorld
    );

    Floppy144RunStateBegin(
        &sRunState,
        144U
    );

    Floppy144RunStateBegin(
        &sBypassState,
        145U
    );

    eFm18 = Floppy144GameDataCollectionId("FM-18");
    eFm13 = Floppy144GameDataCollectionId("FM-13");
    eT041 = Floppy144GameDataTriggerId("T-041");
    eT042 = Floppy144GameDataTriggerId("T-042");

    F144_CHECK(
        eFm18 < FLOPPY144_COLLECTION_COUNT &&
        eFm13 < FLOPPY144_COLLECTION_COUNT &&
        eT041 < FLOPPY144_TRIGGER_COUNT &&
        eT042 < FLOPPY144_TRIGGER_COUNT,
        "FM-18 progression regression IDs resolve"
    );

    F144_CHECK(
        Floppy144RunStateSetAct(
            &sRunState,
            FLOPPY144_RUN_ACT_III
        ),
        "FM-18 progression fixture enters Act III"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionAvailable(
            &sRunState,
            eFm18
        ),
        "FM-18 remains unavailable in Act III before FM-13 restoration"
    );

    F144_CHECK(
        Floppy144RunStateSetAct(
            &sBypassState,
            FLOPPY144_RUN_ACT_III
        ) &&
        Floppy144RunStateReconstructRoom(
            &sBypassState,
            FLOPPY144_ROOM_SERVER_ROOM
        ),
        "FM-18 bypass fixture reconstructs Server Room in Act III"
    );

    F144_CHECK(
        !Floppy144RunStateCollectionAvailable(
            &sBypassState,
            eFm18
        ),
        "Server Room reconstruction cannot bypass FM-18 dependency on FM-13"
    );

    F144_CHECK(
        Floppy144RunStateBitSet(
            sRunState.collections,
            (uint32_t)eFm13
        ),
        "FM-18 dependency fixture restores FM-13"
    );

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sRunState,
            eFm18
        ),
        "FM-18 becomes available in Act III after FM-13 restoration"
    );

    F144_CHECK(
        Floppy144RunStateBitSet(
            sRunState.collections,
            (uint32_t)eFm18
        ) &&
        Floppy144WorldRestoreCollection(
            &sWorld,
            eFm18
        ),
        "FM-18 progression fixture restores collection"
    );

    F144_CHECK(
        Floppy144DocumentFindRecordId(
            Floppy144TestDocumentForTrigger(
                eFm18,eT041
            ) != NULL
                ? Floppy144TestDocumentForTrigger(
                    eFm18,eT041
                )->record_id_override : "MISSING",
            &eCollection,
            &uEnvironmentalRecordIndex
        ) &&
        eCollection == eFm18 &&
        Floppy144DocumentFindRecordId(
            Floppy144TestDocumentForTrigger(
                eFm18,eT042
            ) != NULL
                ? Floppy144TestDocumentForTrigger(
                    eFm18,eT042
                )->record_id_override : "MISSING",
            &eCollection,
            &uSuppressionRecordIndex
        ) &&
        eCollection == eFm18,
        "FM-18 player-facing progression records resolve"
    );

    F144_CHECK(
        Floppy144DocumentApplyEffects(
            &sWorld,
            &sRunState,
            eFm18,
            uEnvironmentalRecordIndex
        ) &&
        Floppy144RunStateTriggerFired(
            &sRunState,
            eT041
        ) &&
        Floppy144GameDataPhysicalItemRevealed(
            &sRunState,
            "P-037"
        ),
        "FM-18-RS-0135 restores the Facilities environmental event log"
    );

    F144_CHECK(
        Floppy144DocumentApplyEffects(
            &sWorld,
            &sRunState,
            eFm18,
            uSuppressionRecordIndex
        ) &&
        !Floppy144RunStateTriggerFired(
            &sRunState,
            eT042
        ) &&
        !Floppy144GameDataPhysicalItemRevealed(
            &sRunState,
            "P-103"
        ),
        "FM-18-RS-0098 cannot restore the Server Room panel before Server Room reconstruction"
    );

    F144_CHECK(
        Floppy144RunStateReconstructRoom(
            &sRunState,
            FLOPPY144_ROOM_SERVER_ROOM
        ),
        "FM-18 progression fixture reconstructs Server Room"
    );

    F144_CHECK(
        Floppy144DocumentApplyEffects(
            &sWorld,
            &sRunState,
            eFm18,
            uSuppressionRecordIndex
        ) &&
        Floppy144RunStateTriggerFired(
            &sRunState,
            eT042
        ) &&
        Floppy144GameDataPhysicalItemRevealed(
            &sRunState,
            "P-103"
        ),
        "FM-18-RS-0098 restores P-103 once Server Room is reconstructed"
    );
}

static void Floppy144TestAvailableRecoveryAndAutomaticCompletion(void)
{
    Floppy144RunState sRunState;
    Floppy144CollectionId eFm18;
    Floppy144EvidenceId eE020,eE021,eE022,eE023;
    Floppy144InteractionId eI034;
    uint32_t uCollection;

    /*
     * Isolate the exhaustion rule from normal progression. Mark every
     * collection restored except FM-18, which remains dependency-locked in
     * the default Prologue state because it requires Act III. If the recovery
     * scan accidentally ignores availability, FM-18 will make this report
     * that more usable disk data remains.
     */
    Floppy144RunStateBegin(&sRunState,144U);
    eFm18=Floppy144GameDataCollectionId("FM-18");

    for(uCollection=0U;uCollection<(uint32_t)FLOPPY144_COLLECTION_COUNT;++uCollection)
    {
        if(uCollection!=(uint32_t)eFm18)
        {
            (void)Floppy144RunStateBitSet(
                sRunState.collections,
                uCollection
            );
        }
    }

    F144_CHECK(
        eFm18<FLOPPY144_COLLECTION_COUNT &&
        !Floppy144RunStateCollectionAvailable(&sRunState,eFm18) &&
        !Floppy144RunStateAnyUnrestoredCollectionFits(&sRunState) &&
        !Floppy144RunStateAvailableRecoveryCapacityExhausted(&sRunState),
        "locked dependency collections do not count as exhausted available recovery capacity"
    );

    /*
     * Conversely, an actually available collection which cannot fit must
     * trip the terminal-exit completion condition.
     */
    Floppy144RunStateBegin(&sRunState,144U);
    for(uCollection=1U;uCollection<(uint32_t)FLOPPY144_COLLECTION_COUNT;++uCollection)
    {
        (void)Floppy144RunStateBitSet(
            sRunState.collections,
            uCollection
        );
    }

    F144_CHECK(
        Floppy144RunStateCollectionAvailable(
            &sRunState,
            FLOPPY144_COLLECTION_DR01
        ) &&
        !Floppy144RunStateCanRestoreCollection(
            &sRunState,
            FLOPPY144_COLLECTION_DR01
        ) &&
        Floppy144RunStateAvailableRecoveryCapacityExhausted(&sRunState),
        "available collection larger than remaining capacity triggers exhaustion"
    );

    Floppy144RunStateBegin(&sRunState,144U);
    eE020=Floppy144GameDataEvidenceId("E-020");
    eE021=Floppy144GameDataEvidenceId("E-021");
    eE022=Floppy144GameDataEvidenceId("E-022");
    eE023=Floppy144GameDataEvidenceId("E-023");
    eI034=Floppy144GameDataInteractionId("I-034");
    (void)Floppy144RunStateEstablishEvidence(&sRunState,eE020);
    (void)Floppy144RunStateEstablishEvidence(&sRunState,eE021);
    (void)Floppy144RunStateEstablishEvidence(&sRunState,eE022);
    Floppy144GameDataResolveEvidence(&sRunState);
    F144_CHECK(
        Floppy144RunStateInteractionCompleted(&sRunState,eI034) &&
        Floppy144RunStateEvidenceEstablished(&sRunState,eE023) &&
        Floppy144GameDataEvidenceResolved(&sRunState) &&
        Floppy144RunStateAct(&sRunState)==FLOPPY144_RUN_ACT_COMPLETE,
        "E-020/E-021/E-022 automatically synthesise resolved E-023 and complete the recovery"
    );
}


/*
 * S4E-07 acceptance: fixed post-expansion player-facing slots with
 * independently seeded workstream payloads. Both initial branch choices
 * are played through OPEN, trigger, V2 save/reload and re-entry for BOTH
 * permutation classes. Existing reconstruction tests cover later acts.
 */
static void Floppy144TestDr04SeededWorkstreamSlots(void)
{
    static const uint32_t seeds[] = { 144U, 146U };
    uint32_t i, choice;
    Floppy144CollectionId dr = Floppy144GameDataCollectionId("DR-04");
    Floppy144EvidenceId e003 = Floppy144GameDataEvidenceId("E-003");
    const Floppy144DocumentDefinition *records =
        Floppy144TestDocumentForTrigger(dr, Floppy144GameDataTriggerId("T-010"));
    const Floppy144DocumentDefinition *technology =
        Floppy144TestDocumentForTrigger(dr, Floppy144GameDataTriggerId("T-011"));

    F144_CHECK(dr < FLOPPY144_COLLECTION_COUNT &&
        e003 < FLOPPY144_EVIDENCE_COUNT &&
        records != NULL && technology != NULL,
        "DR-04 pair resolves by stable authored triggers");
    if(records == NULL || technology == NULL)
        return;

    F144_CHECK(
        strcmp(records->record_id_override, "DR-04-RS-0216") == 0 &&
        strcmp(technology->record_id_override, "DR-04-RS-0147") == 0,
        "S4E-08 current slot numbers are retained without renumbering"
    );

    for(i = 0U; i < 2U; ++i)
    for(choice = 0U; choice < 2U; ++choice)
    {
        uint32_t seed = seeds[i];
        bool swapped = i == 0U;
        Floppy144WorldState world;
        Floppy144RunState run, loaded;
        Floppy144TerminalState term;
        Floppy144CatalogueState catalogue;
        uint8_t payload[FLOPPY144_SAVE_PAYLOAD_V2_SIZE];
        char record_id[24], title[48], command[48], row[96];
        const Floppy144DocumentDefinition *chosen =
            choice == 0U ? records : technology;
        const Floppy144DocumentDefinition *other =
            choice == 0U ? technology : records;
        uint32_t selected_slot = Floppy144DocumentSlotForSeed(chosen,seed);
        uint32_t other_slot = Floppy144DocumentSlotForSeed(other,seed);
        Floppy144CollectionId resolved;
        uint32_t index;
        const char *chosen_id =
            Floppy144DocumentRecordIdForSeed(chosen,seed);

        F144_CHECK(Floppy144DocumentDr04Swapped(seed) == swapped,
            "seed class selects the expected DR-04 arrangement");
        F144_CHECK(selected_slot == (swapped ? other->record_index :
            chosen->record_index), "workstream moves to expected stable slot");
        F144_CHECK(
            Floppy144DocumentGetForSeed(dr,selected_slot,seed) == chosen &&
            Floppy144DocumentGetForSeed(dr,other_slot,seed) == other,
            "both slots resolve to their expected authored payloads"
        );
        Floppy144CatalogueBuildRecordForSeed(
            dr,selected_slot,seed,record_id,sizeof(record_id),title,sizeof(title));
        F144_CHECK(strcmp(record_id,chosen_id) == 0 &&
            strcmp(title,chosen->title_override) == 0,
            "LIST title and fixed record number agree with seed mapping");
        F144_CHECK(Floppy144CatalogueFindRecord(
            chosen_id,&resolved,&index) && resolved == dr &&
            index == selected_slot, "OPEN ID lookup remains fixed to slot");

        Floppy144WorldReset(&world);
        Floppy144RunStateBegin(&run,seed);
        F144_CHECK(Floppy144WorldInitialiseArchiveServices(&world) &&
            Floppy144RunStateInitialiseArchiveServices(&run),
            "seeded DR-04 fixture initialises archive services");
        F144_CHECK(Floppy144RunStateBitSet(run.collections,(uint32_t)dr) &&
            Floppy144WorldRestoreCollection(&world,dr) &&
            Floppy144RunStateEstablishEvidence(&run,e003),
            "seeded DR-04 fixture restores collection and branch evidence");
        Floppy144TerminalReset(&term,&world);
        term.debug_guidance=true;
        term.default_record_collection=dr;
        term.default_record_collection_valid=true;

        Floppy144TestSubmitCommand(&term,&world,&run,"LIST DR-04 2");
        Floppy144CatalogueBuildRecordForSeed(
            dr,technology->record_index,seed,record_id,sizeof(record_id),
            title,sizeof(title));
        (void)snprintf(row,sizeof(row),"%s  %s",record_id,title);
        F144_CHECK(Floppy144TestTerminalContains(&term,row),
            "paged LIST shows correct seed-specific title in Technology base slot");
        Floppy144TerminalCloseRecordPager(&term);

        Floppy144TestSubmitCommand(&term,&world,&run,"LIST DR-04 3");
        Floppy144CatalogueBuildRecordForSeed(
            dr,records->record_index,seed,record_id,sizeof(record_id),
            title,sizeof(title));
        (void)snprintf(row,sizeof(row),"%s  %s",record_id,title);
        F144_CHECK(Floppy144TestTerminalContains(&term,row),
            "paged LIST shows correct seed-specific title in Records base slot");
        Floppy144TerminalCloseRecordPager(&term);

        (void)snprintf(command,sizeof(command),"OPEN %s",chosen_id);
        Floppy144TestSubmitCommand(&term,&world,&run,command);
        F144_CHECK(term.open_record_requested &&
            term.requested_collection == dr &&
            term.requested_record_index == selected_slot,
            "OPEN routes the selected record ID to the same seed-specific slot");
        Floppy144CatalogueReset(&catalogue,dr);
        catalogue.recovery_seed=seed;
        catalogue.selected_index=selected_slot;
        Floppy144CatalogueOpenDocument(&catalogue);
        F144_CHECK(Floppy144CatalogueDocumentOpen(&catalogue) &&
            Floppy144DocumentGetForSeed(dr,catalogue.selected_index,
            catalogue.recovery_seed)->trigger == chosen->trigger,
            "document viewer uses the correct seeded workstream body");
        F144_CHECK(Floppy144DocumentApplyEffects(
            &world,&run,dr,selected_slot),
            "opening selected workstream invokes its authored trigger");
        F144_CHECK(Floppy144RunStateTriggerFired(&run,chosen->trigger) &&
            run.branch == (uint8_t)(choice == 0U ?
                FLOPPY144_RUN_BRANCH_RECORDS_FIRST :
                FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST),
            "authored trigger identity, not record slot, commits branch");
        F144_CHECK(!Floppy144DocumentAccessible(&run,dr,other_slot),
            "unselected alternate remains gated after first choice");
        F144_CHECK(Floppy144PersistenceEncodeRunState(
            &run,payload,(uint32_t)sizeof(payload)) &&
            Floppy144PersistenceDecodeRunState(
                &loaded,payload,(uint32_t)sizeof(payload)),
            "actual V2 save/reload retains seeded workstream route");
        F144_CHECK(loaded.recovery_seed == seed &&
            loaded.branch == run.branch &&
            Floppy144DocumentGetForSeed(dr,selected_slot,
                loaded.recovery_seed)->trigger == chosen->trigger &&
            !Floppy144DocumentAccessible(&loaded,dr,other_slot),
            "reload keeps arrangement, fired trigger and alternate branch gate");
        Floppy144TerminalReset(&term,&world);
        term.default_record_collection=dr;
        term.default_record_collection_valid=true;
        Floppy144TestSubmitCommand(&term,&world,&loaded,command);
        F144_CHECK(term.open_record_requested &&
            term.requested_record_index == selected_slot,
            "terminal re-entry preserves the exact selected workstream slot");
    }
}

/* S4G-01: real terminal LIST/OPEN route, not just the registry shortcut. */
static void Floppy144TestGreyDoorOrphanTerminalRoute(void)
{
    Floppy144WorldState world;
    Floppy144RunState run;
    Floppy144TerminalState terminal;
    Floppy144CatalogueState catalogue;
    uint32_t original_kb;

    Floppy144TestReachOpeningCollections(&world,&run,&terminal);
    original_kb=Floppy144RunStateRecoveredKb(&run);
    F144_CHECK(run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE,
        "orphan record is unavailable before it is viewed");
    Floppy144TestSubmitCommand(&terminal,&world,&run,"LIST DR-01 2");
    F144_CHECK(Floppy144TerminalRecordPagerActive(&terminal) &&
        Floppy144TestTerminalContains(&terminal,FLOPPY144_GREY_DOOR_RECORD_ID),
        "last DR-01 terminal index page contains the uncounted orphan record");
    Floppy144TerminalCloseRecordPager(&terminal);
    Floppy144TestSubmitCommand(&terminal,&world,&run,
        "OPEN DR-00-RS-0144");
    F144_CHECK(terminal.open_record_requested &&
        terminal.requested_collection==FLOPPY144_GREY_DOOR_RECORD_COLLECTION &&
        terminal.requested_record_index==FLOPPY144_GREY_DOOR_RECORD_INDEX,
        "OPEN routes orphan through the normal command/document protocol");
    F144_CHECK(Floppy144CatalogueOpenRecord(&catalogue,
        terminal.requested_collection,terminal.requested_record_index) &&
        Floppy144CatalogueDocumentOpen(&catalogue),
        "orphan opens in the standard scrollable document viewer");
    F144_CHECK(Floppy144DocumentApplyEffects(&world,&run,
        terminal.requested_collection,terminal.requested_record_index) &&
        run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE &&
        Floppy144RunStateRecoveredKb(&run)==original_kb,
        "the normal document-open event enables the door without capacity cost");
}

int main(void)
{
    Floppy144TestFm18SuppressionRecordRestoresServerPanel();
    Floppy144TestAvailableRecoveryAndAutomaticCompletion();
    Floppy144TestStaleEvidenceSaveRecovery();
    Floppy144TestRestoreProgress();
    Floppy144TestExtendedGlyphs();
    Floppy144TestCatalogueRecordResolution();
    Floppy144TestDocumentViewerContentStates();
    Floppy144TestDocumentBodyScrolling();
    Floppy144TestCollectionListPresentation();
    Floppy144TestMultipleCollectionCommands();
    Floppy144TestPlayerRecoveryPolicy();
    Floppy144TestServerRoomItTerminal();
    Floppy144TestCommandHistory();
    Floppy144TestBranchDocumentAccessGate();
    Floppy144TestDr04SeededWorkstreamSlots();
    Floppy144TestGreyDoorOrphanTerminalRoute();

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
