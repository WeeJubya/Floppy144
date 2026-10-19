# BF36: Character Model Corrections

## Canonical references

User-supplied `Characters(2).pptx` and the exported `Slide1(1).PNG`
(Character A) and `Slide2(1).PNG` (Character B), received 19 October 2026,
show the front, rear, left and right orthographic character silhouettes.
These supersede the earlier inaccessible attachment. They were inspected
directly before updating the character renderer.

## Geometry changes

Both characters' separate skin-coloured neck capsules were removed in the
earlier BF36 commit. The head and torso remain connected using the original
round skull, face and overlapping jacket strokes. No extra neck primitive is
introduced.

Type A keeps the simple brown short cap in front, a round brown back of head,
and one rear hemisphere in each profile. There is no dangling hair or
ponytail.

Type B keeps the same head proportions but gains a single thin side lock
down the left of the face in front view. The rear hair has two small,
continuous lower-corner extensions framing the blue scarf. Both profiles
continue the round brown rear hemisphere down slightly, without adding a
large fringe or ponytail. The cyan scarf and existing dark outfit are
unchanged.

The same `f144BuildOfficeCharacter` renderer still handles Site play,
Profile previews and the larger Developer variant used by the Grey Door
Easter Egg. The Developer retains short silver hair and receives no Type B
hair geometry. No changes to body-style storage, movement, collision, foot
position, directional tie-breaking, gait or platform dependencies.

## Automated verification

`tools/stage4_player_visual_tests.c` extends the existing direction, idle,
movement, clipping and neck tests with colour-specific coverage probes for:

- Type B's front-left hair lock and absence of a matching front-right lock.
- Both lower rear corners of Type B's head.
- Type B rear hair coverage in the two mirrored profile views.
- Every check in idle and both two-frame walking phases.

The existing `F144_BF25_CAPTURE_DIR` test output generates 16 actual
640x360 PPM captures (both styles x four directions x idle/walking) for
reference comparison. Those captures are not stored as runtime bitmap assets.
User visual acceptance of the character render remains a separate check.

## Status

Reference-grounded hairstyle corrections implemented. The on-branch Windows
build and regressions determine the technical verification status; do not
treat any unaudited image captures as automatic proof of exact visual match.

## BF36 follow-up: centred side-view heads

The subsequent in-game visual review found both left and right profiles had
a displaced face disc. The original implementation centred hair on the
character's existing head position but placed the pink face at
`cx + direction * scale * 3.3`, so the front of the skull protruded past the
front/rear head footprint.

Both side views now render a painter-ordered **half-disc of face within the
original hair-disc radius and centre**. The face is the travel-facing half,
with the hair in the rear half; the outer round silhouette, height and
centre are identical to the front and rear orientations. Type B's existing
rear hair coverage is drawn behind the centred head as before. The new
half-disc uses the existing circle primitive payload and software rasterizer;
no bitmap, model or external asset has been added.

The `TestCentredProfileFaces` regression asserts exact leading and trailing
radius bounds, expected pink/brown halves and no protruding side pixels for
both styles, both profiles and idle/two walking phases. The existing BF36
hair-coverage, no-neck, movement and clip regressions remain in place.

The user's in-game visual sign-off remains outstanding until the new
capture sheet and actual executable have been reviewed.
