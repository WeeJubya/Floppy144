# BF42: Grey Room poster and source screen ISO rendering

Branch: `development/Stage-4`.

## Root cause

BF35 moved the walls, floor and furniture to the Site ISO scene, but
`Poster()` and `SourceMonitor()` still used absolute framebuffer
coordinates. The final draw pass put those flat overlays above furniture.

## Implementation

Poster designs and source-code text remain procedurally rendered, now into
small temporary surfaces. A new `WallPanel()` rasteriser inverse-maps
the art onto the projected wall plane using the shared ISO camera.
The first two posters sit on the visible x=1 faces of the existing west
wall mounts. The third poster and source-code monitor attach to the north
wall (y=1) with shallow prism frames. Wall attachments draw after the
walls and before furniture for natural occlusion; HUD and dialogue remain
foreground elements.

No authored gameplay geometry, save format, content, encounter timing,
glitch sequence or external assets were changed.

## Regression and release checks

The Grey Door source audit checks the ISO projection and all four mounts,
and rejects the four old flat overlay calls. The persistence-path encounter
test samples four projected framebuffer pixels in Explore, supplementing
the existing encounter, timing, one-shot and persistence checks.

Run `tools/test_stage4g_grey_door.ps1`,
`tools/test_stage4_persistence.ps1` and the existing Windows Stage 4
Release Candidate workflow, which treats MSVC warnings as errors and checks
the 1,474,560-byte executable limit.

**Manual verification outstanding until played on Windows:** look at all
three rear-wall posters and the source monitor; confirm perspective,
wall attachment, furniture occlusion, speech overlay and text legibility.
