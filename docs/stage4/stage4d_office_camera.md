# S4D-02 Office / Site Camera Scale

## Selected presentation scale

Stage 4D reduces the Site renderer from **20 pixels per Site unit** to
**12 pixels per Site unit**, or **60% of the previous scale**.

The playable Site remains the existing 576 x 252 viewport inside the 640 x 360
framebuffer and UI shell. At 12 px/unit the camera sees exactly **48 x 21 Site
units** because the fixed-point camera span is 768 x16 ticks by 336 x16 ticks.

This is a presentation-only change. The canonical Site remains 100 x 100 units
with sixteen fixed-point ticks per unit.

## Architecture inspected

The exploration view is not a post-render framebuffer zoom.

The 2D renderer projects canonical x16 world coordinates directly into the
fixed 576 x 252 Site viewport. The camera stores its top-left world position in
x16 fixed-point coordinates and follows the player's canonical Site position.
When a room is smaller than the visible span the room is centred; when it is
larger, the camera follows the player until clamped against room bounds.

Canonical Site/view orientation is already identical. The old hidden
90-degree runtime transform was removed during Stage 3, so the current
world-to-screen path is:

1. canonical Site x16 position;
2. identity Site-view transform;
3. subtract camera x16/y16;
4. multiply by integer pixels per Site unit;
5. divide by 16 fixed-point ticks;
6. add the fixed viewport origin at (32, 32).

Rectangle dimensions are projected at the same integer pixels-per-unit scale.
World geometry, collision footprints, proximity targeting, doors and trigger
coordinates are not screen-space data.

S4D-02 isolates that fixed-point transform in
`floppy144_site_2d_camera.c/.h`. The renderer consumes the shared camera
result, while the dedicated regression can exercise the same production
transform directly without linking unrelated furniture, cabinet, drawing or
progression systems.

## Candidate evaluation

The deferred requirement requested an approximate 50-75% presentation scale.
The practical integer candidates against the 20 px/unit baseline were:

| Candidate | Pixels/unit | Camera span | Player visual | 0.5-unit step | Result |
| --- | ---: | ---: | ---: | ---: | --- |
| 50% | 10 | 57.5625 x 25.1875 U | 35 x 60 px | 5 px | Rejected: very wide context, but 1-unit features fall to 10 px and ordinary half-unit wall-fixture depth to 5 px; more miniaturisation than is needed |
| **60%** | **12** | **48 x 21 U** | **42 x 72 px** | **6 px** | **Selected: exact viewport division, crisp movement, readable small fixtures, much wider room context** |
| 65% | 13 | 44.25 x 19.375 U | 45 x 78 px | 6.5 px | Rejected: half-unit movement cannot remain on a uniform whole-pixel cadence |
| 70% | 14 | 41.125 x 18 U | 49 x 84 px | 7 px | Clean, but provides materially less additional context than 60% without a compensating readability benefit |
| 75% | 15 | 38.375 x 16.75 U | 52 x 90 px | 7.5 px | Rejected: smaller gain and fractional half-unit cadence |

The 13 and 15 px/unit candidates are not inherently invalid, but repeated
half-unit camera movement must alternate between integer pixel positions after
fixed-point truncation. The even 12 and 14 px/unit candidates avoid that
source of visual judder completely.

Twelve pixels per unit leaves a one-unit authored feature at 12 pixels and an
ordinary half-unit wall-fixture visual depth at 6 pixels, while preserving the
existing one-pixel outlines and procedural furniture detail. Compared with the
50% candidate it retains 20% more pixels on every world-space feature without
giving up the large contextual gain over the original scale.

## Room coverage

The 60% camera was evaluated against every generated room bound.

Horizontal room span including the existing one-unit left/right camera gutters:

| Room | Authored width | Width with camera gutters | 48-unit viewport |
| --- | ---: | ---: | --- |
| Reception | 34 U | 36 U | Fully fits |
| Main Office | 34 U | 36 U | Fully fits |
| Records Office | 46 U | **48 U** | **Exactly fits** |
| Facilities | 16 U | 18 U | Fully fits |
| IT Support | 14 U | 16 U | Fully fits |
| Security | 14 U | 16 U | Fully fits |
| Staff Room | 25 U | 27 U | Fully fits |
| Secretary's Office | 32 U | 34 U | Fully fits |
| Director's Office | 32 U | 34 U | Fully fits |
| Server Room | 29 U | 31 U | Fully fits |
| Corridor | 52 U | 54 U | Retains horizontal camera scrolling |

The Records Office is a particularly useful fit: its 46-unit combined room
width plus the established two units of camera gutter occupies exactly the
48-unit horizontal viewport. This removes unnecessary horizontal panning there
while retaining vertical scrolling through the long Records layout.

Large/tall rooms still scroll vertically. The change therefore increases
context without converting exploration into a whole-building overview.

## Pixel stability

The player moves in canonical 0.5-unit increments, or eight x16 ticks. At
12 px/unit every movement step projects to exactly six pixels.

The selected camera span is also integral in fixed-point space:

- horizontal: 576 px / 12 = 48 Site units;
- vertical: 252 px / 12 = 21 Site units.

This avoids scale-induced camera-centre drift and avoids alternating 6/7 or
7/8 pixel movement that would occur with 13 or 15 px/unit candidates.

The renderer retains its existing viewport clipping and final frame redraw, so
world primitives crossing the camera edge remain clipped rather than resized.

## Gameplay invariants

S4D-02 does not change:

- 100 x 100 Site geometry;
- sixteen x16 ticks per Site unit;
- 3.5-unit visual player width;
- 2 x 2-unit player collision footprint;
- 0.5-unit movement step;
- furniture rectangles or hitboxes;
- interaction/proximity rules;
- door positions or topology;
- room regions;
- trigger coordinates;
- collection/evidence progression;
- generated Site data.

The player sees more of the same world.

## Regression coverage

The dedicated Stage 4D camera regression:

- evaluates 10, 12, 13, 14 and 15 px/unit camera spans;
- checks whole-pixel versus fractional half-unit movement cadence;
- proves the selected scale is 12 px/unit;
- proves the 576 x 252 viewport resolves to exactly 48 x 21 Site units;
- verifies a known Main Office player position projects to a stable centre;
- verifies a known Main Office desk origin;
- verifies a half-unit camera move pans world content by exactly six pixels;
- builds the 60% camera against all eleven generated room bounds;
- proves the Records Office gutter-to-gutter width exactly fills the viewport;
- audits the canonical fixed-point, collision-width/depth and movement-step
  constants so camera work cannot silently resize the player's world.

The complete Stage 3B suite remains the authority for collision, door,
interaction, container and Site gameplay behaviour.
