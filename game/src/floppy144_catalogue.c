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

/*
 * Build a deterministic record ID and title
 *
 * The same collection and index always produce the same output. Different
 * multipliers give HR-02 and FA-03 distinct record-number sequences.
 * FA-03 record 047 is overridden with its stable authored identity.
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

    record_number =
        definition->record_number_base +
        (
            (
                index *
                definition->record_number_multiplier
            ) +
            definition->record_number_offset
        ) %
        1000U;

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
        20,
        20,
        600,
        284,
        panel
    );

    Floppy144DrawRect(
        surface,
        20,
        20,
        600,
        284,
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
        20,
        312,
        600,
        28,
        background
    );

    Floppy144DrawRect(
        surface,
        20,
        312,
        600,
        28,
        border
    );

    Floppy144DrawText(
        surface,
        32,
        322,
        "UP DOWN SELECT",
        1,
        text
    );

    Floppy144DrawText(
        surface,
        198,
        322,
        "PGUP PGDN PAGE",
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        384,
        322,
        "ENTER VIEW CONTENTS",
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        512,
        322,
        "BACKSPACE RETURN",
        1,
        muted
    );
}

/*
 * Draw the recovered FA-03 authored document
 *
 * This record is laid out directly because its complete contents exist on
 * Disk 144. Reading it reveals the reconstructed suppression control panel.
 */

static void Floppy144CatalogueDrawFa03ServiceNote(
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
        "FA-03",
        1,
        green
    );

    Floppy144DrawFillRect(
        surface,
        20,
        20,
        600,
        284,
        panel
    );

    Floppy144DrawRect(
        surface,
        20,
        20,
        600,
        284,
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
        "FA-03-RS-0047",
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
        20,
        316,
        600,
        28,
        background
    );

    Floppy144DrawRect(
        surface,
        20,
        316,
        600,
        28,
        border
    );

    Floppy144DrawText(
        surface,
        32,
        326,
        "RECOVERED AUTHORED RECORD",
        1,
        green
    );

    Floppy144DrawText(
        surface,
        526,
        326,
        "BACKSPACE BACK",
        1,
        muted
    );
}
/*
 * Draw the selected document
 *
 * FA-03 record 047 has a dedicated renderer. Other entries use this shared
 * viewer, which either shows the HR-02 authored memorandum or explains that
 * only the index entry was recovered.
 */

static const char *const floppy144_dr01_help_terminal_lines[] =
{
    "COMMANDS ARE ENTERED AT THE TERMINAL PROMPT.",
    "TYPE HELP TO DISPLAY AVAILABLE COMMANDS.",
    "TYPE EXIT TO CLOSE THE TERMINAL SESSION.",
    "USE BACKSPACE TO CORRECT THE CURRENT ENTRY.",
    "PRESS ENTER TO SUBMIT A COMMAND.",
    "ESC OPENS GDR SESSION CONTROL.",
    NULL
};

static const char *const floppy144_dr01_help_restoration_lines[] =
{
    "ARCHIVE SERVICES MAY INITIALLY BE OFFLINE.",
    "TYPE INITIATE TO INITIALISE RECOVERY SERVICES.",
    "TYPE LIST TO DISPLAY COLLECTIONS ON DISK 144.",
    "TYPE RESTORE CODE TO RESTORE A COLLECTION.",
    "EXAMPLE: RESTORE HR-02",
    "RESTORED COLLECTIONS REMAIN AVAILABLE.",
    "SOME COLLECTIONS MAY REVEAL PARTS OF THE SITE.",
    NULL
};

static const char *const floppy144_dr01_help_record_lines[] =
{
    "TYPE LIST CODE TO OPEN THE RECORD INDEX.",
    "PRESS SPACE TO DISPLAY THE NEXT PAGE.",
    "PRESS BACKSPACE TO DISPLAY THE PREVIOUS PAGE.",
    "PRESS ENTER TO RETURN TO THE TERMINAL PROMPT.",
    "TYPE OPEN RECORD-ID TO RETRIEVE A RECORD.",
    "EXAMPLE: OPEN HR-02-RS-1419",
    "ONLY RESTORED COLLECTIONS MAY BE SEARCHED.",
    "BACKSPACE RETURNS TO THE PREVIOUS VIEW.",
    NULL
};

/*
 * Draw one DR-01 operating-guidance page.
 */

