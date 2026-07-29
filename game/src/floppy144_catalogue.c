#include "floppy144_catalogue.h"

#include "floppy144_draw.h"

#include <stddef.h>
#include <stdio.h>

#define FLOPPY144_CATALOGUE_COUNT 100U
#define FLOPPY144_CATALOGUE_ROWS  10U
#define FLOPPY144_AUTHORED_RECORD 37U

static const char *floppy144_record_subjects[10] =
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

static void Floppy144CatalogueBuildRecord(
    uint32_t index,
    char *record_id,
    size_t record_id_size,
    char *title,
    size_t title_size
)
{
    uint32_t subject_index =
        index % 10U;

    uint32_t group_index =
        index / 10U;

    uint32_t form_index =
        (group_index + subject_index * 3U) % 10U;

    uint32_t record_number =
        1400U + ((index * 37U + 19U) % 1000U);

    snprintf(
        record_id,
        record_id_size,
        "HR-02-RS-%04u",
        (unsigned)record_number
    );

    snprintf(
        title,
        title_size,
        "%s %s",
        floppy144_record_subjects[subject_index],
        floppy144_record_forms[form_index]
    );
}

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

static void Floppy144CatalogueDrawRow(
    Floppy144Surface *surface,
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
        "PERSONNEL LIFECYCLE RECORDS",
        2,
        text
    );

    Floppy144DrawText(
        surface,
        40,
        60,
        "COLLECTION: HR-02",
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

static void Floppy144CatalogueDrawDocument(
    Floppy144Surface *surface,
    const Floppy144CatalogueState *catalogue
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

    char record_id[24];
    char title[48];

    bool authored =
        catalogue->selected_index ==
        FLOPPY144_AUTHORED_RECORD;

    Floppy144CatalogueBuildRecord(
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

void Floppy144CatalogueOpenDocument(
    Floppy144CatalogueState *catalogue
)
{
    catalogue->document_open = true;
}

bool Floppy144CatalogueSelectedProvidesEvidence(
    const Floppy144CatalogueState *catalogue
)
{
    return
        catalogue->collection ==
            FLOPPY144_COLLECTION_HR02 &&
        catalogue->selected_index ==
            FLOPPY144_AUTHORED_RECORD;
}
void Floppy144CatalogueCloseDocument(
    Floppy144CatalogueState *catalogue
)
{
    catalogue->document_open = false;
}

bool Floppy144CatalogueDocumentOpen(
    const Floppy144CatalogueState *catalogue
)
{
    return catalogue->document_open;
}

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




