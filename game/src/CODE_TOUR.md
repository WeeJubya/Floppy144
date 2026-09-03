# Floppy//144 commented source tour

This folder contains the game source with architecture-oriented comments. C comments do not add to the release executable size.

## Start here

Open the files in this order:

1. `floppy144_collection.h` - stable archive collection IDs.
2. `floppy144_run_state.h` - authoritative state for one recovery session.
3. `floppy144_world.h` - hydrated runtime view used by legacy/current world-facing systems.
4. `floppy144_main.c` - screen state machine, Win32 input and top-level routing.
5. `floppy144_site.h` / `floppy144_site.c` - generated Site geometry, collision and movement.
6. `floppy144_site_rooms.h` / `floppy144_site_rooms.c` - floor-owned room membership and camera/view regions.
7. `floppy144_site_2d.c` - Site Exploration shell, scrolling viewport and procedural 2D furniture rendering.
8. `floppy144_site_object.c` - Site-space bridge to registered object interactions/visibility.
9. `floppy144_terminal.c` - collection selection and restoration.
10. `floppy144_catalogue.c` - generated records, authored documents and evidence detection.
11. `floppy144_draw.c` - small rectangle and bitmap-text renderer.
12. `floppy144_recovery.c` - opening recovery interface.

## Site source of truth

The human-authored physical Site lives in `site_layout.jsonc` at the project root.

Development pipeline:

```text
site_layout.jsonc
        |
        v
tools/site_compiler.c
        |
        v
game/src/floppy144_site_generated.def
        |
        +--> floppy144_site.c
        +--> floppy144_site_rooms.c
        +--> Site rendering / collision / room topology
```

`floppy144_site_generated.def` is generated build data. Do not hand-edit it. Change `site_layout.jsonc` and regenerate instead.

Room rules:

- `FLOOR_A` / `FLOOR_B` / `FLOOR_C` / `FLOOR_D` own walkable room membership.
- authored room `regions` define camera/view extents and may overlap on shared boundaries.
- doors are physical shared geometry with explicit bidirectional room topology.
- windows, walls and other shared boundary structures do not own a room floor.

## What owns what?

`floppy144_main.c` owns the active screen and coordinates modules.

`Floppy144RunState` is authoritative for one recovery session. It owns the player's canonical Site position, reconstructed-room bits, object visibility/access state, restored collections, triggers, interactions, evidence, notebook state, capabilities and projection state.

`Floppy144WorldState` is a runtime world-facing view hydrated from `Floppy144RunState` where existing systems still need it. New persistent gameplay state should normally be added to RunState rather than creating another parallel owner.

`Floppy144TerminalState` owns temporary terminal UI state, such as selection and detail-panel state.

`Floppy144CatalogueState` owns temporary catalogue UI state, such as selected record, scroll position and document-open state.

The player has no separate prototype Office-player object. Canonical position lives in `Floppy144RunState.player_site_x` / `player_site_y` using Site fixed-point coordinates.

## Site Exploration flow

```text
RECOVERY
  Enter once: recovery begins
  Enter again: SITE EXPLORATION

SITE EXPLORATION
  WASD / arrows: movement through generated Site geometry
  E near eligible object: interaction
  valid reconstructed doorway: move into adjoining room
  blocked/unreconstructed doorway: remain in current room
  Escape: RECOVERY

TERMINAL
  Enter on collection: open details
  Enter on unrestored collection: restore it
  Enter on restored optional collection: CATALOGUE
  Escape: close details, then return to SITE EXPLORATION

CATALOGUE
  Enter: open selected document and possibly establish evidence
  Escape: close document, then return to TERMINAL
```

## Site drawing model

The logical canvas is 640x360. Site Exploration keeps the fixed Stage-1 shell and draws the scrolling world only inside its inset viewport.

Rendering order is broadly:

```text
room floor
room perimeter
shared doors
windows / structural geometry
furniture / fixtures
player
fixed Site Exploration UI
```

The camera uses a fixed Site zoom, follows the player and clamps to the current room view extent with a small gutter so room edges remain visible.

Furniture geometry comes from the generated Site data. `floppy144_site_2d.c` adds procedural visual detail so the JSONC floorplan remains physical/layout data rather than sprite artwork.

## Collision model

Collision is handled in `floppy144_site.c`.

The player's visible 2D character is deliberately larger than the movement footprint. The movement footprint is a compact ground-space box so the player can pass through authored doorways while retaining the intended character proportions on screen.

A valid player footprint must remain over walkable floor/door geometry and must not intersect blocking furniture or structure. Horizontal and vertical movement are resolved independently so the player can slide along obstacles.

Room classification is derived from floor ownership. Door thresholds use generated door topology to resolve the transition between adjoining rooms.

## Object interaction and reconstruction visibility

`floppy144_site_object.c` bridges canonical Site coordinates to the existing registered-object interaction system.

Ordinary Site furniture is scenery and appears with its reconstructed room. Geometry that has migrated to a registered gameplay object can additionally obey object visibility/reveal/collection state.

Optional `object` hooks authored in `site_layout.jsonc` are future-facing metadata. They do not require every named chair, cabinet or fixture to become a gameplay object immediately.

## Example: reconstruction opening the Site

```text
restore collection
        |
        v
RunState collection bit changes
        |
        +--> required room reconstruction bits are set
        |
        +--> WorldState is hydrated as needed
        |
        +--> Site renderer can display the reconstructed room
        |
        +--> movement may cross doors into that room
```

## Catalogue generation

The catalogue does not store 100 complete titles per collection. It combines collection-specific vocabulary, shared document-form vocabulary and deterministic index arithmetic. The same collection and record index always generate the same ID/title, while authored evidence records override procedural output where required.

## Safest places to extend next

- Add stable collection IDs/metadata through the collection registry files.
- Add persistent session facts to `Floppy144RunState`.
- Add authored physical Site geometry to `site_layout.jsonc`, then regenerate the Site data.
- Add Site collision/movement behaviour in `floppy144_site.c`.
- Add room/topology queries in `floppy144_site_rooms.c`.
- Add Site-space interaction bridging in `floppy144_site_object.c`.
- Add procedural 2D presentation in `floppy144_site_2d.c`.
- Add terminal/catalogue behaviour in their existing modules rather than duplicating state in the Site layer.

Keep permanent build configuration in Premake rather than editing generated `.vcxproj` files.
