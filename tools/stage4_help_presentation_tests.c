/*
 * FLOPPY//144 Stage 4D Help presentation regression.
 *
 * S4D-01 is deliberately presentation-only. These checks drive the public
 * Terminal API, verify all three Help pages and both page-3 capability states,
 * and prove the reflow stays inside the existing 640x360 terminal shell.
 */

#include "floppy144_terminal.h"

#include <stdio.h>
#include <string.h>

#define HELP_TEXT_LEFT_X       34U
#define HELP_TEXT_RIGHT_X     606U
#define HELP_TEXT_WIDTH       (HELP_TEXT_RIGHT_X - HELP_TEXT_LEFT_X)
#define HELP_OUTPUT_START_Y    84U
#define HELP_OUTPUT_STEP_Y     15U
#define HELP_GLYPH_HEIGHT       7U
#define HELP_DIVIDER_Y        260U
#define HELP_BODY_FIRST_LINE    6U

static int g_failures = 0;
static uint32_t g_pixels[640U * 360U];

#define F144_EXPECT(condition, label)                                  \
    do                                                                 \
    {                                                                  \
        if(!(condition))                                               \
        {                                                              \
            ++g_failures;                                              \
            printf("FAIL: %s\n", (label));                            \
        }                                                              \
    }                                                                  \
    while(0)

static void SubmitCommand(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world,
    Floppy144RunState *run_state,
    const char *command
)
{
    const char *cursor;

    for(cursor = command; *cursor != '\0'; ++cursor)
    {
        Floppy144TerminalInputCharacter(
            terminal,
            *cursor
        );
    }

    Floppy144TerminalSubmitInput(
        terminal,
        world,
        run_state
    );

    if(Floppy144TerminalRestoreInProgress(terminal))
    {
        Floppy144TerminalAdvanceRestore(
            terminal,
            world,
            run_state,
            terminal->restoration_duration_ms
        );
    }
}

static int OutputContains(
    const Floppy144TerminalState *terminal,
    const char *needle
)
{
    uint32_t line_index;

    for(
        line_index = 0U;
        line_index < terminal->output_count;
        ++line_index
    )
    {
        if(
            strstr(
                terminal->output[line_index],
                needle
            ) != NULL
        )
        {
            return 1;
        }
    }

    return 0;
}

static uint32_t MaxBodyWidth(
    const Floppy144TerminalState *terminal
)
{
    uint32_t line_index;
    uint32_t max_width = 0U;

    for(
        line_index = HELP_BODY_FIRST_LINE;
        line_index < terminal->output_count;
        ++line_index
    )
    {
        uint32_t width =
            Floppy144DrawTextWidth(
                terminal->output[line_index],
                1U
            );

        if(width > max_width)
        {
            max_width =
                width;
        }
    }

    return max_width;
}

static int PageFitsLayout(
    const Floppy144TerminalState *terminal
)
{
    uint32_t line_index;

    if(
        terminal == NULL ||
        terminal->output_count == 0U ||
        terminal->output_count >
            FLOPPY144_TERMINAL_OUTPUT_LINES
    )
    {
        return 0;
    }

    for(
        line_index = 0U;
        line_index < terminal->output_count;
        ++line_index
    )
    {
        if(
            Floppy144DrawTextWidth(
                terminal->output[line_index],
                1U
            ) >
            HELP_TEXT_WIDTH
        )
        {
            return 0;
        }
    }

    return
        HELP_OUTPUT_START_Y +
        (terminal->output_count - 1U) *
            HELP_OUTPUT_STEP_Y +
        HELP_GLYPH_HEIGHT <=
        HELP_DIVIDER_Y;
}

