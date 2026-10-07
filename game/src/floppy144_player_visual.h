/*
 * FLOPPY//144 - lightweight directional player presentation.
 *
 * This module owns only transient cosmetic state and software drawing. Canonical
 * Site position, collision, interaction range and persistence remain elsewhere.
 */

#pragma once

#include "f144_platform.h"
#include "floppy144_draw.h"
#include "floppy144_profile.h"

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_PLAYER_WALK_FRAME_MS 160U

typedef enum Floppy144PlayerFacing
{
    FLOPPY144_PLAYER_FACING_DOWN = 0,
    FLOPPY144_PLAYER_FACING_LEFT,
    FLOPPY144_PLAYER_FACING_RIGHT,
    FLOPPY144_PLAYER_FACING_UP,

    FLOPPY144_PLAYER_FACING_COUNT
}
Floppy144PlayerFacing;

typedef struct Floppy144PlayerVisualState
{
    uint32_t animation_accumulator_ms;
    uint8_t facing;
    uint8_t moving;
    uint8_t walk_frame;
}
Floppy144PlayerVisualState;

void Floppy144PlayerVisualReset(
    Floppy144PlayerVisualState *state
);

/*
 * Update direction from the logical movement vector.
 *
 * recent_action breaks a diagonal tie in favour of the direction most recently
 * pressed. Passing F144_ACTION_NONE preserves the current facing when possible.
 * A zero vector settles to idle while retaining the last valid direction.
 */
bool Floppy144PlayerVisualSetMovement(
    Floppy144PlayerVisualState *state,
    int32_t movement_x,
    int32_t movement_y,
    F144Action recent_action
);

/*
 * Advance the two-frame walk cycle using elapsed time supplied by the portable
 * Stage 4 timing layer. Returns true only when the visible frame changes.
 */
bool Floppy144PlayerVisualAdvance(
    Floppy144PlayerVisualState *state,
    uint32_t elapsed_ms
);

/*
 * Draw one procedural character at the supplied screen-space foot point.
 * The clip rectangle allows the same renderer to serve Site exploration and
 * the Profile preview without external bitmap assets.
 */
void Floppy144PlayerVisualDraw(
    Floppy144Surface *surface,
    int32_t foot_x,
    int32_t foot_y,
    int32_t sprite_width,
    int32_t sprite_height,
    int32_t collision_shadow_width,
    Floppy144OperatorBodyStyle body_style,
    const Floppy144PlayerVisualState *state,
    int32_t clip_x,
    int32_t clip_y,
    int32_t clip_width,
    int32_t clip_height
);
