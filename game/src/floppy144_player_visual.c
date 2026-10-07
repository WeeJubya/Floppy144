/*
 * FLOPPY//144 - lightweight directional player presentation.
 */

#include "floppy144_player_visual.h"

#include <stddef.h>

static int32_t Floppy144PlayerAbs(
    int32_t value
)
{
    return value < 0 ? -value : value;
}

static Floppy144PlayerFacing Floppy144PlayerFacingFromAction(
    F144Action action
)
{
    switch(action)
    {
        case F144_ACTION_MOVE_LEFT:
            return FLOPPY144_PLAYER_FACING_LEFT;

        case F144_ACTION_MOVE_RIGHT:
            return FLOPPY144_PLAYER_FACING_RIGHT;

        case F144_ACTION_MOVE_UP:
            return FLOPPY144_PLAYER_FACING_UP;

        case F144_ACTION_MOVE_DOWN:
            return FLOPPY144_PLAYER_FACING_DOWN;

        default:
            return FLOPPY144_PLAYER_FACING_COUNT;
    }
}

static bool Floppy144PlayerFacingMatchesVector(
    Floppy144PlayerFacing facing,
    int32_t x,
    int32_t y
)
{
    switch(facing)
    {
        case FLOPPY144_PLAYER_FACING_LEFT:
            return x < 0;

        case FLOPPY144_PLAYER_FACING_RIGHT:
            return x > 0;

        case FLOPPY144_PLAYER_FACING_UP:
            return y < 0;

        case FLOPPY144_PLAYER_FACING_DOWN:
            return y > 0;

        default:
            return false;
    }
}

void Floppy144PlayerVisualReset(
    Floppy144PlayerVisualState *state
)
{
    if(state == NULL)
    {
        return;
    }

    state->animation_accumulator_ms = 0U;
    state->facing = (uint8_t)FLOPPY144_PLAYER_FACING_DOWN;
    state->moving = 0U;
    state->walk_frame = 0U;
}

bool Floppy144PlayerVisualSetMovement(
    Floppy144PlayerVisualState *state,
    int32_t movement_x,
    int32_t movement_y,
    F144Action recent_action
)
{
    Floppy144PlayerFacing old_facing;
    Floppy144PlayerFacing facing;
    Floppy144PlayerFacing preferred;
    bool old_moving;
    bool moving;

    if(state == NULL)
    {
        return false;
    }

    old_facing =
        state->facing < (uint8_t)FLOPPY144_PLAYER_FACING_COUNT
            ? (Floppy144PlayerFacing)state->facing
            : FLOPPY144_PLAYER_FACING_DOWN;

    old_moving =
        state->moving != 0U;

    moving =
        movement_x != 0 ||
        movement_y != 0;

    if(!moving)
    {
        bool changed =
            old_moving ||
            state->walk_frame != 0U ||
            state->animation_accumulator_ms != 0U;

        state->facing =
            (uint8_t)old_facing;
        state->moving = 0U;
        state->walk_frame = 0U;
        state->animation_accumulator_ms = 0U;

        return changed;
    }

    preferred =
        Floppy144PlayerFacingFromAction(
            recent_action
        );

    if(
        preferred < FLOPPY144_PLAYER_FACING_COUNT &&
        Floppy144PlayerFacingMatchesVector(
            preferred,
            movement_x,
            movement_y
        )
    )
    {
        facing =
            preferred;
    }
    else if(
        Floppy144PlayerFacingMatchesVector(
            old_facing,
            movement_x,
            movement_y
        )
    )
    {
        facing =
            old_facing;
    }
    else if(movement_x < 0)
    {
        facing =
            FLOPPY144_PLAYER_FACING_LEFT;
    }
    else if(movement_x > 0)
    {
        facing =
            FLOPPY144_PLAYER_FACING_RIGHT;
    }
    else if(movement_y < 0)
    {
        facing =
            FLOPPY144_PLAYER_FACING_UP;
    }
    else
    {
        facing =
            FLOPPY144_PLAYER_FACING_DOWN;
    }

    if(
        !old_moving ||
        facing != old_facing
    )
    {
        state->walk_frame = 0U;
        state->animation_accumulator_ms = 0U;
    }

    state->facing =
        (uint8_t)facing;
    state->moving = 1U;

    return
        !old_moving ||
        facing != old_facing;
}

