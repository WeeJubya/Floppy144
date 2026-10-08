/*
 * FLOPPY//144 S4E-02 calendar provider regression.
 * All content-facing test fixtures use a fake date backend. Only the separate
 * native smoke check queries the real local calendar, without date assumptions.
 */
#include "f144_platform.h"
#include "f144_startup_config.h"
#include "f144_win32_calendar.h"
#include "f144_win32_startup_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct FakeCalendar
{
    F144CalendarDate today;
    uint32_t queries;
    bool succeeds;
} FakeCalendar;

static int failures = 0;

static void Expect(bool condition, const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n", label);
    }
}

static bool FakeLocalDate(F144Platform *platform, F144CalendarDate *date)
{
    FakeCalendar *state = (FakeCalendar *)platform->state;
    ++state->queries;
    if(!state->succeeds)
    {
        return false;
    }
    *date = state->today;
    return true;
}

static uint64_t FakeMonotonicMs(F144Platform *platform)
{
    (void)platform;
    return 123456789ULL;
}

static bool SameDate(F144CalendarDate date, uint16_t year, uint8_t month, uint8_t day)
{
    return date.year == year && date.month == month && date.day == day;
}

static void SetFakeDate(FakeCalendar *state, uint16_t year, uint8_t month, uint8_t day)
{
    state->today.year = year;
    state->today.month = month;
    state->today.day = day;
}

static void BindFake(
    F144Platform *platform,
    F144PlatformApi *api,
    FakeCalendar *state
)
{
    memset(platform, 0, sizeof(*platform));
    memset(api, 0, sizeof(*api));
    memset(state, 0, sizeof(*state));
    state->succeeds = true;
    SetFakeDate(state, 2026U, 10U, 8U);
    api->local_calendar_date = FakeLocalDate;
    api->monotonic_ms = FakeMonotonicMs;
    platform->api = api;
    platform->state = state;
}

static void TestNormalAndDisabledOverride(void)
{
    F144Platform platform;
    F144PlatformApi api;
    FakeCalendar fake;
    F144StartupConfig config;
    F144CalendarDate date;

    BindFake(&platform, &api, &fake);
    f144StartupConfigReset(&config);

    Expect(
        f144PlatformCalendarDate(&platform, NULL, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "ordinary calendar query uses native local date"
    );
    Expect(fake.queries == 1U, "ordinary query dispatches once");

    Expect(
        f144StartupConfigSetFixedDateOverride(&config, 2026U, 12U, 20U),
        "test harness can install a validated date programmatically"
    );
    Expect(
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "date override is ignored while debug is disabled"
    );
    Expect(fake.queries == 2U, "disabled override still queries live local date");
    Expect(
        f144PlatformMonotonicMs(&platform) == 123456789ULL,
        "monotonic time is unaffected by wall-clock calendar"
    );

    SetFakeDate(&fake, 2027U, 1U, 1U);
    Expect(
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2027U, 1U, 1U),
        "normal mode reads current local date on each call"
    );
}

static void TestFixedDates(void)
{
    F144Platform platform;
    F144PlatformApi api;
    FakeCalendar fake;
    F144StartupConfig config;
    F144CalendarDate date;
    uint32_t count;

    BindFake(&platform, &api, &fake);
    f144StartupConfigReset(&config);
    f144StartupConfigSetDebugEnabled(&config, true);

    Expect(
        f144StartupConfigSetFixedDateOverride(&config, 2026U, 1U, 3U),
        "fixed January date accepted"
    );
    for(count = 0U; count < 16U; ++count)
    {
        Expect(
            f144PlatformCalendarDate(&platform, &config, &date) &&
            SameDate(date, 2026U, 1U, 3U),
            "repeated fixed January query is deterministic"
        );
    }
    Expect(fake.queries == 0U, "override does not call native date provider");

    Expect(
        f144StartupConfigSetFixedDateOverride(&config, 2028U, 2U, 29U) &&
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2028U, 2U, 29U),
        "Gregorian leap-day override"
    );

    Expect(
        f144StartupConfigSetFixedDateOverride(&config, 2026U, 12U, 31U) &&
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 12U, 31U),
        "end-of-year override"
    );
    Expect(
        fake.queries == 0U,
        "all fixed-date queries remain independent of live machine date"
    );

    /* A test override can be evaluated without a bound native platform. */
    Expect(
        f144PlatformCalendarDate(NULL, &config, &date) &&
        SameDate(date, 2026U, 12U, 31U),
        "test override works without OS calendar access"
    );

    f144StartupConfigSetDebugEnabled(&config, false);
    Expect(
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "disabling developer mode returns to the normal date"
    );
    Expect(fake.queries == 1U, "native provider resumed after debug disabled");
}

