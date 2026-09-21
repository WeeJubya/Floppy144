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

    if(!Floppy144RunStateArchiveServicesInitialised(pRunState))
    {
        return;
    }

    for(
        uCollectionIndex = 0U;
        uCollectionIndex < (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++uCollectionIndex
    )
    {
        Floppy144CollectionId eCollection =
            (Floppy144CollectionId)uCollectionIndex;

        const Floppy144DocumentDefinition *pDocument =
            Floppy144DocumentFirstPendingTrigger(
                pRunState,
                eCollection
            );

        if(pDocument != NULL && pDocument->record_id_override != NULL)
        {
            const char *pszRecordId =
                pDocument->record_id_override;

            /*
             * Immediately after a collection is restored, allow the player
             * to use the collection-local RS-#### form. The full identifier
             * remains valid at all times.
             */
            if(pTerminal->default_record_collection_valid)
            {
                const Floppy144CollectionDefinition *pDefaultDefinition =
                    Floppy144CollectionGet(
                        pTerminal->default_record_collection
                    );

                size_t uCodeLength =
                    strlen(pDefaultDefinition->code);

                if(
                    strncmp(
                        pszRecordId,
                        pDefaultDefinition->code,
                        uCodeLength
                    ) == 0 &&
                    pszRecordId[uCodeLength] == '-'
                )
                {
                    pszRecordId +=
                        uCodeLength + 1U;
                }
            }

            snprintf(
                szLine,
                sizeof(szLine),
                "NEXT RECOVERY ACTION: OPEN %s",
                pszRecordId
            );

            Floppy144TerminalPushWrappedLine(
                pTerminal,
                szLine
            );

            return;
        }
    }

    /*
     * Remember the next restorable collection before considering the Site
     * hand-off. Once rooms exist, both actions are useful: the player may
     * explore immediately or continue archive restoration on a later visit.
     */
    for(
        uCollectionIndex = 0U;
        uCollectionIndex < (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++uCollectionIndex
    )
    {
        Floppy144CollectionId eCollection =
            (Floppy144CollectionId)uCollectionIndex;

        const Floppy144CollectionDefinition *pDefinition =
            Floppy144CollectionGet(eCollection);

        if(
            pDefinition != NULL &&
            Floppy144RunStateCollectionAvailable(
                pRunState,
                eCollection
            ) &&
            !Floppy144RunStateCollectionRestored(
                pRunState,
                eCollection
            )
        )
        {
            pszAvailableCollectionCode = pDefinition->code;
            break;
        }
    }

    for(
        uRoomIndex = 0U;
        uRoomIndex < (uint32_t)FLOPPY144_ROOM_COUNT;
        ++uRoomIndex
    )
    {
        if(
            Floppy144RunStateRoomReconstructed(
                pRunState,
                (Floppy144RoomId)uRoomIndex
            )
        )
        {
            if(pszAvailableCollectionCode != NULL)
            {
                snprintf(
                    szLine,
                    sizeof(szLine),
                    "NEXT: EXIT TO SITE / RESTORE %s",
                    pszAvailableCollectionCode
                );

                Floppy144TerminalPushLine(
                    pTerminal,
                    szLine
                );
            }
            else
            {
                Floppy144TerminalPushLine(
                    pTerminal,
                    "NEXT RECOVERY ACTION: EXIT TO SITE"
                );
            }

            return;
        }
    }

    if(pszAvailableCollectionCode != NULL)
    {
        snprintf(
            szLine,
            sizeof(szLine),
            "NEXT RECOVERY ACTION: RESTORE %s",
            pszAvailableCollectionCode
        );

        Floppy144TerminalPushLine(
            pTerminal,
            szLine
        );

        return;
    }

    Floppy144TerminalPushLine(
        pTerminal,
        "NO RECOVERY ACTION IS CURRENTLY AVAILABLE."
    );
}

/*
 * Push text into terminal history, wrapping it to the visible panel width.
 */

static void Floppy144TerminalPushWrappedLine(
    Floppy144TerminalState *terminal,
    const char *text
)
{
    const char *cursor;
    char line[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];
    uint32_t length;

    if(
        terminal == NULL ||
        text == NULL
    )
    {
        return;
    }

    if(text[0] == '\0')
    {
        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        return;
    }

    cursor =
        text;

    while(cursor[0] != '\0')
    {
        length =
            0U;

        line[0] =
            '\0';

        while(
            cursor[length] != '\0' &&
            length + 1U <
                FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY
        )
        {
            line[length] =
                cursor[length];

            line[length + 1U] =
                '\0';

            if(
                Floppy144DrawTextWidth(
                    line,
                    1U
                ) >
                540U
            )
            {
                line[length] =
                    '\0';

                break;
            }

            ++length;
        }

        if(length == 0U)
        {
            line[0] =
                cursor[0];

            line[1] =
                '\0';

            length =
                1U;
        }

        Floppy144TerminalPushLine(
            terminal,
            line
        );

        cursor +=
            length;
    }
}
/*
 * Command-line editing and history helpers.
 */

static void Floppy144TerminalSetInput(
    Floppy144TerminalState *terminal,
    const char *text
)
{
    if(terminal == NULL || text == NULL)
    {
        return;
    }

    snprintf(
        terminal->input,
        sizeof(terminal->input),
        "%s",
        text
    );

    terminal->input_length =
        (uint32_t)strlen(terminal->input);
}

static void Floppy144TerminalLeaveHistoryNavigation(
    Floppy144TerminalState *terminal
)
{
    if(terminal == NULL)
    {
        return;
    }

    terminal->history_cursor = -1;
    terminal->history_draft[0] = '\0';
}

/*
 * Canonicalise a submitted command. Leading/trailing whitespace disappears
 * and repeated interior spaces collapse to one. InputCharacter has already
 * converted alphabetic input to uppercase.
 */
static bool Floppy144TerminalNormaliseCommand(
    const char *input,
    char *output,
    size_t output_capacity
)
{
    size_t output_length = 0U;
    bool pending_space = false;

    if(
        input == NULL ||
        output == NULL ||
        output_capacity == 0U
    )
    {
        return false;
    }

    output[0] = '\0';

    while(*input != '\0')
    {
        if(*input == ' ' || *input == '\t')
        {
            if(output_length > 0U)
            {
                pending_space = true;
            }

            ++input;
            continue;
        }

        if(pending_space)
        {
            if(output_length + 1U >= output_capacity)
            {
                break;
            }

            output[output_length++] = ' ';
            pending_space = false;
        }

        if(output_length + 1U >= output_capacity)
        {
            break;
        }

        output[output_length++] = *input;
        ++input;
    }

    output[output_length] = '\0';
    return output_length > 0U;
}

/*
 * Keep only a compact in-memory command history. The newest command is at
 * history_count - 1. Blank commands never reach this helper and consecutive
 * duplicates are deliberately ignored.
 */
static void Floppy144TerminalStoreHistory(
    Floppy144TerminalState *terminal,
    const char *command
)
{
    if(
        terminal == NULL ||
        command == NULL ||
        command[0] == '\0'
    )
    {
        return;
    }

    if(
        terminal->history_count > 0U &&
        strcmp(
            terminal->history[terminal->history_count - 1U],
            command
        ) == 0
    )
    {
        Floppy144TerminalLeaveHistoryNavigation(terminal);
        return;
    }

    if(
        terminal->history_count >=
        FLOPPY144_TERMINAL_HISTORY_ENTRIES
    )
    {
        memmove(
            terminal->history[0],
            terminal->history[1],
            (FLOPPY144_TERMINAL_HISTORY_ENTRIES - 1U) *
                sizeof(terminal->history[0])
        );

        terminal->history_count =
            FLOPPY144_TERMINAL_HISTORY_ENTRIES - 1U;
    }

    snprintf(
        terminal->history[terminal->history_count],
        FLOPPY144_TERMINAL_INPUT_CAPACITY,
        "%s",
        command
    );

    ++terminal->history_count;
    Floppy144TerminalLeaveHistoryNavigation(terminal);
}

/*
 * Append one printable character to the command line.
 */

void Floppy144TerminalInputCharacter(
    Floppy144TerminalState *terminal,
    char character
)
{
    if(terminal == NULL)
    {
        return;
    }


    if(
        character >= 'a' &&
        character <= 'z'
    )
    {
        character =
            (char)(
                character -
                'a' +
                'A'
            );
    }

    if(
        character < 32 ||
        character > 126
    )
    {
        return;
    }

    if(
        terminal->input_length + 1U >=
        FLOPPY144_TERMINAL_INPUT_CAPACITY
    )
    {
        return;
    }

    Floppy144TerminalLeaveHistoryNavigation(
        terminal
    );

    terminal->input[terminal->input_length] =
        character;

    ++terminal->input_length;

    terminal->input[terminal->input_length] =
        '\0';
}

/*
 * Remove the final command-line character.
 */

void Floppy144TerminalBackspace(
    Floppy144TerminalState *terminal
)
{
    if(
        terminal == NULL ||
        terminal->input_length == 0U
    )
    {
        return;
    }

    Floppy144TerminalLeaveHistoryNavigation(
        terminal
    );

    --terminal->input_length;

    terminal->input[terminal->input_length] =
        '\0';
}

/*
 * Recall commands from the current terminal session.
 *
 * The first Up preserves the live draft. Down past the newest recalled entry
 * restores that draft. Navigation clamps at the oldest command rather than
 * wrapping, which avoids surprising edits on long command histories.
 */
void Floppy144TerminalMoveHistory(
    Floppy144TerminalState *terminal,
    int32_t direction
)
{
    if(
        terminal == NULL ||
        terminal->history_count == 0U ||
        direction == 0
    )
    {
        return;
    }

    if(direction < 0)
    {
        if(terminal->history_cursor < 0)
        {
            snprintf(
                terminal->history_draft,
                sizeof(terminal->history_draft),
                "%s",
                terminal->input
            );

            terminal->history_cursor =
                (int32_t)terminal->history_count - 1;
        }
        else if(terminal->history_cursor > 0)
        {
            --terminal->history_cursor;
        }

        Floppy144TerminalSetInput(
            terminal,
            terminal->history[terminal->history_cursor]
        );

        return;
    }

    if(terminal->history_cursor < 0)
    {
        return;
    }

    if(
        terminal->history_cursor + 1 <
        (int32_t)terminal->history_count
    )
    {
        ++terminal->history_cursor;

        Floppy144TerminalSetInput(
            terminal,
            terminal->history[terminal->history_cursor]
        );

        return;
    }

    terminal->history_cursor = -1;
    Floppy144TerminalSetInput(
        terminal,
        terminal->history_draft
    );
    terminal->history_draft[0] = '\0';
}

/*
 * Command processor
 *
 * Commands are matched without allocation. Input has already been converted
 * to uppercase, while leading and trailing spaces remain acceptable.
 */

static bool Floppy144TerminalCommandMatches(
    const char *input,
    const char *command
)
{
    while(input[0] == ' ')
    {
        ++input;
    }

    while(
        input[0] != '\0' &&
        command[0] != '\0' &&
        input[0] == command[0]
    )
    {
        ++input;
        ++command;
    }

    while(input[0] == ' ')
    {
        ++input;
    }

    return
        input[0] == '\0' &&
        command[0] == '\0';
}

/*
 * Return the text following a command name.
 *
 * NULL means the command name did not match. An empty returned string means
 * the command matched but no argument was supplied.
 */

static const char *Floppy144TerminalCommandArguments(
    const char *input,
    const char *command
)
{
    while(input[0] == ' ')
    {
        ++input;
    }

    while(
        input[0] != '\0' &&
        command[0] != '\0' &&
        input[0] == command[0]
    )
    {
        ++input;
        ++command;
    }

    if(command[0] != '\0')
    {
        return NULL;
    }

    if(
        input[0] != '\0' &&
        input[0] != ' '
    )
    {
        return NULL;
    }

    while(input[0] == ' ')
    {
        ++input;
    }

    return input;
}

/*
 * Resolve a player-facing collection code through the registry.
 */

static bool Floppy144TerminalFindCollection(
    const char *code,
    Floppy144CollectionId *collection
)
{
    uint32_t collection_index;

    if(
        code == NULL ||
        collection == NULL
    )
    {
        return false;
    }

    for(
        collection_index = 0U;
        collection_index <
            (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        Floppy144CollectionId candidate =
            (Floppy144CollectionId)collection_index;

        const Floppy144CollectionDefinition *definition =
            Floppy144CollectionGet(
                candidate
            );

        if(
            Floppy144TerminalCommandMatches(
                code,
                definition->code
            )
        )
        {
            *collection =
                candidate;

            return true;
        }
    }

    return false;
}

/*
 * Record retrieval becomes available once at least one restored collection
 * exposes catalogue records. In the Prologue this is DR-01, but deriving the
 * capability from registered data keeps the terminal reusable.
 */
static bool Floppy144TerminalRecordAccessAvailable(
    const Floppy144WorldState *world
)
{
    uint32_t collection_index;

    if(world == NULL)
    {
        return false;
    }

    for(
        collection_index = 0U;
        collection_index < (uint32_t)FLOPPY144_COLLECTION_COUNT;
        ++collection_index
    )
    {
        Floppy144CollectionId collection =
            (Floppy144CollectionId)collection_index;

        const Floppy144CollectionDefinition *definition =
            Floppy144CollectionGet(collection);

        if(
            definition != NULL &&
            definition->catalogue.record_count > 0U &&
            Floppy144WorldCollectionRestored(
                world,
                collection
            )
        )
        {
            return true;
        }
    }

    return false;
}

#define FLOPPY144_TERMINAL_HELP_PAGE_COUNT 3U

/*
 * Built-in GDR operating guidance.
 *
 * This is Terminal software documentation, not recovered archive material.
 * It therefore consumes no reconstruction capacity and requires no restored
 * collection.
 */

static const char *const floppy144_terminal_help_page_1[] =
{
    "TERMINAL OPERATION",
    "",
    "ENTER COMMANDS AT THE A:\\GDR> PROMPT.",
    "BACKSPACE EDITS THE CURRENT ENTRY.",
    "UP/DOWN RECALL SESSION COMMANDS.",
    "ENTER SUBMITS THE COMMAND.",
    "ESC OPENS GDR SESSION CONTROL.",
    "EXIT CLOSES THE TERMINAL SESSION.",
    NULL
};

static const char *const floppy144_terminal_help_page_2[] =
{
    "RESTORING COLLECTIONS",
    "",
    "LIST DISPLAYS COLLECTION STATUS.",
    "RESTORE <CODE> RECOVERS ONE COLLECTION.",
    "EXAMPLE: RESTORE DR-01",
    "RESTORATION USES DISK RECOVERY CAPACITY.",
    "MAXIMUM AVAILABLE PER RECOVERY: 1440 KB.",
    NULL
};

static const char *const floppy144_terminal_help_page_3_locked[] =
{
    "RECORD ACCESS",
    "",
    "RECORD RETRIEVAL IS NOT YET AVAILABLE.",
    "RESTORE A COLLECTION TO ENABLE OPEN.",
    "SPACE OR ENTER: NEXT PAGE.",
    "BACKSPACE: PREVIOUS PAGE.",
    "Q: RETURN TO COMMAND PROMPT.",
    NULL
};

static const char *const floppy144_terminal_help_page_3[] =
{
    "RECORD ACCESS",
    "",
    "LIST <CODE> OPENS A RESTORED RECORD INDEX.",
    "OPEN <RECORD-ID> RETRIEVES ONE RECORD.",
    "AFTER RESTORE, RS-#### USES THAT COLLECTION.",
    "SPACE OR ENTER: NEXT PAGE.",
    "BACKSPACE: PREVIOUS PAGE.  Q: RETURN.",
    NULL
};

static void Floppy144TerminalPrintHelpPage(
    Floppy144TerminalState *terminal
)
{
    const char *const *lines;
    char line[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];
    uint32_t line_index;

    if(
        terminal == NULL ||
        !terminal->help_pager_active
    )
    {
        return;
    }

    switch(terminal->help_pager_page)
    {
        case 1U:
        {
            lines =
            floppy144_terminal_help_page_1;

            break;
        }

        case 2U:
        {
            lines =
            floppy144_terminal_help_page_2;

            break;
        }

        case 3U:
        default:
        {
            lines =
                terminal->open_command_available
                    ? floppy144_terminal_help_page_3
                    : floppy144_terminal_help_page_3_locked;

            break;
        }
    }

    terminal->output_count =
    0U;

    Floppy144TerminalPushLine(
        terminal,
        "GDR ARCHIVE RECOVERY SYSTEM"
    );

    Floppy144TerminalPushLine(
        terminal,
        "OPERATOR HELP"
    );

    snprintf(
        line,
        sizeof(line),
             "PAGE %u OF %u",
             (unsigned)terminal->help_pager_page,
             (unsigned)FLOPPY144_TERMINAL_HELP_PAGE_COUNT
    );

    Floppy144TerminalPushLine(
        terminal,
        line
    );

    Floppy144TerminalPushLine(
        terminal,
        ""
    );

    for(
        line_index = 0U;
    lines[line_index] != NULL;
    ++line_index
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            lines[line_index]
        );
    }
}

static void Floppy144TerminalOpenHelpPager(
    Floppy144TerminalState *terminal
)
{
    if(terminal == NULL)
    {
        return;
    }

    terminal->record_pager_active =
    false;

    terminal->help_pager_active =
    true;

    terminal->help_pager_page =
    1U;

    Floppy144TerminalPrintHelpPage(
        terminal
    );
}

static void Floppy144TerminalPrintHelp(
    Floppy144TerminalState *terminal,
    bool services_initialised,
    const char *topic
)
{
    Floppy144TerminalPushLine(
        terminal,
        ""
    );

    if(
        topic == NULL ||
        topic[0] == '\0'
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "AVAILABLE COMMANDS:"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "  HELP [COMMAND]"
        );

        if(services_initialised)
        {
            Floppy144TerminalPushLine(
                terminal,
                "  RESTORE <CODE>"
            );

            Floppy144TerminalPushLine(
                terminal,
                "  LIST [CODE]"
            );

            if(terminal->open_command_available)
            {
                Floppy144TerminalPushLine(
                    terminal,
                    "  OPEN <RECORD>"
                );
            }
        }
        else
        {
            Floppy144TerminalPushLine(
                terminal,
                "  INITIATE"
            );
        }

        Floppy144TerminalPushLine(
            terminal,
            "  EXIT"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "TYPE HELP <COMMAND> FOR DETAILED GUIDANCE."
        );

        return;
    }

    if(
        Floppy144TerminalCommandMatches(
            topic,
            "HELP"
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "HELP HELP"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "HELP"
        );

        Floppy144TerminalPushLine(
            terminal,
            "DISPLAY THE AVAILABLE COMMAND SUMMARY."
        );

        Floppy144TerminalPushLine(
            terminal,
            "HELP <COMMAND>"
        );

        Floppy144TerminalPushLine(
            terminal,
            "DISPLAY GUIDANCE FOR AN AVAILABLE COMMAND."
        );

        if(!services_initialised)
        {
            Floppy144TerminalPushLine(
                terminal,
                "TOPICS: INITIATE, EXIT."
            );
        }
        else if(terminal->open_command_available)
        {
            Floppy144TerminalPushLine(
                terminal,
                "TOPICS: RESTORE, LIST, OPEN, EXIT."
            );
        }
        else
        {
            Floppy144TerminalPushLine(
                terminal,
                "TOPICS: RESTORE, LIST, EXIT."
            );
        }

        return;
    }

    if(
        Floppy144TerminalCommandMatches(
            topic,
            "EXIT"
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "HELP EXIT"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "EXIT FROM THE TERMINAL SESSION."
        );

        return;
    }

    if(
        !services_initialised &&
        Floppy144TerminalCommandMatches(
            topic,
            "INITIATE"
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "HELP INITIATE"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "INITIATE"
        );

        Floppy144TerminalPushLine(
            terminal,
            "INITIALISE ARCHIVE RECOVERY SERVICES."
        );

        Floppy144TerminalPushLine(
            terminal,
            "AVAILABLE ONLY BEFORE SERVICES ARE ONLINE."
        );

        return;
    }

    if(
        services_initialised &&
        Floppy144TerminalCommandMatches(
            topic,
            "RESTORE"
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "HELP RESTORE"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "RESTORE <CODE>"
        );

        Floppy144TerminalPushLine(
            terminal,
            "RESTORE ONE COLLECTION FROM DISK 144."
        );

        Floppy144TerminalPushLine(
            terminal,
            "EXAMPLE: RESTORE HR-01"
        );

        return;
    }

    if(
        services_initialised &&
        Floppy144TerminalCommandMatches(
            topic,
            "LIST"
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "HELP LIST"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "LIST"
        );

        Floppy144TerminalPushLine(
            terminal,
            "DISPLAY COLLECTION STATUS."
        );

        Floppy144TerminalPushLine(
            terminal,
            "LIST <CODE>"
        );

        Floppy144TerminalPushLine(
            terminal,
            "OPEN THE INTERACTIVE RECORD INDEX."
        );

        Floppy144TerminalPushLine(
            terminal,
            "SPACE OR ENTER: NEXT PAGE."
        );

        Floppy144TerminalPushLine(
            terminal,
            "BACKSPACE: PREVIOUS PAGE."
        );

        Floppy144TerminalPushLine(
            terminal,
            "Q: RETURN TO COMMAND PROMPT."
        );

        Floppy144TerminalPushLine(
            terminal,
            "ONLY RESTORED COLLECTIONS MAY BE SEARCHED."
        );

        return;
    }

    if(
        services_initialised &&
        terminal->open_command_available &&
        Floppy144TerminalCommandMatches(
            topic,
            "OPEN"
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "HELP OPEN"
        );

        Floppy144TerminalPushLine(
            terminal,
            ""
        );

        Floppy144TerminalPushLine(
            terminal,
            "OPEN <RECORD>"
        );

        Floppy144TerminalPushLine(
            terminal,
            "RETRIEVE ONE RECORD BY ID."
        );

        Floppy144TerminalPushLine(
            terminal,
            "FULL IDS ARE ALWAYS ACCEPTED."
        );

        Floppy144TerminalPushLine(
            terminal,
            "AFTER RESTORE: OPEN RS-0001 USES THAT COLLECTION."
        );

        Floppy144TerminalPushLine(
            terminal,
            "ONLY RESTORED COLLECTIONS MAY BE SEARCHED."
        );

        Floppy144TerminalPushLine(
            terminal,
            "BACKSPACE RETURNS FROM RECORD VIEW."
        );

        return;
    }

    Floppy144TerminalPushLine(
        terminal,
        "NO GUIDANCE AVAILABLE IN CURRENT RECOVERY STATE."
    );

    Floppy144TerminalPushLine(
        terminal,
        "TYPE HELP TO DISPLAY AVAILABLE COMMANDS."
    );
}

