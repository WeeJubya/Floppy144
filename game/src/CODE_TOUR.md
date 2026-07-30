# Floppy//144 commented source tour

This folder contains the same game-source code with explanatory comments added. The comments are deliberately aimed at understanding the architecture rather than narrating every assignment. C comments are removed by the compiler, so they do not add to the executable size.

## Start here

Open the files in this order:

1. `floppy144_collection.h` - stable IDs for archive collections.
2. `floppy144_world.h` - persistent facts about the current reconstruction.
3. `floppy144_main.c` - screen state machine, Win32 input and top-level routing.
4. `floppy144_terminal.c` - collection selection and restoration.
5. `floppy144_catalogue.c` - generated records, authored documents and evidence detection.
6. `floppy144_office.c` - collision, interactions and physical reconstruction.
7. `floppy144_draw.c` - the tiny rectangle and bitmap-text renderer.
8. `floppy144_recovery.c` - the opening recovery interface.

## What owns what?

`floppy144_main.c` owns the active screen and coordinates the modules. It is the only place where a catalogue evidence result is translated into a specific world-state flag.

`Floppy144WorldState` owns facts that should remain true when the player leaves a screen, such as `hr02_restored` or `fa03_suppression_service_read`.

`Floppy144TerminalState` owns temporary terminal UI state, such as which collection is selected and whether the detail panel is open.

`Floppy144CatalogueState` owns temporary catalogue UI state, such as the selected record, scroll position and whether a document is open.

`Floppy144Player` owns only the player's logical-canvas position.

## Screen flow

```text
RECOVERY
  Enter once: recovery begins
  Enter again: OFFICE

OFFICE
  E near terminal: TERMINAL
  E near evidence object: show inspection notice
  Escape: RECOVERY

TERMINAL
  Enter on collection: open details
  Enter on unrestored collection: restore it
  Enter on restored optional collection: CATALOGUE
  Escape: close details, then return to OFFICE

CATALOGUE
  Enter: open selected document and possibly record evidence
  Escape: close document, then return to TERMINAL
```

## Example: reading FA-03 record 063

```text
Win32 sends VK_RETURN
        |
        v
floppy144_main.c calls Floppy144CatalogueOpenDocument
        |
        v
Floppy144CatalogueSelectedProvidesEvidence checks:
    collection == FA-03
    selected_index == 62  (visible record 063)
        |
        v
global_world.fa03_suppression_service_read = true
        |
        +--> terminal list displays EVIDENCE
        +--> terminal details display STATUS: EVIDENCE FOUND
        +--> future office code can unlock suppression-panel inspection
```

## Example: HR-02 desk evidence

```text
Restore HR-02
        |
        v
Office labels and desk props appear
        |
        v
Read HR-02 visible record 038
        |
        v
hr02_desk_reallocation_read becomes true
        |
        v
Standing near Desk 01 or Desk 04 shows an inspection prompt
        |
        v
E displays the reconstructed meaning of that desk detail
```

## Drawing model

Every screen creates a `Floppy144Surface` that points at river2D's software backbuffer. The game then draws a complete frame using only:

- clear surface
- filled rectangle
- rectangle outline
- 5x7 bitmap text

The logical canvas is 640x360. river2D presents it in a 1280x720 Win32 window, preserving crisp two-times scaling.

## Collision model

The office keeps a table of solid furniture rectangles. A proposed player position is valid only when it remains within the room and does not overlap any obstacle.

Movement tests horizontal and vertical axes separately. This lets the player slide along furniture instead of stopping completely when one axis is blocked.

Interaction zones are separate from collision. Desk interaction expands the relevant obstacle by eight pixels, creating a small halo without enlarging the solid desk itself.

## Catalogue generation

The catalogue does not store 100 complete titles per collection. It combines:

- collection-specific subject vocabulary
- shared document-form vocabulary
- deterministic index arithmetic

The same collection and record index always generate the same ID and title. Authored evidence records override this procedural output with stable IDs and titles.

## Safest places to extend next

- Add collection IDs in `floppy144_collection.h`.
- Add persistent restoration and evidence fields in `floppy144_world.h`.
- Add terminal labels and restoration handling in `floppy144_terminal.c`.
- Add catalogue vocabulary, authored record positions and renderers in `floppy144_catalogue.c`.
- Add physical objects and proximity functions in `floppy144_office.c`.
- Route new evidence flags in the catalogue Enter block in `floppy144_main.c`.

Keep permanent build configuration in Premake rather than editing generated `.vcxproj` files.
