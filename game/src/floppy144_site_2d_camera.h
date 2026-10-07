/*
 * Floppy//144 - fixed-point Site 2D camera transform
 *
 * Presentation-only conversion between canonical x16 Site coordinates and the
 * fixed 576 x 252 exploration viewport. No collision, interaction or world
 * geometry lives in this module.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_run_state.h"
#include "floppy144_site.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Stage 4D selected scale: 12 px per Site unit, 60% of the historical
 * 20 px/unit presentation.
 */
#define FLOPPY144_SITE_2D_PIXELS_PER_UNIT 12

#define FLOPPY144_SITE_2D_CAMERA_GUTTER_X16 \
    FLOPPY144_SITE_FIXED_ONE

#define FLOPPY144_SITE_2D_CAMERA_TOP_GUTTER_X16 \
    (6 * FLOPPY144_SITE_FIXED_ONE)

#define FLOPPY144_SITE_2D_VIEWPORT_X       32
#define FLOPPY144_SITE_2D_VIEWPORT_Y       32
#define FLOPPY144_SITE_2D_VIEWPORT_WIDTH  576
#define FLOPPY144_SITE_2D_VIEWPORT_HEIGHT 252

typedef struct Floppy144SiteCamera2D
{
    /* Top-left visible point in Site/view coordinates, fixed-point x16. */
    int32_t x16;
    int32_t y16;

    /* Fixed screen-space viewport occupied by the scrolling Site view. */
    int32_t screen_x;
    int32_t screen_y;
    int32_t width;
    int32_t height;

    int32_t pixels_per_unit;
    int32_t visible_width16;
    int32_t visible_height16;
}
Floppy144SiteCamera2D;

typedef struct Floppy144SiteScreenRect
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
}
Floppy144SiteScreenRect;

/*
 * Production camera builder using the selected Stage 4D presentation scale.
 */
bool Floppy144Site2DBuildCamera(
    Floppy144RoomId room,
    const Floppy144RunState *run_state,
    const Floppy144Surface *surface,
    Floppy144SiteCamera2D *camera
);

/*
 * Parameterised form retained for deterministic camera-scale evaluation.
 * Gameplay never calls this with an alternate scale.
 */
bool Floppy144Site2DBuildCameraAtScale(
    Floppy144RoomId room,
    const Floppy144RunState *run_state,
    const Floppy144Surface *surface,
    int32_t pixels_per_unit,
    Floppy144SiteCamera2D *camera
);

int32_t Floppy144Site2DProjectViewX16(
    const Floppy144SiteCamera2D *camera,
    int32_t view_x16
);

int32_t Floppy144Site2DProjectViewY16(
    const Floppy144SiteCamera2D *camera,
    int32_t view_y16
);

void Floppy144Site2DProjectPoint(
    const Floppy144SiteCamera2D *camera,
    int32_t world_x16,
    int32_t world_y16,
    int32_t *screen_x,
    int32_t *screen_y
);

bool Floppy144Site2DProjectRect(
    const Floppy144SiteCamera2D *camera,
    const Floppy144SiteRect *rect,
    Floppy144SiteScreenRect *screen_rect
);
