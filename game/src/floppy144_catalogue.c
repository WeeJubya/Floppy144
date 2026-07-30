/*
 * Floppy//144 - record catalogue implementation
 *
 * Generates deterministic record listings for each collection, renders the
 * scrollable catalogue, displays recovered documents and identifies evidence.
 */

#include "floppy144_catalogue.h"

#include "floppy144_draw.h"

#include <stddef.h>
#include <stdio.h>

/*
 * Catalogue size and authored-record positions
 *
 * Each collection exposes 100 deterministic index entries in this slice.
 * The authored constants are zero-based, so 37 is visible record 038 and
 * 62 is visible record 063.
 */

#define FLOPPY144_CATALOGUE_COUNT       100U
#define FLOPPY144_CATALOGUE_ROWS         10U
#define FLOPPY144_HR02_AUTHORED_RECORD   37U
#define FLOPPY144_FA03_AUTHORED_RECORD   62U

/*
 * Procedural title vocabulary
 *
 * Subjects vary by collection while document forms are shared. Combining
 * the two tables produces plausible archive titles without storing 200 strings.
 */

static const char *floppy144_hr02_record_subjects[10] =
{
    "APPOINTMENT",
    "TRANSFER",
    "ABSENCE",
    "TRAINING",
    "ACCESS",
    "PAYROLL",
    "LEAVE",
    "DESK ALLOCATION",
    "APPRAISAL",
    "EXIT"
};

static const char *floppy144_fa03_record_subjects[10] =
{
    "SUPPRESSION PANEL",
    "HALON CYLINDER",
    "ALARM CIRCUIT",
    "SERVER ROOM SAFETY",
    "EMERGENCY CONTROL",
    "VENTILATION SYSTEM",
    "FIRE DOOR",
    "DETECTOR LOOP",
    "MAINTENANCE ACCESS",
    "PRESSURE SENSOR"
};

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

/*
 * Build a deterministic record ID and title
 *
 * The same collection and index always produce the same output. Different
 * multipliers give HR-02 and FA-03 distinct record-number sequences.
 * FA-03 record 063 is overridden with its stable authored identity.
 */

