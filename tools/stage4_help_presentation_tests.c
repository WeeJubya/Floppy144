/*
 * Floppy//144 continuous terminal Help manual regression.
 *
 * Exercise the public terminal commands and dynamically computed pages for
 * both record-access capability states. Layout is verified in 640x360 space.
 */
#include "floppy144_terminal.h"

#include <stdio.h>
#include <string.h>

#define HELP_TEXT_LEFT_X 34U
#define HELP_TEXT_RIGHT_X 606U
#define HELP_TEXT_WIDTH (HELP_TEXT_RIGHT_X - HELP_TEXT_LEFT_X)
#define HELP_OUTPUT_START_Y 84U
#define HELP_OUTPUT_STEP_Y 15U
#define HELP_GLYPH_HEIGHT 7U
#define HELP_DIVIDER_Y 260U
#define HELP_HEADER_ROWS 2U
#define HELP_BODY_ROWS \
    (((HELP_DIVIDER_Y - HELP_OUTPUT_START_Y - HELP_GLYPH_HEIGHT) / \
       HELP_OUTPUT_STEP_Y) + 1U - HELP_HEADER_ROWS)

static int failures = 0;
static uint32_t pixels[640U * 360U];

#define EXPECT(condition,message) \
    do { if(!(condition)) { ++failures; printf("FAIL: %s\n",(message)); } } while(0)

static void Submit(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world,
    Floppy144RunState *state,
    const char *command)
{
    const char *c;
    for(c=command;*c!='\0';++c)
        Floppy144TerminalInputCharacter(terminal,*c);
    Floppy144TerminalSubmitInput(terminal,world,state);
    if(Floppy144TerminalRestoreInProgress(terminal))
        Floppy144TerminalAdvanceRestore(
            terminal,world,state,terminal->restoration_duration_ms);
}

static int PageWithinBounds(const Floppy144TerminalState *terminal)
{
    uint32_t row;
    if(terminal->output_count < HELP_HEADER_ROWS ||
       terminal->output_count > FLOPPY144_TERMINAL_OUTPUT_LINES)
        return 0;

    if(HELP_OUTPUT_START_Y +
       (terminal->output_count - 1U) * HELP_OUTPUT_STEP_Y +
       HELP_GLYPH_HEIGHT > HELP_DIVIDER_Y)
        return 0;

    for(row=0U;row<terminal->output_count;++row)
    {
        if(Floppy144DrawTextWidth(terminal->output[row],1U) >
           HELP_TEXT_WIDTH)
            return 0;
    }
    return 1;
}

static int RenderInsideHelpBand(
    Floppy144TerminalState *terminal,
    const Floppy144RunState *state)
{
    Floppy144Surface surface = {pixels,640U,360U};
    uint32_t x,y;
    const uint32_t help_colour = FLOPPY144_RGB(127,196,146);

    memset(pixels,0,sizeof(pixels));
    Floppy144TerminalDraw(&surface,terminal,state);
    for(y=HELP_OUTPUT_START_Y;y<HELP_DIVIDER_Y;++y)
        for(x=HELP_TEXT_RIGHT_X;x<640U;++x)
            if(pixels[y*640U+x]==help_colour)
                return 0;
    return 1;
}

static void AppendText(char *dest, size_t capacity, const char *text)
{
    size_t size = strlen(dest);
    size_t length = strlen(text);
    if(size + length + 2U >= capacity)
    {
        EXPECT(0,"Help transcript aggregation exceeded test buffer");
        return;
    }
    if(size > 0U) dest[size++] = ' ';
    memcpy(dest+size,text,length+1U);
}

static uint32_t CountOccurrences(const char *text, const char *needle)
{
    uint32_t n = 0U;
    const char *at = text;
    while((at=strstr(at,needle)) != NULL)
    {
        ++n;
        at += strlen(needle);
    }
    return n;
}

