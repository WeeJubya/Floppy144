/*
 * FLOPPY//144 Stage 4D directional-player presentation regression.
 *
 * Presentation state is deliberately independent of RunState. These tests
 * exercise direction changes, idle settling, timing-driven gait, body-style
 * variants and the 60% Site presentation scale without altering gameplay.
 */

#include "floppy144_player_visual.h"
#include "floppy144_site.h"
#include "floppy144_site_2d_camera.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_WIDTH 640U
#define TEST_HEIGHT 360U

static uint32_t g_pixels[TEST_WIDTH * TEST_HEIGHT];
static int g_failures;

_Static_assert(
    FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16 == 56,
    "S4D-03 must not resize the canonical player visual width"
);

_Static_assert(
    FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 == 32,
    "S4D-03 must not change player collision width"
);

_Static_assert(
    FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16 == 32,
    "S4D-03 must not change player collision depth"
);

_Static_assert(
    FLOPPY144_SITE_MOVE_STEP_X16 == 8,
    "S4D-03 must not change the half-unit movement step"
);

_Static_assert(
    FLOPPY144_SITE_2D_PIXELS_PER_UNIT == 12,
    "S4D-03 must retain the S4D-02 60 percent camera scale"
);

#define EXPECT(condition, label)                                      \
    do                                                                 \
    {                                                                  \
        if(!(condition))                                               \
        {                                                              \
            ++g_failures;                                              \
            printf("FAIL: %s\n", (label));                            \
        }                                                              \
    }                                                                  \
    while(0)

static uint32_t SurfaceHash(void)
{
    uint32_t hash = 2166136261U;
    uint32_t index;

    for(index = 0U; index < TEST_WIDTH * TEST_HEIGHT; ++index)
    {
        hash ^= g_pixels[index];
        hash *= 16777619U;
    }

    return hash;
}

static uint32_t DrawHash(
    Floppy144PlayerVisualState *state,
    Floppy144OperatorBodyStyle body_style
)
{
    Floppy144Surface surface =
    {
        g_pixels,
        TEST_WIDTH,
        TEST_HEIGHT
    };

    memset(
        g_pixels,
        0,
        sizeof(g_pixels)
    );

    Floppy144PlayerVisualDraw(
        &surface,
        320,
        220,
        42,
        72,
        24,
        body_style,
        state,
        32,
        32,
        576,
        252
    );

    return SurfaceHash();
}

static void SetIdleFacing(
    Floppy144PlayerVisualState *state,
    int32_t x,
    int32_t y,
    F144Action action
)
{
    (void)Floppy144PlayerVisualSetMovement(
        state,
        x,
        y,
        action
    );

    (void)Floppy144PlayerVisualSetMovement(
        state,
        0,
        0,
        F144_ACTION_NONE
    );
}

static void TestDirectionAndIdle(void)
{
    Floppy144PlayerVisualState state;
    uint32_t down_hash;
    uint32_t left_hash;
    uint32_t right_hash;
    uint32_t up_hash;

    Floppy144PlayerVisualReset(
        &state
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_DOWN &&
        state.moving == 0U &&
        state.walk_frame == 0U,
        "fresh presentation state is idle and faces down"
    );

    down_hash =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        );

    SetIdleFacing(
        &state,
        -8,
        0,
        F144_ACTION_MOVE_LEFT
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_LEFT &&
        state.moving == 0U,
        "stationary state preserves left facing"
    );

    left_hash =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        );

    SetIdleFacing(
        &state,
        8,
        0,
        F144_ACTION_MOVE_RIGHT
    );

    right_hash =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        );

    SetIdleFacing(
        &state,
        0,
        -8,
        F144_ACTION_MOVE_UP
    );

    up_hash =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        );

    EXPECT(
        down_hash != left_hash &&
        down_hash != right_hash &&
        down_hash != up_hash &&
        left_hash != right_hash &&
        left_hash != up_hash &&
        right_hash != up_hash,
        "all four idle facings are visually distinct"
    );

    EXPECT(
        !Floppy144PlayerVisualAdvance(
            &state,
            FLOPPY144_PLAYER_WALK_FRAME_MS * 4U
        ) &&
        state.walk_frame == 0U,
        "idle character does not animate in place"
    );
}

static void TestQuickDirectionTransitions(void)
{
    Floppy144PlayerVisualState state;

    Floppy144PlayerVisualReset(
        &state
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        -8,
        0,
        F144_ACTION_MOVE_LEFT
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        8,
        0,
        F144_ACTION_MOVE_RIGHT
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_RIGHT &&
        state.moving != 0U,
        "left-to-right transition faces right immediately"
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        0,
        -8,
        F144_ACTION_MOVE_UP
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        -8,
        0,
        F144_ACTION_MOVE_LEFT
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_LEFT,
        "up-to-left transition faces left immediately"
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        0,
        8,
        F144_ACTION_MOVE_DOWN
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        8,
        0,
        F144_ACTION_MOVE_RIGHT
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_RIGHT,
        "down-to-right transition faces right immediately"
    );

    /*
     * The movement model permits simultaneous cardinal actions. Rendering keeps
     * the full diagonal vector but uses the most recently pressed axis as the
     * four-way facing tie-breaker.
     */
    (void)Floppy144PlayerVisualSetMovement(
        &state,
        -8,
        -8,
        F144_ACTION_MOVE_UP
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_UP,
        "diagonal movement honours the most recent vertical action"
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        -8,
        -8,
        F144_ACTION_MOVE_LEFT
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_LEFT,
        "diagonal movement honours the most recent horizontal action"
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        -8,
        0,
        F144_ACTION_NONE
    );

    EXPECT(
        state.facing == (uint8_t)FLOPPY144_PLAYER_FACING_LEFT,
        "releasing one diagonal axis preserves the remaining facing"
    );
}

