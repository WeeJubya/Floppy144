/*
 * Floppy//144 - record catalogue implementation
 *
 * Generates deterministic record listings for each collection, renders the
 * scrollable catalogue, displays recovered documents and identifies evidence.
 */

#include "floppy144_catalogue.h"

#include "floppy144_draw.h"
#include "floppy144_collection_registry.h"
#include "floppy144_document.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*
 * Catalogue size
 *
 * Act I collections expose 50 deterministic index entries. Authored record
 * positions are supplied by the central document registry.
 */

#define FLOPPY144_CATALOGUE_ROWS         10U

/*
 * Shared procedural document forms
 *
 * Collection-specific subjects and numbering rules belong to the collection
 * registry. These forms are shared by every generated catalogue.
 */
static const char *floppy144_record_forms[10] =
{
    "FILE",
    "REVIEW",
    "NOTICE",
    "AUTHORISATION",
    "AMENDMENT",
    "SUMMARY",
    "REGISTER",
    "SCHEDULE",
    "CONFIRMATION",
    "CHECKLIST"
};

#define FLOPPY144_RECORD_FORM_COUNT                                \
    ((uint32_t)(sizeof(floppy144_record_forms) /                   \
                sizeof(floppy144_record_forms[0])))


#define FLOPPY144_RECORD_NUMBER_DENSITY 11U

/*
 * Build the nth numerically ordered member of the collection's deterministic
 * permutation. This restores the less mechanical record-number texture from
 * the Technical Slice without returning to an out-of-order catalogue.
 */
static uint32_t Floppy144CatalogueOrderedRecordNumber(
    const Floppy144CatalogueDefinition *definition,
    uint32_t index
)
{
    uint32_t modulus;
    uint32_t source;

    if(
        definition == NULL ||
        definition->record_count == 0U ||
        index >= definition->record_count
    )
    {
        return 0U;
    }

    modulus =
        definition->record_count *
        FLOPPY144_RECORD_NUMBER_DENSITY;

    for(source = 0U; source < definition->record_count; ++source)
    {
        uint32_t candidate =
            definition->record_number_base +
            1U +
            (
                (
                    source *
                    definition->record_number_multiplier +
                    definition->record_number_offset
                ) %
                modulus
            );

        uint32_t rank = 0U;
        uint32_t other;

        for(other = 0U; other < definition->record_count; ++other)
        {
            uint32_t other_candidate =
                definition->record_number_base +
                1U +
                (
                    (
                        other *
                        definition->record_number_multiplier +
                        definition->record_number_offset
                    ) %
                    modulus
                );

            if(other_candidate < candidate)
            {
                ++rank;
            }
        }

        if(rank == index)
        {
            return candidate;
        }
    }

    return
        definition->record_number_base +
        index +
        1U;
}

/*
 * Build a deterministic record ID and title
 *
 * The same collection and index always produce the same output. Different
 * multipliers give HR-01 and FM-13 distinct record-number sequences.
 * FM-13 record 047 is overridden with its stable authored identity.
 */

void Floppy144CatalogueBuildRecord(
    Floppy144CollectionId collection,
    uint32_t index,
    char *record_id,
    size_t record_id_size,
    char *title,
    size_t title_size
)
{
    const Floppy144CatalogueDefinition *definition =
        &Floppy144CollectionGet(collection)->catalogue;

    const Floppy144DocumentDefinition *authored_document;

    uint32_t subject_index;
    uint32_t group_index;
    uint32_t form_index;
    uint32_t record_number;

    if(
        definition->record_id_prefix == NULL ||
        definition->subjects == NULL ||
        definition->subject_count == 0U
    )
    {
        snprintf(
            record_id,
            record_id_size,
            "UNAVAILABLE"
        );

        snprintf(
            title,
            title_size,
            "CATALOGUE CONFIGURATION UNAVAILABLE"
        );

        return;
    }

    subject_index =
        index %
        definition->subject_count;

    group_index =
        index /
        definition->subject_count;

    form_index =
        (
            group_index +
            subject_index * 3U
        ) %
        FLOPPY144_RECORD_FORM_COUNT;

    /*
     * Catalogue positions remain numerically ordered, but the numbers
     * themselves come from the collection's deterministic permutation rather
     * than a visible 0010/0020/0030 ladder.
     */
    record_number =
        Floppy144CatalogueOrderedRecordNumber(
            definition,
            index
        );

    snprintf(
        record_id,
        record_id_size,
        "%s-%04u",
        definition->record_id_prefix,
        (unsigned)record_number
    );

    if(definition->exact_titles)
    {
        snprintf(
            title,
            title_size,
            "%s",
            definition->subjects[subject_index]
        );
    }
    else
    {
        snprintf(
            title,
            title_size,
            "%s %s",
            definition->subjects[subject_index],
            floppy144_record_forms[form_index]
        );
    }

    authored_document =
        Floppy144DocumentGet(
            collection,
            index
        );

    if(authored_document == NULL)
    {
        return;
    }

    if(authored_document->record_id_override != NULL)
    {
        snprintf(
            record_id,
            record_id_size,
            "%s",
            authored_document->record_id_override
        );
    }

    if(authored_document->title_override != NULL)
    {
        snprintf(
            title,
            title_size,
            "%s",
            authored_document->title_override
        );
    }
}