static int RenderedHelpStaysInsideTextBand(
    Floppy144TerminalState *terminal,
    const Floppy144RunState *run_state
)
{
    Floppy144Surface surface =
    {
        g_pixels,
        640U,
        360U
    };

    const uint32_t text_colour =
        FLOPPY144_RGB(127, 196, 146);

    uint32_t x;
    uint32_t y;

    memset(
        g_pixels,
        0,
        sizeof(g_pixels)
    );

    Floppy144TerminalDraw(
        &surface,
        terminal,
        run_state
    );

    /*
     * Inspect only transcript rows. Header/status/footer text legitimately
     * lives elsewhere; Help output must never paint beyond its safe right edge.
     */
    for(
        y = HELP_OUTPUT_START_Y;
        y < HELP_DIVIDER_Y;
        ++y
    )
    {
        for(
            x = HELP_TEXT_RIGHT_X;
            x < 640U;
            ++x
        )
        {
            if(
                g_pixels[y * 640U + x] ==
                text_colour
            )
            {
                return 0;
            }
        }
    }

    return 1;
}

static void CheckPageOne(
    const Floppy144TerminalState *terminal
)
{
    F144_EXPECT(
        terminal->help_pager_page == 1U,
        "Help opens on page 1"
    );
    F144_EXPECT(
        OutputContains(terminal, "PAGE 1 OF 3"),
        "page 1 reports three-page Help"
    );
    F144_EXPECT(
        OutputContains(terminal, "TERMINAL OPERATION"),
        "page 1 title is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "ENTER COMMANDS AT THE A:\\GDR> PROMPT."
        ),
        "page 1 prompt guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "BACKSPACE EDITS THE CURRENT ENTRY."
        ),
        "page 1 Backspace guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "UP/DOWN RECALL SESSION COMMANDS."
        ),
        "page 1 history guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "ENTER SUBMITS THE COMMAND."
        ),
        "page 1 Enter guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "ESC OPENS GDR SESSION CONTROL."
        ),
        "page 1 Escape guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "EXIT CLOSES THE TERMINAL SESSION."
        ),
        "page 1 Exit guidance is preserved"
    );
    F144_EXPECT(
        MaxBodyWidth(terminal) >= 500U,
        "page 1 uses the widened reading band"
    );
    F144_EXPECT(
        PageFitsLayout(terminal),
        "page 1 fits Help bounds"
    );
}

static void CheckPageTwo(
    const Floppy144TerminalState *terminal
)
{
    F144_EXPECT(
        terminal->help_pager_page == 2U,
        "forward navigation reaches page 2"
    );
    F144_EXPECT(
        OutputContains(terminal, "PAGE 2 OF 3"),
        "page 2 counter is correct"
    );
    F144_EXPECT(
        OutputContains(terminal, "RESTORING COLLECTIONS"),
        "page 2 title is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "LIST DISPLAYS COLLECTION STATUS."
        ),
        "page 2 LIST guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "RESTORE <CODE> RECOVERS ONE COLLECTION."
        ),
        "page 2 RESTORE guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "MAXIMUM AVAILABLE PER RECOVERY: 1440 KB."
        ),
        "page 2 capacity guidance is preserved"
    );
    F144_EXPECT(
        MaxBodyWidth(terminal) >= 400U,
        "page 2 uses the widened reading band"
    );
    F144_EXPECT(
        PageFitsLayout(terminal),
        "page 2 fits Help bounds"
    );
}

static void CheckLockedPageThree(
    const Floppy144TerminalState *terminal
)
{
    F144_EXPECT(
        terminal->help_pager_page == 3U,
        "forward navigation reaches final Help page"
    );
    F144_EXPECT(
        OutputContains(terminal, "PAGE 3 OF 3"),
        "final page counter is correct"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "RECORD RETRIEVAL IS NOT YET AVAILABLE."
        ),
        "locked page 3 content is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "RESTORE A COLLECTION TO ENABLE OPEN."
        ),
        "locked page 3 unlock guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "SPACE OR ENTER: NEXT PAGE."
        ),
        "locked page 3 next-page guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "BACKSPACE: PREVIOUS PAGE."
        ),
        "locked page 3 previous-page guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "Q: RETURN TO COMMAND PROMPT."
        ),
        "locked page 3 return guidance is preserved"
    );
    F144_EXPECT(
        MaxBodyWidth(terminal) >= 480U,
        "locked page 3 uses the widened reading band"
    );
    F144_EXPECT(
        PageFitsLayout(terminal),
        "locked page 3 fits Help bounds"
    );
}

