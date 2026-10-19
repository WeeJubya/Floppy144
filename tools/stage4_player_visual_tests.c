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

#define _CRT_SECURE_NO_WARNINGS 1
#include <stdbool.h>
#include <stdlib.h>
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
    int32_t head_centre_y = 160;
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
     * The fitted hair cap occupies the round skull inside the canonical
     * 42x72 Site footprint. A square head would paint its corner, unlike
     * the supplied circular hair/face silhouette.
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

/*
 * BF36: the head and jacket must touch without a separately rendered
 * skin-coloured neck at the upper torso. The sample is below the round
 * face and above the chest, including both phases of the existing gait.
 * These checks deliberately cover both body styles and all four facings.
 */
static void TestNecklessHeadAttachment(void)
{
    uint32_t style;
    uint32_t facing;
    uint32_t frame;
    const uint32_t skin = FLOPPY144_RGB(208, 166, 157);

    for(style = 0U; style < 2U; ++style)
    {
        for(facing = 0U; facing < FLOPPY144_PLAYER_FACING_COUNT; ++facing)
        {
            for(frame = 0U; frame < 3U; ++frame)
            {
                Floppy144PlayerVisualState state;
                uint32_t band_y;
                uint32_t centre;

                Floppy144PlayerVisualReset(&state);
                state.facing = (uint8_t)facing;
                state.moving = (uint8_t)(frame != 0U);
                state.walk_frame = (uint8_t)(frame == 2U);
                (void)DrawHash(
                    &state,
                    style == 0U ? FLOPPY144_OPERATOR_BODY_STYLE_A :
                                  FLOPPY144_OPERATOR_BODY_STYLE_B
                );

                /* The walking phase has a two-pixel presentation bob. */
                band_y = 172U + (frame != 0U ? 2U : 0U);
                centre = g_pixels[band_y * TEST_WIDTH + 320U];

                EXPECT(
                    centre != skin,
                    "BF36 upper torso has no exposed neck-coloured stalk"
                );
                EXPECT(
                    centre != 0U,
                    "BF36 head and jacket meet without a centreline gap"
                );
            }
        }
    }
}

/*
 * BF36 canonical reference: Type A has only a short hair cap; Type B adds
 * one front-left lock and slightly fuller rear/side coverage. Both must
 * remain visible with the stationary pose and both walking gait phases.
 * Count hair-coloured framebuffer pixels instead of relying on image hashes,
 * which do not establish which direction the new hair coverage extends.
 */
static uint32_t CountReferenceHair(
    uint32_t x0,
    uint32_t y0,
    uint32_t x1,
    uint32_t y1
)
{
    uint32_t x;
    uint32_t y;
    uint32_t count = 0U;
    const uint32_t hair = FLOPPY144_RGB(133, 52, 13);

    for(y = y0; y < y1; ++y)
    {
        for(x = x0; x < x1; ++x)
        {
            if(g_pixels[y * TEST_WIDTH + x] == hair)
            {
                ++count;
            }
        }
    }
    return count;
}

static uint32_t ReferenceHairAtFacing(
    Floppy144OperatorBodyStyle style,
    Floppy144PlayerFacing facing,
    uint32_t gait,
    uint32_t x0,
    uint32_t y0,
    uint32_t x1,
    uint32_t y1
)
{
    Floppy144PlayerVisualState state;
    Floppy144PlayerVisualReset(&state);
    state.facing = (uint8_t)facing;
    state.moving = (uint8_t)(gait != 0U);
    state.walk_frame = (uint8_t)(gait == 2U);
    (void)DrawHash(&state,style);
    return CountReferenceHair(x0,y0,x1,y1);
}

