# S4G-03: GREY DOOR REPUBLIC encounter

## Architecture and normal-world isolation

The Site `A ACCESS` handler opens `FLOPPY144_SCREEN_GREY_ENCOUNTER`
only when the existing S4G-02 `Floppy144GreyDoorNearby` targeting
succeeds. An explicitly ephemeral `Floppy144GreyEncounter` stores only:
local scene avatar coordinates, current phase, elapsed monotonic
presentation time, inspected flag, and the original exact GDR foot
point. It is **not** a Room, Site rectangle, collection or saved struct.

Nothing in the module receives a writable run state or a Profile pointer.
All local motion is clamped to a tiny vignette rectangle. The genuine
Site world, persistent player position, collision and facing remain
untouched throughout.

Stages: entry transition -> explore -> Developer identified ->
slow turn -> exact spoken line -> presentation-only 144% ->
horizontal CRT glitch -> return. The explore stage requires the
player to approach the Developer and press existing `I INSPECT`.
Entry uses existing `A ACCESS`; no new shortcut or physical key.

## Art direction

`floppy144_grey_encounter.c` composes directly in the existing
640x360 software framebuffer using the ordinary pixel/text primitives.
It intentionally skips the normal GDR CRT filter. Higher-contrast cool
panels, glass, contemporary furnishings, perspective floor bands and
soft blue ambient light visibly break the canonical archive palette.

The scene contains the **GREY DOOR REPUBLIC** wall sign, three desks,
three tiny original fictional concepts (drowned observatory, crimson
library staircase and mechanical whale), a lit monitor showing compact
FLOPPY//144 C-like code, and a half-eaten sandwich, unremarked.
A seated character begins facing away. Only Inspect labels
**DEVELOPER**. The turn then takes 1.9 seconds before the exact line:

`You're not supposed to be able to get in here.`

The false `RESTORATION CAPACITY: 144%` is written **only** to the
framebuffer. A short deterministic horizontal displacement and
inversion glitch closes the scene.

## Return and saving

At scene finish, the coordinator verifies the untouched original GDR
player foot point and marks only `grey_door_state=COMPLETED`.
The scene is discarded, player movement is reset and normal
`FLOPPY144_SCREEN_OFFICE` rendering resumes. The S4G-02 overlay
query now returns false, leaving an ordinary solid wall without scars.
Completion survives the existing V3 save format. The isolated bit can
never re-enter the available state by re-reading the orphan document.

The vignette cannot enter Session Control or the manual-save screen.
The automatic and lifecycle-triggered save path explicitly returns
without writing while the encounter screen is active. A process exit
while the scene is active can at worst return to a previous normal
checkpoint; temporary scene data is never serialised and reloaded
into incorrect Site geometry. The Profile and GDR run data are never
modified to reflect the false percentage. Normal autosave resumes
after returning to the Corridor.

## Regression and size

Stage4 persistence regression now enters from **every safe candidate
and both candidate orientations**, traverses each phase, validates
the no-save policy, all frame buffer rendering, unchanged real run
and Profile fields, the exact return foot point and wall restoration,
and V3 completed-run save/reload and non-repeatability. Windows
Actions performs all existing gameplay tests, warning-clean Release
build, and the 1,474,560-byte size gate.
