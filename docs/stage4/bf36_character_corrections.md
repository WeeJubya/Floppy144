# BF36: playable character geometry corrections

## Scope and status

The latest Stage 4 branch at commencement was `bed1b7e71cb44e1eccc44d9629c7596d11cb8065`.
This commit is **partial BF36 implementation**, not final visual acceptance.

### Applied: remove separate neck primitives

The shared vector renderer previously painted thin skin-coloured capsules
from the lower head into the upper jacket after rendering the torso. These
were visible as unwanted neck extensions, particularly in Type A profiles and
front-facing characters. Both separate capsules are removed; the existing
head/face discs and jacket upper stroke already overlap directly.

No adjustments were made to body sizes, hair, clothing, palettes, facing,
walking cadence, movement, saved body-style identity, ISO projection, or the
shared Developer/Easter Egg character entry point.

### Regression

`tools/stage4_player_visual_tests.c` now samples the head/torso boundary for
both body styles, four directions and idle plus both walking frames. It
checks that the centreline contains no skin-coloured neck segment or gap.
The existing directional, gait, silhouette, viewport and Easter Egg tests
should also be run.

### Blocked: source-art-accurate hairstyles

`Characters(1).pptx` was located in the original user uploads. Its two
visual slide images were **not available to inspect** in the current
environment: the file service returned text only and raw-byte materialisation
failed. Existing BF25 documentation describes general shapes but is not a
substitute for the canonical artwork. In accordance with BF36, hairstyle
geometry was **not guessed or modified**.

Before final BF36 sign-off:
1. Obtain viewable original Character A and B slide artwork.
2. Compare each type to all directional renderer captures, including walking.
3. Adjust only reference-supported hair geometry, including rear/side coverage.
4. Repeat visual tests and Release / size verification.
5. Obtain manual visual approval.

This status does not claim visual comparison or Release verification.
