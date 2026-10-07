# FLOPPY//144 Stage 4 Developer Configuration

## Ownership boundary

Launch configuration is split into two layers.

- `src/f144_win32_startup_config.c` understands the raw Windows command-line tail.
- `src/f144_startup_config.c` stores and validates portable semantic configuration.
- Game code queries `F144StartupConfig`; it does not parse `-debug` or other
  Windows argument spellings.

Future launchers can populate the same portable configuration from platform
arguments, developer settings, test harnesses or another native mechanism.

## Supported Windows developer options

### `-debug`

Enables the existing terminal recovery-guidance mode.

Without this switch, normal production launch keeps document-level recovery
breadcrumbs hidden after the Site becomes available. The normal first-profile
DR-02/DR-03 onboarding hint remains player-facing behaviour and is not a debug
facility.

### `-seed <non-zero uint32>`

Provides a deterministic recovery seed. This option is applied only when
`-debug` is also present.

Ordinary play continues to derive the recovery seed from the platform monotonic
clock exactly as before.

### `-date <YYYY-MM-DD>`

Provides a validated fixed Gregorian calendar date. This option is applied only
when `-debug` is also present.

S4B-07 only establishes the configuration route. No current production system
reads the platform calendar for gameplay, so this override intentionally has no
player-visible effect yet. Stage 4E date-sensitive regression work can query it
without adding a Windows dependency.

## Current debug/diagnostic audit

Production debug functionality currently consists only of terminal recovery
breadcrumbs controlled by `-debug`.

There is no separate debug-only document registry, debug UI screen, diagnostic
overlay, developer keyboard shortcut, fixed seed/date override from the old
baseline, or dedicated production logging mode.

Headless and regression hooks live under `tools/` and are separate test
executables/scripts. They can inject `F144StartupConfig` directly and do not need the
player's real command line or environment.

The older F144 runtime contains unconditional stderr diagnostics for certain
runtime/rendering failures. Those messages are not controlled by `-debug` and
remain unchanged because S4B-07 is configuration normalisation rather than a
logging-system rewrite.

## Test injection

Tests can construct `F144StartupConfig`, call `f144StartupConfigReset()`, then use:

```c
f144StartupConfigSetDebugEnabled(
    &config,
    true
);

f144StartupConfigSetRecoverySeedOverride(
    &config,
    144U
);

f144StartupConfigSetFixedDateOverride(
    &config,
    2026U,
    12U,
    24U
);
```

This is the intended Stage 4E route for deterministic seed/date fixtures.