/*
 * List every collection currently registered on Disk 144.
 */

#define FLOPPY144_TERMINAL_COLLECTIONS_PER_PAGE 10U

static uint32_t Floppy144TerminalObfuscationHash(
    uint32_t uSeed,
    uint32_t uOrdinal
)
{
    uint32_t uValue = uSeed ^ (0x9E3779B9U * (uOrdinal + 1U));
    uValue ^= uValue >> 16U;
    uValue *= 0x7FEB352DU;
    uValue ^= uValue >> 15U;
    uValue *= 0x846CA68BU;
    uValue ^= uValue >> 16U;
    return uValue;
}

static void Floppy144TerminalPseudoCollectionIdentity(
    uint32_t uSeed,
    uint32_t uOrdinal,
    char *pszCode,
    uint32_t uCodeCapacity,
    char *pszName,
    uint32_t uNameCapacity
)
{
    uint32_t uHashA =
        Floppy144TerminalObfuscationHash(uSeed, uOrdinal);

    uint32_t uHashB =
        Floppy144TerminalObfuscationHash(
            uSeed ^ 0xA5C31F27U,
            uOrdinal + 37U
        );

    /*
     * N/A rows must look like damaged index data, not plausible collection
     * names. Keep them deterministic per recovery while revealing no semantic
     * hints such as "BLANK PACKET" or a fixed synthetic UX namespace.
     */
    if(pszCode != NULL && uCodeCapacity > 0U)
    {
        (void)snprintf(
            pszCode,
            uCodeCapacity,
            "%c%c-%04X",
            (char)('A' + (uHashA % 26U)),
            (char)('A' + ((uHashA >> 5U) % 26U)),
            (unsigned)(uHashB & 0xFFFFU)
        );
    }

    if(pszName != NULL && uNameCapacity > 0U)
    {
        (void)snprintf(
            pszName,
            uNameCapacity,
            "%04X %04X %04X",
            (unsigned)(uHashA & 0xFFFFU),
            (unsigned)((uHashA >> 16U) & 0xFFFFU),
            (unsigned)(uHashB & 0xFFFFU)
        );
    }
}

