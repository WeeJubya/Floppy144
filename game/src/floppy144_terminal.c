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
 * This translates registered metadata plus indexed world state into
 * player-facing code, title, class, restoration status and evidence messages.
 */

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
     * Derive selected collection state
     *
     * These values keep the rendering expressions readable and ensure that
     * the same indexed state controls both list and detail views.
     */

    const Floppy144CollectionDefinition *definition =
        Floppy144CollectionGet(
            terminal->selected_collection
        );

    bool collection_restored =
        Floppy144WorldCollectionRestored(
            world,
            terminal->selected_collection
        );

    bool evidence_found =
        Floppy144WorldCollectionEvidenceFound(
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
                ? definition->evidence_description != NULL
                    ? definition->evidence_description
                    : "EVIDENCE FOUND: RECOVERED AUTHORED RECORD."
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
 * The terminal displays one narrative act at a time. Selection remains within
 * that act, while act navigation skips pages containing no registered
 * collections.
 */

static bool Floppy144TerminalActHasCollections(
    Floppy144Act act
)
{
    uint32_t collection_index;

    for(
        collection_index = 0U;
        collection_index <
            (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        const Floppy144CollectionDefinition *definition =
            Floppy144CollectionGet(
                (Floppy144CollectionId)collection_index
            );

        if(definition->act == act)
        {
            return true;
        }
    }

    return false;
}

static Floppy144CollectionId Floppy144TerminalFirstCollectionInAct(
    Floppy144Act act
)
{
    uint32_t collection_index;

    for(
        collection_index = 0U;
        collection_index <
            (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        Floppy144CollectionId collection =
            (Floppy144CollectionId)collection_index;

        const Floppy144CollectionDefinition *definition =
            Floppy144CollectionGet(
                collection
            );

        if(definition->act == act)
        {
            return collection;
        }
    }

    return FLOPPY144_COLLECTION_XX01;
}

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal
)
{
    terminal->selected_act =
        FLOPPY144_ACT_PROLOGUE;

    terminal->selected_collection =
        Floppy144TerminalFirstCollectionInAct(
            terminal->selected_act
        );

    terminal->detail_open =
        false;

    terminal->restoration_notice =
        false;
}

/*
 * Move the highlighted collection within the visible act.
 */

void Floppy144TerminalMoveSelection(
    Floppy144TerminalState *terminal,
    int32_t direction
)
{
    int32_t next_selection;
    int32_t step;
    uint32_t attempts;

    if(
        terminal->detail_open ||
        direction == 0
    )
    {
        return;
    }

    step =
        direction < 0
            ? -1
            : 1;

    next_selection =
        (int32_t)terminal->selected_collection;

    for(
        attempts = 0U;
        attempts <
            (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++attempts
    )
    {
        const Floppy144CollectionDefinition *definition;

        next_selection +=
            step;

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
            next_selection =
                0;
        }

        definition =
            Floppy144CollectionGet(
                (Floppy144CollectionId)next_selection
            );

        if(
            definition->act ==
            terminal->selected_act
        )
        {
            terminal->selected_collection =
                (Floppy144CollectionId)next_selection;

            return;
        }
    }
}

/*
 * Move to the next populated act page.
 */

void Floppy144TerminalMoveAct(
    Floppy144TerminalState *terminal,
    int32_t direction
)
{
    int32_t next_act;
    int32_t step;
    uint32_t attempts;

    if(
        terminal->detail_open ||
        direction == 0
    )
    {
        return;
    }

    step =
        direction < 0
            ? -1
            : 1;

    next_act =
        (int32_t)terminal->selected_act;

    for(
        attempts = 0U;
        attempts <
            (uint32_t)FLOPPY144_ACT_COUNT;
        ++attempts
    )
    {
        next_act +=
            step;

        if(next_act < 0)
        {
            next_act =
                (int32_t)FLOPPY144_ACT_COUNT - 1;
        }

        if(
            next_act >=
            (int32_t)FLOPPY144_ACT_COUNT
        )
        {
            next_act =
                0;
        }

        if(
            Floppy144TerminalActHasCollections(
                (Floppy144Act)next_act
            )
        )
        {
            terminal->selected_act =
                (Floppy144Act)next_act;

            terminal->selected_collection =
                Floppy144TerminalFirstCollectionInAct(
                    terminal->selected_act
                );

            terminal->restoration_notice =
                false;

            return;
        }
    }
}
/*
 * Open details or restore a collection
 *
 * First Enter opens the detail overlay. A later Enter asks the world-state
 * system to restore whichever registered collection is selected.
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
        Floppy144WorldRestoreCollection(
            world,
            terminal->selected_collection
        )
    )
    {
        terminal->restoration_notice = true;
    }
}

/*
 * Close collection details
 *
 * Also clears the temporary restoration-complete message.
 */

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

/*
 * Derive the compact status displayed beside a collection.
 *
 * Runtime state comes from the indexed world-state API, so this helper does
 * not need collection-specific branches.
 */

static const char *Floppy144TerminalCollectionStatusText(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    if(
        Floppy144WorldCollectionEvidenceFound(
            world,
            collection
        )
    )
    {
        return "EVIDENCE";
    }

    if(
        Floppy144WorldCollectionRestored(
            world,
            collection
        )
    )
    {
        return "RESTORED";
    }

    return "AVAILABLE";
}

/*
 * Determine whether a restored collection exposes catalogue records.
 *
 * Catalogue availability comes entirely from registered collection metadata.
 * Mandatory and optional collections follow the same rule.
 */

static bool Floppy144TerminalCollectionCanViewRecords(
    const Floppy144WorldState *world,
    Floppy144CollectionId collection
)
{
    const Floppy144CollectionDefinition *definition =
        Floppy144CollectionGet(collection);

    return
        definition->catalogue.record_count > 0U &&
        Floppy144WorldCollectionRestored(
            world,
            collection
        );
}

/*
 * Determine whether Enter should open the selected collection catalogue.
 */

bool Floppy144TerminalCanOpenCatalogue(
    const Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
)
{
    if(
        terminal == NULL ||
        world == NULL
    )
    {
        return false;
    }

    return
        Floppy144TerminalDetailOpen(
            terminal
        ) &&
        Floppy144TerminalCollectionCanViewRecords(
            world,
            terminal->selected_collection
        );
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

    /*
     * Dynamic list and footer text
     *
     * Restored collections display green. Evidence-bearing collections display
     * amber. Footer instructions change with the current navigation depth.
     */

    char site_status[16];

    snprintf(
        site_status,
        sizeof(site_status),
        "SITE %02u%%",
        (unsigned)Floppy144WorldReconstructionPercent(world)
    );

    bool selected_restored =
        Floppy144WorldCollectionRestored(
            world,
            terminal->selected_collection
        );

    bool selected_can_view_records =
        Floppy144TerminalCollectionCanViewRecords(
            world,
            terminal->selected_collection
        );

    const char *footer_left =
        terminal->detail_open
            ? ""
            : "LEFT RIGHT ACT";

    const char *footer_middle =
        terminal->detail_open
            ? !selected_restored
                ? "ENTER RESTORE"
                : selected_can_view_records
                    ? "ENTER VIEW RECORDS"
                    : ""
            : "UP DOWN SELECT";

    const char *footer_right =
        terminal->detail_open
            ? "ESC CLOSE"
            : "ENTER OPEN  ESC RETURN";

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

    /*
     * Draw the selected narrative act
     *
     * Collections remain in registry order. Only rows belonging to the
     * terminal's selected act are painted.
     */

    char act_heading[32];

    uint32_t collection_index;
    uint32_t collection_y =
        154U;

    snprintf(
        act_heading,
        sizeof(act_heading),
        "COLLECTIONS: %s",
        Floppy144CollectionActText(
            terminal->selected_act
        )
    );

    Floppy144DrawText(
        &surface,
        44,
        136,
        act_heading,
        1,
        muted
    );

    for(
        collection_index = 0U;
        collection_index <
            (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        Floppy144CollectionId collection =
            (Floppy144CollectionId)collection_index;

        const Floppy144CollectionDefinition *definition =
            Floppy144CollectionGet(
                collection
            );

        bool restored;
        bool evidence_found;
        uint32_t status_colour;

        if(
            definition->act !=
            terminal->selected_act
        )
        {
            continue;
        }

        restored =
            Floppy144WorldCollectionRestored(
                world,
                collection
            );

        evidence_found =
            Floppy144WorldCollectionEvidenceFound(
                world,
                collection
            );

        status_colour =
            evidence_found
                ? amber
                : restored
                    ? green
                    : amber;

        Floppy144TerminalDrawCollection(
            &surface,
            collection_y,
            terminal->selected_collection ==
                collection,
            Floppy144TerminalCollectionStatusText(
                world,
                collection
            ),
            definition->code,
            definition->title,
            status_colour
        );

        collection_y +=
            26U;
    }

    Floppy144DrawFillRect(
        &surface,
        40,
        300,
        560,
        1,
        border
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


