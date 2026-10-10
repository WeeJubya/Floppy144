/*
 * Floppy//144 - scrolling flat 2D Site projection
 *
 * The authoritative Site remains in canonical 100 x 100 coordinates.
 * This module owns presentation only: a fixed-zoom camera, viewport culling
 * and lightweight procedural furniture detail. Canonical Site coordinates are
 * already stored in player-facing orientation.
 *
 * FM-23 can later replace this projection without changing geometry,
 * collision, interactions or the player's persistent Site position.
 */

#include "floppy144_site_2d.h"
#include "floppy144_site_2d_camera.h"
#include "floppy144_player_visual.h"

#include "floppy144_draw.h"
#include "floppy144_drawing_runtime.h"
#include "floppy144_game_data.h"
#include "floppy144_cabinet.h"
#include "floppy144_site.h"
#include "floppy144_site_object.h"
#include "floppy144_grey_door.h"
#include "floppy144_site_rooms.h"
#include "floppy144_site_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * Stage 4D presentation scale.
 *
 * The camera transform owns the selected 12 px/Site-unit (60%) scale. This
 * renderer consumes only projected screen coordinates; canonical world,
 * collision and interaction geometry remain unchanged.
 */
#define FLOPPY144_SITE_2D_PLAYER_VISUAL_HEIGHT_X16 (6 * FLOPPY144_SITE_FIXED_ONE)

/*
 * Site Exploration shell.
 *
 * The scrolling world is deliberately inset into the same central window used
 * by the original Stage 1 Office view. The surrounding UI remains fixed so
 * reconstruction status, room/notice text and interaction prompts are never
 * obscured by the camera.
 */

#define FLOPPY144_SITE_2D_FRAME_X          20
#define FLOPPY144_SITE_2D_FRAME_Y          20
#define FLOPPY144_SITE_2D_FRAME_WIDTH     600
#define FLOPPY144_SITE_2D_FRAME_HEIGHT    276

#define FLOPPY144_SITE_2D_ROOM_LABEL_X     36
#define FLOPPY144_SITE_2D_ROOM_LABEL_Y    300

#define FLOPPY144_SITE_2D_FOOTER_X         20
#define FLOPPY144_SITE_2D_FOOTER_Y        312
#define FLOPPY144_SITE_2D_FOOTER_WIDTH    600
#define FLOPPY144_SITE_2D_FOOTER_HEIGHT    28

typedef struct Floppy144SiteStyle2D
{
    uint8_t draw_outline;
    uint32_t colour;
}
Floppy144SiteStyle2D;

typedef struct Floppy144SitePoint2D
{
    int32_t x;
    int32_t y;
}
Floppy144SitePoint2D;

static const Floppy144SiteStyle2D
floppy144_site_styles[FLOPPY144_SITE_ELEMENT_COUNT] =
{
    { 0U, FLOPPY144_RGB(61,  67,  65) },
    { 0U, FLOPPY144_RGB(67,  71,  67) },
    { 0U, FLOPPY144_RGB(56,  64,  65) },
    { 0U, FLOPPY144_RGB(73,  74,  67) },

    { 1U, FLOPPY144_RGB(184, 133,  67) },
    { 1U, FLOPPY144_RGB(100, 151, 166) },
    { 1U, FLOPPY144_RGB(111, 108,  97) },
    { 1U, FLOPPY144_RGB(75,   78,  74) },
    { 1U, FLOPPY144_RGB(96,   99,  94) },
    { 1U, FLOPPY144_RGB(73,   77,  75) },
    { 1U, FLOPPY144_RGB(52,   57,  57) },
    { 1U, FLOPPY144_RGB(152, 159, 155) },
    { 1U, FLOPPY144_RGB(93,   83,  67) },
    { 1U, FLOPPY144_RGB(67,   91, 104) },
    { 1U, FLOPPY144_RGB(134, 132, 120) },
    { 1U, FLOPPY144_RGB(137, 140, 132) },
    { 1U, FLOPPY144_RGB(116, 112, 101) },
    { 1U, FLOPPY144_RGB(103, 118, 116) },
    { 1U, FLOPPY144_RGB(75,   67,  58) },
    { 1U, FLOPPY144_RGB(103,  84,  84) },
    { 1U, FLOPPY144_RGB(54,   68,  77) },
    { 1U, FLOPPY144_RGB(84,   80,  68) },
    { 1U, FLOPPY144_RGB(98,   91,  81) },
    { 1U, FLOPPY144_RGB(120, 105,  85) }
};

static bool Floppy144Site2DIsFloor(
    Floppy144SiteElement element
)
{
    return
        element >= FLOPPY144_SITE_FLOOR_A &&
        element <= FLOPPY144_SITE_FLOOR_D;
}

/*
 * Room visibility from explicit boundary ownership.
 *
 * Ordinary geometry is visible only in its owning room. Boundary geometry is
 * visible only from one of its endpoints, and an internal boundary does not
 * appear until both endpoint rooms have been reconstructed. OUTSIDE is not a
 * reconstructed room and therefore never blocks an exterior door/window.
 */
static bool Floppy144Site2DRectVisibleInRoom(
    const Floppy144RunState *run_state,
    Floppy144RoomId room,
    const Floppy144SiteRect *rect
)
{
    if(
        rect == NULL ||
        !Floppy144SiteRectRuntimeVisible(
            run_state,
            rect
        ) ||
        !Floppy144SiteWallFixtureVisibleFromPlayer(run_state, rect)
    )
    {
        return false;
    }

    /* Ordinary geometry belongs only to its owning active room. */
    if(rect->from_room == rect->to_room)
    {
        return rect->room == (uint8_t)room;
    }

    /*
     * Runtime visibility has already verified both boundary endpoints. The 2D
     * camera now only needs to decide whether this active room owns one side.
     */
    return
        rect->from_room == (uint8_t)room ||
        rect->to_room == (uint8_t)room;
}

static bool Floppy144Site2DClipRect(
    const Floppy144Surface *surface,
    int32_t *x,
    int32_t *y,
    int32_t *width,
    int32_t *height
)
{
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;

    if(
        surface == NULL ||
        x == NULL ||
        y == NULL ||
        width == NULL ||
        height == NULL ||
        *width <= 0 ||
        *height <= 0
    )
    {
        return false;
    }

    x0 = *x;
    y0 = *y;
    x1 = x0 + *width;
    y1 = y0 + *height;

    if(
        x1 <= FLOPPY144_SITE_2D_VIEWPORT_X ||
        y1 <= FLOPPY144_SITE_2D_VIEWPORT_Y ||
        x0 >= FLOPPY144_SITE_2D_VIEWPORT_X + FLOPPY144_SITE_2D_VIEWPORT_WIDTH ||
        y0 >= FLOPPY144_SITE_2D_VIEWPORT_Y + FLOPPY144_SITE_2D_VIEWPORT_HEIGHT
    )
    {
        return false;
    }

    if(x0 < FLOPPY144_SITE_2D_VIEWPORT_X)
    {
        x0 = FLOPPY144_SITE_2D_VIEWPORT_X;
    }

    if(y0 < FLOPPY144_SITE_2D_VIEWPORT_Y)
    {
        y0 = FLOPPY144_SITE_2D_VIEWPORT_Y;
    }

    if(x1 > FLOPPY144_SITE_2D_VIEWPORT_X + FLOPPY144_SITE_2D_VIEWPORT_WIDTH)
    {
        x1 = FLOPPY144_SITE_2D_VIEWPORT_X + FLOPPY144_SITE_2D_VIEWPORT_WIDTH;
    }

    if(y1 > FLOPPY144_SITE_2D_VIEWPORT_Y + FLOPPY144_SITE_2D_VIEWPORT_HEIGHT)
    {
        y1 = FLOPPY144_SITE_2D_VIEWPORT_Y + FLOPPY144_SITE_2D_VIEWPORT_HEIGHT;
    }

    *x = x0;
    *y = y0;
    *width = x1 - x0;
    *height = y1 - y0;

    return
        *width > 0 &&
        *height > 0;
}

static void Floppy144Site2DFill(
    Floppy144Surface *surface,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t colour
)
{
    if(
        Floppy144Site2DClipRect(
            surface,
            &x,
            &y,
            &width,
            &height
        )
    )
    {
        Floppy144DrawFillRect(
            surface,
            (uint32_t)x,
            (uint32_t)y,
            (uint32_t)width,
            (uint32_t)height,
            colour
        );
    }
}

static void Floppy144Site2DOutline(
    Floppy144Surface *surface,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t colour
)
{
    if(
        surface == NULL ||
        width <= 0 ||
        height <= 0
    )
    {
        return;
    }

    /*
     * Preserve the authored rectangle when it crosses the camera edge.
     * Clipping the rectangle first and then outlining the clipped result
     * creates a false edge at the viewport and makes objects appear to resize.
     */
    Floppy144Site2DFill(
        surface,
        x,
        y,
        width,
        1,
        colour
    );

    if(height > 1)
    {
        Floppy144Site2DFill(
            surface,
            x,
            y + height - 1,
            width,
            1,
            colour
        );
    }

    if(height > 2)
    {
        Floppy144Site2DFill(
            surface,
            x,
            y + 1,
            1,
            height - 2,
            colour
        );

        if(width > 1)
        {
            Floppy144Site2DFill(
                surface,
                x + width - 1,
                y + 1,
                1,
                height - 2,
                colour
            );
        }
    }
}


/*
 * Tiny clipped pixel/line helpers used only for diagonal furniture detail.
 * Keeping them local avoids growing the shared drawing API for one Site-only
 * presentation requirement.
 */
static void Floppy144Site2DPixel(
    Floppy144Surface *surface,
    int32_t x,
    int32_t y,
    uint32_t colour
)
{
    if(
        surface == NULL ||
        surface->pixels == NULL ||
        x < FLOPPY144_SITE_2D_VIEWPORT_X ||
        y < FLOPPY144_SITE_2D_VIEWPORT_Y ||
        x >= FLOPPY144_SITE_2D_VIEWPORT_X +
            FLOPPY144_SITE_2D_VIEWPORT_WIDTH ||
        y >= FLOPPY144_SITE_2D_VIEWPORT_Y +
            FLOPPY144_SITE_2D_VIEWPORT_HEIGHT ||
        x < 0 ||
        y < 0 ||
        (uint32_t)x >= surface->width ||
        (uint32_t)y >= surface->height
    )
    {
        return;
    }

    surface->pixels[
        (uint32_t)y * surface->width +
        (uint32_t)x
    ] = colour;
}