/* Independently inspect every rendered page; no fixed page-3 assumptions. */
static void AuditHelp(
    Floppy144TerminalState *terminal,
    const Floppy144RunState *state,
    int unlocked)
{
    uint32_t page;
    uint32_t page_count = Floppy144TerminalHelpPageCount(terminal);
    char manual[4096] = {0};

    EXPECT(Floppy144TerminalHelpPagerActive(terminal),
           "HELP is active after entering the command");
    EXPECT(page_count >= 2U && page_count <= 16U,
           "Help derives a sensible multi-page count from its content");
    EXPECT(terminal->help_pager_page==1U,"HELP starts on page one");

    for(page=1U;page<=page_count;++page)
    {
        char label[32];
        uint32_t row;
        uint32_t body_rows;
        (void)snprintf(label,sizeof(label),"PAGE %u OF %u",
                       (unsigned)page,(unsigned)page_count);
        EXPECT(terminal->help_pager_page==page,
               "page navigation follows dynamic page count");
        EXPECT(strstr(terminal->output[1],label)!=NULL,
               "page label matches actual dynamic count");
        EXPECT(PageWithinBounds(terminal),"no Help row clips or overlaps");
        EXPECT(RenderInsideHelpBand(terminal,state),
               "rendering stays inside widened text band");

        body_rows = terminal->output_count - HELP_HEADER_ROWS;
        EXPECT(body_rows <= HELP_BODY_ROWS,
               "Help body uses no more than geometric row capacity");
        if(page<page_count)
            EXPECT(body_rows >= HELP_BODY_ROWS-2U,
                   "intermediate Help page is not half empty");

        EXPECT(terminal->output[HELP_HEADER_ROWS][0]!='\0',
               "page does not begin with an unnecessary blank");
        if(page<page_count)
        {
            const char *last = terminal->output[terminal->output_count-1U];
            EXPECT(last[0]!='\0',
                   "intermediate page does not end with a redundant blank");
            EXPECT(strcmp(last,"TERMINAL OPERATION")!=0 &&
                   strcmp(last,"RESTORING COLLECTIONS")!=0 &&
                   strcmp(last,"RECORD ACCESS")!=0 &&
                   strcmp(last,"HELP NAVIGATION")!=0,
                   "section heading is not orphaned at bottom of a page");
        }

        for(row=HELP_HEADER_ROWS;row<terminal->output_count;++row)
        {
            const char *line=terminal->output[row];
            EXPECT(strstr(line,"  ")==NULL,
                   "Help text uses single spaces between sentences");
            EXPECT(strstr(line,"BACKSPACE EDITS THE CURRENT ENTRY.")==NULL,
                   "obsolete Backspace sentence removed");
            if(line[0]!='\0')
                AppendText(manual,sizeof(manual),line);
        }

        /* Neither direction depends on a literal 3-page count. */
        Floppy144TerminalMoveHelpPager(terminal,1);
    }

    EXPECT(terminal->help_pager_page==1U,
           "advancing after final page wraps to first");
    Floppy144TerminalMoveHelpPager(terminal,-1);
    EXPECT(terminal->help_pager_page==page_count,
           "backward navigation wraps to computed final page");
    Floppy144TerminalMoveHelpPager(terminal,1);
    EXPECT(terminal->help_pager_page==1U,
           "forward navigation returns from wrapped final page");

    EXPECT(CountOccurrences(manual,"TERMINAL OPERATION")==1U,
           "manual has one terminal operation section");
    EXPECT(CountOccurrences(manual,"RESTORING COLLECTIONS")==1U,
           "manual has one collection recovery section");
    EXPECT(CountOccurrences(manual,"RECORD ACCESS")==1U,
           "manual has one record access section");
    EXPECT(CountOccurrences(manual,"HELP NAVIGATION")==1U,
           "manual has one Help navigation section");
    EXPECT(strstr(manual,"ENTER COMMANDS AT THE A:\\GDR> PROMPT.")!=NULL,
           "command prompt instructions survive automatic wrapping");
    EXPECT(strstr(manual,"INITIATE STARTS ARCHIVE SERVICES WHEN OFFLINE.")!=NULL,
           "manual describes INITIATE correctly");
    EXPECT(strstr(manual,"UP/DOWN RECALL COMMANDS FROM THIS SESSION.")!=NULL,
           "command history instruction preserved");
    EXPECT(strstr(manual,"ENTER SUBMITS A COMMAND.")!=NULL,
           "command submission instruction preserved");
    EXPECT(strstr(manual,"ESC OPENS GDR SESSION CONTROL.")!=NULL,
           "session control instruction preserved");
    EXPECT(strstr(manual,"EXIT CLOSES THE TERMINAL SESSION.")!=NULL,
           "EXIT instruction preserved");
    EXPECT(strstr(manual,"LIST SHOWS COLLECTION STATUS")!=NULL,
           "collection-level LIST guidance preserved");
    EXPECT(strstr(manual,"RESTORE <CODE> RECOVERS AN AVAILABLE COLLECTION.")!=NULL,
           "RESTORE guidance preserved");
    EXPECT(strstr(manual,"LIST <CODE> SHOWS THE RECORD INDEX")!=NULL,
           "record-level LIST remains distinct");
    EXPECT(strstr(manual,"LIMIT: 1440 KB.")!=NULL,
           "recovery capacity guidance preserved");
    EXPECT(strstr(manual,"HELP <COMMAND> SHOWS COMMAND-SPECIFIC GUIDANCE.")!=NULL,
           "manual describes topical HELP");
    EXPECT(strstr(manual,"SPACE OR ENTER: NEXT HELP PAGE.")!=NULL,
           "Help navigation instruction preserved");
    EXPECT(strstr(manual,"BACKSPACE: PREVIOUS HELP PAGE.")!=NULL,
           "Backspace is documented only as Help-page navigation");
    EXPECT(strstr(manual,"Q: RETURN TO THE COMMAND PROMPT.")!=NULL,
           "Help return instruction preserved");

    if(unlocked)
    {
        EXPECT(strstr(manual,"OPEN <RECORD-ID> RETRIEVES A RECOVERED RECORD.")!=NULL,
               "unlocked Help describes OPEN");
        EXPECT(strstr(manual,"RS-#### USES THE CURRENT RECORD COLLECTION.")!=NULL,
               "unlocked Help describes record shorthand");
        EXPECT(strstr(manual,"RESTORE A COLLECTION TO ENABLE OPEN.")==NULL,
               "unlocked Help does not show locked capability message");
    }
    else
    {
        EXPECT(strstr(manual,"RESTORE A COLLECTION TO ENABLE OPEN.")!=NULL,
               "locked Help explains how OPEN is enabled");
        EXPECT(strstr(manual,"OPEN <RECORD-ID> RETRIEVES A RECOVERED RECORD.")==NULL,
               "locked Help does not advertise unavailable OPEN");
    }
}

