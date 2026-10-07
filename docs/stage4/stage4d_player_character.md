# S4D-03 Player Character and Directional Animation

## Summary

Stage 4D replaces the Stage 3 rectangular Site avatar with a lightweight
procedural directional character. No external sprite sheet or bitmap asset is
introduced.

The upgraded player remains a software-rendered composition of tiny filled
shapes and lines. A small transient presentation state records facing, whether
the player is currently moving, the current two-frame gait phase and a timing
accumulator.

None of that state is saved into the recovery session.

## Previous presentation

At the S4D-02 60% camera scale the historical Site character occupied:

- 42 px visual width, derived from the existing 3.5 Site-unit visual width;
- 72 px standing presentation height;
- a 24 px shadow derived from the existing 2 Site-unit collision width.

The avatar used:

- a square head;
- one rectangular torso;
- two static rectangular legs;
- no arms;
- no facing state;
- no walking animation.

Profile Type B already widened the torso cosmetically, but movement direction
was otherwise invisible.

## New presentation

The same outer Site presentation footprint is retained.

The player is now built procedurally from:

- a round, scanline-filled head;
- a bordered uniform torso;
- two independently drawn arms;
- two independently drawn legs;
- a collision-footprint shadow;
- tiny directional face/back markers;
- Profile-dependent uniform detail.

The renderer uses no external image data.

### Direction states

The transient presentation state supports:

- DOWN;
- LEFT;
- RIGHT;
- UP;
- IDLE while preserving the last valid facing direction.

Horizontal direction is reinforced by a side eye/nose marker as well as limb
motion. Down uses a front-facing pair of eyes. Up removes the face and adds a
small rear hair/uniform-yoke treatment.

The existing input system can combine one horizontal and one vertical action.
For diagonal movement the player continues to move using the unchanged full
movement vector. The presentation chooses the most recently pressed axis as
the four-way facing tie-breaker. Releasing one diagonal axis retains the
remaining valid facing.

## Animation

The walk cycle is deliberately only two frames.

Each phase reverses opposing arm and leg swing. Horizontal movement therefore
has an obvious alternating stride; vertical movement also changes both arms and
feet so the character does not slide as a static pawn.

The gait cadence is **160 ms per frame**, or approximately 3.1 complete
two-frame walk cycles per second.

Animation time comes from the Stage 4B deterministic timing layer:

1. the platform supplies only its monotonic millisecond clock;
2. Floppy144TimingAdvance reports presentation elapsed milliseconds;
3. the Site coordinator passes that elapsed time to the transient player visual
   state only while Site exploration is active;
4. accumulated elapsed time advances the gait at the 160 ms boundary;
5. variable or delayed frame timing is reduced deterministically by modulo
   arithmetic rather than depending on native timer frequency.

No direct Win32 timing API exists in the player renderer.

On movement-key release the gait settles to the neutral frame and retains the
last facing direction.

## Profile body styles

The Stage 4C body-style value remains cosmetic.

- **Type A** uses the narrower historical 3/4-width torso with a single central
  uniform marker.
- **Type B** uses the broader historical 7/8-width torso with twin shoulder
  markers.

Both use the same:

- outer 3.5-unit visual width;
- player foot point;
- collision shadow;
- movement speed;
- collision footprint;
- interaction range.

The Profile preview now uses the same procedural character renderer in a
smaller idle/down pose, so the selected body style and in-game avatar remain
visually consistent.

The existing V1 Profile body-style byte is unchanged. Stage 3B persistence
regression now explicitly round-trips Type B through profile encode/decode.

## Gameplay invariants

S4D-03 does not change:

- FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16 = 56;
- FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 = 32;
- FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16 = 32;
- FLOPPY144_SITE_MOVE_STEP_X16 = 8;
- the half-unit generated-data interaction range;
- room geometry;
- furniture hitboxes;
- door geometry;
- trigger positions;
- reconstruction/progression rules;
- simultaneous-direction movement behaviour;
- S4D-02's 12 px/Site-unit camera scale.

Facing and gait are presentation metadata only.

## Regression coverage

The dedicated Stage 4D player regression verifies:

- idle/down reset;
- left, right, up and down facing;
- stationary last-facing retention;
- left-to-right, up-to-left and down-to-right rapid transitions;
- diagonal tie-breaking without changing the movement vector;
- horizontal gait changes in both directions;
- visible vertical animation in both directions;
- 160 ms animation boundaries;
- variable elapsed-time accumulation;
- deterministic delayed-frame parity;
- idle settling;
- distinct Type A and Type B rendering;
- the round-head silhouette rather than a square bounding box;
- clipping to the existing S4D-02 Site viewport;
- unchanged visual width, collision width/depth, movement step and camera scale.

The full Stage 3B suite remains authoritative for collision, movement,
interaction targeting, doors and container behaviour.