bool Floppy144PlayerVisualAdvance(
    Floppy144PlayerVisualState *state,
    uint32_t elapsed_ms
)
{
    uint64_t accumulated;
    uint32_t frames;

    if(
        state == NULL ||
        state->moving == 0U ||
        elapsed_ms == 0U
    )
    {
        return false;
    }

    accumulated =
        (uint64_t)state->animation_accumulator_ms +
        (uint64_t)elapsed_ms;

    frames =
        (uint32_t)(
            accumulated /
            FLOPPY144_PLAYER_WALK_FRAME_MS
        );

    state->animation_accumulator_ms =
        (uint32_t)(
            accumulated %
            FLOPPY144_PLAYER_WALK_FRAME_MS
        );

    if(frames == 0U)
    {
        return false;
    }

    if((frames & 1U) == 0U)
    {
        return false;
    }

    state->walk_frame ^=
        1U;

    return true;
}

static void Floppy144PlayerFill(
    Floppy144Surface *surface,
    int32_t clip_x,
    int32_t clip_y,
    int32_t clip_width,
    int32_t clip_height,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t colour
)
{
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;

    if(
        surface == NULL ||
        surface->pixels == NULL ||
        width <= 0 ||
        height <= 0 ||
        clip_width <= 0 ||
        clip_height <= 0
    )
    {
        return;
    }

    x0 = x;
    y0 = y;
    x1 = x + width;
    y1 = y + height;

    if(x0 < clip_x)
    {
        x0 = clip_x;
    }

    if(y0 < clip_y)
    {
        y0 = clip_y;
    }

    if(x1 > clip_x + clip_width)
    {
        x1 = clip_x + clip_width;
    }

    if(y1 > clip_y + clip_height)
    {
        y1 = clip_y + clip_height;
    }

    if(x0 < 0)
    {
        x0 = 0;
    }

    if(y0 < 0)
    {
        y0 = 0;
    }

    if(x1 > (int32_t)surface->width)
    {
        x1 = (int32_t)surface->width;
    }

    if(y1 > (int32_t)surface->height)
    {
        y1 = (int32_t)surface->height;
    }

    if(x1 <= x0 || y1 <= y0)
    {
        return;
    }

    Floppy144DrawFillRect(
        surface,
        (uint32_t)x0,
        (uint32_t)y0,
        (uint32_t)(x1 - x0),
        (uint32_t)(y1 - y0),
        colour
    );
}

static void Floppy144PlayerLine(
    Floppy144Surface *surface,
    int32_t clip_x,
    int32_t clip_y,
    int32_t clip_width,
    int32_t clip_height,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    int32_t thickness,
    uint32_t colour
)
{
    int32_t dx =
        Floppy144PlayerAbs(x1 - x0);

    int32_t sx =
        x0 < x1 ? 1 : -1;

    int32_t dy =
        -Floppy144PlayerAbs(y1 - y0);

    int32_t sy =
        y0 < y1 ? 1 : -1;

    int32_t error =
        dx + dy;

    int32_t half =
        thickness / 2;

    for(;;)
    {
        Floppy144PlayerFill(
            surface,
            clip_x,
            clip_y,
            clip_width,
            clip_height,
            x0 - half,
            y0 - half,
            thickness,
            thickness,
            colour
        );

        if(x0 == x1 && y0 == y1)
        {
            break;
        }

        {
            int32_t twice_error =
                error * 2;

            if(twice_error >= dy)
            {
                error += dy;
                x0 += sx;
            }

            if(twice_error <= dx)
            {
                error += dx;
                y0 += sy;
            }
        }
    }
}

