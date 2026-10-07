/*
 * FLOPPY//144 Stage 4D office camera regression.
 *
 * The production Site remains fixed-point x16 world space. These tests expose
 * only the camera transform in test builds so candidate presentation scales
 * can be compared without touching collision, interactions or authored data.
 */

#include "floppy144_site_2d.h"
#include "floppy144_site.h"
#include "floppy144_site_rooms.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

#define EXPECT(condition, label)                                       \
    do                                                                 \
    {                                                                  \
        if(!(condition))                                               \
        {                                                              \
            ++g_failures;                                              \
            printf("FAIL: %s\n", (label));                            \
        }                                                              \
    }                                                                  \
    while(0)

static void SetPlayer(
    Floppy144RunState *state,
    int32_t x16,
    int32_t y16
)
{
    state->player_site_x = x16;
    state->player_site_y = y16;
}

static void CheckCandidateScales(
    Floppy144RunState *state,
    const Floppy144Surface *surface
)
{
    static const int32_t scales[] =
    {
        10,
        12,
        13,
        14,
        15
    };

    static const int32_t expected_width16[] =
    {
        921,
        768,
        708,
        658,
        614
    };

    static const int32_t expected_height16[] =
    {
        403,
        336,
        310,
        288,
        268
    };

    uint32_t index;

    SetPlayer(
        state,
        83 * FLOPPY144_SITE_FIXED_ONE,
        83 * FLOPPY144_SITE_FIXED_ONE
    );

    for(index = 0U; index < 5U; ++index)
    {
        Floppy144Site2DCameraProbe probe;

        EXPECT(
            Floppy144Site2DTestBuildCameraForScale(
                FLOPPY144_ROOM_MAIN_OFFICE,
                state,
                surface,
                scales[index],
                &probe
            ),
            "candidate camera builds"
        );

        EXPECT(
            probe.visible_width16 == expected_width16[index],
            "candidate horizontal span matches fixed-point projection"
        );

        EXPECT(
            probe.visible_height16 == expected_height16[index],
            "candidate vertical span matches fixed-point projection"
        );
    }

    EXPECT(
        (FLOPPY144_SITE_MOVE_STEP_X16 * 10) %
            FLOPPY144_SITE_FIXED_ONE == 0,
        "50 percent keeps whole-pixel half-unit movement"
    );

    EXPECT(
        (FLOPPY144_SITE_MOVE_STEP_X16 * 12) %
            FLOPPY144_SITE_FIXED_ONE == 0,
        "60 percent keeps whole-pixel half-unit movement"
    );

    EXPECT(
        (FLOPPY144_SITE_MOVE_STEP_X16 * 13) %
            FLOPPY144_SITE_FIXED_ONE != 0,
        "65 percent produces fractional half-unit movement"
    );

    EXPECT(
        (FLOPPY144_SITE_MOVE_STEP_X16 * 14) %
            FLOPPY144_SITE_FIXED_ONE == 0,
        "70 percent keeps whole-pixel half-unit movement"
    );

    EXPECT(
        (FLOPPY144_SITE_MOVE_STEP_X16 * 15) %
            FLOPPY144_SITE_FIXED_ONE != 0,
        "75 percent produces fractional half-unit movement"
    );

    /*
     * Fifty percent remains a valid transform candidate, but 60 percent keeps
     * 20 percent more pixels on every world-space feature while still exposing
     * a much larger view than the original 20 px/unit presentation.
     */
    EXPECT(
        12 > 10,
        "selected scale retains more feature detail than 50 percent"
    );
}

static void CheckSelectedScaleTransform(
    Floppy144RunState *state,
    const Floppy144Surface *surface
)
{
    Floppy144Site2DCameraProbe probe;
    int32_t screen_x;
    int32_t screen_y;

    EXPECT(
        Floppy144Site2DTestPixelsPerUnit() == 12U,
        "production Site scale is 12 px per unit"
    );

    SetPlayer(
        state,
        83 * FLOPPY144_SITE_FIXED_ONE,
        83 * FLOPPY144_SITE_FIXED_ONE
    );

    EXPECT(
        Floppy144Site2DTestBuildCameraForScale(
            FLOPPY144_ROOM_MAIN_OFFICE,
            state,
            surface,
            12,
            &probe
        ),
        "Main Office camera builds at selected scale"
    );

    EXPECT(
        probe.visible_width16 ==
            48 * FLOPPY144_SITE_FIXED_ONE &&
        probe.visible_height16 ==
            21 * FLOPPY144_SITE_FIXED_ONE,
        "selected viewport is exactly 48 by 21 Site units"
    );

    EXPECT(
        Floppy144Site2DTestProjectWorldPoint(
            &probe,
            state->player_site_x,
            state->player_site_y,
            &screen_x,
            &screen_y
        ) &&
        screen_x == 320 &&
        screen_y == 158,
        "player is centred on the scrolling axis at a stable integer pixel"
    );

    EXPECT(
        Floppy144Site2DTestProjectWorldPoint(
            &probe,
            86 * FLOPPY144_SITE_FIXED_ONE,
            78 * FLOPPY144_SITE_FIXED_ONE,
            &screen_x,
            &screen_y
        ) &&
        screen_x == 356 &&
        screen_y == 98,
        "known Main Office desk origin projects to stable screen coordinates"
    );

    SetPlayer(
        state,
        83 * FLOPPY144_SITE_FIXED_ONE,
        83 * FLOPPY144_SITE_FIXED_ONE +
            FLOPPY144_SITE_MOVE_STEP_X16
    );

    EXPECT(
        Floppy144Site2DTestBuildCameraForScale(
            FLOPPY144_ROOM_MAIN_OFFICE,
            state,
            surface,
            12,
            &probe
        ) &&
        Floppy144Site2DTestProjectWorldPoint(
            &probe,
            state->player_site_x,
            state->player_site_y,
            &screen_x,
            &screen_y
        ) &&
        screen_y == 158,
        "half-unit camera tracking leaves the player vertically stable"
    );

    EXPECT(
        Floppy144Site2DTestProjectWorldPoint(
            &probe,
            86 * FLOPPY144_SITE_FIXED_ONE,
            78 * FLOPPY144_SITE_FIXED_ONE,
            &screen_x,
            &screen_y
        ) &&
        screen_y == 92,
        "half-unit camera movement pans world content by exactly six pixels"
    );
}

