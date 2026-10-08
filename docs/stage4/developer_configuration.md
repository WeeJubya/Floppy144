# FLOPPY//144 Stage 4 Developer Configuration

## Ownership boundary

Launch configuration is split into two layers.

- `src/f144_win32_startup_config.c` understands the raw Windows command-line tail.
- `src/f144_startup_config.c` stores and validates portable semantic configuration.
- Game code queries `F144StartupConfig`; it does not parse `-GDR-CinderEllie` or other
  Windows argument spellings.

Future launchers can populate the same portable configuration from platform
arguments, developer settings, test harnesses or another native mechanism.

## Supported Windows developer options

### `-GDR-CinderEllie` (exact case)

This replaces the former `-debug` spelling, which is **no longer
accepted**. All developer switches are matched by exact byte-for-byte
token comparison: alternative case variants do nothing.

Enables the existing terminal recovery-guidance mode.

Without this switch, normal production launch keeps document-level recovery
breadcrumbs hidden after the Site becomes available. The normal first-profile
DR-02/DR-03 onboarding hint remains player-facing behaviour and is not a debug
facility.

### `-GDR-Hathaway` (exact case)

Independent disposable **full-site visual inspection** mode. It wins over
CinderEllie in either argument order and explicitly disables ordinary
debug breadcrumbs, `-seed` and `-date` overrides when both switches
are present.

Select Initiate Session from the normal menu: the game starts directly
in the playable Site instead of requiring terminal reconstruction.
A RAM-only RunState inspection flag reveals **all collections and rooms**,
objects, capabilities and necessary access prerequisites. All secure
cabinets are unlocked. The Site Directory remains at its original
Corridor position `(35,56)` with size `6x1`. A fixed, interactive Grey
Door panel occupies the immediately adjacent south Corridor wall
`(29,56)`, size `4x1`, rather than using seeded placement. The temporary
Grey Door Republic scene is fully accessible and can be replayed
repeatedly without ever setting the normal run's discovery/completed bit.

The Win32 coordinator centrally prevents normal run-file saves,
autosaves, Profile updates and Settings writes while inspection is on.
Manual Record Session reports `INSPECTION MODE - SAVING DISABLED`.
Existing real saved sessions cannot be reinstated into the synthetic
world, and legacy migration is skipped. The completion transition is
suppressed so all restored visuals can be inspected freely. Closing
the application discards everything; a normal launch uses the original
progression, normal seeded Grey Door and ordinary saving.

No new persistent save or Profile schema field is introduced. The
`hathaway_inspection` flag is never encoded by the V3 run codec.
All visual changes are ordinary render/interaction consequences of the
ephemeral all-restored RunState, not permanent records.

### `-seed <non-zero uint32>`

Provides a deterministic recovery seed. This option is applied only when
`-GDR-CinderEllie` is also present.

Ordinary play continues to derive the recovery seed from the platform monotonic
clock exactly as before.

### `-date <YYYY-MM-DD>`

Provides a validated fixed Gregorian calendar date. This option is applied only
when `-GDR-CinderEllie` is also present.

S4E-02 connects this existing override to
`f144PlatformCalendarDate(platform, config, &date)`. When debug is enabled,
the validated fixed date is returned without querying Windows. Without debug,
the provider reads the OS **local** calendar date, not the monotonic timer.
Content should call the provider, not Win32 time functions. The Stage 4E seasonal/date-aware features consume this date provider
without involving Hathaway.

## Current debug/diagnostic audit

Production CinderEllie functionality remains terminal recovery breadcrumbs,
plus the existing explicit seeded/date test overrides. Hathaway is a separate
visual inspection mode and does not enable CinderEllie facilities.

There is no separate CinderEllie diagnostic screen, secret-document registry,
developer keyboard shortcut or new production logging mode.

Headless and regression hooks live under `tools/` and are separate test
executables/scripts. They can inject `F144StartupConfig` directly and do not need the
player's real command line or environment.

The older F144 runtime contains unconditional stderr diagnostics for certain
runtime/rendering failures. Those messages are not controlled by `-GDR-CinderEllie` and
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