static void Floppy144PlayerCircle(
    Floppy144Surface *surface,
    int32_t clip_x,
    int32_t clip_y,
    int32_t clip_width,
    int32_t clip_height,
    int32_t centre_x,
    int32_t centre_y,
    int32_t radius,
    uint32_t colour
)
{
    int32_t y;

    if(radius <= 0)
    {
        return;
    }

    for(y = -radius; y <= radius; ++y)
    {
        int32_t x =
            radius;

        while(
            x > 0 &&
            x * x + y * y >
                radius * radius
        )
        {
            --x;
        }

        Floppy144PlayerFill(
            surface,
            clip_x,
            clip_y,
            clip_width,
            clip_height,
            centre_x - x,
            centre_y + y,
            x * 2 + 1,
            1,
            colour
        );
    }
}

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
)
{
    const uint32_t body_colour =
        FLOPPY144_RGB(72, 103, 118);

    const uint32_t shirt_light =
        FLOPPY144_RGB(112, 139, 148);

    const uint32_t skin_colour =
        FLOPPY144_RGB(205, 186, 158);

    const uint32_t trouser_colour =
        FLOPPY144_RGB(39, 48, 54);

    const uint32_t edge_colour =
        FLOPPY144_RGB(18, 23, 26);

    const uint32_t shadow_colour =
        FLOPPY144_RGB(31, 35, 34);

    Floppy144PlayerFacing facing =
        FLOPPY144_PLAYER_FACING_DOWN;

    bool moving =
        false;

    int32_t phase =
        0;

    int32_t sprite_y;
    int32_t head_radius;
    int32_t head_x;
    int32_t head_y;
    int32_t torso_width;
    int32_t torso_x;
    int32_t torso_y;
    int32_t leg_y;
    int32_t torso_height;
    int32_t shoulder_y;
    int32_t arm_thickness;
    int32_t leg_thickness;
    int32_t arm_swing;
    int32_t leg_swing;
    int32_t left_hand_x;
    int32_t left_hand_y;
    int32_t right_hand_x;
    int32_t right_hand_y;
    int32_t left_foot_x;
    int32_t left_foot_y;
    int32_t right_foot_x;
    int32_t right_foot_y;

    if(
        surface == NULL ||
        surface->pixels == NULL ||
        sprite_width <= 0 ||
        sprite_height <= 0 ||
        collision_shadow_width <= 0
    )
    {
        return;
    }

    if(
        body_style < 0 ||
        body_style >= FLOPPY144_OPERATOR_BODY_STYLE_COUNT
    )
    {
        body_style =
            FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT;
    }

    if(
        state != NULL &&
        state->facing <
            (uint8_t)FLOPPY144_PLAYER_FACING_COUNT
    )
    {
        facing =
            (Floppy144PlayerFacing)state->facing;

        moving =
            state->moving != 0U;

        if(moving)
        {
            phase =
                state->walk_frame != 0U
                    ? 1
                    : -1;
        }
    }

    sprite_y =
        foot_y - sprite_height;

    head_radius =
        sprite_width / 5;

    if(head_radius < 5)
    {
        head_radius = 5;
    }

    head_x =
        foot_x;

    head_y =
        sprite_y + head_radius + 1;

    torso_width =
        body_style == FLOPPY144_OPERATOR_BODY_STYLE_B
            ? sprite_width * 7 / 8
            : sprite_width * 3 / 4;

    if(torso_width < 8)
    {
        torso_width = 8;
    }

    torso_x =
        foot_x - torso_width / 2;

    torso_y =
        sprite_y + head_radius * 2;

    leg_y =
        foot_y - sprite_height / 4;

    torso_height =
        leg_y - torso_y + 2;

    if(torso_height < 8)
    {
        torso_height = 8;
    }

    shoulder_y =
        torso_y + torso_height / 4;

    arm_thickness =
        sprite_width / 12;

    if(arm_thickness < 3)
    {
        arm_thickness = 3;
    }

    leg_thickness =
        torso_width / 5;

    if(leg_thickness < 4)
    {
        leg_thickness = 4;
    }

    arm_swing =
        moving
            ? sprite_height / 14
            : 0;

    if(arm_swing < 3 && moving)
    {
        arm_swing = 3;
    }

    leg_swing =
        moving
            ? sprite_width / 9
            : sprite_width / 14;

    if(leg_swing < 2)
    {
        leg_swing = 2;
    }

    Floppy144PlayerFill(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        foot_x - collision_shadow_width / 2,
        foot_y - 2,
        collision_shadow_width,
        5,
        shadow_colour
    );

    left_hand_x =
        torso_x - arm_thickness;
    right_hand_x =
        torso_x + torso_width + arm_thickness;
    left_hand_y =
        torso_y + torso_height * 3 / 4;
    right_hand_y =
        left_hand_y;

    left_foot_x =
        foot_x - torso_width / 4;
    right_foot_x =
        foot_x + torso_width / 4;
    left_foot_y =
        foot_y;
    right_foot_y =
        foot_y;

    if(
        facing == FLOPPY144_PLAYER_FACING_LEFT ||
        facing == FLOPPY144_PLAYER_FACING_RIGHT
    )
    {
        int32_t forward =
            facing == FLOPPY144_PLAYER_FACING_RIGHT
                ? 1
                : -1;

        left_hand_x +=
            phase * arm_swing;

        right_hand_x -=
            phase * arm_swing;

        left_hand_x +=
            forward * 2;

        right_hand_x +=
            forward * 2;

        left_foot_x +=
            phase * leg_swing;

        right_foot_x -=
            phase * leg_swing;

        if(moving)
        {
            left_foot_x +=
                forward * 2;

            right_foot_x +=
                forward * 2;
        }
    }
    else
    {
        left_hand_x -=
            moving ? 2 : 0;

        right_hand_x +=
            moving ? 2 : 0;

        left_hand_y +=
            phase * arm_swing;

        right_hand_y -=
            phase * arm_swing;

        left_foot_x +=
            phase * leg_swing;

        right_foot_x -=
            phase * leg_swing;

        if(moving)
        {
            left_foot_y -=
                phase > 0 ? 2 : 0;

            right_foot_y -=
                phase < 0 ? 2 : 0;
        }
    }

    /*
     * Limbs are drawn behind the uniform jacket so the body remains readable
     * even when the two-frame walk cycle crosses the centre line.
     */
    Floppy144PlayerLine(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        torso_x + 2,
        shoulder_y,
        left_hand_x,
        left_hand_y,
        arm_thickness,
        skin_colour
    );

    Floppy144PlayerLine(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        torso_x + torso_width - 3,
        shoulder_y,
        right_hand_x,
        right_hand_y,
        arm_thickness,
        skin_colour
    );

    Floppy144PlayerLine(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        foot_x - torso_width / 5,
        leg_y,
        left_foot_x,
        left_foot_y,
        leg_thickness,
        trouser_colour
    );

    Floppy144PlayerLine(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        foot_x + torso_width / 5,
        leg_y,
        right_foot_x,
        right_foot_y,
        leg_thickness,
        trouser_colour
    );

    Floppy144PlayerFill(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        torso_x,
        torso_y,
        torso_width,
        torso_height,
        edge_colour
    );

    Floppy144PlayerFill(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        torso_x + 1,
        torso_y + 1,
        torso_width - 2,
        torso_height - 2,
        body_colour
    );

    /*
     * Type A is a narrower straight jacket with one badge. Type B keeps the
     * same outer player footprint but uses a broader torso and twin shoulder
     * tabs. The difference is cosmetic only.
     */
    if(body_style == FLOPPY144_OPERATOR_BODY_STYLE_B)
    {
        Floppy144PlayerFill(
            surface,
            clip_x,
            clip_y,
            clip_width,
            clip_height,
            torso_x + 4,
            torso_y + 4,
            5,
            3,
            shirt_light
        );

        Floppy144PlayerFill(
            surface,
            clip_x,
            clip_y,
            clip_width,
            clip_height,
            torso_x + torso_width - 9,
            torso_y + 4,
            5,
            3,
            shirt_light
        );
    }
    else
    {
        Floppy144PlayerFill(
            surface,
            clip_x,
            clip_y,
            clip_width,
            clip_height,
            torso_x + torso_width / 2 - 2,
            torso_y + 5,
            4,
            6,
            shirt_light
        );
    }

    /* Round head: dark one-pixel-ish rim, then skin inset. */
    Floppy144PlayerCircle(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        head_x,
        head_y,
        head_radius,
        edge_colour
    );

    Floppy144PlayerCircle(
        surface,
        clip_x,
        clip_y,
        clip_width,
        clip_height,
        head_x,
        head_y,
        head_radius - 1,
        skin_colour
    );

    /*
     * Direction marks are intentionally tiny. They make left/right immediate,
     * distinguish front from back vertically, and cost no external sprite data.
     */
    switch(facing)
    {
        case FLOPPY144_PLAYER_FACING_LEFT:
        {
            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                head_x - head_radius / 3 - 1,
                head_y - 2,
                2,
                2,
                edge_colour
            );

            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                head_x - head_radius - 1,
                head_y,
                2,
                2,
                skin_colour
            );

            break;
        }

        case FLOPPY144_PLAYER_FACING_RIGHT:
        {
            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                head_x + head_radius / 3,
                head_y - 2,
                2,
                2,
                edge_colour
            );

            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                head_x + head_radius,
                head_y,
                2,
                2,
                skin_colour
            );

            break;
        }

        case FLOPPY144_PLAYER_FACING_UP:
        {
            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                head_x - head_radius / 2,
                head_y - head_radius + 2,
                head_radius,
                3,
                trouser_colour
            );

            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                torso_x + 3,
                torso_y + 3,
                torso_width - 6,
                3,
                shirt_light
            );

            break;
        }

        case FLOPPY144_PLAYER_FACING_DOWN:
        default:
        {
            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                head_x - head_radius / 3 - 2,
                head_y - 1,
                2,
                2,
                edge_colour
            );

            Floppy144PlayerFill(
                surface,
                clip_x,
                clip_y,
                clip_width,
                clip_height,
                head_x + head_radius / 3,
                head_y - 1,
                2,
                2,
                edge_colour
            );

            break;
        }
    }
}
