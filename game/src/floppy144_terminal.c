    /*
 * Floppy//144 - archive terminal implementation
 *
 * Renders the collection list and detail overlay, handles terminal-local
 * navigation, and writes collection restoration into the shared world state.
 */

#include "floppy144_terminal.h"
#include "floppy144_catalogue.h"
#include "floppy144_document.h"
#include "floppy144_recovery.h"

#include "floppy144_draw.h"
#include "floppy144_collection_registry.h"

#include <stdio.h>
#include <string.h>

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
    char collection_domain[32];

    snprintf(
        code,
        sizeof(code),
        "COLLECTION %s",
        definition->code
    );

    snprintf(
        collection_domain,
        sizeof(collection_domain),
             "DOMAIN: %s",
             Floppy144CollectionDomainText(
                 definition->domain
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
        54,
        62,
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
        62,
        70,
        496,
        32,
        panel
    );

    Floppy144DrawText(
        surface,
        74,
        81,
        code,
        1,
        text
    );

    Floppy144DrawText(
        surface,
        74,
        112,
        title,
        1,
        text
    );

    Floppy144DrawFillRect(
        surface,
        70,
        132,
        480,
        1,
        border
    );

    Floppy144DrawText(
        surface,
        74,
        148,
        status,
        1,
        status_colour
    );

    Floppy144DrawText(
        surface,
        74,
        168,
        collection_domain,
        1,
        muted
    );

    Floppy144DrawText(
        surface,
        74,
        198,
        description_one,
        1,
        text
    );

    Floppy144DrawText(
        surface,
        74,
        218,
        description_two,
        1,
        terminal->restoration_notice
            ? green
            : muted
    );

    Floppy144DrawFillRect(
        surface,
        70,
        242,
        480,
        1,
        border
    );


}

/*
 * Command-shell output
 *
 * The fixed line buffer avoids allocation and retains only the most recent
 * terminal output that can be displayed.
 */

static void Floppy144TerminalPushLine(
    Floppy144TerminalState *terminal,
    const char *text
)
{
    uint32_t line_index;

    if(
        terminal == NULL ||
        text == NULL
    )
    {
        return;
    }

    if(
        terminal->output_count >=
        FLOPPY144_TERMINAL_OUTPUT_LINES
    )
    {
        for(
            line_index = 1U;
            line_index <
                FLOPPY144_TERMINAL_OUTPUT_LINES;
            ++line_index
        )
        {
            snprintf(
                terminal->output[line_index - 1U],
                FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY,
                "%s",
                terminal->output[line_index]
            );
        }

        terminal->output_count =
            FLOPPY144_TERMINAL_OUTPUT_LINES - 1U;
    }

    snprintf(
        terminal->output[terminal->output_count],
        FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY,
        "%s",
        text
    );

    ++terminal->output_count;
}

static void Floppy144TerminalPushWrappedLine(
    Floppy144TerminalState *terminal,
    const char *text
);

/*
 * Print the next useful recovery action from current persistent state.
 *
 * Priority is deliberate:
 *   1. initialise archive services;
 *   2. open an eligible trigger document from a restored collection;
 *   3. leave the terminal once a Site room exists;
 *   4. restore the next available collection.
 *
 * No collection, document or trigger ID is embedded here. The canonical JSON
 * and generated registries decide which action satisfies each step.
 */
