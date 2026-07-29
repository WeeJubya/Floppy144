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
    const Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
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

    bool hr02_selected =
        terminal->selected_collection ==
        FLOPPY144_COLLECTION_HR02;

    bool fa03_selected =
        terminal->selected_collection ==
        FLOPPY144_COLLECTION_FA03;

    bool collection_restored =
        hr02_selected
            ? world->hr02_restored
            : fa03_selected
                ? world->fa03_restored
                : true;

    bool evidence_found =
        hr02_selected &&
        world->hr02_desk_reallocation_read;

    const char *code =
        hr02_selected
            ? "COLLECTION HR-02"
            : fa03_selected
                ? "COLLECTION FA-03"
                : "COLLECTION XX-01";

    const char *title =
        hr02_selected
            ? "PERSONNEL LIFECYCLE RECORDS"
            : fa03_selected
                ? "SITE SAFETY AND SUPPRESSION SYSTEMS"
                : "SITE RECOVERY INDEX";

    const char *status =
        evidence_found
            ? "STATUS: EVIDENCE FOUND"
            : collection_restored
                ? "STATUS: RESTORED"
                : "STATUS: AVAILABLE";

    const char *collection_class =
        hr02_selected
            ? "CLASS: ADMINISTRATIVE"
            : fa03_selected
                ? "CLASS: OPERATIONAL"
                : "CLASS: MANDATORY";

    const char *description_one =
        hr02_selected
            ? "CONTAINS STAFF ALLOCATION AND DESK ASSIGNMENT RECORDS."
            : fa03_selected
                ? "CONTAINS FIRE SAFETY AND SUPPRESSION MAINTENANCE RECORDS."
                : "MINIMUM INDEX REQUIRED TO RECONSTRUCT A PARTIAL SITE.";

    const char *description_two =
        terminal->restoration_notice
            ? "RESTORATION COMPLETE. SITE SYSTEM DATA UPDATED."
            : evidence_found
                ? "EVIDENCE FOUND: DESK REALLOCATION MEMORANDUM."
                : collection_restored
                    ? "COLLECTION DATA IS PRESENT IN THE RECONSTRUCTED SITE."
                    : "PRESS ENTER TO RESTORE THIS COLLECTION.";

    uint32_t status_colour =
        evidence_found
            ? amber
            : collection_restored
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
        terminal->restoration_notice
            ? green
            : muted
    );

    Floppy144DrawFillRect(
        surface,
        80,
        252,
        480,
        1,
        border
    );


}

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal
)
{
    terminal->selected_collection = FLOPPY144_COLLECTION_XX01;
    terminal->detail_open = false;
    terminal->restoration_notice = false;
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
        next_selection =
            (int32_t)FLOPPY144_COLLECTION_COUNT - 1;
    }

    if(
        next_selection >=
        (int32_t)FLOPPY144_COLLECTION_COUNT
    )
    {
        next_selection = 0;
    }

    terminal->selected_collection =
        (Floppy144CollectionId)next_selection;
}
void Floppy144TerminalOpenSelection(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world
)
{
    if(!terminal->detail_open)
    {
        terminal->detail_open = true;
        terminal->restoration_notice = false;
        return;
    }

    if(
        terminal->selected_collection ==
            FLOPPY144_COLLECTION_HR02 &&
        !world->hr02_restored
    )
    {
        world->hr02_restored = true;
        terminal->restoration_notice = true;
        return;
    }

    if(
        terminal->selected_collection ==
            FLOPPY144_COLLECTION_FA03 &&
        !world->fa03_restored
    )
    {
        world->fa03_restored = true;
        terminal->restoration_notice = true;
    }
}
void Floppy144TerminalCloseDetail(
    Floppy144TerminalState *terminal
)
{
    terminal->detail_open = false;
    terminal->restoration_notice = false;
}

bool Floppy144TerminalDetailOpen(
    const Floppy144TerminalState *terminal
)
{
    return terminal->detail_open;
}

void Floppy144TerminalDraw(
    EngineData *engine,
    const Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
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

    const char *site_status =
        world->hr02_restored &&
        world->fa03_restored
            ? "SITE 20%"
            : world->hr02_restored ||
              world->fa03_restored
                ? "SITE 12%"
                : "SITE 04%";

    const char *hr02_status =
        world->hr02_desk_reallocation_read
            ? "EVIDENCE"
            : world->hr02_restored
                ? "RESTORED"
                : "AVAILABLE";

    uint32_t hr02_colour =
        world->hr02_desk_reallocation_read
            ? amber
            : world->hr02_restored
                ? green
                : amber;

    const char *fa03_status =
        world->fa03_restored
            ? "RESTORED"
            : "AVAILABLE";

    uint32_t fa03_colour =
        world->fa03_restored
            ? green
            : amber;

    const char *footer_left =
        terminal->detail_open
            ? ""
            : "UP DOWN SELECT";

    const char *footer_middle =
        terminal->detail_open
            ? terminal->selected_collection ==
                FLOPPY144_COLLECTION_HR02
                ? world->hr02_restored
                    ? "ENTER VIEW RECORDS"
                    : "ENTER RESTORE"
                : terminal->selected_collection ==
                    FLOPPY144_COLLECTION_FA03
                    ? world->fa03_restored
                        ? "ENTER VIEW RECORDS"
                        : "ENTER RESTORE"
                    : ""
            : "ENTER OPEN";

    const char *footer_right =
        terminal->detail_open
            ? "ESC CLOSE"
            : "ESC RETURN";

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
        site_status,
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

    Floppy144DrawText(&surface, 44, 72, "SITE:", 1, muted);
    Floppy144DrawText(&surface, 122, 72, "PARTIAL RECONSTRUCTION", 1, text);

    Floppy144DrawText(&surface, 44, 88, "MEDIA:", 1, muted);
    Floppy144DrawText(&surface, 122, 88, "DISK 144", 1, text);

    Floppy144DrawText(&surface, 44, 104, "PROTOCOL:", 1, muted);
    Floppy144DrawText(&surface, 122, 104, "APS-12", 1, amber);

    Floppy144DrawFillRect(&surface, 40, 124, 560, 1, border);

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
        world->hr02_restored
            ? "RESTORED COLLECTIONS"
            : "AVAILABLE COLLECTIONS",
        1,
        muted
    );

    Floppy144TerminalDrawCollection(
        &surface,
        198,
        terminal->selected_collection == 1,
        hr02_status,
        "HR-02",
        "PERSONNEL LIFECYCLE RECORDS",
        hr02_colour
    );

    Floppy144TerminalDrawCollection(
        &surface,
        246,
        terminal->selected_collection ==
            FLOPPY144_COLLECTION_FA03,
        fa03_status,
        "FA-03",
        "SITE SAFETY AND SUPPRESSION SYSTEMS",
        fa03_colour
    );

    Floppy144DrawFillRect(&surface, 40, 286, 560, 1, border);


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
        footer_left,
        1,
        text
    );

    Floppy144DrawText(
        &surface,
        250,
        326,
        footer_middle,
        1,
        amber
    );

    Floppy144DrawText(
        &surface,
        490,
        326,
        footer_right,
        1,
        muted
    );

    if(terminal->detail_open)
    {
        Floppy144TerminalDrawDetail(
            &surface,
            terminal,
            world
        );
    }
}
















