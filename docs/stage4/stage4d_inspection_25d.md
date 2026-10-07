# S4D-04 Pseudo-Isometric Inspection / Contents Presentation

## Scope

S4D-04 upgrades only the parent-object presentation inside the existing
Inspection/Contents screen.

The established interaction framework remains authoritative for:

- physical-item enumeration;
- seeded ordering;
- selection and scrolling;
- item names and descriptions;
- Inspect;
- Backspace layering;
- secure-cabinet access;
- parent/child IDs;
- gameplay effects.

No world geometry is changed.

## Catalogue audit

The generated Site currently contains 151 furniture records and 91 fixture
records. Across those records there are 32 canonical visual variants. Twenty
eight variants currently own one or more physical-item children in the
Inspection framework.

The catalogue includes:

- STANDARD_DESK;
- GDR_TERMINAL and IT_TERMINAL;
- WORKBENCH;
- SECURE_CABINET and NONSECURE_CABINET;
- BOOKCASE, SHELVING_FULL and SHELVING;
- BOARDROOM_TABLE, DINING_TABLE, COFFEE_TABLE and RECEPTION_TABLE;
- CHAIR;
- SOFA;
- TROLLEY;
- SERVER;
- FRIDGE;
- WORKTOP;
- SINK;
- COFFEE_MAKER;
- DOOR;
- SITE_DIRECTORY;
- NOTICEBOARD;
- PATCH_PANEL;
- MONITOR_BANK;
- SUPPRESSION_PANEL;
- FIRST_AID_KIT;
- KEY_CABINET;
- CABLE_RISER;
- PARTITION_WALL;
- WINDOW.

The renderer treats structural element type and canonical variant separately.
This matters for WALL_MOUNTED_ITEM records, whose structural type describes
placement while the variant describes the actual object.

## Renderer architecture

The enhanced renderer lives in floppy144_cabinet_25d.c/.h.

It uses a compact shared primitive vocabulary:

- clipped software pixels and Bresenham lines;
- convex four-point face filling;
- reusable pseudo-isometric boxes;
- parameterised footprint scaling;
- shared top/front/side surface colours;
- class-specific detail layered onto the shared geometry.

There are no bitmap, sprite-sheet or external image assets.

The reusable visual families are:

1. desks / terminal desks / workbench;
2. cabinets / bookcases / shelving;
3. tables / worktops / sink;
4. chairs;
5. trolley;
6. sofa;
7. server / fridge;
8. wall-mounted fixtures;
9. doors;
10. small appliance;
11. generic future-object fallback.

This avoids one bespoke drawing routine per authored parent.

## Dimensions

The generated parent record supplies the authored width and height/depth in
n2/n3. The pseudo-isometric footprint scales both axes independently, so a
6x4 desk is visibly different from a 2x6 bookcase and an 18x5 worktop retains
its unusually wide proportions.

Scaling is presentation-only. The source dimensions are read, never rewritten.

## Orientation

The flat game-data registry does not store the authored rotation because its
numeric slots are already used by runtime geometry/secure-code metadata.

The projection-neutral Site model does retain signed authored rotation and
authored width/height for rotated rectangles.

S4D-04 therefore adds Floppy144SiteRectForParentId(), which resolves an
Inspection parent back to its canonical Site rectangle using the same generated
geometry mapping already used by Site interaction targeting.

The 2.5D renderer uses that rectangle to:

- retain the signed rotation;
- choose the leading pseudo-isometric axis;
- mirror/swap the visible footprint for diagonal orientations;
- draw a compact orientation tick on the leading edge.

The Secretary Office diagonal desk is regression-checked at 135 degrees.

## Wall-mounted objects

WALL_MOUNTED_ITEM parents use a shallow panel renderer instead of the
freestanding box vocabulary.

Patch panels, monitor banks, directories/noticeboards, first-aid kits, key
cabinets, suppression panels and cable risers add compact category-specific
details to that shallow mounting.

This prevents a 1-unit-deep wall fixture from appearing as a giant cabinet.

## Main Office presentation unlock

The previous schematic representation remains active until the Main Office
room is reconstructed.

The gate is:

Floppy144RunStateRoomReconstructed(run_state, FLOPPY144_ROOM_MAIN_OFFICE)

After that presentation-only condition becomes true, the same CabinetState and
physical-item list are drawn with the enhanced pseudo-isometric parent view.

The unlock does not alter:

- parent availability;
- contents visibility;
- secure-cabinet state;
- item order;
- selected item;
- interactions;
- progression.

A Stage 3B regression renders the same Reception desk before and after Main
Office reconstruction and verifies that only the framebuffer presentation
changes while contents count and current selection stay identical.

## Fallback

Unknown/future parent variants are rendered as a bounded generic 2.5D box with
a crossed front face.

The draw call reports that fallback was used, allowing regression coverage to
prove the object still renders safely without requiring the game to understand
the new class first.

## Regression coverage

Dedicated S4D-04 presentation tests:

- enumerate every current canonical furniture/fixture variant;
- verify every current variant maps to a specialised renderer;
- ensure the renderer stays inside the left-hand Inspection presentation area;
- compare 6x4 and 2x6 dimensional silhouettes;
- verify an authored 135-degree rotation changes presentation;
- verify a wall-mounted patch panel differs from a freestanding worktop;
- verify the 18x5 worktop retains unusual dimensions;
- render empty, one-item and many-item states;
- verify selected markers remain visible in dense contents;
- exercise unknown-class fallback.

Stage 3B Cabinet regression additionally retains coverage for:

- secure locked/unavailable keypad state;
- exact secure-code unlock behaviour;
- empty/storage Contents availability;
- one/two-item chair limits;
- multi-item desks;
- seeded ordering;
- scrolling/selection;
- item inspection and detail;
- Backspace layering;
- recovered-item gameplay effects;
- all Corridor door containers;
- persistence of secure-cabinet unlocks;
- Site focus and interaction-range rules.

## Size strategy

S4D-04 is code-only and adds no binary art assets. Shared box/face geometry and
parameterised families are intentionally used to keep the executable impact
small compared with a per-object sprite or bitmap approach.