static void TestInvalidOverridesAndBackends(void)
{
    F144Platform platform;
    F144PlatformApi api;
    FakeCalendar fake;
    F144StartupConfig config;
    F144CalendarDate date;

    BindFake(&platform, &api, &fake);
    f144StartupConfigReset(&config);
    f144StartupConfigSetDebugEnabled(&config, true);

    Expect(
        !f144StartupConfigSetFixedDateOverride(&config, 0U, 1U, 1U),
        "year zero rejected"
    );
    Expect(
        !f144StartupConfigSetFixedDateOverride(&config, 2026U, 0U, 1U) &&
        !f144StartupConfigSetFixedDateOverride(&config, 2026U, 13U, 1U),
        "out-of-range months rejected"
    );
    Expect(
        !f144StartupConfigSetFixedDateOverride(&config, 2026U, 4U, 31U) &&
        !f144StartupConfigSetFixedDateOverride(&config, 2026U, 2U, 29U),
        "invalid day and non-leap February rejected"
    );
    Expect(
        !f144StartupConfigSetFixedDateOverride(&config, 1900U, 2U, 29U) &&
        f144StartupConfigSetFixedDateOverride(&config, 2000U, 2U, 29U),
        "century leap-year validation"
    );
    f144StartupConfigReset(&config);
    f144StartupConfigSetDebugEnabled(&config, true);
    Expect(
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "rejected override falls back to live date"
    );

    /* Defensive validation also rejects a directly corrupted config. */
    config.fixed_date_override_enabled = true;
    config.fixed_date_override.year = 2026U;
    config.fixed_date_override.month = 13U;
    config.fixed_date_override.day = 2U;
    Expect(
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "invalid forced date cannot bypass validation"
    );

    SetFakeDate(&fake, 2026U, 2U, 29U);
    Expect(
        !f144PlatformCalendarDate(&platform, NULL, &date) &&
        SameDate(date, 0U, 0U, 0U),
        "invalid native date fails closed and clears output"
    );

    fake.succeeds = false;
    Expect(
        !f144PlatformCalendarDate(&platform, NULL, &date) &&
        SameDate(date, 0U, 0U, 0U),
        "failed native date request fails closed"
    );

    api.local_calendar_date = NULL;
    Expect(
        !f144PlatformCalendarDate(&platform, NULL, &date) &&
        SameDate(date, 0U, 0U, 0U),
        "missing native date callback fails closed"
    );
    Expect(
        !f144PlatformCalendarDate(NULL, NULL, &date) &&
        SameDate(date, 0U, 0U, 0U),
        "missing platform fails safely"
    );
    Expect(
        !f144PlatformCalendarDate(&platform, NULL, NULL),
        "null output pointer is rejected"
    );
}

static void TestExistingCommandLinePath(void)
{
    F144Platform platform;
    F144PlatformApi api;
    FakeCalendar fake;
    F144StartupConfig config;
    F144CalendarDate date;

    BindFake(&platform, &api, &fake);

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-date 2026-12-20 -debug", &config
        ) &&
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 12U, 20U),
        "existing -debug -date parsing drives fixed-date provider"
    );
    Expect(fake.queries == 0U, "CLI fixed date avoids native query");

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-date 2026-12-20", &config
        ) &&
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "CLI date without debug does not override production date"
    );

    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-debug -date 2026-12-20extra", &config
        ) &&
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "overlong malformed CLI date falls back safely"
    );
    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-debug -date 2026/12/20", &config
        ) &&
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "incorrect delimiter falls back safely"
    );
    Expect(
        f144Win32StartupConfigFromCommandLine(
            "-debug -date 2027-02-29", &config
        ) &&
        f144PlatformCalendarDate(&platform, &config, &date) &&
        SameDate(date, 2026U, 10U, 8U),
        "invalid leap-day command falls back safely"
    );
}

static void TestActualWin32LocalDateContract(void)
{
    F144Platform platform;
    F144PlatformApi api;
    F144CalendarDate date;

    memset(&api, 0, sizeof(api));
    memset(&platform, 0, sizeof(platform));

    api.local_calendar_date = f144Win32LocalCalendarDate;
    platform.api = &api;

    /* Do not assert a hard-coded real date or assume the runner's time zone. */
    Expect(
        f144PlatformCalendarDate(&platform, NULL, &date) &&
        f144CalendarDateValid(date.year, date.month, date.day),
        "native Win32 backend returns a valid current local calendar date"
    );
    Expect(
        date.year >= 1601U,
        "Win32 local date uses a calendar year rather than a monotonic tick"
    );
}

int main(void)
{
    TestNormalAndDisabledOverride();
    TestFixedDates();
    TestInvalidOverridesAndBackends();
    TestExistingCommandLinePath();
    TestActualWin32LocalDateContract();

    if(failures != 0)
    {
        printf("STAGE 4E CALENDAR TESTS: FAIL (%d)\n", failures);
        return 1;
    }
    puts("STAGE 4E CALENDAR TESTS: PASS");
    return 0;
}
