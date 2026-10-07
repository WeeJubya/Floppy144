/*
 * FLOPPY//144 Stage 4C terminal operator-authentication regression.
 *
 * Authentication is deliberately presentation-only. These tests exercise the
 * terminal API directly so command semantics remain covered independently of
 * the Win32 coordinator.
 */

#include "floppy144_run_state.h"
#include "floppy144_terminal.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void Expect(
    bool condition,
    const char *label
)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n", label);
    }
}

static bool TerminalContains(
    const Floppy144TerminalState *terminal,
    const char *text
)
{
    uint32_t line;

    if(
        terminal == NULL ||
        text == NULL
    )
    {
        return false;
    }

    for(
        line = 0U;
        line < terminal->output_count;
        ++line
    )
    {
        if(
            strstr(
                terminal->output[line],
                text
            ) != NULL
        )
        {
            return true;
        }
    }

    return false;
}

static void SubmitCommand(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world,
    Floppy144RunState *run_state,
    const char *command
)
{
    const char *character;

    for(
        character = command;
        character != NULL &&
        *character != '\0';
        ++character
    )
    {
        Floppy144TerminalInputCharacter(
            terminal,
            *character
        );
    }

    Floppy144TerminalSubmitInput(
        terminal,
        world,
        run_state
    );
}

static void TestNamedFreshAuthentication(void)
{
    Floppy144WorldState world;
    Floppy144RunState run_state;
    Floppy144TerminalState terminal;

    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run_state,144U);
    Floppy144TerminalReset(&terminal,&world);

    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        "GLYNN WILLIAMS",
        true
    );

    Expect(
        terminal.output_count <=
            FLOPPY144_TERMINAL_OUTPUT_LINES,
        "authentication remains inside terminal transcript capacity"
    );

    Expect(
        strstr(
            terminal.output[0],
            "GDR ARCHIVE RECOVERY ENVIRONMENT"
        ) != NULL,
        "authentication preserves the terminal environment identity"
    );

    Expect(
        TerminalContains(
            &terminal,
            "GDR NETWORK ACCESS"
        ) &&
        TerminalContains(
            &terminal,
            "OPERATOR: GLYNN WILLIAMS"
        ) &&
        TerminalContains(
            &terminal,
            "VERIFYING PROFILE..."
        ) &&
        TerminalContains(
            &terminal,
            "ACCESS ACCEPTED"
        ),
        "named profile receives the compact authentication sequence"
    );

    SubmitCommand(
        &terminal,
        &world,
        &run_state,
        "INITIATE"
    );

    Expect(
        Floppy144WorldArchiveServicesInitialised(
            &world
        ) &&
        Floppy144RunStateArchiveServicesInitialised(
            &run_state
        ),
        "existing terminal command works immediately after authentication"
    );

    Expect(
        terminal.history_count == 1U,
        "authentication does not enter synthetic commands into history"
    );
}

static void TestUnnamedAuthentication(void)
{
    Floppy144WorldState world;
    Floppy144RunState run_state;
    Floppy144TerminalState terminal;

    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run_state,145U);
    Floppy144TerminalReset(&terminal,&world);

    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        "",
        true
    );

    Expect(
        TerminalContains(
            &terminal,
            "OPERATOR: GDR OPERATOR"
        ),
        "unnamed profile uses the safe generic operator label"
    );

    Expect(
        TerminalContains(
            &terminal,
            "ACCESS ACCEPTED"
        ),
        "unnamed profile remains playable without forced profile setup"
    );
}

static void TestRepeatedAccessAndRename(void)
{
    Floppy144WorldState world;
    Floppy144RunState run_state;
    Floppy144TerminalState terminal;

    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run_state,146U);

    /*
     * A later physical-terminal access keeps identity but skips the full
     * verification ceremony.
     */
    Floppy144TerminalResetAtRoom(
        &terminal,
        &world,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        "GLYNN WILLIAMS",
        false
    );

    Expect(
        TerminalContains(
            &terminal,
            "OPERATOR: GLYNN WILLIAMS"
        ) &&
        !TerminalContains(
            &terminal,
            "VERIFYING PROFILE..."
        ) &&
        !TerminalContains(
            &terminal,
            "ACCESS ACCEPTED"
        ),
        "repeated terminal access identifies operator without re-authentication"
    );

    SubmitCommand(
        &terminal,
        &world,
        &run_state,
        "EXIT"
    );

    Expect(
        terminal.exit_requested,
        "existing EXIT command still closes the terminal session"
    );

    /*
     * Re-entry after EXIT is another reset, but the application passes
     * authenticate=false once the recovery session has already authenticated.
     */
    Floppy144TerminalResetAtRoom(
        &terminal,
        &world,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        "GLYNN WILLIAMS",
        false
    );

    Expect(
        !terminal.exit_requested &&
        !TerminalContains(
            &terminal,
            "VERIFYING PROFILE..."
        ),
        "terminal exit and re-entry do not replay authentication"
    );

    /*
     * A profile rename is picked up from the value supplied to the next reset
     * terminal session without requiring terminal persistence or save changes.
     */
    Floppy144TerminalResetAtRoom(
        &terminal,
        &world,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        "RENAMED OPERATOR",
        false
    );

    Expect(
        TerminalContains(
            &terminal,
            "OPERATOR: RENAMED OPERATOR"
        ) &&
        !TerminalContains(
            &terminal,
            "OPERATOR: GLYNN WILLIAMS"
        ),
        "profile rename appears on the next terminal session"
    );
}

static void TestRestoredSessionAuthentication(void)
{
    Floppy144WorldState world;
    Floppy144RunState run_state;
    Floppy144TerminalState terminal;

    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run_state,147U);

    /*
     * Reinstate rebuilds transient terminal state from the saved recovery.
     * The coordinator treats that as a newly-established application session,
     * so the same compact sequence is shown once after restoration.
     */
    Floppy144TerminalResetAtRoom(
        &terminal,
        &world,
        FLOPPY144_ROOM_RECORDS_OFFICE
    );

    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        "RESTORED OPERATOR",
        true
    );

    Expect(
        TerminalContains(
            &terminal,
            "OPERATOR: RESTORED OPERATOR"
        ) &&
        TerminalContains(
            &terminal,
            "VERIFYING PROFILE..."
        ) &&
        TerminalContains(
            &terminal,
            "ACCESS ACCEPTED"
        ),
        "restored session receives one fresh application authentication"
    );
}

int main(void)
{
    TestNamedFreshAuthentication();
    TestUnnamedAuthentication();
    TestRepeatedAccessAndRename();
    TestRestoredSessionAuthentication();

    if(failures != 0)
    {
        printf(
            "STAGE 4C TERMINAL AUTH TESTS: FAIL (%d)\n",
            failures
        );

        return 1;
    }

    printf(
        "STAGE 4C TERMINAL AUTH TESTS: PASS\n"
    );

    return 0;
}