static void Floppy144CatalogueBuildRecord(
    Floppy144CollectionId collection,
    uint32_t index,
    char *record_id,
    size_t record_id_size,
    char *title,
    size_t title_size
)
{
    /* Select the collection-specific vocabulary table. */
    const char *const *subjects =
        collection == FLOPPY144_COLLECTION_FA03
            ? floppy144_fa03_record_subjects
            : floppy144_hr02_record_subjects;

    /* Turn the index into repeatable subject, form and record-number components. */
    uint32_t subject_index =
        index % 10U;

    uint32_t group_index =
        index / 10U;

    uint32_t form_index =
        (group_index + subject_index * 3U) % 10U;

    uint32_t record_number =
        collection == FLOPPY144_COLLECTION_FA03
            ? 2400U + ((index * 53U + 11U) % 1000U)
            : 1400U + ((index * 37U + 19U) % 1000U);

    /* Stable authored records override the procedural ID and title. */
    if(
        collection == FLOPPY144_COLLECTION_FA03 &&
        index == FLOPPY144_FA03_AUTHORED_RECORD
    )
    {
        snprintf(
            record_id,
            record_id_size,
            "FA-03-RS-0063"
        );

        snprintf(
            title,
            title_size,
            "SUPPRESSION CONTROL PANEL SERVICE NOTE"
        );

        return;
    }

    snprintf(
        record_id,
        record_id_size,
        collection == FLOPPY144_COLLECTION_FA03
            ? "FA-03-RS-%04u"
            : "HR-02-RS-%04u",
        (unsigned)record_number
    );

    snprintf(
        title,
        title_size,
        "%s %s",
        subjects[subject_index],
        floppy144_record_forms[form_index]
    );
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

    char position_text[24];

    uint32_t visible_row;
    uint32_t thumb_y;

    /* Display uses one-based record numbers even though state uses zero-based indices. */
    snprintf(
        position_text,
        sizeof(position_text),
        "RECORD %03u OF 100",
        (unsigned)(catalogue->selected_index + 1U)
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
        "HR-02",
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
        catalogue->collection == FLOPPY144_COLLECTION_FA03
            ? "SITE SAFETY AND SUPPRESSION SYSTEMS"
            : "PERSONNEL LIFECYCLE RECORDS",
        2,
        text
    );

    Floppy144DrawText(
        surface,
        40,
        60,
        catalogue->collection == FLOPPY144_COLLECTION_FA03
            ? "COLLECTION: FA-03"
            : "COLLECTION: HR-02",
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
        "GENERATED INDEX ENTRIES: 100",
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

    /* Map top_index onto the scrollbar track so the thumb follows the viewport. */
    thumb_y =
        98U +
        catalogue->top_index *
        168U /
        (FLOPPY144_CATALOGUE_COUNT -
         FLOPPY144_CATALOGUE_ROWS);

    Floppy144DrawFillRect(
        surface,
        592,
        thumb_y,
        8,
        20,
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
        "ESC RETURN",
        1,
        muted
    );
}

/*
 * Draw the recovered FA-03 authored document
 *
 * This record is laid out directly because its complete contents exist on
 * Disk 144. Reading it unlocks the suppression-service evidence flag.
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
        "FA-03-RS-0063",
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
        "ESC BACK",
        1,
        muted
    );
}
/*
 * Draw the selected document
 *
 * FA-03 record 063 has a dedicated renderer. Other entries use this shared
 * viewer, which either shows the HR-02 authored memorandum or explains that
 * only the index entry was recovered.
 */

static void Floppy144CatalogueDrawDocument(
    Floppy144Surface *surface,
    const Floppy144CatalogueState *catalogue
)
{
    /* Route the special FA-03 authored record before drawing the generic viewer. */
    if(
        catalogue->collection ==
            FLOPPY144_COLLECTION_FA03 &&
        catalogue->selected_index ==
            FLOPPY144_FA03_AUTHORED_RECORD
    )
    {
        Floppy144CatalogueDrawFa03ServiceNote(
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

    /* Only HR-02 record 038 is fully authored in the shared viewer. */
    bool authored =
        catalogue->collection ==
            FLOPPY144_COLLECTION_HR02 &&
        catalogue->selected_index ==
            FLOPPY144_HR02_AUTHORED_RECORD;

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
        "HR-02",
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
                "INDEX ENTRY RECONSTRUCTED FROM XX-01 CROSS REFERENCES.",
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
        "ESC RETURN TO RECORD CATALOGUE",
        1,
        amber
    );
}

/*
 * Catalogue state management
 *
 * The following functions initialise navigation, move the viewport, open and
 * close documents, and report whether the selected record supplies evidence.
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
 * Selection is clamped to 0-99. top_index follows when the highlight leaves
 * the current ten-row viewport.
 */

void Floppy144CatalogueMove(
    Floppy144CatalogueState *catalogue,
    int32_t direction
)
{
    int32_t next_index;

    if(catalogue->document_open)
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

    if(next_index >=
       (int32_t)FLOPPY144_CATALOGUE_COUNT)
    {
        next_index =
            (int32_t)FLOPPY144_CATALOGUE_COUNT - 1;
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

void Floppy144CatalogueOpenDocument(
    Floppy144CatalogueState *catalogue
)
{
    catalogue->document_open = true;
}

/*
 * Identify primary evidence records
 *
 * Collection and record index are both checked so identically numbered records
 * in other collections cannot unlock the wrong evidence.
 */

bool Floppy144CatalogueSelectedProvidesEvidence(
    const Floppy144CatalogueState *catalogue
)
{
    bool hr02_evidence =
        catalogue->collection ==
            FLOPPY144_COLLECTION_HR02 &&
        catalogue->selected_index ==
            FLOPPY144_HR02_AUTHORED_RECORD;

    bool fa03_evidence =
        catalogue->collection ==
            FLOPPY144_COLLECTION_FA03 &&
        catalogue->selected_index ==
            FLOPPY144_FA03_AUTHORED_RECORD;

    return
        hr02_evidence ||
        fa03_evidence;
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
 * main.c uses this to make Escape close a document before leaving the catalogue.
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
