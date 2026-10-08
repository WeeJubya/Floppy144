# S4G-02: Deterministic corridor wall overlay

## Candidate discovery

`floppy144_grey_door.c` derives candidate door planes from the two
**generated Corridor floor rectangles**. Four perimeter orientations
are scanned in stable source/edge/cell order, with a 4U long by 1U
deep doorway (the same physical footprint as ordinary boundaries).
No world coordinates are authored specifically for the Grey Door.

Each candidate is accepted only if:

1. all four doorway cells are outside Corridor floor, each immediately
   adjoins Corridor ground and the geometry is inside site bounds;
2. no authored non-floor rectangle intersects its **2U halo**, including
   conventional doors, windows, Site Directory, room plaques,
   noticeboards, first-aid boxes, key cupboards, monitors, wall-hangings,
   furniture and concealed objects;
3. the intended interaction stance is inside Corridor, its **actual
   player collision footprint** is legal and the 0.5U Site proximity
   check succeeds;
4. the wall slot is unique.

Candidate enumeration is bounded (256). An empty/overflowing inventory
returns no Door and cannot change geometry. No unverified fallback is
permitted. The generated Site geometry remains authoritative before,
during and after the encounter.

## Deterministic seed

`Floppy144RunStateGreyDoorPlacementSlot(state,candidate_count)`
uses Stage 4E `Floppy144VariationRange` with its independent
`GREY_DOOR / CORRIDOR_WALL_V1` namespace. The cache is built only
from immutable generated geometry. For the same seed and Site geometry,
the same valid candidate is selected in every process and after every
save/reload. No coordinates or additional save fields are required.

Do not reorder or reinterpret candidates in later releases without
handling the effects on existing saves. A changed authored Site plan
must pass the whole-candidate audit before release.

## Rendering and controls

The existing 2D Site renderer draws the Door **after** its standard
wall shell and before furniture. Its muted grey panel, low-relief
surround and modest handle are distinct without looking like a portal.
It has no ordinary room plaque.

The common Site `A ACCESS` / `I INSPECT` bindings remain authoritative.
Near the Door the standard footer advertises both actions.
`I` reports `GREY DOOR.`, and `A` enters the completed S4G-03
Grey Door Republic vignette. The temporary office uses logical input and
has no permanent collision or world-geometry override.

Placement never calls `Floppy144RunStateGreyDoorComplete`. The
coordinator marks the completed state **only after** the entire encounter
has finished and the original Corridor foot point is verified. S4G-04
then prevents any repeat visit and verifies the restored wall pixel-for-pixel.
Neither interactive branch mutates collection, notebook, evidence,
capacity, Profile or completion history.

## Tests

The normal Stage4 CI builds `stage4_persistence_paths_tests.c`,
which enumerates every candidate and checks the exact geometry halo,
collision-free stance, fixed-seed choices, variation independence,
physical save/reload, interaction range, completion removal, and
unchanged Site rectangle inventory. The dedicated Stage4G source audit
checks integration and architecture; the Windows Core/Win32/Release
build and final 1,474,560-byte gate remain active.

The candidate list deliberately does **not** include anything adjacent
to the Corridor Site Directory where clearance cannot be proven.