static void Floppy144Site2DLine(
    Floppy144Surface *surface,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    uint32_t colour
)
{
    int32_t dx = x1 >= x0 ? x1 - x0 : x0 - x1;
    int32_t sx = x0 < x1 ? 1 : -1;
    int32_t dy = -(y1 >= y0 ? y1 - y0 : y0 - y1);
    int32_t sy = y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;

    for(;;)
    {
        int32_t error2;

        Floppy144Site2DPixel(
            surface,
            x0,
            y0,
            colour
        );

        if(x0 == x1 && y0 == y1)
        {
            break;
        }

        error2 = 2 * error;

        if(error2 >= dy)
        {
            error += dy;
            x0 += sx;
        }

        if(error2 <= dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}

/*
 * Draw a rhombus inscribed inside one projected Site rectangle.
 *
 * The JSON x/y/width/height remain the conservative physical bounds. Diagonal
 * rotation is therefore a presentation orientation inside those bounds, not a
 * second geometry transform.
 */
static void Floppy144Site2DFillDiamond(
    Floppy144Surface *surface,
    const Floppy144SiteScreenRect *screen_rect,
    uint32_t colour
)
{
    int32_t top;
    int32_t bottom;
    int32_t left;
    int32_t right;
    int32_t centre_x;
    int32_t centre_y;
    int32_t y;

    if(
        screen_rect == NULL ||
        screen_rect->width <= 1 ||
        screen_rect->height <= 1
    )
    {
        return;
    }

    left = screen_rect->x;
    right = screen_rect->x + screen_rect->width - 1;
    top = screen_rect->y;
    bottom = screen_rect->y + screen_rect->height - 1;

    centre_x = (left + right) / 2;
    centre_y = (top + bottom) / 2;

    for(y = top; y <= bottom; ++y)
    {
        int32_t x0;
        int32_t x1;

        if(y <= centre_y)
        {
            int32_t span = centre_y - top;
            int32_t step = y - top;

            if(span <= 0)
            {
                x0 = left;
                x1 = right;
            }
            else
            {
                x0 =
                    centre_x -
                    ((centre_x - left) * step) /
                    span;

                x1 =
                    centre_x +
                    ((right - centre_x) * step) /
                    span;
            }
        }
        else
        {
            int32_t span = bottom - centre_y;
            int32_t step = bottom - y;

            if(span <= 0)
            {
                x0 = left;
                x1 = right;
            }
            else
            {
                x0 =
                    centre_x -
                    ((centre_x - left) * step) /
                    span;

                x1 =
                    centre_x +
                    ((right - centre_x) * step) /
                    span;
            }
        }

        Floppy144Site2DFill(
            surface,
            x0,
            y,
            x1 - x0 + 1,
            1,
            colour
        );
    }
}

static void Floppy144Site2DOutlineDiamond(
    Floppy144Surface *surface,
    const Floppy144SiteScreenRect *screen_rect,
    uint32_t colour
)
{
    int32_t left;
    int32_t right;
    int32_t top;
    int32_t bottom;
    int32_t centre_x;
    int32_t centre_y;

    if(
        screen_rect == NULL ||
        screen_rect->width <= 1 ||
        screen_rect->height <= 1
    )
    {
        return;
    }

    left = screen_rect->x;
    right = screen_rect->x + screen_rect->width - 1;
    top = screen_rect->y;
    bottom = screen_rect->y + screen_rect->height - 1;
    centre_x = (left + right) / 2;
    centre_y = (top + bottom) / 2;

    Floppy144Site2DLine(
        surface,
        centre_x,
        top,
        right,
        centre_y,
        colour
    );

    Floppy144Site2DLine(
        surface,
        right,
        centre_y,
        centre_x,
        bottom,
        colour
    );

    Floppy144Site2DLine(
        surface,
        centre_x,
        bottom,
        left,
        centre_y,
        colour
    );

    Floppy144Site2DLine(
        surface,
        left,
        centre_y,
        centre_x,
        top,
        colour
    );
}

static int32_t Floppy144Site2DNormalisedRotation(
    int16_t rotation
)
{
    int32_t normalised =
        (int32_t)rotation % 360;

    if(normalised < 0)
    {
        normalised += 360;
    }

    return normalised;
}

static bool Floppy144Site2DRotationIsDiagonal(
    int16_t rotation
)
{
    return
        (Floppy144Site2DNormalisedRotation(rotation) % 90) != 0;
}

/*
 * Recover the two local axes of a diagonal authored rectangle inside its
 * conservative screen-space AABB.
 *
 * The generated Site definition already carries the original unrotated width
 * and height. Keeping those values lets a 6x4 desk remain a 6x4 rectangle when
 * turned 45 degrees instead of becoming an 8x8 diamond.
 */
static bool Floppy144Site2DDiagonalAxes(
    const Floppy144SiteRect *rect,
    const Floppy144SiteScreenRect *screen_rect,
    int32_t *centre_x,
    int32_t *centre_y,
    int32_t *u_x,
    int32_t *u_y,
    int32_t *v_x,
    int32_t *v_y
)
{
    int32_t width_part_x;
    int32_t width_part_y;
    int32_t height_part_x;
    int32_t height_part_y;
    uint16_t facing;

    if(
        rect == NULL ||
        screen_rect == NULL ||
        centre_x == NULL ||
        centre_y == NULL ||
        u_x == NULL ||
        u_y == NULL ||
        v_x == NULL ||
        v_y == NULL ||
        screen_rect->width <= 1 ||
        screen_rect->height <= 1
    )
    {
        return false;
    }

    if(
        rect->authored_width16 == 0U ||
        rect->authored_height16 == 0U
    )
    {
        return false;
    }

    *centre_x =
        screen_rect->x +
        (screen_rect->width - 1) / 2;

    *centre_y =
        screen_rect->y +
        (screen_rect->height - 1) / 2;

    /*
     * screen_rect is the conservative collision/culling AABB and may be up to
     * a whole Site unit larger on each axis than the actual rotated furniture.
     * Render from the authored dimensions at the fixed Site scale instead.
     *
     * For a 45-degree local axis:
     * component = half-length * 1/sqrt(2).
     * 181/256 is a compact integer approximation of 1/sqrt(2).
     */
    width_part_x =
        (int32_t)(
            (
                (int64_t)rect->authored_width16 *
                FLOPPY144_SITE_2D_PIXELS_PER_UNIT *
                181
            ) /
            (
                (int64_t)FLOPPY144_SITE_FIXED_ONE *
                512
            )
        );

    width_part_y = width_part_x;

    height_part_x =
        (int32_t)(
            (
                (int64_t)rect->authored_height16 *
                FLOPPY144_SITE_2D_PIXELS_PER_UNIT *
                181
            ) /
            (
                (int64_t)FLOPPY144_SITE_FIXED_ONE *
                512
            )
        );

    height_part_y = height_part_x;

    if(
        width_part_x <= 0 ||
        height_part_x <= 0
    )
    {
        return false;
    }

    facing =
        (uint16_t)(
            (
                (
                    Floppy144Site2DNormalisedRotation(
                        rect->rotation
                    ) + 22
                ) / 45
            ) * 45
        ) % 360;

    switch(facing)
    {
        case 45U:
            *u_x = width_part_x;
            *u_y = width_part_y;
            *v_x = -height_part_x;
            *v_y = height_part_y;
            break;

        case 135U:
            *u_x = -width_part_x;
            *u_y = width_part_y;
            *v_x = -height_part_x;
            *v_y = -height_part_y;
            break;

        case 225U:
            *u_x = -width_part_x;
            *u_y = -width_part_y;
            *v_x = height_part_x;
            *v_y = -height_part_y;
            break;

        case 315U:
            *u_x = width_part_x;
            *u_y = -width_part_y;
            *v_x = height_part_x;
            *v_y = height_part_y;
            break;

        default:
            return false;
    }

    return true;
}

static void Floppy144Site2DBuildQuad(
    Floppy144SitePoint2D points[4],
    int32_t centre_x,
    int32_t centre_y,
    int32_t u_x,
    int32_t u_y,
    int32_t v_x,
    int32_t v_y
)
{
    points[0].x = centre_x - u_x - v_x;
    points[0].y = centre_y - u_y - v_y;

    points[1].x = centre_x + u_x - v_x;
    points[1].y = centre_y + u_y - v_y;

    points[2].x = centre_x + u_x + v_x;
    points[2].y = centre_y + u_y + v_y;

    points[3].x = centre_x - u_x + v_x;
    points[3].y = centre_y - u_y + v_y;
}

static void Floppy144Site2DFillQuad(
    Floppy144Surface *surface,
    const Floppy144SitePoint2D points[4],
    uint32_t colour
)
{
    int32_t minimum_y;
    int32_t maximum_y;
    int32_t y;
    uint32_t point_index;

    if(surface == NULL || points == NULL)
    {
        return;
    }

    minimum_y = points[0].y;
    maximum_y = points[0].y;

    for(point_index = 1U; point_index < 4U; ++point_index)
    {
        if(points[point_index].y < minimum_y)
            minimum_y = points[point_index].y;

        if(points[point_index].y > maximum_y)
            maximum_y = points[point_index].y;
    }

    for(y = minimum_y; y <= maximum_y; ++y)
    {
        bool found = false;
        int32_t minimum_x = 0;
        int32_t maximum_x = 0;

        for(point_index = 0U; point_index < 4U; ++point_index)
        {
            const Floppy144SitePoint2D *a =
                &points[point_index];

            const Floppy144SitePoint2D *b =
                &points[(point_index + 1U) % 4U];

            int32_t edge_min_y;
            int32_t edge_max_y;
            int32_t x;

            if(a->y == b->y)
                continue;

            edge_min_y = a->y < b->y ? a->y : b->y;
            edge_max_y = a->y > b->y ? a->y : b->y;

            if(y < edge_min_y || y > edge_max_y)
                continue;

            x =
                a->x +
                (int32_t)(
                    ((int64_t)(y - a->y) *
                    (int64_t)(b->x - a->x)) /
                    (int64_t)(b->y - a->y)
                );

            if(!found)
            {
                minimum_x = x;
                maximum_x = x;
                found = true;
            }
            else
            {
                if(x < minimum_x)
                    minimum_x = x;

                if(x > maximum_x)
                    maximum_x = x;
            }
        }

        if(found)
        {
            Floppy144Site2DFill(
                surface,
                minimum_x,
                y,
                maximum_x - minimum_x + 1,
                1,
                colour
            );
        }
    }
}

static void Floppy144Site2DOutlineQuad(
    Floppy144Surface *surface,
    const Floppy144SitePoint2D points[4],
    uint32_t colour
)
{
    uint32_t point_index;

    if(surface == NULL || points == NULL)
    {
        return;
    }

    for(point_index = 0U; point_index < 4U; ++point_index)
    {
        const Floppy144SitePoint2D *a =
            &points[point_index];

        const Floppy144SitePoint2D *b =
            &points[(point_index + 1U) % 4U];

        Floppy144Site2DLine(
            surface,
            a->x,
            a->y,
            b->x,
            b->y,
            colour
        );
    }
}

static bool Floppy144Site2DRoomOwnsCell(
    Floppy144RoomId room,
    int32_t x,
    int32_t y
)
{
    if(
        x < 0 || y < 0 ||
        x >= FLOPPY144_SITE_SIZE_UNITS ||
        y >= FLOPPY144_SITE_SIZE_UNITS
    )
    {
        return false;
    }

    return
        Floppy144SiteRoomContainsCell(
            room,
            (uint8_t)x,
            (uint8_t)y
        );
}

/*
 * Draw one whole Site-unit wall cell.
 *
 * Doors and windows in the authored Site are one Site unit deep. Room walls
 * now use the same physical depth and the same world-to-view projection path,
 * so walls, doors and windows occupy exactly the same plane on every side of
 * a room. This also avoids pixel-level centring differences after rotation.
 */
static void Floppy144Site2DDrawWallCell(
    Floppy144Surface *surface,
    const Floppy144SiteCamera2D *camera,
    int32_t world_x,
    int32_t world_y,
    uint32_t colour
)
{
    Floppy144SiteRect wall_rect;
    Floppy144SiteScreenRect screen_rect;

    if(
        surface == NULL ||
        camera == NULL ||
        world_x < 0 ||
        world_y < 0 ||
        world_x >= FLOPPY144_SITE_SIZE_UNITS ||
        world_y >= FLOPPY144_SITE_SIZE_UNITS
    )
    {
        return;
    }

    wall_rect.type = (uint8_t)FLOPPY144_SITE_PARTITION_WALL;
    wall_rect.room = FLOPPY144_SITE_ROOM_SHARED;
    wall_rect.from_room = FLOPPY144_SITE_ROOM_SHARED;
    wall_rect.to_room = FLOPPY144_SITE_ROOM_SHARED;
    wall_rect.rotation = 0U;
    wall_rect.x = (uint8_t)world_x;
    wall_rect.y = (uint8_t)world_y;
    wall_rect.width = 1U;
    wall_rect.height = 1U;
    wall_rect.authored_width16 = FLOPPY144_SITE_FIXED_ONE;
    wall_rect.authored_height16 = FLOPPY144_SITE_FIXED_ONE;

    if(
        !Floppy144Site2DProjectRect(
            camera,
            &wall_rect,
            &screen_rect
        )
    )
    {
        return;
    }

    Floppy144Site2DFill(
        surface,
        screen_rect.x,
        screen_rect.y,
        screen_rect.width,
        screen_rect.height,
        colour
    );
}

/*
 * Draw the exposed perimeter of the active room's FLOOR_* union.
 *
 * The authored floors are inset by one Site unit from their containing room
 * shell. Each exposed floor edge therefore receives a one-unit wall cell on
 * the OUTSIDE of the floor rather than a thin line centred on the floor edge.
 *
 * That is the same world-space shell occupied by one-unit-deep doors and
 * windows. Boundary geometry is drawn later, so an available door/window
 * cleanly replaces the wall cell beneath it while an unavailable boundary
 * remains a continuous wall.
 *
 * Convex corners receive their own one-unit corner cell. This closes the four
 * small gaps that can otherwise appear where perpendicular wall runs meet.
 */
static void Floppy144Site2DDrawRoomBorder(
    Floppy144Surface *surface,
    const Floppy144SiteCamera2D *camera,
    Floppy144RoomId room
)
{
    const uint32_t border_colour =
        FLOPPY144_RGB(156, 161, 151);

    uint32_t index;
    uint32_t count = Floppy144SiteRectCount();

    for(index = 0U; index < count; ++index)
    {
        const Floppy144SiteRect *rect = Floppy144SiteRectAt(index);
        int32_t x;
        int32_t y;
        int32_t x0;
        int32_t y0;
        int32_t x1;
        int32_t y1;

        bool top_left_corner;
        bool top_right_corner;
        bool bottom_left_corner;
        bool bottom_right_corner;

        if(
            rect == NULL ||
            rect->room != (uint8_t)room ||
            !Floppy144Site2DIsFloor((Floppy144SiteElement)rect->type)
        )
        {
            continue;
        }

        x0 = rect->x;
        y0 = rect->y;
        x1 = (int32_t)rect->x + (int32_t)rect->width;
        y1 = (int32_t)rect->y + (int32_t)rect->height;

        /*
         * Horizontal wall runs.
         *
         * Top walls occupy y0 - 1; bottom walls occupy y1. This mirrors the
         * authored boundary rectangles instead of centring a stroke on y0/y1.
         */
        for(x = x0; x < x1; ++x)
        {
            if(!Floppy144Site2DRoomOwnsCell(room, x, y0 - 1))
            {
                Floppy144Site2DDrawWallCell(
                    surface,
                    camera,
                    x,
                    y0 - 1,
                    border_colour
                );
            }

            if(!Floppy144Site2DRoomOwnsCell(room, x, y1))
            {
                Floppy144Site2DDrawWallCell(
                    surface,
                    camera,
                    x,
                    y1,
                    border_colour
                );
            }
        }

        /*
         * Vertical wall runs.
         *
         * Left walls occupy x0 - 1; right walls occupy x1. These cells are
         * exactly one Site unit deep, matching vertical door/window rectangles.
         */
        for(y = y0; y < y1; ++y)
        {
            if(!Floppy144Site2DRoomOwnsCell(room, x0 - 1, y))
            {
                Floppy144Site2DDrawWallCell(
                    surface,
                    camera,
                    x0 - 1,
                    y,
                    border_colour
                );
            }

            if(!Floppy144Site2DRoomOwnsCell(room, x1, y))
            {
                Floppy144Site2DDrawWallCell(
                    surface,
                    camera,
                    x1,
                    y,
                    border_colour
                );
            }
        }

        /*
         * Fill convex corner shell cells only when both adjoining exterior
         * directions are exposed. Multi-rectangle rooms therefore keep their
         * internal joins open while true outer corners remain solid.
         */
        top_left_corner =
            !Floppy144Site2DRoomOwnsCell(room, x0 - 1, y0) &&
            !Floppy144Site2DRoomOwnsCell(room, x0, y0 - 1) &&
            !Floppy144Site2DRoomOwnsCell(room, x0 - 1, y0 - 1);

        top_right_corner =
            !Floppy144Site2DRoomOwnsCell(room, x1, y0) &&
            !Floppy144Site2DRoomOwnsCell(room, x1 - 1, y0 - 1) &&
            !Floppy144Site2DRoomOwnsCell(room, x1, y0 - 1);

        bottom_left_corner =
            !Floppy144Site2DRoomOwnsCell(room, x0 - 1, y1 - 1) &&
            !Floppy144Site2DRoomOwnsCell(room, x0, y1) &&
            !Floppy144Site2DRoomOwnsCell(room, x0 - 1, y1);

        bottom_right_corner =
            !Floppy144Site2DRoomOwnsCell(room, x1, y1 - 1) &&
            !Floppy144Site2DRoomOwnsCell(room, x1 - 1, y1) &&
            !Floppy144Site2DRoomOwnsCell(room, x1, y1);

        if(top_left_corner)
        {
            Floppy144Site2DDrawWallCell(
                surface,
                camera,
                x0 - 1,
                y0 - 1,
                border_colour
            );
        }

        if(top_right_corner)
        {
            Floppy144Site2DDrawWallCell(
                surface,
                camera,
                x1,
                y0 - 1,
                border_colour
            );
        }

        if(bottom_left_corner)
        {
            Floppy144Site2DDrawWallCell(
                surface,
                camera,
                x0 - 1,
                y1,
                border_colour
            );
        }

        if(bottom_right_corner)
        {
            Floppy144Site2DDrawWallCell(
                surface,
                camera,
                x1,
                y1,
                border_colour
            );
        }
    }
}

static void Floppy144Site2DDrawFurnitureBase(
    Floppy144Surface *surface,
    const Floppy144SiteScreenRect *screen_rect,
    uint32_t body_colour,
    uint32_t edge_colour,
    uint32_t highlight_colour,
    uint32_t shadow_colour
)
{
    if(screen_rect == NULL)
    {
        return;
    }

    /*
     * Keep all furniture presentation inside the authored footprint.
     *
     * The previous external +3px drop shadow was harmless while objects had
     * generous clearance, but Full Site contains legitimate edge-touching
     * furniture and furniture flush with room walls. That shadow could paint
     * across an adjacent bookcase/cabinet or over the one-unit wall shell.
     */
    Floppy144Site2DFill(
        surface,
        screen_rect->x,
        screen_rect->y,
        screen_rect->width,
        screen_rect->height,
        body_colour
    );

    if(
        screen_rect->width >= 8 &&
        screen_rect->height >= 8
    )
    {
        Floppy144Site2DFill(
            surface,
            screen_rect->x +
                screen_rect->width - 3,
            screen_rect->y + 2,
            2,
            screen_rect->height - 4,
            shadow_colour
        );

        Floppy144Site2DFill(
            surface,
            screen_rect->x + 2,
            screen_rect->y +
                screen_rect->height - 3,
            screen_rect->width - 4,
            2,
            shadow_colour
        );

        Floppy144Site2DFill(
            surface,
            screen_rect->x + 2,
            screen_rect->y + 2,
            screen_rect->width - 4,
            2,
            highlight_colour
        );
    }

    Floppy144Site2DOutline(
        surface,
        screen_rect->x,
        screen_rect->y,
        screen_rect->width,
        screen_rect->height,
        edge_colour
    );
}

static void Floppy144Site2DDrawDiagonalFurnitureBase(
    Floppy144Surface *surface,
    const Floppy144SiteRect *rect,
    const Floppy144SiteScreenRect *screen_rect,
    uint32_t body_colour,
    uint32_t edge_colour,
    uint32_t highlight_colour,
    uint32_t shadow_colour
)
{
    Floppy144SitePoint2D points[4];
    int32_t centre_x;
    int32_t centre_y;
    int32_t u_x;
    int32_t u_y;
    int32_t v_x;
    int32_t v_y;

    if(
        rect == NULL ||
        screen_rect == NULL
    )
    {
        return;
    }

    if(
        !Floppy144Site2DDiagonalAxes(
            rect,
            screen_rect,
            &centre_x,
            &centre_y,
            &u_x,
            &u_y,
            &v_x,
            &v_y
        )
    )
    {
        Floppy144Site2DFillDiamond(
            surface,
            screen_rect,
            body_colour
        );

        Floppy144Site2DOutlineDiamond(
            surface,
            screen_rect,
            edge_colour
        );

        return;
    }

    Floppy144Site2DBuildQuad(
        points,
        centre_x,
        centre_y,
        u_x,
        u_y,
        v_x,
        v_y
    );

    Floppy144Site2DFillQuad(
        surface,
        points,
        body_colour
    );

    /*
     * Keep the same restrained top-left light / lower-right shadow language as
     * the cardinal furniture, but follow the true rotated rectangle edges.
     */
    Floppy144Site2DLine(
        surface,
        points[3].x,
        points[3].y,
        points[0].x,
        points[0].y,
        highlight_colour
    );

    Floppy144Site2DLine(
        surface,
        points[0].x,
        points[0].y,
        points[1].x,
        points[1].y,
        highlight_colour
    );

    Floppy144Site2DLine(
        surface,
        points[1].x,
        points[1].y,
        points[2].x,
        points[2].y,
        shadow_colour
    );

    Floppy144Site2DLine(
        surface,
        points[2].x,
        points[2].y,
        points[3].x,
        points[3].y,
        shadow_colour
    );

    Floppy144Site2DOutlineQuad(
        surface,
        points,
        edge_colour
    );
}

static void Floppy144Site2DDrawCabinetDetails(
    Floppy144Surface *surface,
    const Floppy144SiteScreenRect *screen_rect,
    uint32_t edge_colour,
    uint32_t handle_colour
)
{
    int32_t index;

    if(screen_rect == NULL)
    {
        return;
    }

    for(index = 1; index < 4; ++index)
    {
        if(screen_rect->width >= screen_rect->height)
        {
            int32_t x =
                screen_rect->x +
                (screen_rect->width * index) / 4;

            Floppy144Site2DFill(
                surface,
                x,
                screen_rect->y + 2,
                1,
                screen_rect->height - 4,
                edge_colour
            );
        }
        else
        {
            int32_t y =
                screen_rect->y +
                (screen_rect->height * index) / 4;

            Floppy144Site2DFill(
                surface,
                screen_rect->x + 2,
                y,
                screen_rect->width - 4,
                1,
                edge_colour
            );
        }
    }

    Floppy144Site2DFill(
        surface,
        screen_rect->x + screen_rect->width / 2 - 3,
        screen_rect->y + screen_rect->height / 2 - 1,
        6,
        2,
        handle_colour
    );
}


/*
 * Stage 3C Task 12 renderer bridge.
 *
 * SiteRect remains deliberately compact, so presentation metadata is resolved
 * from the generated furniture/fixture records using the authored room and
 * geometry. This keeps variant-specific artwork out of collision/persistence
 * state while allowing the renderer to honour the JSON drawing contract.
 */
static const Floppy144DataRecord *Floppy144Site2DPlacementForRect(
    const Floppy144SiteRect *rect
)
{
    uint32_t index;

    if(rect == NULL || rect->room >= (uint8_t)FLOPPY144_ROOM_COUNT)
    {
        return NULL;
    }

    for(index = 0U; index < Floppy144GameDataRecordCount(); ++index)
    {
        const Floppy144DataRecord *record =
            Floppy144GameDataRecordAt(index);

        if(
            record == NULL ||
            (
                record->eKind != FLOPPY144_DATA_FURNITURE &&
                record->eKind != FLOPPY144_DATA_FIXTURE
            ) ||
            record->pszA == NULL ||
            Floppy144GameDataRoomId(record->pszA) !=
                (Floppy144RoomId)rect->room ||
            record->n0 != (int32_t)rect->x ||
            record->n1 != (int32_t)rect->y ||
            record->n2 != (int32_t)rect->width ||
            record->n3 != (int32_t)rect->height
        )
        {
            continue;
        }

        return record;
    }

    return NULL;
}

static bool Floppy144Site2DVariantIs(
    const Floppy144DataRecord *placement,
    const char *variant
)
{
    return
        placement != NULL &&
        placement->pszC != NULL &&
        variant != NULL &&
        strcmp(placement->pszC, variant) == 0;
}

static uint32_t Floppy144Site2DClutterHash(
    const Floppy144SiteRect *rect
)
{
    uint32_t value = 2166136261U;

    if(rect == NULL)
    {
        return value;
    }

#define FLOPPY144_SITE_2D_HASH_BYTE(v) \
    do { value ^= (uint32_t)(v); value *= 16777619U; } while(0)

    FLOPPY144_SITE_2D_HASH_BYTE(rect->room);
    FLOPPY144_SITE_2D_HASH_BYTE(rect->type);
    FLOPPY144_SITE_2D_HASH_BYTE(rect->x);
    FLOPPY144_SITE_2D_HASH_BYTE(rect->y);
    FLOPPY144_SITE_2D_HASH_BYTE(rect->width);
    FLOPPY144_SITE_2D_HASH_BYTE(rect->height);

#undef FLOPPY144_SITE_2D_HASH_BYTE

    return value;
}

/*
 * Draw deterministic visual clutter without creating runtime objects.
 *
 * Facilities shelving is intentionally busier than ordinary bookcases and
 * shelving. The authored rectangle identity is the seed, therefore save/load,
 * redraws and projection changes do not reshuffle the scene.
 */
static void Floppy144Site2DDrawShelfClutter(
    Floppy144Surface *surface,
    const Floppy144SiteRect *rect,
    const Floppy144SiteScreenRect *screen_rect,
    bool wall_shelving
)
{
    const uint32_t paper_colour = FLOPPY144_RGB(188, 183, 161);
    const uint32_t box_colour = FLOPPY144_RGB(117, 100, 75);
    uint32_t state;
    int32_t count;
    int32_t index;
    int32_t margin_x;
    int32_t margin_y;
    int32_t usable_w;
    int32_t usable_h;

    if(surface == NULL || rect == NULL || screen_rect == NULL)
    {
        return;
    }

    margin_x = screen_rect->width > 12 ? 4 : 2;
    margin_y = screen_rect->height > 12 ? 4 : 2;
    usable_w = screen_rect->width - margin_x * 2;
    usable_h = screen_rect->height - margin_y * 2;

    if(usable_w < 4 || usable_h < 4)
    {
        return;
    }

    count =
        rect->room == (uint8_t)FLOPPY144_ROOM_FACILITIES
            ? 15
            : (wall_shelving ? 8 : 6);

    state = Floppy144Site2DClutterHash(rect);

    for(index = 0; index < count; ++index)
    {
        int32_t x;
        int32_t y;
        int32_t w;
        int32_t h;

        state = state * 1664525U + 1013904223U;
        x =
            screen_rect->x + margin_x +
            (int32_t)(state % (uint32_t)usable_w);

        state = state * 1664525U + 1013904223U;
        y =
            screen_rect->y + margin_y +
            (int32_t)(state % (uint32_t)usable_h);

        state = state * 1664525U + 1013904223U;
        w = 2 + (int32_t)(state % 4U);

        state = state * 1664525U + 1013904223U;
        h = 2 + (int32_t)(state % 3U);

        if(x + w > screen_rect->x + screen_rect->width - margin_x)
        {
            x = screen_rect->x + screen_rect->width - margin_x - w;
        }

        if(y + h > screen_rect->y + screen_rect->height - margin_y)
        {
            y = screen_rect->y + screen_rect->height - margin_y - h;
        }

        Floppy144Site2DFill(
            surface,
            x,
            y,
            w,
            h,
            (state & 1U) != 0U ? paper_colour : box_colour
        );
    }
}

typedef enum Floppy144Site2DWallAttachment
{
    FLOPPY144_SITE_2D_WALL_NONE = 0,
    FLOPPY144_SITE_2D_WALL_LEFT,
    FLOPPY144_SITE_2D_WALL_RIGHT,
    FLOPPY144_SITE_2D_WALL_TOP,
    FLOPPY144_SITE_2D_WALL_BOTTOM
}
Floppy144Site2DWallAttachment;

static bool Floppy144Site2DRangesOverlap(
    int32_t a0,
    int32_t a1,
    int32_t b0,
    int32_t b1
)
{
    return a0 < b1 && b0 < a1;
}

/*
 * Identify which edge of a wall-mounted fixture actually touches its wall.
 *
 * Partition walls are explicit Site rectangles. Perimeter walls are generated
 * from the inset floor boundary, so a fixture touching a floor edge is treated
 * as mounted to the corresponding perimeter wall. This keeps the authored 1U
 * placement footprint for targeting/collision while letting the visual recipe
 * project only 0.5U into the room.
 */
static Floppy144Site2DWallAttachment
Floppy144Site2DWallFixtureAttachment(
    const Floppy144SiteRect *rect
)
{
    uint32_t index;
    uint32_t rect_count;
    bool vertical;

    if(rect == NULL)
    {
        return FLOPPY144_SITE_2D_WALL_NONE;
    }

    vertical = rect->width <= rect->height;
    rect_count = Floppy144SiteRectCount();

    /* Prefer an explicit partition-wall neighbour. */
    for(index = 0U; index < rect_count; ++index)
    {
        const Floppy144SiteRect *other =
            Floppy144SiteRectAt(index);

        if(
            other == NULL ||
            other == rect ||
            other->room != rect->room ||
            other->type != (uint8_t)FLOPPY144_SITE_PARTITION_WALL
        )
        {
            continue;
        }

        if(
            vertical &&
            Floppy144Site2DRangesOverlap(
                (int32_t)rect->y,
                (int32_t)rect->y + (int32_t)rect->height,
                (int32_t)other->y,
                (int32_t)other->y + (int32_t)other->height
            )
        )
        {
            if(
                (int32_t)other->x + (int32_t)other->width ==
                (int32_t)rect->x
            )
            {
                return FLOPPY144_SITE_2D_WALL_LEFT;
            }

            if(
                (int32_t)rect->x + (int32_t)rect->width ==
                (int32_t)other->x
            )
            {
                return FLOPPY144_SITE_2D_WALL_RIGHT;
            }
        }
        else if(
            !vertical &&
            Floppy144Site2DRangesOverlap(
                (int32_t)rect->x,
                (int32_t)rect->x + (int32_t)rect->width,
                (int32_t)other->x,
                (int32_t)other->x + (int32_t)other->width
            )
        )
        {
            if(
                (int32_t)other->y + (int32_t)other->height ==
                (int32_t)rect->y
            )
            {
                return FLOPPY144_SITE_2D_WALL_TOP;
            }

            if(
                (int32_t)rect->y + (int32_t)rect->height ==
                (int32_t)other->y
            )
            {
                return FLOPPY144_SITE_2D_WALL_BOTTOM;
            }
        }
    }

    /* Fall back to the owning room's inset floor edge. */
    for(index = 0U; index < rect_count; ++index)
    {
        const Floppy144SiteRect *floor =
            Floppy144SiteRectAt(index);

        if(
            floor == NULL ||
            floor->room != rect->room ||
            !Floppy144Site2DIsFloor(
                (Floppy144SiteElement)floor->type
            )
        )
        {
            continue;
        }

        if(
            vertical &&
            Floppy144Site2DRangesOverlap(
                (int32_t)rect->y,
                (int32_t)rect->y + (int32_t)rect->height,
                (int32_t)floor->y,
                (int32_t)floor->y + (int32_t)floor->height
            )
        )
        {
            if(rect->x == floor->x)
            {
                return FLOPPY144_SITE_2D_WALL_LEFT;
            }

            if(
                (int32_t)rect->x + (int32_t)rect->width ==
                (int32_t)floor->x + (int32_t)floor->width
            )
            {
                return FLOPPY144_SITE_2D_WALL_RIGHT;
            }
        }
        else if(
            !vertical &&
            Floppy144Site2DRangesOverlap(
                (int32_t)rect->x,
                (int32_t)rect->x + (int32_t)rect->width,
                (int32_t)floor->x,
                (int32_t)floor->x + (int32_t)floor->width
            )
        )
        {
            if(rect->y == floor->y)
            {
                return FLOPPY144_SITE_2D_WALL_TOP;
            }

            if(
                (int32_t)rect->y + (int32_t)rect->height ==
                (int32_t)floor->y + (int32_t)floor->height
            )
            {
                return FLOPPY144_SITE_2D_WALL_BOTTOM;
            }
        }
    }

    return FLOPPY144_SITE_2D_WALL_NONE;
}

/*
 * Canonical Site coordinates are already player-facing.
 *
 * The old renderer rotated world coordinates 90 degrees clockwise at runtime,
 * which also required LEFT/TOP/etc. wall attachments to be rotated. Stage 3B.3
 * removed that hidden transform: floppy144_site_view.c is now deliberately an
 * identity facade. Wall attachment directions must therefore remain unchanged.
 */
static Floppy144Site2DWallAttachment
Floppy144Site2DWallAttachmentToView(
    Floppy144Site2DWallAttachment attachment
)
{
    return attachment;
}

/*
 * Wall fixtures occupy their authored collision footprint but are visually
 * shallow. Ordinary boards/panels/cabinets project only 0.5 Site units from
 * the wall. The CRT monitor bank remains one unit deep and the IT shelving
 * keeps its authored two-unit depth.
 */
static void Floppy144Site2DWallFixtureVisualRect(
    const Floppy144SiteRect *rect,
    const Floppy144DataRecord *placement,
    const Floppy144SiteScreenRect *screen_rect,
    Floppy144SiteScreenRect *visual_rect
)
{
    const int32_t mount_pixels =
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT / 2;

    int32_t depth_pixels =
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT / 2;

    Floppy144Site2DWallAttachment attachment;

    if(screen_rect == NULL || visual_rect == NULL)
    {
        return;
    }

    *visual_rect = *screen_rect;

    attachment =
        Floppy144Site2DWallAttachmentToView(
            Floppy144Site2DWallFixtureAttachment(rect)
        );

    if(Floppy144Site2DVariantIs(placement, "MONITOR_BANK"))
    {
        depth_pixels = FLOPPY144_SITE_2D_PIXELS_PER_UNIT;
    }
    else if(
        rect != NULL &&
        rect->room == (uint8_t)FLOPPY144_ROOM_IT_SUPPORT &&
        Floppy144Site2DVariantIs(placement, "SHELVING")
    )
    {
        depth_pixels =
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT * 2;
    }

    /*
     * First preserve the authored run along the wall while reducing only the
     * depth projecting into the room. Canonical Site axes already match the
     * screen-facing view, so the same attachment direction applies directly.
     */
    if(visual_rect->width <= visual_rect->height)
    {
        if(depth_pixels < visual_rect->width)
        {
            if(attachment == FLOPPY144_SITE_2D_WALL_RIGHT)
            {
                visual_rect->x +=
                    visual_rect->width - depth_pixels;
            }
            else if(attachment != FLOPPY144_SITE_2D_WALL_LEFT)
            {
                visual_rect->x +=
                    (visual_rect->width - depth_pixels) / 2;
            }

            visual_rect->width = depth_pixels;
        }
    }
    else
    {
        if(depth_pixels < visual_rect->height)
        {
            if(attachment == FLOPPY144_SITE_2D_WALL_BOTTOM)
            {
                visual_rect->y +=
                    visual_rect->height - depth_pixels;
            }
            else if(attachment != FLOPPY144_SITE_2D_WALL_TOP)
            {
                visual_rect->y +=
                    (visual_rect->height - depth_pixels) / 2;
            }

            visual_rect->height = depth_pixels;
        }
    }

    /*
     * Authored fixture coordinates live on the first interior Site cell so
     * targeting remains simple. Visually move a wall-hanging half a Site unit
     * outward onto the room's wall plane. This keeps Site Directories,
     * suppression panels, patch panels and other hangings on the wall without
     * changing collision or interaction geometry.
     */
    switch(attachment)
    {
        case FLOPPY144_SITE_2D_WALL_LEFT:
            visual_rect->x -= mount_pixels;
            break;

        case FLOPPY144_SITE_2D_WALL_RIGHT:
            visual_rect->x += mount_pixels;
            break;

        case FLOPPY144_SITE_2D_WALL_TOP:
            visual_rect->y -= mount_pixels;
            break;

        case FLOPPY144_SITE_2D_WALL_BOTTOM:
            visual_rect->y += mount_pixels;
            break;

        default:
            break;
    }
}

static void Floppy144Site2DDrawWallFixture(
    Floppy144Surface *surface,
    const Floppy144SiteRect *rect,
    const Floppy144DataRecord *placement,
    const Floppy144SiteScreenRect *screen_rect,
    uint32_t edge_colour
)
{
    const uint32_t detail_colour = FLOPPY144_RGB(65, 70, 68);
    const uint32_t light_colour = FLOPPY144_RGB(151, 151, 137);
    const uint32_t paper_colour = FLOPPY144_RGB(188, 183, 161);
    const uint32_t screen_colour = FLOPPY144_RGB(58, 112, 117);
    const uint32_t amber_colour = FLOPPY144_RGB(202, 155, 69);
    Floppy144SiteScreenRect visual;
    int32_t index;

    if(surface == NULL || rect == NULL || screen_rect == NULL)
    {
        return;
    }

    Floppy144Site2DWallFixtureVisualRect(
        rect,
        placement,
        screen_rect,
        &visual
    );

    /*
     * Camera clipping is a separate Z-level from world geometry. Keep the
     * complete projected fixture rectangle intact and use primitive-level
     * clipping instead. A partially visible wall hanging must therefore retain
     * exactly the same width/height as it scrolls through the camera edge.
     */
    {
        Floppy144SiteScreenRect visible_probe = visual;

        if(
            !Floppy144Site2DClipRect(
                surface,
                &visible_probe.x,
                &visible_probe.y,
                &visible_probe.width,
                &visible_probe.height
            )
        )
        {
            return;
        }
    }

    /*
     * Prefer the authored drawing definition. Task 12 may replace the generic
     * wall-item definition with a variant recipe without requiring renderer
     * changes. The compact overlays below also keep the older Stage 3B data
     * visually distinct.
     */
    if(
        placement == NULL ||
        placement->pszE == NULL ||
        !Floppy144DrawingRuntimeDrawClipped(
            surface,
            placement->pszE,
            visual.x,
            visual.y,
            visual.width,
            visual.height,
            FLOPPY144_SITE_2D_VIEWPORT_X,
            FLOPPY144_SITE_2D_VIEWPORT_Y,
            FLOPPY144_SITE_2D_VIEWPORT_WIDTH,
            FLOPPY144_SITE_2D_VIEWPORT_HEIGHT
        )
    )
    {
        Floppy144Site2DFill(
            surface,
            visual.x,
            visual.y,
            visual.width,
            visual.height,
            light_colour
        );

        Floppy144Site2DOutline(
            surface,
            visual.x,
            visual.y,
            visual.width,
            visual.height,
            edge_colour
        );
    }

    if(Floppy144Site2DVariantIs(placement, "SITE_DIRECTORY"))
    {
        for(index = 1; index < 4; ++index)
        {
            Floppy144Site2DFill(
                surface,
                visual.x + visual.width / 6,
                visual.y + visual.height * index / 5,
                visual.width * 2 / 3,
                2,
                detail_colour
            );
        }
    }
    else if(Floppy144Site2DVariantIs(placement, "SUPPRESSION_PANEL"))
    {
        Floppy144Site2DFill(
            surface,
            visual.x + visual.width / 5,
            visual.y + visual.height / 5,
            visual.width * 3 / 5,
            visual.height * 3 / 5,
            detail_colour
        );

        Floppy144Site2DFill(
            surface,
            visual.x + visual.width / 2 - 2,
            visual.y + visual.height / 2 - 2,
            4,
            4,
            amber_colour
        );
    }
    else if(Floppy144Site2DVariantIs(placement, "PATCH_PANEL"))
    {
        int32_t ports =
            visual.width >= visual.height ? 8 : 5;

        for(index = 0; index < ports; ++index)
        {
            int32_t x =
                visual.x +
                visual.width * (index + 1) / (ports + 1);

            Floppy144Site2DFill(
                surface,
                x - 1,
                visual.y + visual.height / 2 - 1,
                3,
                3,
                screen_colour
            );
        }
    }
    else if(Floppy144Site2DVariantIs(placement, "NOTICEBOARD"))
    {
        Floppy144Site2DFill(
            surface,
            visual.x + 3,
            visual.y + 3,
            visual.width - 6,
            visual.height - 6,
            paper_colour
        );

        Floppy144Site2DFill(
            surface,
            visual.x + visual.width / 3,
            visual.y + visual.height / 4,
            3,
            3,
            amber_colour
        );
    }
    else if(Floppy144Site2DVariantIs(placement, "KEY_CABINET"))
    {
        Floppy144Site2DOutline(
            surface,
            visual.x + 3,
            visual.y + 3,
            visual.width - 6,
            visual.height - 6,
            edge_colour
        );

        for(index = 1; index < 4; ++index)
        {
            Floppy144Site2DFill(
                surface,
                visual.x + visual.width * index / 4,
                visual.y + visual.height / 3,
                2,
                visual.height / 3,
                amber_colour
            );
        }
    }
    else if(Floppy144Site2DVariantIs(placement, "FIRST_AID_KIT"))
    {
        int32_t centre_x = visual.x + visual.width / 2;
        int32_t centre_y = visual.y + visual.height / 2;

        Floppy144Site2DFill(
            surface,
            centre_x - 2,
            visual.y + visual.height / 5,
            4,
            visual.height * 3 / 5,
            light_colour
        );

        Floppy144Site2DFill(
            surface,
            visual.x + visual.width / 5,
            centre_y - 2,
            visual.width * 3 / 5,
            4,
            light_colour
        );
    }
    else if(Floppy144Site2DVariantIs(placement, "MONITOR_BANK"))
    {
        int32_t screens =
            visual.width >= visual.height ? 4 : 6;

        for(index = 0; index < screens; ++index)
        {
            int32_t x;
            int32_t y;
            int32_t w;
            int32_t h;

            if(visual.width >= visual.height)
            {
                w = visual.width / screens - 3;
                h = visual.height - 6;
                x = visual.x + 2 + index * visual.width / screens;
                y = visual.y + 3;
            }
            else
            {
                w = visual.width - 6;
                h = visual.height / screens - 3;
                x = visual.x + 3;
                y = visual.y + 2 + index * visual.height / screens;
            }

            if(w > 2 && h > 2)
            {
                Floppy144Site2DFill(
                    surface,
                    x,
                    y,
                    w,
                    h,
                    screen_colour
                );

                Floppy144Site2DOutline(
                    surface,
                    x,
                    y,
                    w,
                    h,
                    edge_colour
                );
            }
        }
    }
    else if(Floppy144Site2DVariantIs(placement, "CABLE_RISER"))
    {
        /*
         * The riser is a tall, shallow service route mounted beside the
         * Server Room partition wall. Give it a dedicated conduit pattern so
         * it remains visibly distinct from both the wall and generic panels.
         */
        if(visual.height >= visual.width)
        {
            for(index = 1; index <= 2; ++index)
            {
                Floppy144Site2DFill(
                    surface,
                    visual.x + visual.width * index / 3,
                    visual.y + 3,
                    1,
                    visual.height - 6,
                    detail_colour
                );
            }

            for(index = 1; index < 5; ++index)
            {
                Floppy144Site2DFill(
                    surface,
                    visual.x + 2,
                    visual.y + visual.height * index / 5,
                    visual.width - 4,
                    2,
                    index == 2 ? amber_colour : edge_colour
                );
            }
        }
        else
        {
            for(index = 1; index <= 2; ++index)
            {
                Floppy144Site2DFill(
                    surface,
                    visual.x + 3,
                    visual.y + visual.height * index / 3,
                    visual.width - 6,
                    1,
                    detail_colour
                );
            }

            for(index = 1; index < 5; ++index)
            {
                Floppy144Site2DFill(
                    surface,
                    visual.x + visual.width * index / 5,
                    visual.y + 2,
                    2,
                    visual.height - 4,
                    index == 2 ? amber_colour : edge_colour
                );
            }
        }
    }
    else if(Floppy144Site2DVariantIs(placement, "SHELVING"))
    {
        for(index = 1; index < 4; ++index)
        {
            if(visual.width >= visual.height)
            {
                Floppy144Site2DFill(
                    surface,
                    visual.x + visual.width * index / 4,
                    visual.y + 2,
                    2,
                    visual.height - 4,
                    edge_colour
                );
            }
            else
            {
                Floppy144Site2DFill(
                    surface,
                    visual.x + 2,
                    visual.y + visual.height * index / 4,
                    visual.width - 4,
                    2,
                    edge_colour
                );
            }
        }

        Floppy144Site2DDrawShelfClutter(
            surface,
            rect,
            &visual,
            true
        );
    }
}

static void Floppy144Site2DDrawRecessedSink(
    Floppy144Surface *surface,
    const Floppy144SiteScreenRect *screen_rect,
    uint32_t edge_colour
)
{
    const uint32_t bowl_colour = FLOPPY144_RGB(54, 68, 70);
    const uint32_t rim_colour = FLOPPY144_RGB(151, 151, 137);
    int32_t inset_x;
    int32_t inset_y;
    int32_t inset_w;
    int32_t inset_h;
    int32_t tap_x;
    int32_t tap_y;

    if(surface == NULL || screen_rect == NULL)
    {
        return;
    }

    inset_x = screen_rect->x + screen_rect->width / 6;
    inset_y = screen_rect->y + screen_rect->height / 6;
    inset_w = screen_rect->width * 2 / 3;
    inset_h = screen_rect->height * 2 / 3;

    Floppy144Site2DFill(
        surface,
        inset_x,
        inset_y,
        inset_w,
        inset_h,
        bowl_colour
    );

    Floppy144Site2DOutline(
        surface,
        inset_x,
        inset_y,
        inset_w,
        inset_h,
        rim_colour
    );

    /*
     * Mixer tap: stem plus short spout. It is intentionally drawn from the
     * rear edge of the recess instead of treating the sink as a freestanding
     * four-unit block.
     */
    tap_x = screen_rect->x + screen_rect->width / 2;
    tap_y = screen_rect->y + 2;

    Floppy144Site2DFill(
        surface,
        tap_x - 1,
        tap_y,
        3,
        screen_rect->height / 4,
        rim_colour
    );

    Floppy144Site2DFill(
        surface,
        tap_x,
        tap_y,
        screen_rect->width / 5,
        2,
        rim_colour
    );

    Floppy144Site2DFill(
        surface,
        inset_x + inset_w / 2 - 1,
        inset_y + inset_h / 2 - 1,
        3,
        3,
        edge_colour
    );
}

static void Floppy144Site2DDrawFurnitureDetails(
    Floppy144Surface *surface,
    Floppy144SiteElement element,
    const Floppy144SiteRect *rect,
    const Floppy144SiteScreenRect *screen_rect,
    int16_t rotation,
    uint32_t edge_colour
)
{
    const uint32_t detail_colour =
        FLOPPY144_RGB(65, 70, 68);

    const uint32_t light_colour =
        FLOPPY144_RGB(151, 151, 137);

    const uint32_t paper_colour =
        FLOPPY144_RGB(188, 183, 161);

    const uint32_t screen_colour =
        FLOPPY144_RGB(58, 112, 117);

    const uint32_t amber_colour =
        FLOPPY144_RGB(202, 155, 69);

    int32_t inset_x;
    int32_t inset_y;
    int32_t inset_w;
    int32_t inset_h;
    int32_t index;
    uint16_t facing;

    if(screen_rect == NULL)
    {
        return;
    }

    inset_x = screen_rect->x + screen_rect->width / 8;
    inset_y = screen_rect->y + screen_rect->height / 8;
    inset_w = screen_rect->width * 3 / 4;
    inset_h = screen_rect->height * 3 / 4;

    /*
     * Cardinal facing comes directly from JSON. Diagonal furniture keeps its
     * exact authored angle in data; this compact renderer approximates its
     * directional detail using the nearest cardinal edge.
     */
    facing =
        (uint16_t)(
            (
                (
                    Floppy144Site2DNormalisedRotation(
                        rotation
                    ) + 45
                ) / 90
            ) * 90
        ) % 360;

    switch(element)
    {
        case FLOPPY144_SITE_STANDARD_DESK:
        {
            int32_t equipment_w =
                screen_rect->width / 4;
            int32_t equipment_h =
                screen_rect->height / 3;
            int32_t equipment_x;
            int32_t equipment_y;
            int32_t paper_w =
                screen_rect->width / 5;
            int32_t paper_x;
            int32_t paper_y;

            /*
             * Diagonal desks need their desktop objects rotated with the desk,
             * rather than snapped to the nearest cardinal orientation.  The
             * desk body already renders as a diamond, so use the same geometry
             * for the equipment and a diagonal stroke for the loose paper.
             */
            if(Floppy144Site2DRotationIsDiagonal(rotation))
            {
                Floppy144SitePoint2D equipment_points[4];
                int32_t centre_x;
                int32_t centre_y;
                int32_t u_x;
                int32_t u_y;
                int32_t v_x;
                int32_t v_y;
                int32_t detail_centre_x;
                int32_t detail_centre_y;

                if(
                    rect != NULL &&
                    Floppy144Site2DDiagonalAxes(
                        rect,
                        screen_rect,
                        &centre_x,
                        &centre_y,
                        &u_x,
                        &u_y,
                        &v_x,
                        &v_y
                    )
                )
                {
                    /*
                     * Desktop equipment belongs to the desk's own local axes.
                     * Position the dark blotter/tray toward the authored back
                     * edge and keep the loose paper/pen stroke parallel to the
                     * long edge of the desktop.
                     */
                    detail_centre_x =
                        centre_x -
                        v_x / 4;

                    detail_centre_y =
                        centre_y -
                        v_y / 4;

                    Floppy144Site2DBuildQuad(
                        equipment_points,
                        detail_centre_x,
                        detail_centre_y,
                        u_x / 3,
                        u_y / 3,
                        v_x / 3,
                        v_y / 3
                    );

                    Floppy144Site2DFillQuad(
                        surface,
                        equipment_points,
                        detail_colour
                    );

                    Floppy144Site2DOutlineQuad(
                        surface,
                        equipment_points,
                        edge_colour
                    );

                    Floppy144Site2DLine(
                        surface,
                        centre_x - u_x / 3 + v_x / 3,
                        centre_y - u_y / 3 + v_y / 3,
                        centre_x + u_x / 3 + v_x / 3,
                        centre_y + u_y / 3 + v_y / 3,
                        paper_colour
                    );
                }

                break;
            }

            /*
             * Cardinal detail follows the authored back-edge rotation.
             */
            switch(facing)
            {
                case 90U:
                    equipment_x =
                        screen_rect->x + screen_rect->width -
                        screen_rect->width / 8 - equipment_w;
                    equipment_y = inset_y;
                    paper_x = screen_rect->x + screen_rect->width / 2;
                    paper_y = screen_rect->y + screen_rect->height * 3 / 5;
                    break;

                case 180U:
                    equipment_x =
                        screen_rect->x + screen_rect->width -
                        screen_rect->width / 8 - equipment_w;
                    equipment_y =
                        screen_rect->y + screen_rect->height -
                        screen_rect->height / 8 - equipment_h;
                    paper_x = screen_rect->x + screen_rect->width / 8;
                    paper_y =
                        screen_rect->y + screen_rect->height -
                        screen_rect->height / 5 - 3;
                    break;

                case 270U:
                    equipment_x = inset_x;
                    equipment_y =
                        screen_rect->y + screen_rect->height -
                        screen_rect->height / 8 - equipment_h;
                    paper_x = screen_rect->x + screen_rect->width / 3;
                    paper_y = screen_rect->y + screen_rect->height / 5;
                    break;

                case 0U:
                default:
                    equipment_x = inset_x;
                    equipment_y = inset_y;
                    paper_x = screen_rect->x + screen_rect->width * 5 / 8;
                    paper_y = screen_rect->y + screen_rect->height / 5;
                    break;
            }

            Floppy144Site2DFill(
                surface,
                equipment_x,
                equipment_y,
                equipment_w,
                equipment_h,
                detail_colour
            );

            Floppy144Site2DOutline(
                surface,
                equipment_x,
                equipment_y,
                equipment_w,
                equipment_h,
                edge_colour
            );

            Floppy144Site2DFill(
                surface,
                paper_x,
                paper_y,
                paper_w,
                3,
                paper_colour
            );

            break;
        }

        case FLOPPY144_SITE_TERMINAL_DESK:
        {
            int32_t monitor_w = screen_rect->width / 2;
            int32_t monitor_h = screen_rect->height / 2;
            int32_t monitor_x;
            int32_t monitor_y;
            int32_t keyboard_y;

            /*
             * Monitor and keyboard placement follows the authored back edge.
             */
            switch(facing)
            {
                case 90U:
                    monitor_x =
                        screen_rect->x + screen_rect->width -
                        screen_rect->width / 8 - monitor_w;
                    monitor_y = screen_rect->y + screen_rect->height / 8;
                    keyboard_y = -1;
                    break;

                case 180U:
                    monitor_x =
                        screen_rect->x + screen_rect->width -
                        screen_rect->width / 8 - monitor_w;
                    monitor_y =
                        screen_rect->y + screen_rect->height -
                        screen_rect->height / 8 - monitor_h;
                    keyboard_y = screen_rect->y + screen_rect->height / 4;
                    break;

                case 270U:
                    monitor_x = screen_rect->x + screen_rect->width / 8;
                    monitor_y =
                        screen_rect->y + screen_rect->height -
                        screen_rect->height / 8 - monitor_h;
                    keyboard_y = -2;
                    break;

                case 0U:
                default:
                    monitor_x = screen_rect->x + screen_rect->width / 8;
                    monitor_y = screen_rect->y + screen_rect->height / 8;
                    keyboard_y = screen_rect->y + screen_rect->height * 3 / 4;
                    break;
            }

            Floppy144Site2DFill(
                surface,
                monitor_x,
                monitor_y,
                monitor_w,
                monitor_h,
                detail_colour
            );

            Floppy144Site2DFill(
                surface,
                monitor_x + 4,
                monitor_y + 4,
                monitor_w - 8,
                monitor_h - 8,
                screen_colour
            );

            if(keyboard_y >= 0)
            {
                Floppy144Site2DFill(
                    surface,
                    screen_rect->x + screen_rect->width / 4,
                    keyboard_y,
                    screen_rect->width / 2,
                    3,
                    edge_colour
                );
            }
            else
            {
                int32_t keyboard_x =
                    keyboard_y == -1
                        ? screen_rect->x + screen_rect->width / 4
                        : screen_rect->x + screen_rect->width * 3 / 4;

                Floppy144Site2DFill(
                    surface,
                    keyboard_x,
                    screen_rect->y + screen_rect->height / 4,
                    3,
                    screen_rect->height / 2,
                    edge_colour
                );
            }

            break;
        }

        case FLOPPY144_SITE_CHAIR:
        {
            if(
                Floppy144Site2DRotationIsDiagonal(
                    rotation
                )
            )
            {
                Floppy144SiteScreenRect detail_rect;
                int32_t left;
                int32_t right;
                int32_t top;
                int32_t bottom;
                int32_t centre_x;
                int32_t centre_y;
                uint16_t diagonal_facing =
                    (uint16_t)(
                        (
                            (
                                Floppy144Site2DNormalisedRotation(
                                    rotation
                                ) + 22
                            ) / 45
                        ) * 45
                    ) % 360;

                detail_rect.x =
                    screen_rect->x + 5;
                detail_rect.y =
                    screen_rect->y + 5;
                detail_rect.width =
                    screen_rect->width - 10;
                detail_rect.height =
                    screen_rect->height - 10;

                Floppy144Site2DOutlineDiamond(
                    surface,
                    &detail_rect,
                    detail_colour
                );

                left = detail_rect.x;
                right =
                    detail_rect.x +
                    detail_rect.width - 1;
                top = detail_rect.y;
                bottom =
                    detail_rect.y +
                    detail_rect.height - 1;
                centre_x = (left + right) / 2;
                centre_y = (top + bottom) / 2;

                /*
                 * Rotation semantics are the same as cardinal chairs:
                 * 0 = top back edge, 90 = right, 180 = bottom, 270 = left.
                 * The four diagonal values therefore select the four sloping
                 * edges of the diamond.
                 */
                switch(diagonal_facing)
                {
                    case 45U:
                        Floppy144Site2DLine(
                            surface,
                            centre_x,
                            top,
                            right,
                            centre_y,
                            light_colour
                        );
                        break;

                    case 135U:
                        Floppy144Site2DLine(
                            surface,
                            right,
                            centre_y,
                            centre_x,
                            bottom,
                            light_colour
                        );
                        break;

                    case 225U:
                        Floppy144Site2DLine(
                            surface,
                            centre_x,
                            bottom,
                            left,
                            centre_y,
                            light_colour
                        );
                        break;

                    case 315U:
                    default:
                        Floppy144Site2DLine(
                            surface,
                            left,
                            centre_y,
                            centre_x,
                            top,
                            light_colour
                        );
                        break;
                }
            }
            else
            {
                Floppy144Site2DOutline(
                    surface,
                    inset_x,
                    inset_y,
                    inset_w,
                    inset_h,
                    detail_colour
                );

                /* The light strip is the authored back edge of the chair. */
                if(facing == 90U || facing == 270U)
                {
                    int32_t back_x =
                        facing == 90U
                            ? screen_rect->x + screen_rect->width - 7
                            : screen_rect->x + 4;

                    Floppy144Site2DFill(
                        surface,
                        back_x,
                        screen_rect->y + 4,
                        3,
                        screen_rect->height - 8,
                        light_colour
                    );
                }
                else
                {
                    int32_t back_y =
                        facing == 180U
                            ? screen_rect->y + screen_rect->height - 7
                            : screen_rect->y + 4;

                    Floppy144Site2DFill(
                        surface,
                        screen_rect->x + 4,
                        back_y,
                        screen_rect->width - 8,
                        3,
                        light_colour
                    );
                }
            }

            break;
        }

        case FLOPPY144_SITE_NONSECURE_CABINET:
        case FLOPPY144_SITE_SECURE_CABINET_HALF:
        case FLOPPY144_SITE_SECURE_CABINET_FULL:
        {
            Floppy144Site2DDrawCabinetDetails(
                surface,
                screen_rect,
                edge_colour,
                element == FLOPPY144_SITE_NONSECURE_CABINET
                    ? detail_colour
                    : amber_colour
            );

            break;
        }

        case FLOPPY144_SITE_BOOKCASE:
        case FLOPPY144_SITE_SHELVING_FULL:
        {
            for(index = 1; index < 4; ++index)
            {
                if(screen_rect->width >= screen_rect->height)
                {
                    Floppy144Site2DFill(
                        surface,
                        screen_rect->x + screen_rect->width * index / 4,
                        screen_rect->y + 3,
                        2,
                        screen_rect->height - 6,
                        edge_colour
                    );
                }
                else
                {
                    Floppy144Site2DFill(
                        surface,
                        screen_rect->x + 3,
                        screen_rect->y + screen_rect->height * index / 4,
                        screen_rect->width - 6,
                        2,
                        edge_colour
                    );
                }
            }

            break;
        }

        case FLOPPY144_SITE_WALL_MOUNTED_ITEM:
        {
            Floppy144Site2DFill(
                surface,
                inset_x,
                inset_y,
                inset_w,
                inset_h,
                screen_colour
            );

            Floppy144Site2DOutline(
                surface,
                inset_x,
                inset_y,
                inset_w,
                inset_h,
                edge_colour
            );

            Floppy144Site2DFill(
                surface,
                inset_x + 3,
                inset_y + inset_h - 5,
                4,
                3,
                amber_colour
            );

            break;
        }

        case FLOPPY144_SITE_FRIDGE:
        {
            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width / 2,
                screen_rect->y + 3,
                1,
                screen_rect->height - 6,
                edge_colour
            );

            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width / 2 + 4,
                screen_rect->y + screen_rect->height / 3,
                2,
                screen_rect->height / 3,
                detail_colour
            );

            break;
        }

        case FLOPPY144_SITE_WORKTOP:
        case FLOPPY144_SITE_TABLE:
        {
            if(
                element == FLOPPY144_SITE_TABLE &&
                Floppy144Site2DRotationIsDiagonal(
                    rotation
                )
            )
            {
                Floppy144SiteScreenRect detail_rect;

                detail_rect.x =
                    screen_rect->x +
                    screen_rect->width / 5;

                detail_rect.y =
                    screen_rect->y +
                    screen_rect->height / 5;

                detail_rect.width =
                    screen_rect->width * 3 / 5;

                detail_rect.height =
                    screen_rect->height * 3 / 5;

                Floppy144Site2DOutlineDiamond(
                    surface,
                    &detail_rect,
                    detail_colour
                );
            }
            else
            {
                Floppy144Site2DFill(
                    surface,
                    inset_x,
                    screen_rect->y + screen_rect->height / 2,
                    inset_w,
                    2,
                    detail_colour
                );
            }

            break;
        }

        case FLOPPY144_SITE_SINK:
        {
            Floppy144Site2DFill(
                surface,
                inset_x,
                inset_y,
                inset_w,
                inset_h,
                screen_colour
            );

            Floppy144Site2DOutline(
                surface,
                inset_x,
                inset_y,
                inset_w,
                inset_h,
                edge_colour
            );

            break;
        }

        case FLOPPY144_SITE_COFFEE_MAKER:
        {
            Floppy144Site2DFill(
                surface,
                inset_x,
                inset_y,
                inset_w,
                inset_h,
                detail_colour
            );

            Floppy144Site2DFill(
                surface,
                inset_x + 3,
                inset_y + 3,
                4,
                4,
                amber_colour
            );

            break;
        }

        case FLOPPY144_SITE_SOFA:
        {
            Floppy144Site2DOutline(
                surface,
                inset_x,
                inset_y,
                inset_w,
                inset_h,
                detail_colour
            );

            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width / 2,
                inset_y,
                2,
                inset_h,
                detail_colour
            );

            break;
        }

        case FLOPPY144_SITE_SERVER:
        {
            for(index = 1; index < 5; ++index)
            {
                int32_t y =
                    screen_rect->y +
                    screen_rect->height * index / 6;

                Floppy144Site2DFill(
                    surface,
                    screen_rect->x + 4,
                    y,
                    screen_rect->width - 8,
                    2,
                    detail_colour
                );
            }

            Floppy144Site2DFill(
                surface,
                screen_rect->x + 5,
                screen_rect->y + 5,
                3,
                3,
                amber_colour
            );

            break;
        }

        case FLOPPY144_SITE_TROLLEY:
        {
            Floppy144Site2DOutline(
                surface,
                inset_x,
                inset_y,
                inset_w,
                inset_h,
                detail_colour
            );

            Floppy144Site2DFill(
                surface,
                screen_rect->x + 2,
                screen_rect->y + screen_rect->height - 5,
                4,
                4,
                edge_colour
            );

            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width - 6,
                screen_rect->y + screen_rect->height - 5,
                4,
                4,
                edge_colour
            );

            break;
        }

        default:
        {
            break;
        }
    }
}

