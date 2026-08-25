/*
 * Floppy//144 - archive terminal implementation
 *
 * Renders the collection list and detail overlay, handles terminal-local
 * navigation, and writes collection restoration into the shared world state.
 */

#include "floppy144_terminal.h"
#include "floppy144_catalogue.h"

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

    --terminal->input_length;

    terminal->input[terminal->input_length] =
        '\0';
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

            Floppy144TerminalPushLine(
                terminal,
                "  OPEN <RECORD>"
            );
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

        Floppy144TerminalPushLine(
            terminal,
            services_initialised
                ? "TOPICS: RESTORE, LIST, OPEN, EXIT."
                : "TOPICS: INITIATE, EXIT."
        );

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
            "EXAMPLE: RESTORE HR-02"
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
            "SPACE: NEXT PAGE."
        );

        Floppy144TerminalPushLine(
            terminal,
            "BACKSPACE: PREVIOUS PAGE."
        );

        Floppy144TerminalPushLine(
            terminal,
            "ENTER: RETURN TO COMMAND PROMPT."
        );

        Floppy144TerminalPushLine(
            terminal,
            "ONLY RESTORED COLLECTIONS MAY BE SEARCHED."
        );

        return;
    }

    if(
        services_initialised &&
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
            "RETRIEVE ONE RECORD BY FULL ID."
        );

        Floppy144TerminalPushLine(
            terminal,
            "EXAMPLE: OPEN HR-02-RS-0038"
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

static void Floppy144TerminalPrintCollections(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
)
{
    uint32_t collection_index;

    char line
        [FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    Floppy144TerminalPushLine(
        terminal,
        ""
    );

    Floppy144TerminalPushLine(
        terminal,
        "COLLECTIONS AVAILABLE ON DISK 144:"
    );

    Floppy144TerminalPushLine(
        terminal,
        ""
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

        /*
         * Before DR-01 reconstructs the site, the removable-media recovery
         * environment exposes only its mandatory recovery index.
         */

        if(
            collection != FLOPPY144_COLLECTION_DR01 &&
            !Floppy144WorldCollectionRestored(
                world,
                FLOPPY144_COLLECTION_DR01
            )
        )
        {
            continue;
        }

        const char *status =
            Floppy144WorldCollectionEvidenceFound(
                world,
                collection
            )
                ? "EVIDENCE"
                : Floppy144WorldCollectionRestored(
                    world,
                    collection
                )
                    ? "RESTORED"
                    : "AVAILABLE";

        snprintf(
            line,
            sizeof(line),
            "%s  %s",
            definition->code,
            status
        );

        Floppy144TerminalPushLine(
            terminal,
            line
        );
    }
}

/*
 * Restore one collection identified by its registered code.
 */

#define FLOPPY144_TERMINAL_RECORDS_PER_PAGE 7U

/*
 * Parse LIST arguments into a collection code and optional page number.
 *
 * LIST HR-02 defaults to page one.
 * LIST HR-02 3 requests page three.
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
    const Floppy144CollectionDefinition *definition;

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

    next_page =
        (int32_t)terminal->record_pager_page +
        direction;

    if(next_page < 1)
    {
        next_page =
            1;
    }

    if(next_page > (int32_t)page_count)
    {
        next_page =
            (int32_t)page_count;
    }

    if(
        (uint32_t)next_page ==
        terminal->record_pager_page
    )
    {
        return;
    }

    terminal->record_pager_page =
        (uint32_t)next_page;

    Floppy144TerminalPrintRecordPage(
        terminal
    );
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
        collection != FLOPPY144_COLLECTION_DR01 &&
        !Floppy144WorldCollectionRestored(
            world,
            FLOPPY144_COLLECTION_DR01
        )
    )
    {
        Floppy144TerminalPushLine(
            terminal,
            "COLLECTION UNAVAILABLE IN INITIAL RECOVERY ENVIRONMENT."
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
        !Floppy144WorldRestoreCollection(
            world,
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

    Floppy144RunStateRestoreCollection(
        run_state,
        collection
    );

    if(collection == FLOPPY144_COLLECTION_DR01)
    {
        terminal->site_entry_requested =
            true;
    }

    Floppy144TerminalPushLine(
        terminal,
        "COLLECTION RESTORED."
    );

    snprintf(
        line,
        sizeof(line),
        "SITE RECONSTRUCTION STATUS: %02u%%",
        (unsigned)Floppy144WorldReconstructionPercent(
            world
        )
    );

    Floppy144TerminalPushLine(
        terminal,
        line
    );
}

/*
 * Locate a record by its complete player-facing ID.
 *
 * IDs are generated through the same catalogue function used by LIST and the
 * graphical catalogue, including authored-document overrides.
 */

static bool Floppy144TerminalFindRecord(
    const char *requested_record_id,
    Floppy144CollectionId *collection,
    uint32_t *record_index
)
{
    uint32_t collection_index;

    char generated_record_id[24];
    char generated_title[48];

    if(
        requested_record_id == NULL ||
        collection == NULL ||
        record_index == NULL
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
        Floppy144CollectionId candidate_collection =
            (Floppy144CollectionId)collection_index;

        const Floppy144CatalogueDefinition *catalogue_definition =
            &Floppy144CollectionGet(
                candidate_collection
            )->catalogue;

        uint32_t candidate_index;

        for(
            candidate_index = 0U;
            candidate_index <
                catalogue_definition->record_count;
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

            if(
                Floppy144TerminalCommandMatches(
                    requested_record_id,
                    generated_record_id
                )
            )
            {
                *collection =
                    candidate_collection;

                *record_index =
                    candidate_index;

                return true;
            }
        }
    }

    return false;
}

/*
 * Validate an OPEN request and pass the resolved catalogue position to main.
 */

static void Floppy144TerminalRequestOpenRecord(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world,
    const char *record_id
)
{
    Floppy144CollectionId collection;
    uint32_t record_index;

    const Floppy144CollectionDefinition *definition;

    char canonical_record_id[24];
    char title[48];
    char line[FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    if(
        !Floppy144TerminalFindRecord(
            record_id,
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

    snprintf(
        submitted_line,
        sizeof(submitted_line),
        "A:\\GDR> %s",
        terminal->input
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
            terminal->input,
            "HELP"
        );

    restore_arguments =
        Floppy144TerminalCommandArguments(
            terminal->input,
            "RESTORE"
        );

    list_arguments =
        Floppy144TerminalCommandArguments(
            terminal->input,
            "LIST"
        );

    open_arguments =
        Floppy144TerminalCommandArguments(
            terminal->input,
            "OPEN"
        );

    if(
        Floppy144TerminalCommandMatches(
            terminal->input,
            "EXIT"
        )
    )
    {
        terminal->exit_requested =
            true;
    }
    else if(help_arguments != NULL)
    {
        Floppy144TerminalPrintHelp(
            terminal,
            services_initialised,
            help_arguments
        );
    }
    else if(
        !services_initialised &&
        Floppy144TerminalCommandMatches(
            terminal->input,
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
                "NEW COMMANDS: RESTORE, LIST, OPEN"
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
                world
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

    return FLOPPY144_COLLECTION_DR01;
}

void Floppy144TerminalReset(
    Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
)
{
    uint32_t line_index;

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

    terminal->suppress_next_character =
        true;

    terminal->exit_requested =
        false;

    terminal->site_entry_requested =
        false;

    terminal->open_record_requested =
        false;

    terminal->record_pager_active =
        false;

    terminal->record_pager_collection =
        FLOPPY144_COLLECTION_DR01;

    terminal->record_pager_page =
        1U;

    terminal->requested_collection =
        FLOPPY144_COLLECTION_DR01;

    terminal->requested_record_index =
        0U;

    terminal->input_length =
        0U;

    terminal->input[0] =
        '\0';

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

        if(
            terminal->selected_collection ==
                FLOPPY144_COLLECTION_DR01
        )
        {
            terminal->site_entry_requested =
                true;
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
    F144Runtime *engine,
    const Floppy144TerminalState *terminal,
    const Floppy144WorldState *world
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
        (uint32_t *)engine->backbuffer.data,
        engine->backbuffer.width,
        engine->backbuffer.height
    };

    char site_status[16];

    char prompt
        [FLOPPY144_TERMINAL_OUTPUT_LINE_CAPACITY];

    uint32_t line_index;
    uint32_t output_y =
        84U;

    snprintf(
        site_status,
        sizeof(site_status),
        "SITE %02u%%",
        (unsigned)Floppy144WorldReconstructionPercent(
            world
        )
    );

    if(terminal->record_pager_active)
    {
        snprintf(
            prompt,
            sizeof(prompt),
            "SPACE NEXT  BACKSPACE PREVIOUS  ENTER RETURN"
        );
    }
    else
    {
        snprintf(
            prompt,
            sizeof(prompt),
            "A:\\GDR> %s",
            terminal->input
        );
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
        538,
        5,
        site_status,
        1,
        text
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
        "GDR ARCHIVE RECOVERY TERMINAL",
        2,
        bright
    );

    Floppy144DrawFillRect(
        &surface,
        40,
        68,
        560,
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
            44,
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
        40,
        270,
        560,
        1,
        border
    );

    Floppy144DrawText(
        &surface,
        44,
        284,
        prompt,
        1,
        bright
    );

    Floppy144DrawFillRect(
        &surface,
        44 +
            Floppy144DrawTextWidth(
                prompt,
                1
            ),
        284,
        5,
        8,
        bright
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
        "TYPE COMMAND   BACKSPACE EDIT   ENTER SUBMIT",
        1,
        text
    );

    Floppy144DrawText(
        &surface,
        490,
        326,
        "TYPE EXIT TO CLOSE",
        1,
        muted
    );
}
