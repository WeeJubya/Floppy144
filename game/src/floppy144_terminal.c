/*
 * Floppy//144 - archive terminal implementation
 *
 * Renders the collection list and detail overlay, handles terminal-local
 * navigation, and writes collection restoration into the shared world state.
 */

#include "floppy144_terminal.h"

#include "floppy144_draw.h"
#include "floppy144_collection_registry.h"

#include <stdio.h>

/*
 * Small terminal drawing helpers
 *
 * The first helper centres headings. The second draws one selectable
 * collection row with status, code, title and selection border.
 */

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

/*
 * Draw one collection row
 *
 * All rows share the same geometry. The caller supplies only the row-specific
 * content and status colour.
 */

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

/*
 * Draw the selected collection overlay
 *
 * This translates the selected collection plus world flags into human-readable
 * code, title, class, restoration status and evidence messages.
 */

/*
 * Temporary collection-state bridge
 *
 * Collection metadata now comes from the registry, but the technical slice
 * still stores restoration and evidence in separate HR-02 and FA-03 fields.
 *
 * The next refactor phase replaces these switches with indexed collection
 * state. Until then, all legacy state knowledge is contained here.
 */

static bool Floppy144TerminalCollectionRestored(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    switch(collection)
    {
        case FLOPPY144_COLLECTION_HR02:
        {
            return world->hr02_restored;
        }

        case FLOPPY144_COLLECTION_FA03:
        {
            return world->fa03_restored;
        }

        case FLOPPY144_COLLECTION_XX01:
        {
            return true;
        }

        default:
        {
            return
                Floppy144CollectionGet(collection)->
                    auto_restored;
        }
    }
}

static bool Floppy144TerminalCollectionEvidenceFound(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    switch(collection)
    {
        case FLOPPY144_COLLECTION_HR02:
        {
            return
                world->hr02_desk_reallocation_read;
        }

        case FLOPPY144_COLLECTION_FA03:
        {
            return
                world->fa03_suppression_service_read;
        }

        default:
        {
            return false;
        }
    }
}

static const char *Floppy144TerminalEvidenceDescription(
    Floppy144CollectionId collection
)
{
    switch(collection)
    {
        case FLOPPY144_COLLECTION_HR02:
        {
            return
                "EVIDENCE FOUND: DESK REALLOCATION MEMORANDUM.";
        }

        case FLOPPY144_COLLECTION_FA03:
        {
            return
                "EVIDENCE FOUND: SUPPRESSION PANEL SERVICE NOTE.";
        }

        default:
        {
            return
                "EVIDENCE FOUND: RECOVERED AUTHORED RECORD.";
        }
    }
}

static bool Floppy144TerminalRestoreCollection(
    Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    switch(collection)
    {
        case FLOPPY144_COLLECTION_HR02:
        {
            world->hr02_restored = true;
            return true;
        }

        case FLOPPY144_COLLECTION_FA03:
        {
            world->fa03_restored = true;
            return true;
        }

        default:
        {
            return false;
        }
    }
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

    /*
     * Derive collection-specific state
     *
     * These booleans keep the rendering expressions readable and ensure that
     * the same evidence flag controls both list and detail views.
     */

    const Floppy144CollectionDefinition *definition =
        Floppy144CollectionGet(
            terminal->selected_collection
        );

    bool collection_restored =
        Floppy144TerminalCollectionRestored(
            world,
            terminal->selected_collection
        );

    bool evidence_found =
        Floppy144TerminalCollectionEvidenceFound(
            world,
            terminal->selected_collection
        );

    char code[32];
    char collection_class[32];

    snprintf(
        code,
        sizeof(code),
        "COLLECTION %s",
        definition->code
    );

    snprintf(
        collection_class,
        sizeof(collection_class),
        "CLASS: %s",
        Floppy144CollectionClassText(
            definition->collection_class
        )
    );

    const char *title =
        definition->title;

    const char *status =
        evidence_found
            ? "STATUS: EVIDENCE FOUND"
            : collection_restored
                ? "STATUS: RESTORED"
                : "STATUS: AVAILABLE";

    const char *description_one =
        definition->description;

    const char *description_two =
        terminal->restoration_notice
            ? "RESTORATION COMPLETE. SITE SYSTEM DATA UPDATED."
            : evidence_found
                ? Floppy144TerminalEvidenceDescription(
                    terminal->selected_collection
                )
                : collection_restored
                    ? "COLLECTION DATA IS PRESENT IN THE RECONSTRUCTED SITE."
                    : "PRESS ENTER TO RESTORE THIS COLLECTION.";
    uint32_t status_colour =
        evidence_found
            ? amber
            : collection_restored
                ? green
                : amber;

    /* Paint the modal panel over the already-drawn terminal list. */
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

/*
 * Terminal state management
 *
 * Reset selects XX-01 and closes overlays. Navigation wraps around the
 * collection enum while details are closed.
 */

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal
)
{
    terminal->selected_collection = FLOPPY144_COLLECTION_XX01;
    terminal->detail_open = false;
    terminal->restoration_notice = false;
}

/*
 * Move the highlighted collection
 *
 * direction is normally -1 or +1. Selection wraps from either end.
 */

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
/*
 * Open details or restore a collection
 *
 * First Enter opens the detail overlay. A later Enter restores HR-02 or
 * FA-03 if needed. Restoring changes the persistent world state.
 */

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
        !Floppy144TerminalCollectionRestored(
            world,
            terminal->selected_collection
        ) &&
        Floppy144TerminalRestoreCollection(
            world,
            terminal->selected_collection
        )
    )
    {
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

/*
 * Query whether the detail overlay is active
 *
 * main.c uses this to decide whether Escape closes details or leaves the terminal.
 */

bool Floppy144TerminalDetailOpen(
    const Floppy144TerminalState *terminal
)
{
    return terminal->detail_open;
}

/*
 * Draw the complete terminal screen
 *
 * Builds collection status labels from world state, draws the base list and
 * footer, then overlays details when detail_open is true.
 */

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

    /*
     * Dynamic list and footer text
     *
     * Restored collections display green. Evidence-bearing collections display
     * amber. Footer instructions change with the current navigation depth.
     */

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
        world->fa03_suppression_service_read
            ? "EVIDENCE"
            : world->fa03_restored
                ? "RESTORED"
                : "AVAILABLE";

    uint32_t fa03_colour =
        world->fa03_suppression_service_read
            ? amber
            : world->fa03_restored
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

    /*
     * Draw the terminal base layer
     *
     * The list remains underneath the detail overlay, making close-detail a simple
     * state change followed by a redraw.
     */

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

    /* XX-01 is mandatory and is therefore always shown as restored. */
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

    /* Optional collection rows reflect restoration and evidence flags. */
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

    /* Draw the modal detail panel last so it appears above the collection list. */
    if(terminal->detail_open)
    {
        Floppy144TerminalDrawDetail(
            &surface,
            terminal,
            world
        );
    }
}




