# BF25 / BF27 / BF28 / BF33 rendering and inspection review

Source reference: user-supplied `Characters(1).pptx` (two slide images), original five-entry Main Menu screenshot, and original Grey Door Republik logo PNG. No third-party/generic visual reference substituted.

## BF25: reference matrix

| Direction | Type A artwork | Type B artwork | Procedural fixture |
| --- | --- | --- | --- |
| Front (Down) | Blue suit, deep red tie, white V lapels, brown hair, black shoes | Charcoal jacket and skirt, cyan scarf, loose left-side hair, visible lower legs, black shoes | `bf25_type_a_front_idle.ppm`, `bf25_type_b_front_idle.ppm` |
| Rear (Up) | Blue suit, brown back of head, parallel trouser legs | Dark skirt, cyan collar/scarf behind hair, brown head, pale calves | `bf25_type_a_rear_idle.ppm`, `bf25_type_b_rear_idle.ppm` |
| Right | Blue narrow profile, brown rear skull, two sleeves, black shoe | Slim dark profile, pale legs, brown trailing hair | `bf25_type_a_right_idle.ppm`, `bf25_type_b_right_idle.ppm` |
| Left | Mirrored blue side silhouette with face opposite | Mirrored dark side silhouette, asymmetrical hair | `bf25_type_a_left_idle.ppm`, `bf25_type_b_left_idle.ppm` |

Each idle frame has a matching `_walk.ppm` capture. The leg projection offsets bob from planted feet, keeping the ground plane stable; walking foot lift remains intentional. The same core renderer is invoked in the Grey Door vignette, including the Developer's costume variant. Saved body-style values, movement speeds, collision geometry and action mappings are unchanged.

## BF27: terminal

The central large heading moves up within the existing panel. The status header at y=5 remains unchanged. A default indicator reads `terminal->default_record_collection_valid` and `Floppy144CollectionGet(...)->code`, falling back to NONE, placed on an independent baseline to avoid clipping the prompt. Captures `bf27_terminal_none.ppm` and `bf27_terminal_dr01.ppm`.

## BF28: five-row Main Menu

Original screenshot is authoritative: Initiate, Return to Site, Profile, Settings, Terminate. The former manual Record and Reinstate enum values stay defined outside the five selectable rows so persistent storage procedures remain compiled and autosave remains unchanged. The row coordinates and outer UI geometry remain fixed. Capture: `bf28_main_menu.ppm`.

## BF33: mark

One vector logo function draws the pale circle, grey architectural facade, black door shapes, muted red element, and GREY DOOR / REPUBLIK wordmark in a landscape or square composition. The Credits and Grey Door office call the same renderer. Captures `bf33_credits.ppm` and `bf33_grey_door.ppm`.

## Manual acceptance still required

A build/automated pixel test does not prove visual equivalence with the PowerPoint characters, original screenshot, or supplied logo. Compare the exact user reference originals to corresponding 640x360 captures, especially side profiles, Type B's scarf and hair, Main Menu row spacing and alignment, and the architectural symbol proportions. Check the actual CRT-presented build for header alignment, command indicator overlap during long input, the Credits intro replay and the entire Grey Door sequence. Keep manual visual approval separate from CI.

## BF28 follow-up: original Left/Right save/load access

The original five visible rows still provide Initiate, Return to Site,
Operator Profile, Settings and Terminate. The retained manual persistence
actions are available **from any highlighted Main Menu row**, without pages:

- **Left**: load/reinstate the recorded session (manual checkpoint first,
  autosave fallback and existing Grey Door completion rule).
- **Right**: record the active session to the manual checkpoint.
- Each arrow previews the action in the existing status text area. **Enter**
  confirms; **Backspace** or **Escape** cancels; **Up/Down** also cancel the
  pending action and continue ordinary row selection.
- Unavailable actions show a notice without creating a pending command.
- A successful load retains the existing restoration acknowledgement and
  key-down/key-up handshake. A successful save retains the established
  "CURRENT SESSION RECORDED" notice.
- Only the footer's keyboard hint was extended to identify these shortcuts;
  existing boxes, row count, placement, spacing, title, progress percentage,
  manual save serialization and autosave policy remain unchanged.

Automated checks: S4C restored-session renderer's direction/eligibility
assertions and S4F input-dispatch route audit; full Stage 4 regression and
Release size checks. Interactive acceptance is required for key repeat,
cancellation, stale/corrupted saves and loaded-terminal first key.
