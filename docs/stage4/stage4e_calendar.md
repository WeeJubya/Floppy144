# S4E-02: Platform-neutral local calendar with test override

## Previous state and ownership

Stage 4B already defined `F144CalendarDate` in
`include/f144_startup_config.h` (year, month, day) and validated the
developer-only `-debug -date YYYY-MM-DD` command-line option. Before S4E-02
the configured date was not consumed by a production calendar provider.

Stage 4B's `f144PlatformMonotonicMs()` is **elapsed timing**, used for
animation, autosave, lifecycle and initial recovery-seed generation. It is not
a civil/calendar date. S4E-02 preserves its callback and semantics.

There were no Core content-date calls, `SYSTEMTIME` accesses or C-runtime
`time()/localtime()` calendar reads to migrate.

## API

The existing `F144PlatformApi` gains a distinct `local_calendar_date`
callback, with Win32 implementation in `src/f144_win32_calendar.c`.
Only that native adapter calls `GetLocalTime()` and uses `SYSTEMTIME`.
This code is compiled into `Floppy144PlatformWin32`, not the Core.

Core content should use:

```c
F144CalendarDate today;
if(f144PlatformCalendarDate(platform, config, &today))
{
    /* today.year, today.month, today.day; no weekday or time-of-day */
}
```

`f144PlatformCalendarDate()` is implemented in portable
`src/f144_calendar.c`. It first checks for a validated override **and**
`f144StartupConfigDebugEnabled()`. With both present, the native backend is
never called. Otherwise it dispatches the native local-calendar callback,
validates the returned date, and returns it. With no valid date available
it returns `false` and zeros the output, rather than exposing garbage or
silently using the monotonic timer.

Gregorian validation is centralized in the previously existing date
validator, now public as `f144CalendarDateValid(year, month, day)`. This
same function is used by startup-config injection and live calendar results.
It rejects year zero, out-of-range months/days, invalid February 29 dates,
and implements century leap-year rules.

## Debug override and test strategy

- Ordinary launch: OS **local** date, re-queried on each request, allowing
  calendar rollovers during a long-running game.
- Developer launch: `Floppy144.exe -debug -date 2026-12-20` forces exactly
  that date for all content-provider requests.
- `-date` without `-debug` is ignored as in Stage 4B.
- Headless test: construct `F144StartupConfig`, enable debug, and call
  `f144StartupConfigSetFixedDateOverride(&config, year, month, day)`.
- Disabled/malformed/invalid overrides fall back to the native provider; if
  that provider fails or gives an invalid date, return `false` and zero output.
- Date strings use the existing bounded 10-character YYYY-MM-DD parser;
  no second parser, new setting or new persistent field was added.

The helper intentionally has **no weekday, time zone conversion, timestamp,
date arithmetic or game state persistence**. Date-themed content must use the
same current date when a stable per-run calendar snapshot is required; this
task does not freeze the real date across midnight or persist it.

## Regression

`tools/test_stage4_calendar.ps1` builds `stage4_calendar_tests.c` against
portable Core dispatch/config, the real Win32 calendar adapter and the
existing Win32 command-line adapter. Tests cover:

- valid real Win32 local date (without hardcoding a developer calendar day);
- normal native query and changing local date;
- fixed January, leap-day and end-of-year dates;
- programmatic deterministic repeated queries;
- disabled override, invalid Gregorian dates and malformed CLI strings;
- invalid/failed/unavailable native date backends and safe zero-output failure;
- zero native-date calls while an override is active;
- preservation of the distinct monotonic timing API;
- portable Core/source audit and inclusion in the Win32 build and CI.

No Stage 3 gameplay, generated documents, collections, triggers, interactions,
save payloads, player Settings, or profile structures are changed.
