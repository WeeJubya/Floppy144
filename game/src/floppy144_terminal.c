#include "floppy144_terminal.h"

#include "floppy144_draw.h"

static void Floppy144TerminalTextCentred(
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
        (surface->width - text_width) / 2;

    Floppy144DrawText(
        surface,
        x,
        y,
        text,
        scale,
        colour
    );
}

static void Floppy144TerminalDrawCollection(
    Floppy144Surface *surface,
    uint32_t y,
    bool selected,
    const char *status,
    const char *code,
    const char *title,
    uint32_t status_colour
)
{
    const uint32_t row_colour =
        FLOPPY144_RGB(21, 29, 34);

    const uint32_t selected_colour =
        FLOPPY144_RGB(37, 45, 47);

    const uint32_t border_colour =
        FLOPPY144_RGB(89, 104, 105);

    const uint32_t text_colour =
        FLOPPY144_RGB(202, 211, 205);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    Floppy144DrawFillRect(
        surface,
        40,
        y,
        560,
        22,
        selected ? selected_colour : row_colour
    );

    Floppy144DrawRect(
        surface,
        40,
        y,
        560,
        22,
        selected ? amber : border_colour
    );

    Floppy144DrawText(
        surface,
        48,
        y + 7,
        selected ? ">" : " ",
        1,
        amber
    );

    Floppy144DrawText(
        surface,
        66,
        y + 7,
        status,
        1,
        status_colour
    );

    Floppy144DrawText(
        surface,
        154,
        y + 7,
        code,
        1,
        text_colour
    );

    Floppy144DrawText(
        surface,
        214,
        y + 7,
        title,
        1,
        text_colour
    );
}

static void Floppy144TerminalDrawDetail(
    Floppy144Surface *surface,
    const Floppy144TerminalState *terminal
)
{
    const uint32_t background =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t panel =
        FLOPPY144_RGB(27, 36, 41);

    const uint32_t border =
        FLOPPY144_RGB(108, 121, 119);

    const uint32_t text =
        FLOPPY144_RGB(202, 211, 205);

    const uint32_t muted =
        FLOPPY144_RGB(118, 133, 132);

    const uint32_t green =
        FLOPPY144_RGB(100, 156, 111);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    const char *code =
        terminal->selected_collection == 0
            ? "COLLECTION XX-01"
            : "COLLECTION HR-02";

    const char *title =
        terminal->selected_collection == 0
            ? "SITE RECOVERY INDEX"
            : "PERSONNEL LIFECYCLE RECORDS";

    const char *status =
        terminal->selected_collection == 0
            ? "STATUS: RESTORED"
            : "STATUS: AVAILABLE";

    const char *collection_class =
        terminal->selected_collection == 0
            ? "CLASS: MANDATORY"
            : "CLASS: ADMINISTRATIVE";

    const char *description_one =
        terminal->selected_collection == 0
            ? "MINIMUM INDEX REQUIRED TO RECONSTRUCT A PARTIAL SITE."
            : "CONTAINS STAFF ALLOCATION AND DESK ASSIGNMENT RECORDS.";

    const char *description_two =
        terminal->selected_collection == 0
            ? "THIS COLLECTION WAS RESTORED AUTOMATICALLY."
            : "RESTORATION CONTROL IS NOT YET ENABLED.";

    uint32_t status_colour =
        terminal->selected_collection == 0
            ? green
            : amber;

    Floppy144DrawFillRect(
        surface,
        64,
        72,
        512,
        216,
        background
    );

    Floppy144DrawRect(
        surface,
        64,
        72,
        512,
        216,
        border
    );

    Floppy144DrawFillRect(
        surface,
        72,
        80,
        496,
        32,
        panel
    );

    Floppy144DrawText(
        surface,
        84,
        91,
        code,
        1,
        text
    );

    Floppy144DrawText(
        surface,
        84,
        122,
        title,
        1,
        text
    );

    Floppy144DrawFillRect(
        surface,
        80,
        142,
        480,
        1,
        border
    );

    Floppy144DrawText(
        surface,
        84,
        158,
        status,
        1,
        status_colour
    );

    Floppy144DrawText(
        surface,
        84,
        178,
        collection_class,
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        84,
        208,
        description_one,
        1,
        text
    );

    Floppy144DrawText(
        surface,
        84,
        228,
        description_two,
        1,
        muted
    );

    Floppy144DrawFillRect(
        surface,
        80,
        252,
        480,
        1,
        border
    );

    Floppy144TerminalTextCentred(
        surface,
        268,
        "ESC CLOSE COLLECTION",
        1,
        amber
    );
}

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal
)
{
    terminal->selected_collection = 0;
    terminal->detail_open = false;
}

