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

## BF28 revised: horizontal label carousel (supersedes shortcut draft)

The original five rows and all fixed panel coordinates remain unchanged:

| Row | Default (on opening Main Menu) | After Left or Right |
| --- | --- | --- |
| 1 | INITIATE NEW RECOVERY SESSION | Unchanged |
| 2 | RETURN TO ACTIVE SITE | Unchanged |
| 3 | OPERATOR PROFILE | RECORD CURRENT SESSION |
| 4 | SETTINGS | REINSTATE RECORDED SESSION |
| 5 | TERMINATE RECOVERY ENVIRONMENT | Unchanged |

Both Left and Right toggle the two alternative labels. The current pair
moves **off the screen in the pressed arrow direction**, while the new pair
slides **in from the opposite edge**. Animation lasts 240 ms, uses integer
ease-in/ease-out and the existing presentation timing, and clips text against
the 640x360 surface. Only row 3 and row 4 label pixels animate. The fixed row
selection highlight, menu geometry, percentage, dividers and statuses remain
in place. Arrows pressed during the 240 ms transition are ignored; Enter
completes the transition before activating the visible row. Returning to a
fresh Main Menu resets to Profile/Settings. No extra page or menu row.

Up/Down moves the same five row positions. Enter uses the mapped real action
for the visible label; existing Profile and Settings views are reused, as are
the original manual Record and Reinstate handlers. Save needs an active
session; Load needs a valid saved/manual-or-autosave session. The established
save failure, inspection-mode block, success notice, restore confirmation and
autosave policies are unchanged. No persistence fields record the carousel.

The footer is updated to `UP/DOWN SELECT   LEFT/RIGHT SWITCH   ENTER CONFIRM`
to expose the interaction. The obsolete unconfirmed arrow-key save/load
shortcut introduced during the previous BF28 follow-up was removed.

Regression: Stage 4C renderer exercises both directions and 50% frames,
checks pixel-perfect restoration of the default five-row frame, eligibility
and action mapping; Stage 4F input-flow audit verifies real routing and that
obsolete shortcut code was removed. Windows capture workflow records
four static/intermediate animation frames for manual visual sign-off.