static void Floppy144CatalogueDrawDr01HelpDocument(
    Floppy144Surface *surface,
    const Floppy144CatalogueState *catalogue,
    Floppy144DocumentView view
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

    const Floppy144CollectionDefinition *collection_definition =
        Floppy144CollectionGet(
            catalogue->collection
        );

    const char *const *lines =
        NULL;

    char record_id[24];
    char title[48];

    uint32_t line_index;

    switch(view)
    {
        case FLOPPY144_DOCUMENT_VIEW_DR01_HELP_TERMINAL_ACCESS:
        {
            lines =
                floppy144_dr01_help_terminal_lines;

            break;
        }

        case FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RESTORATION:
        {
            lines =
                floppy144_dr01_help_restoration_lines;

            break;
        }

        case FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RECORDS:
        {
            lines =
                floppy144_dr01_help_record_lines;

            break;
        }

        default:
        {
            return;
        }
    }

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
        20,
        20,
        600,
        284,
        panel
    );

    Floppy144DrawRect(
        surface,
        20,
        20,
        600,
        284,
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

    Floppy144DrawText(
        surface,
        52,
        102,
        "GDR OPERATING GUIDANCE",
        1,
        muted
    );

    for(
        line_index = 0U;
        lines[line_index] != NULL;
        ++line_index
    )
    {
        Floppy144DrawText(
            surface,
            52,
            124 + line_index * 18U,
            lines[line_index],
            1,
            text
        );
    }

    Floppy144DrawFillRect(
        surface,
        20,
        312,
        600,
        28,
        background
    );

    Floppy144DrawRect(
        surface,
        20,
        312,
        600,
        28,
        border
    );

    Floppy144CatalogueTextCentred(
        surface,
        322,
        "BACKSPACE BACK",
        1,
        amber
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
        authored_document->view ==
            FLOPPY144_DOCUMENT_VIEW_FA03_SUPPRESSION_SERVICE
    )
    {
        Floppy144CatalogueDrawFa03ServiceNote(
            surface
        );

        return;
    }

    if(
        authored_document != NULL &&
        (
            authored_document->view ==
                FLOPPY144_DOCUMENT_VIEW_DR01_HELP_TERMINAL_ACCESS ||
            authored_document->view ==
                FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RESTORATION ||
            authored_document->view ==
                FLOPPY144_DOCUMENT_VIEW_DR01_HELP_RECORDS
        )
    )
    {
        Floppy144CatalogueDrawDr01HelpDocument(
            surface,
            catalogue,
            authored_document->view
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

    bool authored =
        authored_document != NULL &&
        authored_document->view ==
            FLOPPY144_DOCUMENT_VIEW_HR02_DESK_REALLOCATION;

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
        20,
        20,
        600,
        284,
        panel
    );

    Floppy144DrawRect(
        surface,
        20,
        20,
        600,
        284,
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

    /* Choose between recovered content and the standard missing-content notice. */
    switch(authored)
    {
        case true:
        {
            Floppy144DrawText(
                surface,
                52,
                102,
                "FROM: SENIOR ARCHIVIST",
                1,
                muted
            );

            Floppy144DrawText(
                surface,
                52,
                118,
                "TO: RECORDS OFFICER",
                1,
                muted
            );

            Floppy144DrawText(
                surface,
                52,
                134,
                "SUBJECT: TEMPORARY DESK REALLOCATION",
                1,
                text
            );

            Floppy144DrawFillRect(
                surface,
                52,
                152,
                536,
                1,
                border
            );

            Floppy144DrawText(
                surface,
                52,
                168,
                "IT SUPPORT IS TO REMAIN AT DESK 04 UNTIL",
                1,
                text
            );

            Floppy144DrawText(
                surface,
                52,
                184,
                "THE TERMINAL CABLE HAS BEEN REPLACED.",
                1,
                text
            );

            Floppy144DrawText(
                surface,
                52,
                210,
                "THE MUG AT DESK 01 IS NOT AN ARCHIVE ITEM",
                1,
                text
            );

            Floppy144DrawText(
                surface,
                52,
                226,
                "AND SHOULD NOT BE ENTERED ON FORM AR-7.",
                1,
                text
            );

            Floppy144DrawText(
                surface,
                52,
                258,
                "SIGNED: SENIOR ARCHIVIST",
                1,
                muted
            );

            break;
        }

        case false:
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

            break;
        }
    }

    Floppy144DrawFillRect(
        surface,
        20,
        312,
        600,
        28,
        background
    );

    Floppy144DrawRect(
        surface,
        20,
        312,
        600,
        28,
        border
    );

    Floppy144CatalogueTextCentred(
        surface,
        322,
        "BACKSPACE BACK",
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
    catalogue->document_open = true;
}
/*
 * Return from document view to the current catalogue position
 */

void Floppy144CatalogueCloseDocument(
    Floppy144CatalogueState *catalogue
)
{
    catalogue->document_open = false;
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
    EngineData *engine,
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