void Floppy144TerminalMoveSelection(
    Floppy144TerminalState *terminal,
    int32_t direction
)
{
    int32_t next_selection;

    if(terminal->detail_open)
    {
        return;
    }

    next_selection =
        (int32_t)terminal->selected_collection +
        direction;

    if(next_selection < 0)
    {
        next_selection = 1;
    }

    if(next_selection > 1)
    {
        next_selection = 0;
    }

    terminal->selected_collection =
        (uint32_t)next_selection;
}

void Floppy144TerminalOpenSelection(
    Floppy144TerminalState *terminal
)
{
    terminal->detail_open = true;
}

void Floppy144TerminalCloseDetail(
    Floppy144TerminalState *terminal
)
{
    terminal->detail_open = false;
}

bool Floppy144TerminalDetailOpen(
    const Floppy144TerminalState *terminal
)
{
    return terminal->detail_open;
}

void Floppy144TerminalDraw(
    EngineData *engine,
    const Floppy144TerminalState *terminal
)
{
    const uint32_t background =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t panel =
        FLOPPY144_RGB(24, 33, 39);

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

    Floppy144Surface surface =
    {
        (uint32_t *)engine->backbuffer.data,
        engine->backbuffer.width,
        engine->backbuffer.height
    };

    Floppy144DrawClear(
        &surface,
        background
    );

    Floppy144DrawFillRect(
        &surface,
        0,
        0,
        640,
        16,
        background
    );

    Floppy144DrawText(
        &surface,
        10,
        5,
        "GDR ARCHIVE RESTORATION TERMINAL",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        538,
        5,
        "READY",
        1,
        green
    );

    Floppy144DrawFillRect(
        &surface,
        20,
        28,
        600,
        280,
        panel
    );

    Floppy144DrawRect(
        &surface,
        20,
        28,
        600,
        280,
        border
    );

    Floppy144TerminalTextCentred(
        &surface,
        42,
        "GDR ARCHIVE RESTORATION TERMINAL",
        2,
        text
    );

    Floppy144DrawText(
        &surface,
        44,
        72,
        "SITE:",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        122,
        72,
        "PARTIAL RECONSTRUCTION",
        1,
        text
    );

    Floppy144DrawText(
        &surface,
        44,
        88,
        "MEDIA:",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        122,
        88,
        "DISK 144",
        1,
        text
    );

    Floppy144DrawText(
        &surface,
        44,
        104,
        "PROTOCOL:",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        122,
        104,
        "APS-12",
        1,
        amber
    );

    Floppy144DrawFillRect(
        &surface,
        40,
        124,
        560,
        1,
        border
    );

    Floppy144DrawText(
        &surface,
        44,
        136,
        "MANDATORY COLLECTIONS",
        1,
        muted
    );

    Floppy144TerminalDrawCollection(
        &surface,
        150,
        terminal->selected_collection == 0,
        "RESTORED",
        "XX-01",
        "SITE RECOVERY INDEX",
        green
    );

    Floppy144DrawText(
        &surface,
        44,
        184,
        "AVAILABLE COLLECTIONS",
        1,
        muted
    );

    Floppy144TerminalDrawCollection(
        &surface,
        198,
        terminal->selected_collection == 1,
        "AVAILABLE",
        "HR-02",
        "PERSONNEL LIFECYCLE RECORDS",
        amber
    );

    Floppy144DrawFillRect(
        &surface,
        40,
        238,
        560,
        1,
        border
    );

    Floppy144TerminalTextCentred(
        &surface,
        258,
        "ENTER OPENS COLLECTION INFORMATION",
        1,
        muted
    );

    Floppy144DrawFillRect(
        &surface,
        20,
        316,
        600,
        28,
        background
    );

    Floppy144DrawRect(
        &surface,
        20,
        316,
        600,
        28,
        border
    );

    Floppy144DrawText(
        &surface,
        32,
        326,
        "UP DOWN SELECT",
        1,
        text
    );

    Floppy144DrawText(
        &surface,
        250,
        326,
        "ENTER OPEN",
        1,
        amber
    );

    Floppy144DrawText(
        &surface,
        490,
        326,
        "ESC RETURN",
        1,
        muted
    );

    if(terminal->detail_open)
    {
        Floppy144TerminalDrawDetail(
            &surface,
            terminal
        );
    }
}
