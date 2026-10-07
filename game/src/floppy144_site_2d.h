/*
 * Floppy//144 - scrolling flat 2D Site projection
 *
 * Renders the projection-neutral Site into the runtime backbuffer using a
 * fixed presentation scale and a player-following camera. The Stage 4D
 * renderer keeps the established 576 x 252 viewport inside the 640 x 360
 * interface shell while changing only the world-to-screen scale.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_profile.h"
#include "floppy144_run_state.h"

#if defined(FLOPPY144_SITE_2D_TEST_ACCESS)
/*
 * Test-only camera probe. Production builds do not expose or compile the
 * diagnostic entry points below.
 */
typedef struct Floppy144Site2DCameraProbe
{
    int32_t x16;
    int32_t y16;
    int32_t screen_x;
    int32_t screen_y;
    int32_t width;
    int32_t height;
    int32_t pixels_per_unit;
    int32_t visible_width16;
    int32_t visible_height16;
}
Floppy144Site2DCameraProbe;

uint32_t Floppy144Site2DTestPixelsPerUnit(
    void
);

bool Floppy144Site2DTestBuildCameraForScale(
    Floppy144RoomId room,
    const Floppy144RunState *run_state,
    const Floppy144Surface *surface,
    int32_t pixels_per_unit,
    Floppy144Site2DCameraProbe *probe
);

bool Floppy144Site2DTestProjectWorldPoint(
    const Floppy144Site2DCameraProbe *probe,
    int32_t world_x16,
    int32_t world_y16,
    int32_t *screen_x,
    int32_t *screen_y
);
#endif

void Floppy144Site2DDraw(
    Floppy144Surface *runtime,
    const Floppy144RunState *run_state,
    const char *notice
);

/*
 * Profile-aware draw used by the Stage 4 coordinator. The legacy entry point
 * above remains for Stage 3 regressions and renders the historical Type A.
 */
void Floppy144Site2DDrawForBodyStyle(
    Floppy144Surface *runtime,
    const Floppy144RunState *run_state,
    const char *notice,
    Floppy144OperatorBodyStyle body_style
);