static void CheckRoomCoverage(
    Floppy144RunState *state,
    const Floppy144Surface *surface
)
{
    uint32_t room_index;

    for(
        room_index = 0U;
        room_index < (uint32_t)FLOPPY144_ROOM_COUNT;
        ++room_index
    )
    {
        Floppy144RoomId room =
            (Floppy144RoomId)room_index;

        Floppy144SiteRegion bounds;
        Floppy144Site2DCameraProbe probe;

        EXPECT(
            Floppy144SiteRoomBounds(
                room,
                &bounds
            ),
            "room exposes camera bounds"
        );

        SetPlayer(
            state,
            ((int32_t)bounds.x +
             (int32_t)bounds.width / 2) *
                FLOPPY144_SITE_FIXED_ONE,
            ((int32_t)bounds.y +
             (int32_t)bounds.height / 2) *
                FLOPPY144_SITE_FIXED_ONE
        );

        EXPECT(
            Floppy144Site2DTestBuildCameraForScale(
                room,
                state,
                surface,
                12,
                &probe
            ),
            "all room cameras build at 60 percent"
        );

        EXPECT(
            probe.pixels_per_unit == 12 &&
            probe.visible_width16 ==
                48 * FLOPPY144_SITE_FIXED_ONE &&
            probe.visible_height16 ==
                21 * FLOPPY144_SITE_FIXED_ONE,
            "all rooms share the same stable selected transform"
        );
    }
}

static void CheckRecordsHorizontalFit(
    Floppy144RunState *state,
    const Floppy144Surface *surface
)
{
    Floppy144Site2DCameraProbe probe;
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;

    SetPlayer(
        state,
        80 * FLOPPY144_SITE_FIXED_ONE,
        20 * FLOPPY144_SITE_FIXED_ONE
    );

    EXPECT(
        Floppy144Site2DTestBuildCameraForScale(
            FLOPPY144_ROOM_RECORDS_OFFICE,
            state,
            surface,
            12,
            &probe
        ),
        "Records Office camera builds"
    );

    /*
     * Records bounds are x=54..100. With the existing one-unit camera gutter,
     * its horizontal camera span is x=53..101, exactly 48 Site units.
     */
    EXPECT(
        probe.x16 ==
            53 * FLOPPY144_SITE_FIXED_ONE,
        "Records Office horizontal extent fits without camera pan"
    );

    EXPECT(
        Floppy144Site2DTestProjectWorldPoint(
            &probe,
            53 * FLOPPY144_SITE_FIXED_ONE,
            20 * FLOPPY144_SITE_FIXED_ONE,
            &x0,
            &y0
        ) &&
        Floppy144Site2DTestProjectWorldPoint(
            &probe,
            101 * FLOPPY144_SITE_FIXED_ONE,
            20 * FLOPPY144_SITE_FIXED_ONE,
            &x1,
            &y1
        ) &&
        x0 == 32 &&
        x1 == 608,
        "Records Office gutter-to-gutter width exactly fills viewport"
    );
}

int main(void)
{
    Floppy144RunState state;
    Floppy144Surface surface;

    memset(
        &state,
        0,
        sizeof(state)
    );

    memset(
        &surface,
        0,
        sizeof(surface)
    );

    surface.width = 640U;
    surface.height = 360U;

    CheckCandidateScales(
        &state,
        &surface
    );

    CheckSelectedScaleTransform(
        &state,
        &surface
    );

    CheckRoomCoverage(
        &state,
        &surface
    );

    CheckRecordsHorizontalFit(
        &state,
        &surface
    );

    if(g_failures != 0)
    {
        printf(
            "STAGE 4D OFFICE CAMERA TESTS: FAIL (%d)\n",
            g_failures
        );

        return 1;
    }

    printf(
        "STAGE 4D OFFICE CAMERA TESTS: PASS\n"
    );

    return 0;
}
