# BF40: ISO furniture container depth and marker priority

## Scope and root cause

The shared 2.5D Inspection renderer (`floppy144_cabinet_25d.c`) drew all
chair/desk/table legs **after** its solid furniture bodies. The rear leg at
the projected back corner consequently painted through the tabletop or seat.
Chair backs were four independent framebuffer lines rather than a projected
solid. Terminal monitors were composited after the inspection child markers,
which hid the currently selected highlight wherever they overlapped.

## Changes

- Draw all support geometry behind the shared opaque box. Its projected top,
  front and side faces mask the hidden sections without shortening supports or
  changing furniture dimensions. Applies to chairs, all desk/terminal/workbench
  variants, and tables/worktops/sinks.
- Replace the line-art chair back with a shallow, filled, four-edged upright
  following the authored chair orientation; draw the seat in front of its
  attachment. All faces reuse the existing quad primitive and colour palette.
- Draw terminal monitor hardware before UI inspection child markers. These
  markers intentionally win the final composite when a screen overlaps one;
  their projection, selected colour and interaction behaviour are unchanged.

The Site/world renderer, authored furniture geometry, canonical physical-item
catalogue, save data, and interaction rules are not modified.

## Regression verification

`tools/stage4_inspection_25d_tests.c` covers rear-leg pixel occlusion for
chair/desk/table cardinal rotations and the terminal family; solid backrest
front/top faces at each cardinal rotation; foreground selection pixels against
the terminal screen for both GDR and IT terminals; and left-panel clipping.

Run `tools/test_stage4_inspection_25d.ps1`. On branch pushes,
`.github/workflows/bf10-inspection-ci.yml` also executes this suite, Stage 3B
cabinet regressions, Release MSVC build with warnings treated as errors, and the
1,474,560-byte executable limit. Existing PPM inspection captures can be
compared before/after for visual sign-off.

## Manual visual acceptance

Review chairs from all cardinal rotations, several desk/table aspect ratios,
GDR/IT terminal desks with dense contents and changing selected items, and the
reconstructed world plus Inspection transition. Pay special attention to support
visibility below the body, chair back thickness, monitor overlap and clipping.
Automated pixels and CI captures are not a substitute for Windows gameplay
visual confirmation.