void Floppy144TerminalPrintNextAction(
    Floppy144TerminalState *pTerminal,
    const Floppy144RunState *pRunState
)
{
    uint32_t uCollectionIndex;
    uint32_t uRoomIndex;
    const char *pszAvailableCollectionCode = NULL;
    char szLine[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    if(pTerminal == NULL || pRunState == NULL)
    {
        return;
    }
}


/*
 * Reset the terminal as a physical Site terminal.
 *
 * The normal reset remains intentionally location-neutral for the first
 * recovery screen and reinstated sessions. A terminal reached from the Site
 * replaces only the first transcript line with its room-qualified identity;
 * the fixed top screen header remains "GDR ARCHIVE RECOVERY TERMINAL".
 */
static const char *Floppy144TerminalRoomName(
    Floppy144RoomId room
)
{
    switch(room)
    {
        case FLOPPY144_ROOM_RECEPTION:        return "RECEPTION";
        case FLOPPY144_ROOM_CORRIDOR:         return "CORRIDOR";
        case FLOPPY144_ROOM_MAIN_OFFICE:      return "MAIN OFFICE";
        case FLOPPY144_ROOM_FACILITIES:       return "FACILITIES";
        case FLOPPY144_ROOM_RECORDS_OFFICE:   return "RECORDS OFFICE";
        case FLOPPY144_ROOM_IT_SUPPORT:       return "IT SUPPORT";
        case FLOPPY144_ROOM_STAFF_ROOM:       return "STAFF ROOM";
        case FLOPPY144_ROOM_SECRETARY_OFFICE: return "SECRETARY OFFICE";
        case FLOPPY144_ROOM_DIRECTOR_OFFICE:  return "DIRECTOR OFFICE";
        case FLOPPY144_ROOM_SECURITY:         return "SECURITY";
        case FLOPPY144_ROOM_SERVER_ROOM:      return "SERVER ROOM";
        default:                              return NULL;
    }
}

void Floppy144TerminalResetAtRoom(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world,
    Floppy144RoomId room
)
{
    const char *room_name;
    char environment_line[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    Floppy144TerminalReset(
        terminal,
        world
    );

    if(
        terminal == NULL ||
        terminal->output_count == 0U
    )
    {
        return;
    }

    room_name =
        Floppy144TerminalRoomName(
            room
        );

    if(room_name == NULL)
    {
        return;
    }

    snprintf(
        environment_line,
        sizeof(environment_line),
        "GDR ARCHIVE RECOVERY ENVIRONMENT - %s TERMINAL",
        room_name
    );

    snprintf(
        terminal->output[0],
        FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY,
        "%s",
        environment_line
    );
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
            definition->domain ==
            terminal->selected_domain
        )
        {
            terminal->selected_collection =
                (Floppy144CollectionId)next_selection;

            return;
        }
    }
}

/*
 * Move to the next populated archive domain.
 */

