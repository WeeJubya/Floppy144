/*
 * FLOPPY//144 Stage 4B portable/developer configuration regression.
 *
 * These tests use explicit strings and direct config injection. They never read
 * the player's command line, clock, locale, profile or other real environment.
 */

#include "f144_startup_config.h"
#include "f144_win32_startup_config.h"

#include <stdbool.h>
#include <stdio.h>

static int failures;

/*
 * Record one configuration regression expectation.
 */
static void Expect(
    bool condition,
    const char *label
)
{
    if(!condition)
    {
        ++failures;

        printf(
            "FAIL: %s\n",
            label
        );
    }
}

/*
 * Verify that an ordinary release-like launch exposes no developer state.
 */
static void TestDefaultConfiguration(void)
{
    F144StartupConfig config;
    uint32_t seed;
    F144CalendarDate date;

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "",
            &config
        ),
        "empty command line parses"
    );

    Expect(
        !f144StartupConfigDebugEnabled(
            &config
        ),
        "default launch keeps debug disabled"
    );

    Expect(
        !f144StartupConfigRecoverySeedOverride(
            &config,
            &seed
        ),
        "default launch has no deterministic seed"
    );

    Expect(
        !f144StartupConfigFixedDateOverride(
            &config,
            &date
        ),
        "default launch has no fixed date"
    );
}

/*
 * Verify that the historical exact -debug switch enables intended debug mode.
 */
static void TestDebugSwitch(void)
{
    F144StartupConfig config;

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-debug",
            &config
        ),
        "-debug command line parses"
    );

    Expect(
        f144StartupConfigDebugEnabled(
            &config
        ),
        "-debug enables semantic debug mode"
    );

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-DEBUG",
            &config
        ),
        "case-variant command line parses"
    );

    Expect(
        !f144StartupConfigDebugEnabled(
            &config
        ),
        "debug switch remains exact and case-sensitive"
    );
}

/*
 * Verify that deterministic developer overrides require explicit debug mode.
 */
static void TestDeveloperOverrideGating(void)
{
    F144StartupConfig config;
    uint32_t seed;
    F144CalendarDate date;

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-seed 144 -date 2026-12-24",
            &config
        ),
        "override-only command line parses"
    );

    Expect(
        !f144StartupConfigDebugEnabled(
            &config
        ),
        "developer overrides do not imply debug mode"
    );

    Expect(
        !f144StartupConfigRecoverySeedOverride(
            &config,
            &seed
        ),
        "seed override is ignored without -debug"
    );

    Expect(
        !f144StartupConfigFixedDateOverride(
            &config,
            &date
        ),
        "date override is ignored without -debug"
    );
}

/*
 * Verify that -debug can activate deterministic seed/date overrides in any
 * supported token order.
 */
static void TestDeveloperOverrides(void)
{
    F144StartupConfig config;
    uint32_t seed;
    F144CalendarDate date;

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-seed 144 -date 2028-02-29 -debug",
            &config
        ),
        "debug override command line parses"
    );

    Expect(
        f144StartupConfigDebugEnabled(
            &config
        ),
        "debug override launch is debug-enabled"
    );

    Expect(
        f144StartupConfigRecoverySeedOverride(
            &config,
            &seed
        ) &&
        seed == 144U,
        "debug seed override is retained"
    );

    Expect(
        f144StartupConfigFixedDateOverride(
            &config,
            &date
        ) &&
        date.year == 2028U &&
        date.month == 2U &&
        date.day == 29U,
        "debug fixed leap-day override is retained"
    );
}

/*
 * Verify that malformed deterministic overrides fail closed to normal values.
 */
static void TestMalformedOverrides(void)
{
    F144StartupConfig config;
    uint32_t seed;
    F144CalendarDate date;

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-debug -seed 0 -date 2027-02-29",
            &config
        ),
        "malformed override command line still parses safely"
    );

    Expect(
        f144StartupConfigDebugEnabled(
            &config
        ),
        "malformed overrides do not disable explicit debug mode"
    );

    Expect(
        !f144StartupConfigRecoverySeedOverride(
            &config,
            &seed
        ),
        "zero recovery seed is rejected"
    );

    Expect(
        !f144StartupConfigFixedDateOverride(
            &config,
            &date
        ),
        "invalid calendar date is rejected"
    );
}

/*
 * Verify that tests/future platforms can inject deterministic configuration
 * without using any command-line or player environment.
 */
static void TestDirectPortableInjection(void)
{
    F144StartupConfig config;
    uint32_t seed;
    F144CalendarDate date;

    f144StartupConfigReset(
        &config
    );

    f144StartupConfigSetDebugEnabled(
        &config,
        true
    );

    Expect(
        f144StartupConfigSetRecoverySeedOverride(
            &config,
            424242U
        ),
        "portable seed injection succeeds"
    );

    Expect(
        f144StartupConfigSetFixedDateOverride(
            &config,
            2030U,
            10U,
            31U
        ),
        "portable date injection succeeds"
    );

    Expect(
        f144StartupConfigRecoverySeedOverride(
            &config,
            &seed
        ) &&
        seed == 424242U,
        "portable seed injection round-trips"
    );

    Expect(
        f144StartupConfigFixedDateOverride(
            &config,
            &date
        ) &&
        date.year == 2030U &&
        date.month == 10U &&
        date.day == 31U,
        "portable date injection round-trips"
    );
}

/*
 * Run the complete Stage 4 configuration regression suite.
 */
int main(void)
{
    TestDefaultConfiguration();
    TestDebugSwitch();
    TestDeveloperOverrideGating();
    TestDeveloperOverrides();
    TestMalformedOverrides();
    TestDirectPortableInjection();

    if(failures != 0)
    {
        printf(
            "STAGE 4 CONFIGURATION TESTS: FAIL (%d)\n",
            failures
        );

        return 1;
    }

    printf(
        "STAGE 4 CONFIGURATION TESTS: PASS\n"
    );

    return 0;
}
