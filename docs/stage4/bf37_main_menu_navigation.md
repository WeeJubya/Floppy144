# BF37: Main Menu hidden-option navigation

Branch: `development/Stage-4` | Scope: GDR Session Control only.

## Root cause

BF28 preserved five fixed row indices but reused rows 3 and 4 for two
different pairs of actions. Drawing and Enter correctly called
`Floppy144MainMenuVisibleOption`, while the Up/Down loop checked the
unmapped row enum. Profile and Settings were always enabled, so it could
select their rows while the visible Save/Load actions were disabled.

## Correction

- `Floppy144MainMenuRowSelectable` is the shared availability rule for
  rendering, focus, and activation. It rejects non-row action enum values.
- `Floppy144MainMenuSelectRow` wraps across the five rows, skips disabled
  visible actions, and retains a valid selection when revalidating after
  a carousel switch. When a middle row becomes disabled it searches
  forward (wrapping) for the next selectable row.
- The coordinator repairs focus immediately when a new slide is accepted.
  Enter finishes the existing 240 ms animation, and refuses activation
  if a stale/disabled selection is detected. Rapid horizontal repeats
  remain blocked until that animation finishes.
- No layout, copy, operator identity, animation geometry, persistence or
  gameplay state was modified.

## Regression and verification

The existing `tools/test_stage4_reinstate_presentation.ps1` suite now
also audits coordinator wiring. The renderer's compiled C regression
(`tools/stage4_reinstate_presentation_tests.c`) checks all availability
and carousel combinations, Up/Down wrapping, disabled items, preserving
or repairing focus on switches, and 128 rapid switch/move sequences.
Existing BF28 framebuffer and animation checks remain in the same suite.

Run on Windows with:
```powershell
.\tools\test_stage4_reinstate_presentation.ps1
.\tools\test_stage4f_presentation_journey.ps1
.\run.ps1 -build release
```

The full Stage 4 Windows CI Release gate also builds with warnings as
errors and enforces the 1,474,560-byte executable size limit. Manual
acceptance should check visible highlight behaviour while repeatedly
pressing Left/Right/Up/Down and Enter before the slide settles.
