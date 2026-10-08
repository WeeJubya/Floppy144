# S4H - Case-sensitive command-line modes and temporary Site inspection

## Audit before edits

- Windows argument parsing is owned by `src/f144_win32_startup_config.c`
  and `f144Win32StartupConfigFromCommandLine`. It tokenises WinMain's
  command-line tail and compares token length and bytes, case-sensitively.
- `-debug` previously activated the portable `debug_enabled` setting.
  It enabled terminal recovery breadcrumbs and allowed `-seed` and `-date`
  developer overrides; the Stage 3 first-profile hint is independent.
- Debug mode was not persisted in RunState, Settings or Profile. Terminal
  configuration reads the semantic flag from `global_config`.
- The legacy spelling existed in the startup parser, the Stage 4 config
  regressions and the Stage 4B developer documentation; the build's
  `-build debug` setting is a distinct compile configuration and is retained.
- The original end-case Grey Door in Stage 4G was seeded and strictly
  one-shot, with collision-free drawing and a transient framebuffer scene.

## Revised exact switches

- `-GDR-CinderEllie`: retains the previous developer breadcrumb plus
  seeded/date override contract, without changing the terminal.
- `-GDR-Hathaway`: independent ephemeral full-Site visual inspection.
  Case-sensitive and takes precedence over CinderEllie in either order.
- `-debug` is no longer a recognized alias. Mixed-case variants are inert.
  All parsing remains at the platform configuration boundary, not in Core.

## Hathaway visual snapshot

`Floppy144RunStateEnableHathawayInspection` sets a RAM-only
`hathaway_inspection` flag and marks every generated collection, room,
runtime object and relevant triggering/capability prerequisite available.
Secure cabinets are unlocked by setting the existing 32-bit cabinet mask.
The coordinator hydrates the normal world from this synthetic state and
starts the player immediately in 2D Site exploration after Initiate Session.

Room-transition progression gates are bypassed only while the RAM flag is
true. Physical movement and collisions still pass through the original
canonical Site geometry. No extra room, wall or generated object is added.
Normal player run creation does not set this flag.

The Corridor Site Directory remains at `(35,56)` with dimensions `6x1`.
Hathaway renders an additional Grey Door panel on the neighbouring wall
`(29,56)` of size `4x1` with interaction stance `(31,55)`.
A runtime check fails closed if the corridor surface/stance becomes invalid
or another non-floor authored object overlaps it.
This override does not use normal Stage 4G seeded placement and does not
write `grey_door_state`.

Hathaway permits opening and replaying the existing Developer encounter:
the normal runtime completion call is suppressed, so the Door is
repainted after every return. In ordinary mode Stage 4G's irreversible
one-shot behaviour and same-seed completed-autosave protection remain
unchanged.

## Save/profile/completion isolation

The main coordinator centralises all regular SaveRunState, SaveProfile
and SaveSettings requests behind a Hathaway-aware no-write boundary.
Manual Record Session expressly refuses with an inspection-only message;
automatic/lifecycle saving cannot write even during the Grey Door
encounter. Profile recovery counts do not increment, discovery merging
does not run, legacy file migration does not write, and previously
saved normal sessions cannot be reinstated into an inspection world.
Hathaway-only state has **no corresponding field in the V3 save payload**.
Normal game completion routing is suppressed so the player can continue
exploring the fully reconstructed environment.

Because all collections are synthetically restored, the inspection HUD
may show a total above the normal reconstruction capacity. This is
deliberate developer-only state, never a production saved recovery.

## Gates

- `tools/stage4_config_tests.c`: exact casing, legacy alias off, unknown
  arguments off, order-independent mode precedence, seed/date gating.
- `tools/stage4_persistence_paths_tests.c`: all collections/rooms,
  cabinet unlocks, fixed door geometry/proximity, Developer scene replay,
  normal run restart and absence of the new RAM flag from decoded saves.
- Existing Stage 3/4 integration, Site, cabinet, completion, software
  framebuffer, generated-data, Core/Win32 and Release size gates remain
  authoritative and are not weakened.

No additional Easter-egg content is introduced.