void Floppy144TerminalMoveDomain(
    Floppy144TerminalState *terminal,
    int32_t direction
)
{
    int32_t next_domain;
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

    next_domain =
    (int32_t)terminal->selected_domain;

    for(
        attempts = 0U;
    attempts <
    (uint32_t)FLOPPY144_COLLECTION_DOMAIN_COUNT;
    ++attempts
    )
    {
        next_domain +=
        step;

        if(next_domain < 0)
        {
            next_domain =
            (int32_t)FLOPPY144_COLLECTION_DOMAIN_COUNT - 1;
        }

        if(
            next_domain >=
            (int32_t)FLOPPY144_COLLECTION_DOMAIN_COUNT
        )
        {
            next_domain =
            0;
        }

        if(
            Floppy144TerminalDomainHasCollections(
                (Floppy144CollectionDomain)next_domain
            )
        )
        {
            terminal->selected_domain =
            (Floppy144CollectionDomain)next_domain;

            terminal->selected_collection =
            Floppy144TerminalFirstCollectionInDomain(
                terminal->selected_domain
            );

            terminal->restoration_notice =
            false;

            return;
        }
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
    const Floppy144CollectionDefinition *definition;

    if(
        terminal == NULL ||
        world == NULL
    )
    {
        return false;
    }

    definition =
        Floppy144CollectionGet(
            terminal->selected_collection
        );

    return
        definition->catalogue.record_count > 0U &&
        Floppy144WorldCollectionRestored(
            world,
            terminal->selected_collection
        );
}

void Floppy144TerminalDraw(
    F144Runtime *runtime,
    const Floppy144TerminalState *terminal,
    const Floppy144RunState *run_state
)
{
    const uint32_t background =
        FLOPPY144_RGB(8, 13, 11);

    const uint32_t panel =
        FLOPPY144_RGB(13, 24, 19);

    const uint32_t border =
        FLOPPY144_RGB(55, 92, 72);

    const uint32_t text =
        FLOPPY144_RGB(127, 196, 146);

    const uint32_t muted =
        FLOPPY144_RGB(76, 119, 91);

    const uint32_t bright =
        FLOPPY144_RGB(172, 231, 183);

    Floppy144Surface surface =
    {
        (uint32_t *)runtime->backbuffer.data,
        runtime->backbuffer.width,
        runtime->backbuffer.height
    };

    char site_status[64];

    char prompt
        [FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    uint32_t line_index;
    uint32_t output_y =
        84U;

    Floppy144RecoveryFormatCapacity(run_state,site_status,(uint32_t)sizeof(site_status));
    if(
        terminal->record_pager_active ||
        terminal->help_pager_active
    )
    {
        snprintf(
            prompt,
            sizeof(prompt),
            "SPACE/ENTER NEXT  BACKSPACE PREVIOUS  Q RETURN"
        );
    }
    else
    {
        snprintf(prompt,sizeof(prompt),"A:\\GDR> %s",terminal->input);
    }

    Floppy144DrawClear(
        &surface,
        background
    );

    Floppy144DrawText(
        &surface,
        10,
        5,
        "GDR ARCHIVE RECOVERY TERMINAL",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        630U - Floppy144DrawTextWidth(site_status,1U),
        5,
        site_status,
        1,
        text
    );

    Floppy144DrawFillRect(
        &surface,
        10,
        18,
        620,
        280,
        panel
    );

    Floppy144DrawRect(
        &surface,
        10,
        18,
        620,
        280,
        border
    );

    Floppy144TerminalTextCentred(
        &surface,
        42,
        "GDR ARCHIVE RECOVERY TERMINAL",
        2,
        bright
    );

    Floppy144DrawFillRect(
        &surface,
        30,
        68,
        580,
        1,
        border
    );

    for(
        line_index = 0U;
        line_index <
            terminal->output_count;
        ++line_index
    )
    {
        Floppy144DrawText(
            &surface,
            34,
            output_y,
            terminal->output[line_index],
            1,
            text
        );

        output_y +=
            15U;
    }

    Floppy144DrawFillRect(
        &surface,
        30,
        260,
        560,
        1,
        border
    );

    if(!terminal->record_pager_active&&!terminal->help_pager_active&&!Floppy144RunStateArchiveServicesInitialised(run_state))
    {
        Floppy144DrawText(&surface,34U,268U,"TYPE INITIATE TO START THE RESTORATION PROCESS.",1U,muted);
        Floppy144DrawText(&surface,34U,284U,prompt,1U,bright);
    }
    else
    {
        Floppy144DrawText(&surface,34U,274U,prompt,1U,bright);
    }

    Floppy144DrawFillRect(
        &surface,
        34 +
            Floppy144DrawTextWidth(
                prompt,
                1
            ),
        (!terminal->record_pager_active&&!terminal->help_pager_active&&!Floppy144RunStateArchiveServicesInitialised(run_state))?284U:274U,
        5,
        8,
        bright
    );

    Floppy144DrawFillRect(
        &surface,
        10,
        306,
        620,
        28,
        background
    );

    Floppy144DrawRect(
        &surface,
        10,
        306,
        620,
        28,
        border
    );

    if(terminal->record_pager_active||terminal->help_pager_active)
    {
        Floppy144DrawText(&surface,22U,316U,"SPACE/ENTER NEXT   BACKSPACE PREVIOUS",1U,text);
        Floppy144DrawText(&surface,630U-Floppy144DrawTextWidth("Q RETURN",1U)-12U,316U,"Q RETURN",1U,muted);
    }
    else
    {
        Floppy144DrawText(&surface,22U,316U,"UP/DOWN HISTORY   BACKSPACE EDIT   ENTER SUBMIT",1U,text);
        Floppy144DrawText(&surface,630U-Floppy144DrawTextWidth("TYPE EXIT TO CLOSE",1U)-12U,316U,"TYPE EXIT TO CLOSE",1U,muted);
    }
}