/*
 * Resolve one complete record ID through the catalogue registry.
 *
 * Centralising this lookup keeps terminal commands, graphical browsing and
 * authored record overrides on one identity path. No story-specific record
 * IDs are embedded here.
 */
bool Floppy144CatalogueFindRecord(
    const char *record_id,
    Floppy144CollectionId *collection,
    uint32_t *record_index
)
{
    uint32_t collection_index;
    char generated_record_id[24];
    char generated_title[48];

    if(
        record_id == NULL ||
        collection == NULL ||
        record_index == NULL
    )
    {
        return false;
    }

    for(
        collection_index = 0U;
        collection_index < (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        Floppy144CollectionId candidate_collection =
            (Floppy144CollectionId)collection_index;

        const Floppy144CatalogueDefinition *definition =
            &Floppy144CollectionGet(candidate_collection)->catalogue;

        uint32_t candidate_index;

        for(
            candidate_index = 0U;
            candidate_index < definition->record_count;
            ++candidate_index
        )
        {
            Floppy144CatalogueBuildRecord(
                candidate_collection,
                candidate_index,
                generated_record_id,
                sizeof(generated_record_id),
                generated_title,
                sizeof(generated_title)
            );

            if(strcmp(record_id, generated_record_id) == 0)
            {
                *collection = candidate_collection;
                *record_index = candidate_index;
                return true;
            }
        }
    }

    return false;
}

/*
 * Catalogue drawing helpers
 *
 * TextCentred positions headings. DrawRow builds and paints one record entry.
 */

static void Floppy144CatalogueTextCentred(
    Floppy144Surface *surface,
    uint32_t y,
    const char *text,
    uint32_t scale,
    uint32_t colour
)
{
    uint32_t text_width =
        Floppy144DrawTextWidth(text, scale);

    uint32_t x =
        (surface->width - text_width) / 2U;

    Floppy144DrawText(
        surface,
        x,
        y,
        text,
        scale,
        colour
    );
}

/*
 * Draw one record row
 *
 * The active collection is passed to the generator so HR and FA rows can
 * share all layout code.
 */

static void Floppy144CatalogueDrawRow(
    Floppy144Surface *surface,
    Floppy144CollectionId collection,
    uint32_t index,
    uint32_t y,
    bool selected,
    uint32_t background,
    uint32_t selected_background,
    uint32_t border,
    uint32_t text,
    uint32_t muted,
    uint32_t amber
)
{
    char record_id[24];
    char title[48];

    Floppy144CatalogueBuildRecord(
        collection,
        index,
        record_id,
        sizeof(record_id),
        title,
        sizeof(title)
    );

    Floppy144DrawFillRect(
        surface,
        40,
        y,
        548,
        17,
        selected
            ? selected_background
            : background
    );

    Floppy144DrawRect(
        surface,
        40,
        y,
        548,
        17,
        selected
            ? amber
            : border
    );

    Floppy144DrawText(
        surface,
        48,
        y + 5,
        selected ? ">" : " ",
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        66,
        y + 5,
        record_id,
        1,
        selected ? text : muted
    );

    Floppy144DrawText(
        surface,
        172,
        y + 5,
        title,
        1,
        text
    );
}

/*
 * Draw the scrollable catalogue list
 *
 * Shows ten entries at a time, a position label, a proportional scrollbar
 * and context instructions in the footer.
 */

static void Floppy144CatalogueDrawList(
    Floppy144Surface *surface,
    const Floppy144CatalogueState *catalogue
)
{
    const uint32_t background =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t panel =
        FLOPPY144_RGB(24, 33, 39);

    const uint32_t row =
        FLOPPY144_RGB(20, 28, 33);

    const uint32_t selected_row =
        FLOPPY144_RGB(39, 48, 50);

    const uint32_t border =
        FLOPPY144_RGB(86, 103, 107);

    const uint32_t text =
        FLOPPY144_RGB(202, 211, 205);

    const uint32_t muted =
        FLOPPY144_RGB(118, 133, 132);

    const uint32_t green =
        FLOPPY144_RGB(100, 156, 111);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    const Floppy144CollectionDefinition *collection_definition =
        Floppy144CollectionGet(
            catalogue->collection
        );

    const Floppy144CatalogueDefinition *definition =
        &collection_definition->catalogue;

    uint32_t record_count =
        definition->record_count;

    char position_text[32];
    char index_count_text[40];
    char collection_text[32];

    uint32_t visible_row;
    uint32_t thumb_y;
    uint32_t thumb_height;
    uint32_t thumb_travel;
    uint32_t max_top_index;

    /* Display uses one-based record numbers while state remains zero-based. */
    snprintf(
        position_text,
        sizeof(position_text),
        "RECORD %03u OF %03u",
        (unsigned)(catalogue->selected_index + 1U),
        (unsigned)record_count
    );

    snprintf(
        index_count_text,
        sizeof(index_count_text),
        "GENERATED INDEX ENTRIES: %u",
        (unsigned)record_count
    );

    snprintf(
        collection_text,
        sizeof(collection_text),
        "COLLECTION: %s",
        collection_definition->code
    );

    Floppy144DrawClear(
        surface,
        background
    );

    Floppy144DrawText(
        surface,
        10,
        5,
        "GDR ARCHIVE RECORD CATALOGUE",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        538,
        5,
        collection_definition->code,
        1,
        green
    );

    Floppy144DrawFillRect(
        surface,
        10,
        18,
        610,
        280,
        panel
    );

    Floppy144DrawRect(
        surface,
        10,
        18,
        610,
        280,
        border
    );

    Floppy144CatalogueTextCentred(
        surface,
        32,
        definition->heading,
        2,
        text
    );

    Floppy144DrawText(
        surface,
        40,
        60,
        collection_text,
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        408,
        60,
        position_text,
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        40,
        76,
        index_count_text,
        1,
        muted
    );

    Floppy144DrawFillRect(
        surface,
        40,
        90,
        560,
        1,
        border
    );

    /* Render only the ten rows in the current viewport. */
    for(
        visible_row = 0;
        visible_row < FLOPPY144_CATALOGUE_ROWS;
        ++visible_row
    )
    {
        uint32_t record_index =
            catalogue->top_index + visible_row;

        if(record_index >= record_count)
        {
            break;
        }

        Floppy144CatalogueDrawRow(
            surface,
            catalogue->collection,
            record_index,
            98U + visible_row * 19U,
            record_index == catalogue->selected_index,
            row,
            selected_row,
            border,
            text,
            muted,
            amber
        );
    }

    Floppy144DrawFillRect(
        surface,
        594,
        98,
        4,
        188,
        border
    );

    /*
     * Size and position the scrollbar from the registered record count.
     */

    thumb_height =
        record_count > 0U
            ? (
                188U *
                FLOPPY144_CATALOGUE_ROWS
              ) / record_count
            : 188U;

    if(thumb_height < 20U)
    {
        thumb_height = 20U;
    }

    if(thumb_height > 188U)
    {
        thumb_height = 188U;
    }

    max_top_index =
        record_count > FLOPPY144_CATALOGUE_ROWS
            ? record_count - FLOPPY144_CATALOGUE_ROWS
            : 0U;

    thumb_travel =
        188U - thumb_height;

    thumb_y =
        max_top_index > 0U
            ? 98U +
                catalogue->top_index *
                thumb_travel /
                max_top_index
            : 98U;

    Floppy144DrawFillRect(
        surface,
        592,
        thumb_y,
        8,
        thumb_height,
        amber
    );

    Floppy144DrawFillRect(
        surface,
        10,
        306,
        610,
        28,
        background
    );

    Floppy144DrawRect(
        surface,
        10,
        306,
        610,
        28,
        border
    );

    Floppy144DrawText(
        surface,
        22,
        316,
        "UP DOWN SELECT",
        1,
        text
    );

    Floppy144DrawText(
        surface,
        188,
        316,
        "PGUP PGDN PAGE",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        374,
        316,
        "ENTER VIEW CONTENTS",
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        502,
        316,
        "BACKSPACE RETURN",
        1,
        muted
    );
}

/*
 * Draw the recovered FM-13 authored document
 *
 * This record is laid out directly because its complete contents exist on
 * Disk 144. Reading it reveals the reconstructed suppression control panel.
 */

static void Floppy144CatalogueDrawFm13ServiceNote(
    Floppy144Surface *surface
)
{
    const uint32_t background =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t panel =
        FLOPPY144_RGB(24, 33, 39);

    const uint32_t document =
        FLOPPY144_RGB(31, 40, 44);

    const uint32_t border =
        FLOPPY144_RGB(86, 103, 107);

    const uint32_t text =
        FLOPPY144_RGB(202, 211, 205);

    const uint32_t muted =
        FLOPPY144_RGB(118, 133, 132);

    const uint32_t green =
        FLOPPY144_RGB(100, 156, 111);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    Floppy144DrawClear(
        surface,
        background
    );

    Floppy144DrawText(
        surface,
        10,
        5,
        "GDR ARCHIVE DOCUMENT VIEWER",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        538,
        5,
        "FM-13",
        1,
        green
    );

    Floppy144DrawFillRect(
        surface,
        10,
        18,
        610,
        280,
        panel
    );

    Floppy144DrawRect(
        surface,
        10,
        18,
        610,
        280,
        border
    );

    Floppy144DrawFillRect(
        surface,
        36,
        36,
        568,
        252,
        document
    );

    Floppy144DrawRect(
        surface,
        36,
        36,
        568,
        252,
        border
    );

    Floppy144DrawText(
        surface,
        52,
        50,
        "FM-13-RS-0047",
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        52,
        68,
        "SUPPRESSION CONTROL PANEL SERVICE NOTE",
        1,
        text
    );

    Floppy144DrawFillRect(
        surface,
        52,
        84,
        536,
        1,
        border
    );

    Floppy144DrawText(
        surface,
        52,
        98,
        "CLASS: OPERATIONAL MAINTENANCE RECORD",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        52,
        114,
        "LOCATION: SERVER ROOM CABLE RISER / SUPPRESSION CONTROL",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        52,
        130,
        "DATE: FRIDAY - FINAL WORKING DAY BEFORE BANK HOLIDAY",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        52,
        158,
        "MANUAL DISCHARGE CONTROL RETAINED PENDING REPLACEMENT",
        1,
        text
    );

    Floppy144DrawText(
        surface,
        52,
        174,
        "OF THE SERVER-ROOM CABLE RISER.",
        1,
        text
    );

    Floppy144DrawText(
        surface,
        52,
        198,
        "INTERMITTENT PANEL INPUT RECORDED DURING MAINTENANCE ACCESS.",
        1,
        text
    );

    Floppy144DrawText(
        surface,
        52,
        222,
        "NO DISCHARGE COMMAND CONFIRMED DURING THE TEST WINDOW.",
        1,
        text
    );

    Floppy144DrawText(
        surface,
        52,
        246,
        "FURTHER TESTING DEFERRED UNTIL TUESDAY AFTER BANK HOLIDAY.",
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        52,
        270,
        "ENGINEERING NOTE: ISOLATE CONTROL BEFORE CABLE WORK.",
        1,
        muted
    );

    Floppy144DrawFillRect(
        surface,
        10,
        306,
        610,
        28,
        background
    );

    Floppy144DrawRect(
        surface,
        10,
        306,
        610,
        28,
        border
    );

    Floppy144DrawText(
        surface,
        22,
        316,
        "RECOVERED AUTHORED RECORD",
        1,
        green
    );

    Floppy144DrawText(
        surface,
        526,
        316,
        "BACKSPACE BACK",
        1,
        muted
    );
}

/*
 * Draw JSON-authored document prose inside the recovered-record panel.
 *
 * The font is tiny, so a lightweight word wrapper is enough here. Explicit
 * newlines from the JSON are honoured and long paragraphs are wrapped to the
 * panel width. Bodies in the Stage 2 data set fit within the available page.
 */
#define FLOPPY144_DOCUMENT_BODY_MAX_LINES 13U
#define FLOPPY144_DOCUMENT_BODY_MAX_WIDTH 536U
#define FLOPPY144_DOCUMENT_BODY_LINE_HEIGHT 14U
#define FLOPPY144_DOCUMENT_BODY_LEFT 52U
#define FLOPPY144_DOCUMENT_BODY_TOP 102U

/*
 * Wrap the complete authored body using the exact same rules whether we are
 * drawing it or merely counting it for scroll bounds.
 *
 * uFirstLine selects the wrapped line shown at the top of the viewport. Passing
 * NULL for pSurface performs a count-only pass.
 */
static uint32_t Floppy144CatalogueProcessBodyText(
    Floppy144Surface *pSurface,
    const char *pszBody,
    uint32_t uColour,
    uint32_t uFirstLine
)
{
    const char *pszCursor = pszBody;
    char szLine[128];
    uint32_t uWrappedLine = 0U;
    uint32_t uLength = 0U;

    if(pszBody == NULL)
    {
        return 0U;
    }

    while(*pszCursor != '\0')
    {
        const char *pszWordStart;
        uint32_t uWordLength;
        char szCandidate[128];
        uint32_t uCandidateLength;

        if(*pszCursor == '\n')
        {
            if(
                pSurface != NULL &&
                uLength > 0U &&
                uWrappedLine >= uFirstLine &&
                uWrappedLine <
                    uFirstLine + FLOPPY144_DOCUMENT_BODY_MAX_LINES
            )
            {
                szLine[uLength] = '\0';

                Floppy144DrawText(
                    pSurface,
                    FLOPPY144_DOCUMENT_BODY_LEFT,
                    FLOPPY144_DOCUMENT_BODY_TOP +
                        (
                            uWrappedLine - uFirstLine
                        ) *
                        FLOPPY144_DOCUMENT_BODY_LINE_HEIGHT,
                    szLine,
                    1U,
                    uColour
                );
            }

            ++uWrappedLine;
            uLength = 0U;
            ++pszCursor;
            continue;
        }

        while(*pszCursor == ' ' || *pszCursor == '\t')
        {
            ++pszCursor;
        }

        if(*pszCursor == '\0')
        {
            break;
        }

        if(*pszCursor == '\n')
        {
            continue;
        }

        pszWordStart = pszCursor;
        uWordLength = 0U;

        while(
            pszCursor[uWordLength] != '\0' &&
            pszCursor[uWordLength] != ' ' &&
            pszCursor[uWordLength] != '\t' &&
            pszCursor[uWordLength] != '\n'
        )
        {
            ++uWordLength;
        }

        if(uWordLength >= sizeof(szLine))
        {
            uWordLength = (uint32_t)sizeof(szLine) - 1U;
        }

        uCandidateLength = uLength;

        if(
            uCandidateLength > 0U &&
            uCandidateLength + 1U < sizeof(szCandidate)
        )
        {
            szCandidate[uCandidateLength++] = ' ';
        }

        if(uCandidateLength + uWordLength >= sizeof(szCandidate))
        {
            uWordLength =
                (uint32_t)sizeof(szCandidate) -
                uCandidateLength -
                1U;
        }

        if(uLength > 0U)
        {
            uint32_t uCopy;

            for(uCopy = 0U; uCopy < uLength; ++uCopy)
            {
                szCandidate[uCopy] = szLine[uCopy];
            }
        }

        {
            uint32_t uCopy;

            for(uCopy = 0U; uCopy < uWordLength; ++uCopy)
            {
                szCandidate[uCandidateLength + uCopy] =
                    pszWordStart[uCopy];
            }
        }

        uCandidateLength += uWordLength;
        szCandidate[uCandidateLength] = '\0';

        if(
            uLength > 0U &&
            Floppy144DrawTextWidth(
                szCandidate,
                1U
            ) > FLOPPY144_DOCUMENT_BODY_MAX_WIDTH
        )
        {
            if(
                pSurface != NULL &&
                uWrappedLine >= uFirstLine &&
                uWrappedLine <
                    uFirstLine + FLOPPY144_DOCUMENT_BODY_MAX_LINES
            )
            {
                szLine[uLength] = '\0';

                Floppy144DrawText(
                    pSurface,
                    FLOPPY144_DOCUMENT_BODY_LEFT,
                    FLOPPY144_DOCUMENT_BODY_TOP +
                        (
                            uWrappedLine - uFirstLine
                        ) *
                        FLOPPY144_DOCUMENT_BODY_LINE_HEIGHT,
                    szLine,
                    1U,
                    uColour
                );
            }

            ++uWrappedLine;
            uLength = 0U;
            continue;
        }

        {
            uint32_t uCopy;

            for(uCopy = 0U; uCopy <= uCandidateLength; ++uCopy)
            {
                szLine[uCopy] = szCandidate[uCopy];
            }
        }

        uLength = uCandidateLength;
        pszCursor += uWordLength;
    }

    if(uLength > 0U)
    {
        if(
            pSurface != NULL &&
            uWrappedLine >= uFirstLine &&
            uWrappedLine <
                uFirstLine + FLOPPY144_DOCUMENT_BODY_MAX_LINES
        )
        {
            szLine[uLength] = '\0';

            Floppy144DrawText(
                pSurface,
                FLOPPY144_DOCUMENT_BODY_LEFT,
                FLOPPY144_DOCUMENT_BODY_TOP +
                    (
                        uWrappedLine - uFirstLine
                    ) *
                    FLOPPY144_DOCUMENT_BODY_LINE_HEIGHT,
                szLine,
                1U,
                uColour
            );
        }

        ++uWrappedLine;
    }

    return uWrappedLine;
}

static uint32_t Floppy144CatalogueDrawBodyText(
    Floppy144Surface *pSurface,
    const char *pszBody,
    uint32_t uColour,
    uint32_t uFirstLine
)
{
    if(pSurface == NULL)
    {
        return 0U;
    }

    return Floppy144CatalogueProcessBodyText(
        pSurface,
        pszBody,
        uColour,
        uFirstLine
    );
}

static void Floppy144CatalogueDrawDocument(
    Floppy144Surface *surface,
    const Floppy144CatalogueState *catalogue
)
{
    const Floppy144DocumentDefinition *authored_document =
        Floppy144DocumentGet(
            catalogue->collection,
            catalogue->selected_index
        );

    const Floppy144CollectionDefinition *collection_definition =
        Floppy144CollectionGet(
            catalogue->collection
        );

    if(
        authored_document != NULL &&
        authored_document->pszBody == NULL &&
        authored_document->view ==
            FLOPPY144_DOCUMENT_VIEW_FM13_SUPPRESSION_SERVICE
    )
    {
        Floppy144CatalogueDrawFm13ServiceNote(
            surface
        );

        return;
    }

    const uint32_t background =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t panel =
        FLOPPY144_RGB(24, 33, 39);

    const uint32_t document =
        FLOPPY144_RGB(31, 40, 44);

    const uint32_t border =
        FLOPPY144_RGB(86, 103, 107);

    const uint32_t text =
        FLOPPY144_RGB(202, 211, 205);

    const uint32_t muted =
        FLOPPY144_RGB(118, 133, 132);

    const uint32_t green =
        FLOPPY144_RGB(100, 156, 111);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    char record_id[24];
    char title[48];

    bool bAuthored =
        authored_document != NULL &&
        authored_document->pszBody != NULL;

    uint32_t uBodyLineCount = 0U;

    Floppy144CatalogueBuildRecord(
        catalogue->collection,
        catalogue->selected_index,
        record_id,
        sizeof(record_id),
        title,
        sizeof(title)
    );

    Floppy144DrawClear(
        surface,
        background
    );

    Floppy144DrawText(
        surface,
        10,
        5,
        "GDR ARCHIVE DOCUMENT VIEWER",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        538,
        5,
        collection_definition->code,
        1,
        green
    );

    Floppy144DrawFillRect(
        surface,
        10,
        18,
        610,
        280,
        panel
    );

    Floppy144DrawRect(
        surface,
        10,
        18,
        610,
        280,
        border
    );

    Floppy144DrawFillRect(
        surface,
        36,
        36,
        568,
        252,
        document
    );

    Floppy144DrawRect(
        surface,
        36,
        36,
        568,
        252,
        border
    );

    Floppy144DrawText(
        surface,
        52,
        50,
        record_id,
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        52,
        68,
        title,
        1,
        text
    );

    Floppy144DrawFillRect(
        surface,
        52,
        86,
        536,
        1,
        border
    );

    /* Draw complete authored JSON text when present. */
    if(bAuthored)
    {
        uBodyLineCount =
            Floppy144CatalogueDrawBodyText(
                surface,
                authored_document->pszBody,
                text,
                catalogue->document_scroll_line
            );
    }
    else
    {
        Floppy144CatalogueTextCentred(
            surface,
            138,
            "RECORD CONTENT NOT PRESENT ON DISK 144",
            1,
            amber
        );

        Floppy144CatalogueTextCentred(
            surface,
            166,
            "INDEX ENTRY RECONSTRUCTED FROM DISK 144 CROSS REFERENCES.",
            1,
            text
        );

        Floppy144CatalogueTextCentred(
            surface,
            184,
            "THE DOCUMENT MAY EXIST IN AN UNAVAILABLE COLLECTION.",
            1,
            muted
        );
    }

    Floppy144DrawFillRect(
        surface,
        10,
        306,
        610,
        28,
        background
    );

    Floppy144DrawRect(
        surface,
        10,
        306,
        610,
        28,
        border
    );

    Floppy144CatalogueTextCentred(
        surface,
        316,
        (
            bAuthored &&
            uBodyLineCount > FLOPPY144_DOCUMENT_BODY_MAX_LINES
        )
            ? "UP/DOWN SCROLL   BACKSPACE BACK"
            : "BACKSPACE BACK",
        1,
        amber
    );
}

/*
 * Catalogue state management
 *
 * The following functions initialise navigation, move the viewport, open and
 * close documents and report the current catalogue navigation state.
 */

void Floppy144CatalogueReset(
    Floppy144CatalogueState *catalogue,
    Floppy144CollectionId collection
)
{
    catalogue->collection = collection;
    catalogue->selected_index = 0;
    catalogue->top_index = 0;
    catalogue->document_scroll_line = 0U;
    catalogue->document_open = false;
}
/*
 * Move selection and keep it visible
 *
 * Selection is clamped to the registered record count. top_index follows
 * when the highlight leaves the current ten-row viewport.
 */

void Floppy144CatalogueMove(
    Floppy144CatalogueState *catalogue,
    int32_t direction
)
{
    const Floppy144CatalogueDefinition *definition =
        &Floppy144CollectionGet(
            catalogue->collection
        )->catalogue;

    uint32_t record_count =
        definition->record_count;

    int32_t next_index;

    if(
        catalogue->document_open ||
        record_count == 0U
    )
    {
        return;
    }

    next_index =
        (int32_t)catalogue->selected_index +
        direction;

    if(next_index < 0)
    {
        next_index = 0;
    }

    if(next_index >= (int32_t)record_count)
    {
        next_index =
            (int32_t)record_count - 1;
    }

    catalogue->selected_index =
        (uint32_t)next_index;

    if(
        catalogue->selected_index <
        catalogue->top_index
    )
    {
        catalogue->top_index =
            catalogue->selected_index;
    }

    if(
        catalogue->selected_index >=
        catalogue->top_index +
        FLOPPY144_CATALOGUE_ROWS
    )
    {
        catalogue->top_index =
            catalogue->selected_index -
            FLOPPY144_CATALOGUE_ROWS +
            1U;
    }
}
/*
 * Move by one visible page
 *
 * Reuses the single-row movement logic with a ten-record step.
 */

void Floppy144CataloguePage(
    Floppy144CatalogueState *catalogue,
    int32_t direction
)
{
    Floppy144CatalogueMove(
        catalogue,
        direction *
        (int32_t)FLOPPY144_CATALOGUE_ROWS
    );
}

/*
 * Open the selected record
 *
 * The evidence flag is set separately by main.c after this state change.
 */

/*
 * Open one exact catalogue record
 *
 * Command-driven record lookup uses this entry point to prepare the existing
 * document viewer without exposing catalogue state changes to main.c.
 */

bool Floppy144CatalogueOpenRecord(
    Floppy144CatalogueState *catalogue,
    Floppy144CollectionId collection,
    uint32_t record_index
)
{
    const Floppy144CatalogueDefinition *definition =
        &Floppy144CollectionGet(
            collection
        )->catalogue;

    if(
        catalogue == NULL ||
        record_index >= definition->record_count
    )
    {
        return false;
    }

    Floppy144CatalogueReset(
        catalogue,
        collection
    );

    catalogue->selected_index =
        record_index;

    catalogue->top_index =
        (
            record_index /
            FLOPPY144_CATALOGUE_ROWS
        ) *
        FLOPPY144_CATALOGUE_ROWS;

    catalogue->document_open =
        true;

    return true;
}
void Floppy144CatalogueOpenDocument(
    Floppy144CatalogueState *catalogue
)
{
    if(catalogue == NULL)
    {
        return;
    }

    catalogue->document_scroll_line = 0U;
    catalogue->document_open = true;
}
/*
 * Return from document view to the current catalogue position
 */

void Floppy144CatalogueCloseDocument(
    Floppy144CatalogueState *catalogue
)
{
    if(catalogue == NULL)
    {
        return;
    }

    catalogue->document_scroll_line = 0U;
    catalogue->document_open = false;
}

void Floppy144CatalogueScrollDocument(
    Floppy144CatalogueState *catalogue,
    int32_t direction
)
{
    const Floppy144DocumentDefinition *pDocument;
    uint32_t uLineCount;
    uint32_t uMaximumScroll;
    int32_t nNext;

    if(
        catalogue == NULL ||
        !catalogue->document_open ||
        direction == 0
    )
    {
        return;
    }

    pDocument =
        Floppy144DocumentGet(
            catalogue->collection,
            catalogue->selected_index
        );

    if(pDocument == NULL || pDocument->pszBody == NULL)
    {
        return;
    }

    uLineCount =
        Floppy144CatalogueProcessBodyText(
            NULL,
            pDocument->pszBody,
            0U,
            0U
        );

    if(uLineCount <= FLOPPY144_DOCUMENT_BODY_MAX_LINES)
    {
        catalogue->document_scroll_line = 0U;
        return;
    }

    uMaximumScroll =
        uLineCount -
        FLOPPY144_DOCUMENT_BODY_MAX_LINES;

    nNext =
        (int32_t)catalogue->document_scroll_line +
        direction;

    if(nNext < 0)
    {
        nNext = 0;
    }

    if(nNext > (int32_t)uMaximumScroll)
    {
        nNext = (int32_t)uMaximumScroll;
    }

    catalogue->document_scroll_line =
        (uint32_t)nNext;
}

/*
 * Query the current catalogue depth
 *
 * main.c uses this to make Backspace close a document before leaving the catalogue.
 */

bool Floppy144CatalogueDocumentOpen(
    const Floppy144CatalogueState *catalogue
)
{
    return catalogue->document_open;
}

/*
 * Catalogue draw dispatcher
 *
 * Chooses list or document rendering from a single boolean state flag.
 */

void Floppy144CatalogueDraw(
    F144Runtime *engine,
    const Floppy144CatalogueState *catalogue
)
{
    Floppy144Surface surface =
    {
        (uint32_t *)engine->backbuffer.data,
        engine->backbuffer.width,
        engine->backbuffer.height
    };

    switch(catalogue->document_open)
    {
        case true:
        {
            Floppy144CatalogueDrawDocument(
                &surface,
                catalogue
            );

            break;
        }

        case false:
        {
            Floppy144CatalogueDrawList(
                &surface,
                catalogue
            );

            break;
        }
    }
}
