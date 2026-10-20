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

## BF40 Staff Room worktop and fridge continuation

The initial shared BF40 fix already addressed rear-leg occlusion, solid chair
backs and terminal-marker priority. This continuation replaces the remaining
thin table-style WORKTOP inspection representation with a fitted 86-pixel-high
cabinet unit, four projected cupboard doors, a recessed sink/tap and a raised
coffee machine. The appliance meshes and physical-item highlights all use the
same projected top quad. Contents are allocated a clear central strip, separate
from the left-hand sink and right-hand coffee maker, without changing item data,
selection, saving or ordinary Site geometry.

The FRIDGE previously shared a short 112-pixel server silhouette and painted
its door lines at raw framebuffer coordinates. It now has a dedicated 158-pixel
open-fridge container. Recess, liner, three shelves and lower drawer are
projected on the visible front face, and child-item markers inherit the same
158-pixel projection. The server remains a separate unchanged family.

The existing BF40 furniture tests remain intact. The Staff Room test now checks
both appliances, no marker/appliance colour loss with twelve contents, selection
changes, all four cardinal rotations, fridge shelf/cavity/drawer colours,
full-height geometry and right-hand Contents gutter clipping.

Visual sign-off remains required for the Worktop and Fridge in live Inspection
at every supported screen scaling setting and for their ordinary Site/world
models, which are deliberately not changed by this rendering-only work.

## Follow-up: chair backs, server rack and FM-23

The chair back's principal face is now opaque and taller, preserving its shallow
2.5D end/top surfaces and existing seat/leg painter order. Server bays now
project onto the front quad instead of drawing free-standing horizontal screen
rectangles; the 4x4 footprint and content selection are preserved, while the
rack height and child front mapping match at 156 pixels. FM-23 filters its
isometric plan to structural partitions, doors, windows and floors, and centres
the footprint of recovered rooms in the display. No furniture registry or
interaction rule is modified by these rendering changes.

## Chair back redesign: October visual acceptance

The earlier BF40 solid backrest was an uninterrupted upright quadrilateral, so
the inspection view looked like a translucent partition attached to a stool.
The replacement uses a shorter inset back cushion, an explicit gap over the
seat, twin posts and solid five-pixel frame legs. All positions are derived
from the same directional projected seat corners, rather than using screen
rectangles or changing the canonical furniture geometry. The rear posts and
legs are painted before the opaque 10-pixel seat, maintaining occlusion. The
BF40 inspection regression now covers real 2x2 chairs at eight rotations,
framed-cushion colours, uprights, leg volume, the seat, item highlight and
the Contents gutter. Live Windows screenshots remain the visual acceptance
gate for Director and Staff Room chairs.