static void Floppy144Site2DDrawSiteRect(
    Floppy144Surface *surface,
    const Floppy144SiteCamera2D *camera,
    const Floppy144SiteRect *rect
)
{
    const uint32_t edge_colour =
        FLOPPY144_RGB(27, 32, 32);

    const uint32_t highlight_colour =
        FLOPPY144_RGB(145, 145, 132);

    const uint32_t shadow_colour =
        FLOPPY144_RGB(24, 28, 28);

    const uint32_t glass_colour =
        FLOPPY144_RGB(135, 184, 194);

    Floppy144SiteElement element;
    const Floppy144SiteStyle2D *style;
    const Floppy144DataRecord *placement;
    Floppy144SiteScreenRect screen_rect;

    if(
        surface == NULL ||
        camera == NULL ||
        rect == NULL ||
        rect->type >= (uint8_t)FLOPPY144_SITE_ELEMENT_COUNT ||
        !Floppy144Site2DProjectRect(camera, rect, &screen_rect)
    )
    {
        return;
    }

    if(
        screen_rect.x >= FLOPPY144_SITE_2D_VIEWPORT_X + FLOPPY144_SITE_2D_VIEWPORT_WIDTH ||
        screen_rect.y >= FLOPPY144_SITE_2D_VIEWPORT_Y + FLOPPY144_SITE_2D_VIEWPORT_HEIGHT ||
        screen_rect.x + screen_rect.width <= FLOPPY144_SITE_2D_VIEWPORT_X ||
        screen_rect.y + screen_rect.height <= FLOPPY144_SITE_2D_VIEWPORT_Y
    )
    {
        return;
    }

    element =
        (Floppy144SiteElement)rect->type;

    style =
        &floppy144_site_styles[element];

    placement =
        Floppy144Site2DPlacementForRect(rect);

    if(Floppy144Site2DIsFloor(element))
    {
        Floppy144Site2DFill(
            surface,
            screen_rect.x,
            screen_rect.y,
            screen_rect.width,
            screen_rect.height,
            style->colour
        );

        return;
    }

    if(element == FLOPPY144_SITE_DOOR)
    {
        Floppy144Site2DFill(
            surface,
            screen_rect.x,
            screen_rect.y,
            screen_rect.width,
            screen_rect.height,
            style->colour
        );

        Floppy144Site2DOutline(
            surface,
            screen_rect.x,
            screen_rect.y,
            screen_rect.width,
            screen_rect.height,
            edge_colour
        );

        if(screen_rect.width >= screen_rect.height)
        {
            Floppy144Site2DFill(
                surface,
                screen_rect.x + 4,
                screen_rect.y + screen_rect.height / 2,
                screen_rect.width - 8,
                2,
                highlight_colour
            );
        }
        else
        {
            Floppy144Site2DFill(
                surface,
                screen_rect.x + screen_rect.width / 2,
                screen_rect.y + 4,
                2,
                screen_rect.height - 8,
                highlight_colour
            );
        }

        return;
    }

    if(element == FLOPPY144_SITE_WINDOW)
    {
        Floppy144Site2DFill(
            surface,
            screen_rect.x,
            screen_rect.y,
            screen_rect.width,
            screen_rect.height,
            style->colour
        );

        Floppy144Site2DOutline(
            surface,
            screen_rect.x,
            screen_rect.y,
            screen_rect.width,
            screen_rect.height,
            edge_colour
        );

        if(screen_rect.width >= screen_rect.height)
        {
            Floppy144Site2DFill(
                surface,
                screen_rect.x + 3,
                screen_rect.y + screen_rect.height / 2,
                screen_rect.width - 6,
                2,
                glass_colour
            );
        }
        else
        {
            Floppy144Site2DFill(
                surface,
                screen_rect.x + screen_rect.width / 2,
                screen_rect.y + 3,
                2,
                screen_rect.height - 6,
                glass_colour
            );
        }

        return;
    }

    if(element == FLOPPY144_SITE_PARTITION_WALL)
    {
        Floppy144Site2DFill(
            surface,
            screen_rect.x,
            screen_rect.y,
            screen_rect.width,
            screen_rect.height,
            style->colour
        );

        Floppy144Site2DOutline(
            surface,
            screen_rect.x,
            screen_rect.y,
            screen_rect.width,
            screen_rect.height,
            edge_colour
        );

        return;
    }

    if(element == FLOPPY144_SITE_WALL_MOUNTED_ITEM)
    {
        Floppy144Site2DDrawWallFixture(
            surface,
            rect,
            placement,
            &screen_rect,
            edge_colour
        );

        return;
    }

    if(
        element == FLOPPY144_SITE_SINK &&
        rect->room == (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
    )
    {
        Floppy144Site2DDrawRecessedSink(
            surface,
            &screen_rect,
            edge_colour
        );

        return;
    }

    if(
        Floppy144Site2DRotationIsDiagonal(
            rect->rotation
        )
    )
    {
        Floppy144Site2DDrawDiagonalFurnitureBase(
            surface,
            rect,
            &screen_rect,
            style->colour,
            edge_colour,
            highlight_colour,
            shadow_colour
        );
    }
    else
    {
        Floppy144Site2DDrawFurnitureBase(
            surface,
            &screen_rect,
            style->colour,
            edge_colour,
            highlight_colour,
            shadow_colour
        );
    }

    Floppy144Site2DDrawFurnitureDetails(
        surface,
        element,
        rect,
        &screen_rect,
        rect->rotation,
        edge_colour
    );

    if(
        element == FLOPPY144_SITE_BOOKCASE ||
        element == FLOPPY144_SITE_SHELVING_FULL
    )
    {
        Floppy144Site2DDrawShelfClutter(
            surface,
            rect,
            &screen_rect,
            false
        );
    }
}

/*
 * The impossible door is a non-colliding drawing overlay on an existing solid
 * wall cell. It is never added to the generated Site list or Room topology.
 */
static void Floppy144Site2DDrawGreyDoor(
    Floppy144Surface *surface,
    const Floppy144SiteCamera2D *camera,
    const Floppy144RunState *state
)
{
    Floppy144GreyDoorCandidate door;
    Floppy144SiteScreenRect projected;
    const uint32_t frame=FLOPPY144_RGB(35,41,43);
    const uint32_t grey=FLOPPY144_RGB(139,143,147);
    const uint32_t sheen=FLOPPY144_RGB(174,177,181);
    if(!Floppy144GreyDoorForRun(state,&door) ||
       !Floppy144Site2DProjectRect(camera,&door.rect,&projected))
        return;

    Floppy144Site2DFill(surface,projected.x,projected.y,
        projected.width,projected.height,frame);
    Floppy144Site2DFill(surface,projected.x+2,projected.y+2,
        projected.width-4,projected.height-4,grey);
    Floppy144Site2DOutline(surface,projected.x,projected.y,
        projected.width,projected.height,frame);
    if(projected.width>projected.height)
    {
        Floppy144Site2DFill(surface,projected.x+5,
            projected.y+projected.height/2,
            projected.width-10,1,sheen);
        Floppy144Site2DFill(surface,
            projected.x+projected.width-8,
            projected.y+projected.height/2+2,2,2,frame);
    }
    else
    {
        Floppy144Site2DFill(surface,
            projected.x+projected.width/2,
            projected.y+5,1,projected.height-10,sheen);
        Floppy144Site2DFill(surface,
            projected.x+projected.width/2+2,
            projected.y+projected.height-8,2,2,frame);
    }
}

static void Floppy144Site2DDrawPlayer(
    Floppy144Surface *surface,
    const Floppy144SiteCamera2D *camera,
    const Floppy144RunState *run_state,
    Floppy144OperatorBodyStyle body_style,
    const Floppy144PlayerVisualState *player_visual
)
{
    int32_t foot_x;
    int32_t foot_y;
    int32_t sprite_width;
    int32_t sprite_height;
    int32_t collision_shadow_width;

    if(
        surface == NULL ||
        camera == NULL ||
        run_state == NULL
    )
    {
        return;
    }

    Floppy144Site2DProjectPoint(
        camera,
        run_state->player_site_x,
        run_state->player_site_y,
        &foot_x,
        &foot_y
    );

    sprite_width =
        (
            FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16 *
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT
        ) /
        FLOPPY144_SITE_FIXED_ONE;

    sprite_height =
        (
            FLOPPY144_SITE_2D_PLAYER_VISUAL_HEIGHT_X16 *
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT
        ) /
        FLOPPY144_SITE_FIXED_ONE;

    collision_shadow_width =
        (
            FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 *
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT
        ) /
        FLOPPY144_SITE_FIXED_ONE;

    Floppy144PlayerVisualDraw(
        surface,
        foot_x,
        foot_y,
        sprite_width,
        sprite_height,
        collision_shadow_width,
        body_style,
        player_visual,
        FLOPPY144_SITE_2D_VIEWPORT_X,
        FLOPPY144_SITE_2D_VIEWPORT_Y,
        FLOPPY144_SITE_2D_VIEWPORT_WIDTH,
        FLOPPY144_SITE_2D_VIEWPORT_HEIGHT
    );
}

static const char *Floppy144Site2DInteractionPrompt(
    const Floppy144RunState *run_state
)
{
    uint32_t uActions;
    Floppy144CabinetState sCabinetProbe;

    if(run_state == NULL)
    {
        return "ARROWS TO MOVE   N NOTEBOOK";
    }

    uActions =
        Floppy144SiteAvailableActions(
            run_state
        );
    if(Floppy144GreyDoorNearby(run_state))
        uActions |= FLOPPY144_SITE_ACTION_ACCESS |
                    FLOPPY144_SITE_ACTION_INSPECT;

    /* STAGE 3B.5 SECURE CABINET ACCESS PROMPT */
    Floppy144CabinetReset(&sCabinetProbe);
    if(Floppy144CabinetOpenNearby(&sCabinetProbe, run_state))
    {
        uActions |= FLOPPY144_SITE_ACTION_ACCESS;
    }

    if(
        (uActions & FLOPPY144_SITE_ACTION_ACCESS) != 0U &&
        (uActions & FLOPPY144_SITE_ACTION_INSPECT) != 0U
    )
    {
        return "A ACCESS   I INSPECT   N NOTEBOOK";
    }

    if((uActions & FLOPPY144_SITE_ACTION_ACCESS) != 0U)
    {
        return "A ACCESS   N NOTEBOOK";
    }

    if((uActions & FLOPPY144_SITE_ACTION_INSPECT) != 0U)
    {
        return "I INSPECT   N NOTEBOOK";
    }

    return "ARROWS TO MOVE   N NOTEBOOK";
}

static const char *Floppy144Site2DRoomLabel(
    Floppy144RoomId room
)
{
    switch(room)
    {
        case FLOPPY144_ROOM_RECEPTION:        return "RECEPTION";
        case FLOPPY144_ROOM_CORRIDOR:         return "CORRIDOR";
        case FLOPPY144_ROOM_MAIN_OFFICE:      return "MAIN OFFICE";
        case FLOPPY144_ROOM_FACILITIES:       return "FACILITIES";
        case FLOPPY144_ROOM_RECORDS_OFFICE:   return "RECORDS OFFICE";
        case FLOPPY144_ROOM_IT_SUPPORT:       return "IT SUPPORT";
        case FLOPPY144_ROOM_STAFF_ROOM:       return "STAFF ROOM";
        case FLOPPY144_ROOM_SECRETARY_OFFICE: return "SECRETARY OFFICE";
        case FLOPPY144_ROOM_DIRECTOR_OFFICE:  return "DIRECTOR OFFICE";
        case FLOPPY144_ROOM_SECURITY:         return "SECURITY";
        case FLOPPY144_ROOM_SERVER_ROOM:      return "SERVER ROOM";
        default:                              return "SITE";
    }
}


/*
 * The shared Site HUD: both normal exploration and the Grey Door scene draw
 * this exact header. The scene may provide a presentation-only percentage;
 * the authoritative recovered KB and RunState remain unchanged.
 */
void Floppy144Site2DDrawHeader(
    Floppy144Surface *surface,
    const Floppy144RunState *run_state,
    uint32_t display_percent,
    bool warning_flash
)
{
    char status_text[64];
    const uint32_t muted=FLOPPY144_RGB(116,132,130);
    const uint32_t normal=FLOPPY144_RGB(100,156,111);
    if(surface==NULL || surface->pixels==NULL || run_state==NULL) return;
    if(display_percent>144U) display_percent=144U;
    (void)snprintf(status_text,sizeof(status_text),
        "RECOVERED %u / %u KB  %u%%",
        (unsigned)Floppy144RunStateRecoveredKb(run_state),
        (unsigned)FLOPPY144_RECOVERY_CAPACITY_KB,
        (unsigned)display_percent);
    /* The HUD strip is stable even while the room behind it corrupts. */
    Floppy144DrawFillRect(surface,0U,0U,640U,20U,
        FLOPPY144_RGB(12,17,21));
    Floppy144DrawText(surface,10U,5U,"GDR SITE RECONSTRUCTION",1U,muted);
    Floppy144DrawText(surface,
        630U-Floppy144DrawTextWidth(status_text,1U),5U,status_text,1U,
        warning_flash?FLOPPY144_RGB(240,68,68):normal);
}

void Floppy144Site2DDrawForPlayerState(
    Floppy144Surface *pSurface,
    const Floppy144RunState *run_state,
    const char *notice,
    Floppy144OperatorBodyStyle body_style,
    const Floppy144PlayerVisualState *player_visual
)
{
    const uint32_t background =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t viewport_background =
        FLOPPY144_RGB(18, 24, 27);

    const uint32_t wall_colour =
        FLOPPY144_RGB(74, 82, 81);

    const uint32_t wall_edge =
        FLOPPY144_RGB(113, 124, 120);

    const uint32_t text_colour =
        FLOPPY144_RGB(201, 210, 203);

    const uint32_t muted =
        FLOPPY144_RGB(116, 132, 130);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    Floppy144Surface surface;
    Floppy144RoomId active_room;
    Floppy144SiteCamera2D camera;

    const char *room_label;
    const char *context_label = NULL;
    const char *prompt;
    const char *menu_prompt = "ESC SESSION CONTROL";
    uint32_t index;
    uint32_t rect_count;
    bool room_reconstructed;

    if(
        pSurface == NULL ||
        run_state == NULL ||
        pSurface->pixels == NULL
    )
    {
        return;
    }

    surface = *pSurface;

    active_room =
        Floppy144SiteRoomAtPosition(
            run_state->player_site_x,
            run_state->player_site_y
        );

    if(active_room == FLOPPY144_ROOM_COUNT)
    {
        active_room = FLOPPY144_ROOM_RECEPTION;
    }

    room_reconstructed =
        Floppy144RunStateRoomReconstructed(
            run_state,
            active_room
        );

    room_label =
        room_reconstructed
            ? Floppy144Site2DRoomLabel(active_room)
            : "ROOM DATA NOT RECONSTRUCTED";

    /*
     * Passive furniture/door labels occupy the same status line as the room
     * name. Explicit interaction notices always win and therefore overwrite
     * the passive label until movement clears the notice.
     */
    if(notice != NULL)
    {
        room_label = notice;
    }
    else if(room_reconstructed)
    {
        context_label = Floppy144GreyDoorNearby(run_state)
            ? "GREY DOOR" : Floppy144SiteContextLabel(run_state);

        if(context_label != NULL)
        {
            room_label = context_label;
        }
    }

    prompt =
        Floppy144Site2DInteractionPrompt(
            run_state
        );

    /*
     * Persistent Site Exploration shell. The scrolling room view is clipped to
     * the central 576 x 252 window, leaving the reconstruction HUD untouched.
     */
    Floppy144DrawClear(
        &surface,
        background
    );

    Floppy144Site2DDrawHeader(&surface,run_state,
        Floppy144RunStateRecoveredPercent(run_state),false);

    Floppy144DrawFillRect(
        &surface,
        FLOPPY144_SITE_2D_FRAME_X,
        FLOPPY144_SITE_2D_FRAME_Y,
        FLOPPY144_SITE_2D_FRAME_WIDTH,
        FLOPPY144_SITE_2D_FRAME_HEIGHT,
        wall_colour
    );

    Floppy144DrawRect(
        &surface,
        FLOPPY144_SITE_2D_FRAME_X,
        FLOPPY144_SITE_2D_FRAME_Y,
        FLOPPY144_SITE_2D_FRAME_WIDTH,
        FLOPPY144_SITE_2D_FRAME_HEIGHT,
        wall_edge
    );

    Floppy144DrawFillRect(
        &surface,
        FLOPPY144_SITE_2D_VIEWPORT_X,
        FLOPPY144_SITE_2D_VIEWPORT_Y,
        FLOPPY144_SITE_2D_VIEWPORT_WIDTH,
        FLOPPY144_SITE_2D_VIEWPORT_HEIGHT,
        viewport_background
    );

    if(
        room_reconstructed &&
        Floppy144Site2DBuildCamera(
            active_room,
            run_state,
            &surface,
            &camera
        )
    )
    {
        rect_count =
            Floppy144SiteRectCount();

        /*
         * Four independent visual Z-levels:
         *
         *   1. FLOOR / ROOM SHELL
         *   2. FURNITURE
         *   3. WALL-HANGINGS
         *   4. CAMERA VIEW (player plus viewport/frame)
         *
         * A rectangle belongs to one visual layer only. In particular,
         * wall-mounted fixtures never alter furniture placement or dimensions.
         */

        /* Z1: room floor. */
        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            Floppy144SiteElement element;

            if(
                rect == NULL ||
                !Floppy144Site2DRectVisibleInRoom(
                    run_state,
                    active_room,
                    rect
                )
            )
            {
                continue;
            }

            element =
                (Floppy144SiteElement)rect->type;

            if(Floppy144Site2DIsFloor(element))
            {
                Floppy144Site2DDrawSiteRect(
                    &surface,
                    &camera,
                    rect
                );
            }
        }

        /*
         * The room shell belongs to the floor/background level. Boundaries
         * replace the generated wall cells but do not participate in furniture
         * or wall-hanging ordering.
         */
        Floppy144Site2DDrawRoomBorder(
            &surface,
            &camera,
            active_room
        );

        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            Floppy144SiteElement element;

            if(
                rect == NULL ||
                !Floppy144Site2DRectVisibleInRoom(
                    run_state,
                    active_room,
                    rect
                )
            )
            {
                continue;
            }

            element =
                (Floppy144SiteElement)rect->type;

            if(
                element == FLOPPY144_SITE_DOOR ||
                element == FLOPPY144_SITE_WINDOW ||
                element == FLOPPY144_SITE_PARTITION_WALL
            )
            {
                Floppy144Site2DDrawSiteRect(
                    &surface,
                    &camera,
                    rect
                );
            }
        }

        /* A closed Grey Door occludes only the original corridor wall.
           It is rendered after ordinary boundary cells and before furniture. */
        if(active_room == FLOPPY144_ROOM_CORRIDOR)
            Floppy144Site2DDrawGreyDoor(&surface,&camera,run_state);

        /*
         * Z2: furniture. Chairs are drawn first within this one layer so desk
         * groupings retain their existing local presentation order.
         */
        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            if(
                rect == NULL ||
                rect->type != (uint8_t)FLOPPY144_SITE_CHAIR ||
                !Floppy144Site2DRectVisibleInRoom(
                    run_state,
                    active_room,
                    rect
                )
            )
            {
                continue;
            }

            Floppy144Site2DDrawSiteRect(
                &surface,
                &camera,
                rect
            );
        }

        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            Floppy144SiteElement element;

            if(
                rect == NULL ||
                !Floppy144Site2DRectVisibleInRoom(
                    run_state,
                    active_room,
                    rect
                )
            )
            {
                continue;
            }

            element =
                (Floppy144SiteElement)rect->type;

            if(
                Floppy144Site2DIsFloor(element) ||
                element == FLOPPY144_SITE_DOOR ||
                element == FLOPPY144_SITE_WINDOW ||
                element == FLOPPY144_SITE_PARTITION_WALL ||
                element == FLOPPY144_SITE_WALL_MOUNTED_ITEM ||
                element == FLOPPY144_SITE_CHAIR
            )
            {
                continue;
            }

            Floppy144Site2DDrawSiteRect(
                &surface,
                &camera,
                rect
            );
        }

        /* Z3: wall-hangings, independently above all furniture. */
        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            if(
                rect == NULL ||
                rect->type !=
                    (uint8_t)FLOPPY144_SITE_WALL_MOUNTED_ITEM ||
                !Floppy144Site2DRectVisibleInRoom(
                    run_state,
                    active_room,
                    rect
                )
            )
            {
                continue;
            }

            Floppy144Site2DDrawSiteRect(
                &surface,
                &camera,
                rect
            );
        }

        /*
         * Z4: camera view. The player is composited after world layers; the
         * fixed viewport/frame below then masks the world at the camera edge.
         */
        Floppy144Site2DDrawPlayer(
            &surface,
            &camera,
            run_state,
            body_style,
            player_visual
        );
    }

    /*
     * Reassert the fixed frame after world drawing. This gives the camera a
     * crisp physical window and masks any edge artefacts at the clip line.
     */
    Floppy144DrawRect(
        &surface,
        FLOPPY144_SITE_2D_VIEWPORT_X - 1U,
        FLOPPY144_SITE_2D_VIEWPORT_Y - 1U,
        FLOPPY144_SITE_2D_VIEWPORT_WIDTH + 2U,
        FLOPPY144_SITE_2D_VIEWPORT_HEIGHT + 2U,
        wall_edge
    );

    Floppy144DrawText(
        &surface,
        FLOPPY144_SITE_2D_ROOM_LABEL_X,
        FLOPPY144_SITE_2D_ROOM_LABEL_Y,
        room_label,
        1U,
        notice != NULL || context_label != NULL || !room_reconstructed
            ? amber
            : muted
    );

    Floppy144DrawFillRect(
        &surface,
        FLOPPY144_SITE_2D_FOOTER_X,
        FLOPPY144_SITE_2D_FOOTER_Y,
        FLOPPY144_SITE_2D_FOOTER_WIDTH,
        FLOPPY144_SITE_2D_FOOTER_HEIGHT,
        background
    );

    Floppy144DrawRect(
        &surface,
        FLOPPY144_SITE_2D_FOOTER_X,
        FLOPPY144_SITE_2D_FOOTER_Y,
        FLOPPY144_SITE_2D_FOOTER_WIDTH,
        FLOPPY144_SITE_2D_FOOTER_HEIGHT,
        wall_edge
    );

    Floppy144DrawText(
        &surface,
        32U,
        322U,
        prompt,
        1U,
        text_colour
    );

    Floppy144DrawText(
        &surface,
        FLOPPY144_SITE_2D_FOOTER_X+
            FLOPPY144_SITE_2D_FOOTER_WIDTH-
            12U-
            Floppy144DrawTextWidth(menu_prompt,1U),
        322U,
        menu_prompt,
        1U,
        muted
    );
}

/*
 * Preserve the Stage 3 renderer contract for tests and callers that do not
 * know about persistent operator identity.
 */
void Floppy144Site2DDrawForBodyStyle(
    Floppy144Surface *pSurface,
    const Floppy144RunState *run_state,
    const char *notice,
    Floppy144OperatorBodyStyle body_style
)
{
    Floppy144Site2DDrawForPlayerState(
        pSurface,
        run_state,
        notice,
        body_style,
        NULL
    );
}

void Floppy144Site2DDraw(
    Floppy144Surface *pSurface,
    const Floppy144RunState *run_state,
    const char *notice
)
{
    Floppy144Site2DDrawForBodyStyle(
        pSurface,
        run_state,
        notice,
        FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT
    );
}