static void TestReferenceMatchedHairCoverage(void)
{
    uint32_t gait;
    uint32_t facing;
    uint32_t baseHair;
    uint32_t longHair;
    const Floppy144PlayerFacing profiles[] =
    {
        FLOPPY144_PLAYER_FACING_RIGHT,
        FLOPPY144_PLAYER_FACING_LEFT
    };

    for(gait = 0U; gait < 3U; ++gait)
    {
        /* Type B's left-front lock runs below the cheek. Type A does not. */
        baseHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_A,
            FLOPPY144_PLAYER_FACING_DOWN,gait,311U,166U,315U,172U
        );
        longHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_B,
            FLOPPY144_PLAYER_FACING_DOWN,gait,311U,166U,315U,172U
        );
        EXPECT(longHair > baseHair,
               "BF36 Type B front-left hair lock is longer than Type A cap");

        /* The opposite front side must not gain a matching second lock. */
        baseHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_A,
            FLOPPY144_PLAYER_FACING_DOWN,gait,325U,166U,329U,172U
        );
        longHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_B,
            FLOPPY144_PLAYER_FACING_DOWN,gait,325U,166U,329U,172U
        );
        EXPECT(longHair <= baseHair,
               "BF36 Type B hair keeps the reference's asymmetric front");

        /* Both rear lower corners retain hair beyond the Type A round cap. */
        baseHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_A,
            FLOPPY144_PLAYER_FACING_UP,gait,313U,166U,317U,173U
        );
        longHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_B,
            FLOPPY144_PLAYER_FACING_UP,gait,313U,166U,317U,173U
        );
        EXPECT(longHair > baseHair,
               "BF36 Type B rear hair covers the lower left side");

        baseHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_A,
            FLOPPY144_PLAYER_FACING_UP,gait,323U,166U,327U,173U
        );
        longHair = ReferenceHairAtFacing(
            FLOPPY144_OPERATOR_BODY_STYLE_B,
            FLOPPY144_PLAYER_FACING_UP,gait,323U,166U,327U,173U
        );
        EXPECT(longHair > baseHair,
               "BF36 Type B rear hair covers the lower right side");

        /* Both profiles have a rounded rear cap; only B extends below it. */
        for(facing = 0U; facing < 2U; ++facing)
        {
            uint32_t x0 = facing == 0U ? 312U : 324U;
            uint32_t x1 = facing == 0U ? 317U : 329U;
            baseHair = ReferenceHairAtFacing(
                FLOPPY144_OPERATOR_BODY_STYLE_A,
                profiles[facing],gait,x0,166U,x1,173U
            );
            longHair = ReferenceHairAtFacing(
                FLOPPY144_OPERATOR_BODY_STYLE_B,
                profiles[facing],gait,x0,166U,x1,173U
            );
            EXPECT(longHair > baseHair,
                   "BF36 Type B profile has fuller hair at the back");
        }
    }
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


/* Real framebuffer fixtures for direct comparison against both user-supplied
 * Characters(1).pptx slide images, never substitute artwork. */
static void CaptureReferenceDirections(void)
{
    const char *dir=getenv("F144_BF25_CAPTURE_DIR");
    const char *labels[]={"front","right","rear","left"};
    uint32_t body,facing,moving,y,x;
    if(dir==NULL||dir[0]=='\0')return;
    for(body=0U;body<2U;++body)
        for(facing=0U;facing<4U;++facing)
            for(moving=0U;moving<2U;++moving)
            {
                Floppy144PlayerVisualState state;
                Floppy144Surface surface={g_pixels,TEST_WIDTH,TEST_HEIGHT};
                char path[512];
                FILE *out;
                Floppy144PlayerVisualReset(&state);
                state.facing=(uint8_t)(facing==0U?
                    FLOPPY144_PLAYER_FACING_DOWN:facing==1U?
                    FLOPPY144_PLAYER_FACING_RIGHT:facing==2U?
                    FLOPPY144_PLAYER_FACING_UP:FLOPPY144_PLAYER_FACING_LEFT);
                state.moving=(uint8_t)moving;
                state.walk_frame=(uint8_t)moving;
                Floppy144DrawClear(&surface,FLOPPY144_RGB(249,249,248));
                Floppy144PlayerVisualDraw(
                    &surface,320,310,122,260,36,
                    body==0U?FLOPPY144_OPERATOR_BODY_STYLE_A:
                        FLOPPY144_OPERATOR_BODY_STYLE_B,
                    &state,0,0,640,360
                );
                (void)snprintf(path,sizeof(path),"%s/bf25_type_%c_%s_%s.ppm",
                    dir,body==0U?'a':'b',labels[facing],
                    moving!=0U?"walk":"idle");
                (void)fopen_s(&out,path,"wb");
                if(out==NULL){EXPECT(false,"open BF25 capture");continue;}
                (void)fprintf(out,"P6\n640 360\n255\n");
                for(y=0U;y<360U;++y)for(x=0U;x<640U;++x)
                {
                    uint32_t pixel=g_pixels[y*640U+x];
                    (void)fputc((int)((pixel>>16U)&255U),out);
                    (void)fputc((int)((pixel>>8U)&255U),out);
                    (void)fputc((int)(pixel&255U),out);
                }
                (void)fclose(out);
            }
}
int main(void)
{
    CaptureReferenceDirections();
    TestDirectionAndIdle();
    TestQuickDirectionTransitions();
    TestAnimation();
    TestBodyStylesAndRoundHead();
    TestNecklessHeadAttachment();
    TestReferenceMatchedHairCoverage();
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