static void TestAnimationForDirection(
    int32_t x,
    int32_t y,
    F144Action action,
    const char *label
)
{
    Floppy144PlayerVisualState state;
    uint32_t frame_zero;
    uint32_t frame_one;

    Floppy144PlayerVisualReset(
        &state
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        x,
        y,
        action
    );

    frame_zero =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        );

    EXPECT(
        !Floppy144PlayerVisualAdvance(
            &state,
            FLOPPY144_PLAYER_WALK_FRAME_MS - 1U
        ),
        "walk frame is not advanced early"
    );

    EXPECT(
        Floppy144PlayerVisualAdvance(
            &state,
            1U
        ),
        "walk frame advances at the timing boundary"
    );

    frame_one =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        );

    EXPECT(
        frame_zero != frame_one,
        label
    );
}

static void TestAnimation(void)
{
    Floppy144PlayerVisualState state;

    TestAnimationForDirection(
        -8,
        0,
        F144_ACTION_MOVE_LEFT,
        "left walking has visible gait animation"
    );

    TestAnimationForDirection(
        8,
        0,
        F144_ACTION_MOVE_RIGHT,
        "right walking has visible gait animation"
    );

    TestAnimationForDirection(
        0,
        -8,
        F144_ACTION_MOVE_UP,
        "up walking has visible arm/body animation"
    );

    TestAnimationForDirection(
        0,
        8,
        F144_ACTION_MOVE_DOWN,
        "down walking has visible arm/body animation"
    );

    Floppy144PlayerVisualReset(
        &state
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        8,
        0,
        F144_ACTION_MOVE_RIGHT
    );

    EXPECT(
        !Floppy144PlayerVisualAdvance(&state,80U) &&
        Floppy144PlayerVisualAdvance(&state,80U) &&
        state.walk_frame == 1U,
        "variable timing accumulates into one stable walk frame"
    );

    EXPECT(
        Floppy144PlayerVisualAdvance(
            &state,
            FLOPPY144_PLAYER_WALK_FRAME_MS * 3U
        ) &&
        state.walk_frame == 0U,
        "delayed timing advances by deterministic frame parity"
    );

    (void)Floppy144PlayerVisualSetMovement(
        &state,
        0,
        0,
        F144_ACTION_NONE
    );

    EXPECT(
        state.moving == 0U &&
        state.walk_frame == 0U &&
        state.animation_accumulator_ms == 0U,
        "stopping settles animation while retaining direction"
    );
}

static void TestBodyStylesAndRoundHead(void)
{
    Floppy144PlayerVisualState state;
    uint32_t type_a_hash;
    uint32_t type_b_hash;
    int32_t head_centre_x = 320;
    int32_t head_centre_y = 157;
    int32_t head_radius = 8;

    Floppy144PlayerVisualReset(
        &state
    );

    type_a_hash =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        );

    type_b_hash =
        DrawHash(
            &state,
            FLOPPY144_OPERATOR_BODY_STYLE_B
        );

    EXPECT(
        type_a_hash != type_b_hash,
        "Profile Type A and Type B produce distinct player silhouettes"
    );

    /*
     * The round head occupies the top centre of the historical 42x72 Site
     * footprint. A square head would paint its corner; the circle does not.
     */
    (void)DrawHash(
        &state,
        FLOPPY144_OPERATOR_BODY_STYLE_A
    );

    EXPECT(
        g_pixels[
            (uint32_t)(head_centre_y - head_radius) *
            TEST_WIDTH +
            (uint32_t)head_centre_x
        ] != 0U,
        "round head paints its top centre"
    );

    EXPECT(
        g_pixels[
            (uint32_t)(head_centre_y - head_radius) *
            TEST_WIDTH +
            (uint32_t)(head_centre_x - head_radius)
        ] == 0U,
        "round head leaves bounding-box corner transparent"
    );
}

static void TestViewportClipping(void)
{
    Floppy144PlayerVisualState state;
    Floppy144Surface surface =
    {
        g_pixels,
        TEST_WIDTH,
        TEST_HEIGHT
    };

    uint32_t x;
    uint32_t y;

    Floppy144PlayerVisualReset(
        &state
    );

    memset(
        g_pixels,
        0,
        sizeof(g_pixels)
    );

    Floppy144PlayerVisualDraw(
        &surface,
        34,
        92,
        42,
        72,
        24,
        FLOPPY144_OPERATOR_BODY_STYLE_A,
        &state,
        32,
        32,
        576,
        252
    );

    for(y = 0U; y < TEST_HEIGHT; ++y)
    {
        for(x = 0U; x < TEST_WIDTH; ++x)
        {
            if(
                x < 32U ||
                x >= 608U ||
                y < 32U ||
                y >= 284U
            )
            {
                if(g_pixels[y * TEST_WIDTH + x] != 0U)
                {
                    EXPECT(
                        false,
                        "player renderer obeys the S4D-02 viewport clip"
                    );
                    return;
                }
            }
        }
    }

    EXPECT(
        true,
        "player renderer obeys the S4D-02 viewport clip"
    );
}

int main(void)
{
    TestDirectionAndIdle();
    TestQuickDirectionTransitions();
    TestAnimation();
    TestBodyStylesAndRoundHead();
    TestViewportClipping();

    if(g_failures != 0)
    {
        printf(
            "STAGE 4D PLAYER VISUAL TESTS: FAIL (%d)\n",
            g_failures
        );

        return 1;
    }

    printf(
        "STAGE 4D PLAYER VISUAL TESTS: PASS\n"
    );

    return 0;
}
