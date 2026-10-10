# BUG FIX 12: Orthogonal Site furniture audit

## Authority and scope

The canonical runtime Site layout is `data/floppy144_game_data.json`,
specifically `site_layout_source.rooms[].geometry` and the matching
`furniture[].geometry`. The generated runtime layout, flattened furniture
registry and compiled Site rectangles must follow it. The legacy
`site_layout.jsonc` and `site_layout.generated.jsonc` remain matching
pre-transform design references.

**Site-wide audit:** 11 rooms; 151 furniture records; 91 fixtures; 12
non-cardinal world placements found and corrected. No non-cardinal
world-geometry rotation remains. The original 634 physical-item owners and
progression logic are unchanged.

## Every non-cardinal authored placement corrected

Coordinates are runtime Site top-left (X,Y) in whole units.

| Room | Object | Prior rotation | New X,Y | New rotation |
|---|---|---:|---:|---:|
| Staff Room | Dining table | 45 | 7,34 | 0 |
| Staff Room | Chair 02 (east side) | 135 | 14,36 | 90 |
| Staff Room | Chair 03 (west side) | 45 | 4,36 | 270 |
| Staff Room | Chair 04 (north side) | 225 | 9,31 | 0 |
| Staff Room | Chair 05 (south side) | 315 | 9,41 | 180 |
| Secretary's Office | Desk | 135 | 45,36 | 0 |
| Secretary's Office | Chair 01 | 135 | 47,42 | 180 |
| Director's Office | Desk | -45 | 43,8 | 0 |
| Director's Office | Chair 01 (Director) | -45 | 46,4 | 0 |
| Director's Office | Chair 02 (visitor) | -45 | 43,14 | 180 |
| Director's Office | Chair 03 (visitor) | -45 | 46,14 | 180 |
| Director's Office | Chair 04 (visitor) | -45 | 49,14 | 180 |

The Director's desk occupies X43..51, Y8..12, with the Director's chair
behind it at X46..48, Y4..6. The three visitor chairs are in an
orthogonal row at Y14..16. There is at least two Site units of clear space
between the desk and either seating row. This leaves an approach around
the seating and through the south door (SECR_DIR).

The Secretary's desk is 6x4 at X45,Y36, with its chair separated to the
south at X47,Y42. The Staff Room dining table remains 6x6 at X7,Y34;
its four chairs now surround rather than overlap it.

## Checks and intentional exclusions

- No furniture was removed and no interaction source or physical-item
  ownership was reassigned.
- Every revised footprint stays inside its room and intersects no other
  solid furniture footprint.
- The Director's desk, Director chair and three visitor chairs remain
  reachable through free floor cells from the south entrance, verified by
  an independent 1-unit grid flood-fill.
- Source and generated Site geometry, legacy inverse-transform layout,
  and emitted `SITE_GEOMETRY`/`SITE_ROTATED_GEOMETRY` rows were checked
  for correspondence.
- Functional rotations remain **supported but were not altered** for
  player animation, dynamic door behaviour and 2.5D Inspection projection.
  Existing tests still check that cardinal parent orientation changes
  Inspection presentation. No authored world placement needs a diagonal
  exception.

The Stage 3B native Site regression now rejects any non-cardinal compiled
world rectangle and checks all twelve specific revised furniture footprints.
The Stage 4D PowerShell integration audit checks canonical and generated
data parity, legacy inverse transforms and placement collision constraints.

**Release gate:** native MSVC tests, room rendering/restore checks, full
progression/collision tests and `cleanbuild.ps1` require Windows execution;
source-level checks alone do not constitute release sign-off.