static uint32_t Floppy144TerminalCollectionPageCount(void)
{
    return
        (
            (uint32_t)FLOPPY144_COLLECTION_COUNT +
            FLOPPY144_TERMINAL_COLLECTIONS_PER_PAGE -
            1U
        ) /
        FLOPPY144_TERMINAL_COLLECTIONS_PER_PAGE;
}

static void Floppy144TerminalPrintCollectionPage(
    Floppy144TerminalState *pTerminal
)
{
    const Floppy144RunState *pRunState;
    uint32_t uPageCount;
    uint32_t uFirst;
    uint32_t uFinal;
    uint32_t uIndex;
    char szLine[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    if(
        pTerminal == NULL ||
        !pTerminal->record_pager_active ||
        pTerminal->record_pager_collection != FLOPPY144_COLLECTION_COUNT ||
        pTerminal->collection_pager_state == NULL
    )
    {
        return;
    }

    pRunState = pTerminal->collection_pager_state;
    uPageCount = Floppy144TerminalCollectionPageCount();

    if(pTerminal->record_pager_page < 1U)
        pTerminal->record_pager_page = 1U;
    if(pTerminal->record_pager_page > uPageCount)
        pTerminal->record_pager_page = uPageCount;

    uFirst =
        (pTerminal->record_pager_page - 1U) *
        FLOPPY144_TERMINAL_COLLECTIONS_PER_PAGE;

    uFinal = uFirst + FLOPPY144_TERMINAL_COLLECTIONS_PER_PAGE;
    if(uFinal > (uint32_t)FLOPPY144_COLLECTION_COUNT)
        uFinal = (uint32_t)FLOPPY144_COLLECTION_COUNT;

    pTerminal->output_count = 0U;

    (void)snprintf(
        szLine,
        sizeof(szLine),
        "%-13s | %-27s | %-9s | %s",
        "COLLECTION ID",
        "COLLECTION NAME",
        "STATUS",
        "SIZE"
    );
    Floppy144TerminalPushLine(pTerminal, szLine);

    (void)snprintf(
        szLine,
        sizeof(szLine),
        "PAGE %u OF %u   Q: EXIT",
        (unsigned)pTerminal->record_pager_page,
        (unsigned)uPageCount
    );
    Floppy144TerminalPushLine(pTerminal, szLine);

    for(uIndex = uFirst; uIndex < uFinal; ++uIndex)
    {
        Floppy144CollectionId eCollection = (Floppy144CollectionId)uIndex;
        const Floppy144CollectionDefinition *pDefinition =
            Floppy144CollectionGet(eCollection);
        bool bRestored =
            Floppy144RunStateCollectionRestored(pRunState, eCollection);
        bool bAvailable =
            Floppy144RunStateCollectionAvailable(pRunState, eCollection);
        const char *pszStatus =
            bRestored ? "RESTORED" : (bAvailable ? "AVAILABLE" : "N/A");
        char szPseudoCode[16];
        char szPseudoName[40];
        const char *pszCode = pDefinition->code;
        const char *pszName = pDefinition->title;

        if(!bRestored && !bAvailable)
        {
            Floppy144TerminalPseudoCollectionIdentity(
                pRunState->recovery_seed,
                uIndex,
                szPseudoCode,
                (uint32_t)sizeof(szPseudoCode),
                szPseudoName,
                (uint32_t)sizeof(szPseudoName)
            );
            pszCode = szPseudoCode;
            pszName = szPseudoName;
        }

        (void)snprintf(
            szLine,
            sizeof(szLine),
            "%-13s | %-27.27s | %-9s | %4u KB",
            pszCode,
            pszName,
            pszStatus,
            (unsigned)pDefinition->size_kb
        );
        Floppy144TerminalPushLine(pTerminal, szLine);
    }
}

static void Floppy144TerminalPrintCollections(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world,
    const Floppy144RunState *run_state
)
{
    (void)world;

    if(terminal == NULL || run_state == NULL)
        return;

    if(
        !Floppy144RunStateCollectionRestored(
            run_state,
            FLOPPY144_COLLECTION_DR01
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "FULL COLLECTION INDEX UNAVAILABLE. RESTORE DR-01."
        );
        return;
    }

    terminal->record_pager_active = true;
    terminal->record_pager_collection = FLOPPY144_COLLECTION_COUNT;
    terminal->collection_pager_state = run_state;
    terminal->record_pager_page = 1U;
    Floppy144TerminalPrintCollectionPage(terminal);
}

/*
 * Restore one collection identified by its registered code.
 */

#define FLOPPY144_TERMINAL_RECORDS_PER_PAGE 10U

/*
 * Parse LIST arguments into a collection code and optional page number.
 *
 * LIST HR-01 defaults to page one.
 * LIST HR-01 3 requests page three.
 */

static bool Floppy144TerminalParseListRequest(
    const char *arguments,
    char *code,
    uint32_t code_capacity,
    uint32_t *page
)
{
    uint32_t code_length =
        0U;

    uint32_t page_value =
        0U;

    if(
        arguments == NULL ||
        code == NULL ||
        code_capacity == 0U ||
        page == NULL
    )
    {
        return false;
    }

    while(arguments[0] == ' ')
    {
        ++arguments;
    }

    while(
        arguments[0] != '\0' &&
        arguments[0] != ' '
    )
    {
        if(code_length + 1U >= code_capacity)
        {
            return false;
        }

        code[code_length] =
            arguments[0];

        ++code_length;
        ++arguments;
    }

    if(code_length == 0U)
    {
        return false;
    }

    code[code_length] =
        '\0';

    while(arguments[0] == ' ')
    {
        ++arguments;
    }

    if(arguments[0] == '\0')
    {
        *page =
            1U;

        return true;
    }

    while(
        arguments[0] >= '0' &&
        arguments[0] <= '9'
    )
    {
        page_value =
            page_value * 10U +
            (uint32_t)(arguments[0] - '0');

        if(page_value > 999U)
        {
            return false;
        }

        ++arguments;
    }

    while(arguments[0] == ' ')
    {
        ++arguments;
    }

    if(
        arguments[0] != '\0' ||
        page_value == 0U
    )
    {
        return false;
    }

    *page =
        page_value;

    return true;
}

/*
 * Print one page of records belonging to a restored collection.
 */

static uint32_t Floppy144TerminalRecordPageCount(
    const Floppy144CollectionDefinition *definition
)
{
    if(
        definition == NULL ||
        definition->catalogue.record_count == 0U
    )
    {
        return 0U;
    }

    return
        (
            definition->catalogue.record_count +
            FLOPPY144_TERMINAL_RECORDS_PER_PAGE -
            1U
        ) /
        FLOPPY144_TERMINAL_RECORDS_PER_PAGE;
}

/*
 * Replace terminal history with the active record-index page.
 */

static void Floppy144TerminalPrintRecordPage(
    Floppy144TerminalState *terminal
)
{
    const Floppy144CollectionDefinition *definition;

    char line[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];
    char record_id[24];
    char title[48];

    uint32_t page_count;
    uint32_t first_index;
    uint32_t final_index;
    uint32_t record_index;

    if(
        terminal == NULL ||
        !terminal->record_pager_active
    )
    {
        return;
    }

    if(terminal->record_pager_collection == FLOPPY144_COLLECTION_COUNT)
    {
        Floppy144TerminalPrintCollectionPage(terminal);
        return;
    }

    definition =
        Floppy144CollectionGet(
            terminal->record_pager_collection
        );

    page_count =
        Floppy144TerminalRecordPageCount(
            definition
        );

    if(page_count == 0U)
    {
        return;
    }

    first_index =
        (
            terminal->record_pager_page -
            1U
        ) *
        FLOPPY144_TERMINAL_RECORDS_PER_PAGE;

    final_index =
        first_index +
        FLOPPY144_TERMINAL_RECORDS_PER_PAGE;

    if(final_index > definition->catalogue.record_count)
    {
        final_index =
            definition->catalogue.record_count;
    }

    terminal->output_count =
        0U;

    snprintf(
        line,
        sizeof(line),
        "COLLECTION %s: %s",
        definition->code,
        definition->title
    );

    Floppy144TerminalPushWrappedLine(
        terminal,
        line
    );

    snprintf(
        line,
        sizeof(line),
        "PAGE %u OF %u",
        (unsigned)terminal->record_pager_page,
        (unsigned)page_count
    );

    Floppy144TerminalPushLine(
        terminal,
        line
    );

    Floppy144TerminalPushLine(
        terminal,
        ""
    );

    for(
        record_index = first_index;
        record_index < final_index;
        ++record_index
    )
    {
        Floppy144CatalogueBuildRecord(
            terminal->record_pager_collection,
            record_index,
            record_id,
            sizeof(record_id),
            title,
            sizeof(title)
        );

        snprintf(
            line,
            sizeof(line),
            "%s  %s",
            record_id,
            title
        );

        Floppy144TerminalPushWrappedLine(
            terminal,
            line
        );
    }
}

/*
 * Validate a LIST request and open the interactive record pager.
 *
 * A supplied page number remains supported, but ordinary use begins at page
 * one and continues through Space and Backspace.
 */

static void Floppy144TerminalPrintCollectionRecords(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world,
    const char *arguments
)
{
    Floppy144CollectionId collection;

    const Floppy144CollectionDefinition *definition;

    char code[16];
    char line[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    uint32_t page;
    uint32_t page_count;

    if(
        !Floppy144TerminalParseListRequest(
            arguments,
            code,
            sizeof(code),
            &page
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "USE LIST <CODE>."
        );

        return;
    }

    if(
        !Floppy144TerminalFindCollection(
            code,
            &collection
        )
    )
    {
        snprintf(
            line,
            sizeof(line),
            "COLLECTION %s NOT FOUND ON DISK 144.",
            code
        );

        Floppy144TerminalPushWrappedLine(
            terminal,
            line
        );

        return;
    }

    definition =
        Floppy144CollectionGet(
            collection
        );

    if(
        !Floppy144WorldCollectionRestored(
            world,
            collection
        )
    )
    {
        snprintf(
            line,
            sizeof(line),
            "COLLECTION %s HAS NOT BEEN RESTORED.",
            definition->code
        );

        Floppy144TerminalPushLine(
            terminal,
            line
        );

        return;
    }

    page_count =
        Floppy144TerminalRecordPageCount(
            definition
        );

    if(page_count == 0U)
    {
        snprintf(
            line,
            sizeof(line),
            "COLLECTION %s HAS NO RECORD INDEX.",
            definition->code
        );

        Floppy144TerminalPushLine(
            terminal,
            line
        );

        return;
    }

    if(page > page_count)
    {
        snprintf(
            line,
            sizeof(line),
            "PAGE %u OUT OF RANGE. VALID RANGE: 1 TO %u.",
            (unsigned)page,
            (unsigned)page_count
        );

        Floppy144TerminalPushLine(
            terminal,
            line
        );

        return;
    }

    terminal->record_pager_active =
        true;

    terminal->record_pager_collection =
        collection;

    terminal->record_pager_page =
        page;

    Floppy144TerminalPrintRecordPage(
        terminal
    );
}

/*
 * Record-pager state and navigation
 */

bool Floppy144TerminalRecordPagerActive(
    const Floppy144TerminalState *terminal
)
{
    return
        terminal != NULL &&
        terminal->record_pager_active;
}

void Floppy144TerminalMoveRecordPager(
    Floppy144TerminalState *terminal,
    int32_t direction
)
{
    const Floppy144CollectionDefinition *definition = NULL;
    uint32_t page_count;
    int32_t next_page;

    if(
        terminal == NULL ||
        !terminal->record_pager_active ||
        direction == 0
    )
    {
        return;
    }

    if(terminal->record_pager_collection == FLOPPY144_COLLECTION_COUNT)
    {
        page_count = Floppy144TerminalCollectionPageCount();
    }
    else
    {
        definition =
            Floppy144CollectionGet(
                terminal->record_pager_collection
            );
        page_count = Floppy144TerminalRecordPageCount(definition);
    }

    if(page_count == 0U)
        return;

    next_page = (int32_t)terminal->record_pager_page + direction;
    if(next_page < 1)
        next_page = 1;
    if(next_page > (int32_t)page_count)
        next_page = (int32_t)page_count;

    if((uint32_t)next_page == terminal->record_pager_page)
        return;

    terminal->record_pager_page = (uint32_t)next_page;
    Floppy144TerminalPrintRecordPage(terminal);
}

void Floppy144TerminalCloseRecordPager(
    Floppy144TerminalState *terminal
)
{
    if(terminal == NULL)
    {
        return;
    }

    terminal->record_pager_active =
        false;
    terminal->collection_pager_state = NULL;
}

bool Floppy144TerminalHelpPagerActive(
    const Floppy144TerminalState *terminal
)
{
    return
    terminal != NULL &&
    terminal->help_pager_active;
}

void Floppy144TerminalMoveHelpPager(
    Floppy144TerminalState *terminal,
    int32_t direction
)
{
    int32_t next_page;

    if(
        terminal == NULL ||
        !terminal->help_pager_active ||
        direction == 0
    )
    {
        return;
    }

    next_page =
        (int32_t)terminal->help_pager_page +
        direction;

    if(next_page < 1)
    {
        next_page =
            (int32_t)FLOPPY144_TERMINAL_HELP_PAGE_COUNT;
    }

    if(
        next_page >
        (int32_t)FLOPPY144_TERMINAL_HELP_PAGE_COUNT
    )
    {
        next_page =
            1;
    }

    terminal->help_pager_page =
        (uint32_t)next_page;

    Floppy144TerminalPrintHelpPage(
        terminal
    );
}

void Floppy144TerminalCloseHelpPager(
    Floppy144TerminalState *terminal
)
{
    if(terminal == NULL)
    {
        return;
    }

    terminal->help_pager_active =
    false;
}

static void Floppy144TerminalRestoreCollection(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world,
    Floppy144RunState *run_state,
    const char *code
)
{
    Floppy144CollectionId collection;

    const Floppy144CollectionDefinition *definition;

    char line
        [FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    if(
        !Floppy144TerminalFindCollection(
            code,
            &collection
        )
    )
    {
        snprintf(
            line,
            sizeof(line),
            "COLLECTION %s NOT FOUND ON DISK 144.",
            code
        );

        Floppy144TerminalPushWrappedLine(
            terminal,
            line
        );

        return;
    }

    definition =
        Floppy144CollectionGet(
            collection
        );

        if(
            !Floppy144RunStateCollectionAvailable(
                run_state,
                collection
            )
        )
        {
            Floppy144TerminalPushLine(
                terminal,
                "COLLECTION NOT YET AVAILABLE FOR RECOVERY."
            );

            return;
        }

    if(
        Floppy144WorldCollectionRestored(
            world,
            collection
        )
    )
    {
        snprintf(
            line,
            sizeof(line),
            "COLLECTION %s ALREADY RESTORED.",
            definition->code
        );

        Floppy144TerminalPushLine(
            terminal,
            line
        );

        return;
    }

    if(
        !Floppy144RunStateCanRestoreCollection(
            run_state,
            collection
        )
    )
    {
        Floppy144TerminalPushLine(terminal, "");
        Floppy144TerminalPushLine(terminal, "RESTORE REFUSED.");
        Floppy144TerminalPushLine(
            terminal,
            "INSUFFICIENT RECOVERY CAPACITY."
        );

        (void)snprintf(
            line,
            sizeof(line),
            "FREE %u KB  COLLECTION REQUIRES %u KB",
            (unsigned)Floppy144RunStateFreeKb(run_state),
            (unsigned)definition->size_kb
        );
        Floppy144TerminalPushLine(terminal, line);
        return;
    }

    snprintf(
        line,
        sizeof(line),
        "RESTORING COLLECTION %s...",
        definition->code
    );

    Floppy144TerminalPushLine(
        terminal,
        ""
    );

    Floppy144TerminalPushLine(
        terminal,
        line
    );

    if(
        !Floppy144RunStateRestoreCollection(
            run_state,
            collection
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "COLLECTION RESTORATION FAILED."
        );

        return;
    }

    /*
     * RunState is authoritative. WorldState mirrors a restoration only after the
     * authoritative state has accepted it.
     */
    Floppy144WorldRestoreCollection(
        world,
        collection
    );

    /*
     * Record shorthand is session-local. The most recently restored
     * collection becomes the implicit prefix for OPEN RS-####.
     */
    terminal->default_record_collection =
        collection;

    terminal->default_record_collection_valid =
        true;

    Floppy144TerminalPushLine(
        terminal,
        "COLLECTION RESTORED."
    );

    /*
     * OPEN is intentionally withheld until a restored collection actually
     * exposes catalogue records. DR-01 is the first such collection in the
     * Prologue, but the rule itself remains data-driven.
     */
    if(
        !terminal->open_command_available &&
        definition->catalogue.record_count > 0U
    )
    {
        terminal->open_command_available =
            true;

        Floppy144TerminalPushLine(
            terminal,
            "NEW COMMAND: OPEN <RECORD>"
        );
    }

    Floppy144RecoveryFormatCapacity(
        run_state,
        line,
        (uint32_t)sizeof(line)
    );
    Floppy144TerminalPushLine(terminal, line);

    Floppy144TerminalPrintNextAction(
        terminal,
        run_state
    );
}

/*
 * Validate an OPEN request and pass the resolved catalogue position to main.
 */

static void Floppy144TerminalRequestOpenRecord(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world,
    const Floppy144RunState *run_state,
    const char *record_id
)
{
    Floppy144CollectionId collection;
    uint32_t record_index;

    const Floppy144CollectionDefinition *definition;
    const char *resolved_record_id =
        record_id;

    char expanded_record_id[32];
    char canonical_record_id[24];
    char title[48];
    char line[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    /*
     * A collection restored during this terminal session becomes the default
     * namespace for short record IDs such as RS-0001. Full record IDs remain
     * valid and bypass this expansion.
     */
    if(
        record_id != NULL &&
        record_id[0] == 'R' &&
        record_id[1] == 'S' &&
        record_id[2] == '-'
    )
    {
        if(!terminal->default_record_collection_valid)
        {
            Floppy144TerminalPushLine(
                terminal,
                "NO DEFAULT COLLECTION. USE THE FULL RECORD ID."
            );

            return;
        }

        definition =
            Floppy144CollectionGet(
                terminal->default_record_collection
            );

        snprintf(
            expanded_record_id,
            sizeof(expanded_record_id),
            "%s-%s",
            definition->code,
            record_id
        );

        resolved_record_id =
            expanded_record_id;
    }

    /*
     * Resolve through the procedural catalogue first. Later collections may
     * still have a zero-row catalogue stub while already owning fully authored
     * trigger documents. Those stable document IDs remain valid OPEN targets,
     * so fall back to the authored-document registry before reporting failure.
     */
    if(
        !Floppy144CatalogueFindRecord(
            resolved_record_id,
            &collection,
            &record_index
        ) &&
        !Floppy144DocumentFindRecordId(
            resolved_record_id,
            &collection,
            &record_index
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "RECORD ID NOT FOUND ON DISK 144."
        );

        return;
    }

    definition =
        Floppy144CollectionGet(
            collection
        );

    if(
        !Floppy144WorldCollectionRestored(
            world,
            collection
        )
    )
    {
        snprintf(
            line,
            sizeof(line),
            "COLLECTION %s HAS NOT BEEN RESTORED.",
            definition->code
        );

        Floppy144TerminalPushLine(
            terminal,
            line
        );

        return;
    }

    /*
     * A restored collection may contain an authored branch-choice document
     * whose trigger is temporarily deferred by persistent recovery state.
     * Keep the record on disk, but do not allow it to bypass its generic
     * trigger gate through a direct OPEN command.
     */
    if(
        !Floppy144DocumentAccessible(
            run_state,
            collection,
            record_index
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "RECORD ACCESS DEFERRED BY RECOVERY SEQUENCE."
        );

        return;
    }

    Floppy144CatalogueBuildRecord(
        collection,
        record_index,
        canonical_record_id,
        sizeof(canonical_record_id),
        title,
        sizeof(title)
    );

    snprintf(
        line,
        sizeof(line),
        "OPENING RECORD %s...",
        canonical_record_id
    );

    Floppy144TerminalPushLine(
        terminal,
        line
    );

    terminal->requested_collection =
        collection;

    terminal->requested_record_index =
        record_index;

    terminal->open_record_requested =
        true;
}
void Floppy144TerminalSubmitInput(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world,
    Floppy144RunState *run_state
)
{
    char submitted_line
        [FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    char normalised_input
        [FLOPPY144_TERMINAL_INPUT_CAPACITY];

    bool services_initialised;

    const char *help_arguments;
    const char *restore_arguments;
    const char *list_arguments;
    const char *open_arguments;

    if(
        terminal == NULL ||
        world == NULL ||
        run_state == NULL ||
        terminal->input_length == 0U
    )
    {
        return;
    }

    terminal->open_record_requested =
        false;

    /*
     * Whitespace-only submissions are true no-ops: they are not printed,
     * stored in history or treated as invalid commands.
     */
    if(
        !Floppy144TerminalNormaliseCommand(
            terminal->input,
            normalised_input,
            sizeof(normalised_input)
        )
    )
    {
        terminal->input_length = 0U;
        terminal->input[0] = '\0';
        Floppy144TerminalLeaveHistoryNavigation(terminal);
        return;
    }

    Floppy144TerminalStoreHistory(
        terminal,
        normalised_input
    );

    snprintf(
        submitted_line,
        sizeof(submitted_line),
        "A:\\GDR> %s",
        normalised_input
    );

    Floppy144TerminalPushWrappedLine(
        terminal,
        submitted_line
    );

    services_initialised =
        Floppy144WorldArchiveServicesInitialised(
            world
        );

    help_arguments =
        Floppy144TerminalCommandArguments(
            normalised_input,
            "HELP"
        );

    restore_arguments =
        Floppy144TerminalCommandArguments(
            normalised_input,
            "RESTORE"
        );

    list_arguments =
        Floppy144TerminalCommandArguments(
            normalised_input,
            "LIST"
        );

    open_arguments =
        Floppy144TerminalCommandArguments(
            normalised_input,
            "OPEN"
        );

    if(
        Floppy144TerminalCommandMatches(
            normalised_input,
            "EXIT"
        )
    )
    {
        terminal->exit_requested =
            true;
    }
    else if(help_arguments != NULL)
    {
        if(
            services_initialised &&
            help_arguments[0] == '\0'
        )
        {
            Floppy144TerminalOpenHelpPager(
                terminal
            );
        }
        else
        {
            Floppy144TerminalPrintHelp(
                terminal,
                services_initialised,
                help_arguments
            );
        }
    }
    else if(
        !services_initialised &&
        Floppy144TerminalCommandMatches(
            normalised_input,
            "INITIATE"
        )
    )
    {
        if(
            Floppy144WorldInitialiseArchiveServices(
                world
            )
        )
        {
            Floppy144RunStateInitialiseArchiveServices(
                run_state
            );

            Floppy144TerminalPushLine(
                terminal,
                ""
            );

            Floppy144TerminalPushLine(
                terminal,
                "ARCHIVE SERVICES INITIALISED."
            );

            Floppy144TerminalPushLine(
                terminal,
                "RECOVERY INDEX AVAILABLE."
            );

            Floppy144TerminalPushLine(
                terminal,
                ""
            );

            Floppy144TerminalPushLine(
                terminal,
                "NEW COMMANDS: RESTORE, LIST"
            );

            Floppy144TerminalPushLine(
                terminal,
                "TYPE HELP FOR OPERATING GUIDANCE."
            );

            Floppy144TerminalPrintNextAction(
                terminal,
                run_state
            );
        }
        else
        {
            Floppy144TerminalPushLine(
                terminal,
                "ARCHIVE SERVICES COULD NOT BE INITIALISED."
            );
        }
    }
    else if(restore_arguments != NULL)
    {
        if(!services_initialised)
        {
            Floppy144TerminalPushLine(
                terminal,
                "COMMAND UNAVAILABLE. RUN INITIATE."
            );
        }
        else if(restore_arguments[0] == '\0')
        {
            Floppy144TerminalPushLine(
                terminal,
                "RESTORE REQUIRES A COLLECTION CODE."
            );
        }
        else
        {
            Floppy144TerminalRestoreCollection(
                terminal,
                world,
                run_state,
                restore_arguments
            );
        }
    }
    else if(list_arguments != NULL)
    {
        if(!services_initialised)
        {
            Floppy144TerminalPushLine(
                terminal,
                "COMMAND UNAVAILABLE. RUN INITIATE."
            );
        }
        else if(list_arguments[0] != '\0')
        {
            Floppy144TerminalPrintCollectionRecords(
                terminal,
                world,
                list_arguments
            );
        }
        else
        {
            Floppy144TerminalPrintCollections(
                terminal,
                world,
                run_state
            );
        }
    }
    else if(open_arguments != NULL)
    {
        if(!services_initialised)
        {
            Floppy144TerminalPushLine(
                terminal,
                "COMMAND UNAVAILABLE. RUN INITIATE."
            );
        }
        else if(!terminal->open_command_available)
        {
            Floppy144TerminalPushLine(
                terminal,
                "COMMAND UNAVAILABLE. RESTORE A COLLECTION FIRST."
            );
        }
        else if(open_arguments[0] == '\0')
        {
            Floppy144TerminalPushLine(
                terminal,
                "OPEN REQUIRES A RECORD ID."
            );
        }
        else
        {
            Floppy144TerminalRequestOpenRecord(
                terminal,
                world,
                run_state,
                open_arguments
            );
        }
    }
    else
    {
        Floppy144TerminalPushLine(
            terminal,
            "UNRECOGNISED COMMAND. TYPE HELP."
        );
    }

    terminal->input_length =
        0U;

    terminal->input[0] =
        '\0';

    Floppy144TerminalLeaveHistoryNavigation(
        terminal
    );
}

/*
 * Terminal state management
 *
 * The terminal displays one archive domain at a time. Selection remains
 * within that domain, while domain navigation skips domains containing no
 * registered collections.
 */

static bool Floppy144TerminalDomainHasCollections(
    Floppy144CollectionDomain domain
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

        if(definition->domain == domain)
        {
            return true;
        }
    }

    return false;
}

static Floppy144CollectionId Floppy144TerminalFirstCollectionInDomain(
    Floppy144CollectionDomain domain
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

        if(definition->domain == domain)
        {
            return collection;
        }
    }

    return FLOPPY144_COLLECTION_DR01;
}

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
)
{
    uint32_t line_index;
    uint32_t history_index;

    terminal->selected_domain =
        FLOPPY144_COLLECTION_DOMAIN_DR;

    terminal->selected_collection =
        Floppy144TerminalFirstCollectionInDomain(
        terminal->selected_domain
    );

    terminal->detail_open =
        false;

    terminal->restoration_notice =
        false;

    terminal->suppress_next_character =
        true;

    terminal->exit_requested =
        false;

    terminal->open_record_requested =
        false;

    terminal->record_pager_active =
        false;

    terminal->help_pager_active =
        false;

    terminal->open_command_available =
        Floppy144TerminalRecordAccessAvailable(
            world
        );

    terminal->default_record_collection_valid =
        false;

    terminal->default_record_collection =
        FLOPPY144_COLLECTION_DR01;

    terminal->record_pager_collection =
        FLOPPY144_COLLECTION_DR01;

    terminal->record_pager_page =
        1U;

    terminal->collection_pager_state =
        NULL;

    terminal->help_pager_page =
        1U;

    terminal->requested_collection =
        FLOPPY144_COLLECTION_DR01;

    terminal->requested_record_index =
        0U;

    terminal->input_length =
        0U;

    terminal->input[0] =
        '\0';

    terminal->history_count = 0U;
    terminal->history_cursor = -1;
    terminal->history_draft[0] = '\0';

    for(
        history_index = 0U;
        history_index < FLOPPY144_TERMINAL_HISTORY_ENTRIES;
        ++history_index
    )
    {
        terminal->history[history_index][0] = '\0';
    }

    terminal->output_count =
        0U;

    for(
        line_index = 0U;
        line_index <
            FLOPPY144_TERMINAL_OUTPUT_LINES;
        ++line_index
    )
    {
        terminal->output[line_index][0] =
            '\0';
    }

    Floppy144TerminalPushLine(
        terminal,
        "GDR ARCHIVE RECOVERY ENVIRONMENT"
    );

    Floppy144TerminalPushLine(
        terminal,
        "MEDIA DETECTED: DISK 144"
    );

    Floppy144TerminalPushLine(
        terminal,
        Floppy144WorldCollectionRestored(
            world,
            FLOPPY144_COLLECTION_DR01
        )
            ? "SITE STATE: PARTIAL RECONSTRUCTION"
            : "SITE STATE: UNAVAILABLE"
    );

    Floppy144TerminalPushLine(
        terminal,
        Floppy144WorldArchiveServicesInitialised(
            world
        )
            ? "ARCHIVE SERVICES: ONLINE"
            : "ARCHIVE SERVICES: OFFLINE"
    );

    Floppy144TerminalPushLine(
        terminal,
        ""
    );

    Floppy144TerminalPushLine(
        terminal,
        "TYPE HELP FOR OPERATING GUIDANCE."
    );

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
    const uint32_t background = FLOPPY144_RGB(8, 13, 11);
    const uint32_t panel = FLOPPY144_RGB(13, 24, 19);
    const uint32_t border = FLOPPY144_RGB(55, 92, 72);
    const uint32_t text = FLOPPY144_RGB(127, 196, 146);
    const uint32_t muted = FLOPPY144_RGB(76, 119, 91);
    const uint32_t bright = FLOPPY144_RGB(172, 231, 183);
    Floppy144Surface surface =
    {
        (uint32_t *)runtime->backbuffer.data,
        runtime->backbuffer.width,
        runtime->backbuffer.height
    };
    char site_status[64];
    char prompt[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];
    uint32_t line_index;
    uint32_t output_y = 84U;
    bool pager_active =
        terminal->record_pager_active ||
        terminal->help_pager_active;

    Floppy144RecoveryFormatCapacity(
        run_state,
        site_status,
        (uint32_t)sizeof(site_status)
    );

    (void)snprintf(
        prompt,
        sizeof(prompt),
        "A:\\GDR> %s",
        terminal->input
    );

    Floppy144DrawClear(&surface, background);
    Floppy144DrawText(
        &surface,
        10U,
        5U,
        "GDR ARCHIVE RECOVERY TERMINAL",
        1U,
        muted
    );
    Floppy144DrawText(
        &surface,
        630U - Floppy144DrawTextWidth(site_status, 1U),
        5U,
        site_status,
        1U,
        text
    );

    Floppy144DrawFillRect(&surface, 10U, 18U, 620U, 280U, panel);
    Floppy144DrawRect(&surface, 10U, 18U, 620U, 280U, border);
    Floppy144TerminalTextCentred(
        &surface,
        42U,
        "GDR ARCHIVE RECOVERY TERMINAL",
        2U,
        bright
    );
    Floppy144DrawFillRect(&surface, 30U, 68U, 580U, 1U, border);

    for(
        line_index = 0U;
        line_index < terminal->output_count;
        ++line_index
    )
    {
        Floppy144DrawText(
            &surface,
            34U,
            output_y,
            terminal->output[line_index],
            1U,
            text
        );
        output_y += 15U;
    }

    Floppy144DrawFillRect(&surface, 30U, 260U, 580U, 1U, border);

    if(!pager_active)
    {
        uint32_t uPromptY =
            Floppy144RunStateArchiveServicesInitialised(run_state)
            ? 274U
            : 284U;

        if(!Floppy144RunStateArchiveServicesInitialised(run_state))
        {
            Floppy144DrawText(
                &surface,
                34U,
                268U,
                "TYPE INITIATE TO START THE RESTORATION PROCESS.",
                1U,
                muted
            );
        }

        Floppy144DrawText(
            &surface,
            34U,
            uPromptY,
            prompt,
            1U,
            bright
        );
        Floppy144DrawFillRect(
            &surface,
            34U + Floppy144DrawTextWidth(prompt, 1U),
            uPromptY,
            5U,
            8U,
            bright
        );
    }

    Floppy144DrawFillRect(&surface, 10U, 306U, 620U, 28U, background);
    Floppy144DrawRect(&surface, 10U, 306U, 620U, 28U, border);

    if(pager_active)
    {
        Floppy144DrawText(
            &surface,
            22U,
            316U,
            "SPACE/ENTER NEXT   BACKSPACE PREVIOUS",
            1U,
            text
        );
        {
            const char *pszReturnPrompt =
                terminal->record_pager_active
                    ? "Q: EXIT"
                    : "Q RETURN";

            Floppy144DrawText(
                &surface,
                630U -
                    Floppy144DrawTextWidth(pszReturnPrompt, 1U) -
                    12U,
                316U,
                pszReturnPrompt,
                1U,
                muted
            );
        }
    }
    else
    {
        Floppy144DrawText(
            &surface,
            22U,
            316U,
            "UP/DOWN HISTORY   BACKSPACE EDIT   ENTER SUBMIT",
            1U,
            text
        );
        Floppy144DrawText(
            &surface,
            630U - Floppy144DrawTextWidth("TYPE EXIT TO CLOSE", 1U) - 12U,
            316U,
            "TYPE EXIT TO CLOSE",
            1U,
            muted
        );
    }
}