int main(void)
{
    Floppy144WorldState world;
    Floppy144RunState run;
    Floppy144TerminalState terminal;
    uint32_t initial_pages;

    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run,144U);
    Floppy144TerminalReset(&terminal,&world);
    Submit(&terminal,&world,&run,"INITIATE");
    Submit(&terminal,&world,&run,"HELP");
    initial_pages=Floppy144TerminalHelpPageCount(&terminal);
    AuditHelp(&terminal,&run,0);

    Floppy144TerminalCloseHelpPager(&terminal);
    EXPECT(!Floppy144TerminalHelpPagerActive(&terminal),
           "Q return closes Help pager");
    Submit(&terminal,&world,&run,"HELP");
    EXPECT(Floppy144TerminalHelpPagerActive(&terminal) &&
           terminal.help_pager_page==1U,
           "re-entering Help restarts at page one");

    Floppy144TerminalCloseHelpPager(&terminal);
    Submit(&terminal,&world,&run,"RESTORE DR-01");
    EXPECT(terminal.open_command_available,
           "restoring DR-01 enables record access for unlocked Help");
    Submit(&terminal,&world,&run,"HELP");
    AuditHelp(&terminal,&run,1);
    EXPECT(initial_pages==2U &&
           Floppy144TerminalHelpPageCount(&terminal)==2U,
           "normal 640x360 manual occupies two automatically filled pages");

    Floppy144TerminalCloseHelpPager(&terminal);
    EXPECT(!Floppy144TerminalHelpPagerActive(&terminal),
           "unlocked Help also closes cleanly");

    Submit(&terminal,&world,&run,"HELP LIST");
    EXPECT(!Floppy144TerminalHelpPagerActive(&terminal),
           "topic-specific HELP keeps its immediate transcript behaviour");
    {
        uint32_t line;
        int found = 0;
        for(line=0U;line<terminal.output_count;++line)
            if(strstr(terminal.output[line],"LIST <CODE>")!=NULL)
                found = 1;
        EXPECT(found,"topic-specific HELP LIST remains accurate");
    }

    if(failures)
    {
        printf("STAGE 4D CONTINUOUS HELP TESTS: FAIL (%d)\n",failures);
        return 1;
    }
    puts("STAGE 4D CONTINUOUS HELP TESTS: PASS");
    return 0;
}