static void CheckUnlockedPageThree(
    const Floppy144TerminalState *terminal
)
{
    F144_EXPECT(
        OutputContains(
            terminal,
            "LIST <CODE> OPENS A RESTORED RECORD INDEX."
        ),
        "unlocked page 3 LIST guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "OPEN <RECORD-ID> RETRIEVES ONE RECORD."
        ),
        "unlocked page 3 OPEN guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "AFTER RESTORE, RS-#### USES THAT COLLECTION."
        ),
        "unlocked page 3 shorthand guidance is preserved"
    );
    F144_EXPECT(
        OutputContains(
            terminal,
            "BACKSPACE: PREVIOUS PAGE.  Q: RETURN."
        ),
        "unlocked page 3 footer guidance is preserved"
    );
    F144_EXPECT(
        MaxBodyWidth(terminal) >= 470U,
        "unlocked page 3 uses the widened reading band"
    );
    F144_EXPECT(
        PageFitsLayout(terminal),
        "unlocked page 3 fits Help bounds"
    );
}

int main(void)
{
    Floppy144WorldState world;
    Floppy144RunState run_state;
    Floppy144TerminalState terminal;

    Floppy144WorldReset(
        &world
    );

    Floppy144RunStateBegin(
        &run_state,
        144U
    );

    Floppy144TerminalReset(
        &terminal,
        &world
    );

    SubmitCommand(
        &terminal,
        &world,
        &run_state,
        "INITIATE"
    );

    SubmitCommand(
        &terminal,
        &world,
        &run_state,
        "HELP"
    );

    F144_EXPECT(
        Floppy144TerminalHelpPagerActive(&terminal),
        "HELP opens the pager after INITIATE"
    );

    CheckPageOne(
        &terminal
    );

    F144_EXPECT(
        RenderedHelpStaysInsideTextBand(
            &terminal,
            &run_state
        ),
        "page 1 render stays inside the Help text band"
    );

    Floppy144TerminalMoveHelpPager(
        &terminal,
        1
    );
    CheckPageTwo(
        &terminal
    );

    Floppy144TerminalMoveHelpPager(
        &terminal,
        1
    );
    CheckLockedPageThree(
        &terminal
    );

    F144_EXPECT(
        RenderedHelpStaysInsideTextBand(
            &terminal,
            &run_state
        ),
        "final-page render stays inside the Help text band"
    );

    Floppy144TerminalMoveHelpPager(
        &terminal,
        1
    );
    F144_EXPECT(
        terminal.help_pager_page == 1U,
        "forward navigation wraps page 3 to page 1"
    );

    Floppy144TerminalMoveHelpPager(
        &terminal,
        -1
    );
    F144_EXPECT(
        terminal.help_pager_page == 3U,
        "back navigation wraps page 1 to page 3"
    );

    /*
     * Presentation fixture for the state-aware unlocked final page. The
     * capability bit is already public Terminal UI state; no gameplay state is
     * changed to exercise this alternate rendering.
     */
    terminal.open_command_available =
        true;

    terminal.help_pager_page =
        2U;

    Floppy144TerminalMoveHelpPager(
        &terminal,
        1
    );
    CheckUnlockedPageThree(
        &terminal
    );

    Floppy144TerminalCloseHelpPager(
        &terminal
    );

    F144_EXPECT(
        !Floppy144TerminalHelpPagerActive(&terminal),
        "Help return closes the pager"
    );

    if(g_failures != 0)
    {
        printf(
            "STAGE 4D HELP PRESENTATION TESTS: FAIL (%d)\n",
            g_failures
        );
        return 1;
    }

    printf(
        "STAGE 4D HELP PRESENTATION TESTS: PASS\n"
    );
    return 0;
}
