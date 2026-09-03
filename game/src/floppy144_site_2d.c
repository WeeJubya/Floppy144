/*
 * Floppy//144 - scrolling flat 2D Site projection
 *
 * The authoritative Site remains in canonical 100 x 100 coordinates.
 * This module owns presentation only: clockwise rotation, a fixed-zoom camera,
 * viewport culling and lightweight procedural furniture detail.
 *
 * FM-23 can later replace this projection without changing geometry,
 * collision, interactions or the player's persistent Site position.
 */

#include "floppy144_site_2d.h"

#include "floppy144_draw.h"
#include "floppy144_object_registry.h"
#include "floppy144_site.h"
#include "floppy144_site_object.h"
#include "floppy144_site_rooms.h"
#include "floppy144_site_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Fixed zoom.
 *
 * At 20 pixels per Site unit Reception's 33 x 34 footprint is approximately
 * 660 x 680 pixels. The logical backbuffer therefore acts as a window onto a
 * larger room rather than forcing the complete room onto one screen.
 *
 * Camera sizing is derived from the actual backbuffer dimensions, so a later
 * 640 x 480 canvas requires no Site-camera rewrite.
 */
#define FLOPPY144_SITE_2D_PIXELS_PER_UNIT 20
#define FLOPPY144_SITE_2D_CAMERA_GUTTER_X16 FLOPPY144_SITE_FIXED_ONE
#define FLOPPY144_SITE_2D_CAMERA_TOP_GUTTER_X16 (6 * FLOPPY144_SITE_FIXED_ONE)
#define FLOPPY144_SITE_2D_ROOM_BORDER_PIXELS 5

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

#define FLOPPY144_SITE_2D_VIEWPORT_X       32
#define FLOPPY144_SITE_2D_VIEWPORT_Y       32
#define FLOPPY144_SITE_2D_VIEWPORT_WIDTH  576
#define FLOPPY144_SITE_2D_VIEWPORT_HEIGHT 252

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

typedef struct Floppy144SiteCamera2D
{
    /* Top-left visible point in clockwise view coordinates, fixed-point x16. */
    int32_t x16;
    int32_t y16;

    /* Fixed screen-space viewport occupied by the scrolling Site view. */
    int32_t screen_x;
    int32_t screen_y;
    int32_t width;
    int32_t height;
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

static bool Floppy144Site2DIsBoundaryElement(
    Floppy144SiteElement element
)
{
    return
        element == FLOPPY144_SITE_DOOR ||
        element == FLOPPY144_SITE_WINDOW ||
        element == FLOPPY144_SITE_PARTITION_WALL;
}

static int32_t Floppy144Site2DClamp(
    int32_t value,
    int32_t minimum,
    int32_t maximum
)
{
    if(value < minimum)
    {
        return minimum;
    }

    if(value > maximum)
    {
        return maximum;
    }

    return value;
}

/*
 * Room visibility without a second door database.
 *
 * Exact physical door/window/partition rectangles live only in site_layout.
 * Boundary elements receive a one-unit visibility halo so a threshold lying
 * on one half-open room edge remains visible from the room on either side.
 */
static bool Floppy144Site2DRectVisibleInRoom(
    Floppy144RoomId room,
    const Floppy144SiteRect *rect
)
{
    Floppy144SiteElement element;

    uint8_t expanded_x;
    uint8_t expanded_y;
    uint8_t expanded_width;
    uint8_t expanded_height;

    uint16_t expanded_x1;
    uint16_t expanded_y1;

    if(rect == NULL)
    {
        return false;
    }

    /*
     * Room-owned geometry carries its generated owner directly. Only shared
     * structure needs spatial visibility inference from overlapping view
     * regions. This keeps neighbouring furniture from leaking across walls.
     */
    if(rect->room != FLOPPY144_SITE_ROOM_SHARED)
    {
        return rect->room == (uint8_t)room;
    }

    if(
        Floppy144SiteRoomIntersectsRect(
            room,
            rect->x,
            rect->y,
            rect->width,
            rect->height
        )
    )
    {
        return true;
    }

    element =
        (Floppy144SiteElement)rect->type;

    if(!Floppy144Site2DIsBoundaryElement(element))
    {
        return false;
    }

    expanded_x =
        rect->x > 0U
            ? (uint8_t)(rect->x - 1U)
            : 0U;

    expanded_y =
        rect->y > 0U
            ? (uint8_t)(rect->y - 1U)
            : 0U;

    expanded_x1 =
        (uint16_t)rect->x +
        (uint16_t)rect->width +
        1U;

    expanded_y1 =
        (uint16_t)rect->y +
        (uint16_t)rect->height +
        1U;

    if(expanded_x1 > FLOPPY144_SITE_SIZE_UNITS)
    {
        expanded_x1 = FLOPPY144_SITE_SIZE_UNITS;
    }

    if(expanded_y1 > FLOPPY144_SITE_SIZE_UNITS)
    {
        expanded_y1 = FLOPPY144_SITE_SIZE_UNITS;
    }

    expanded_width =
        (uint8_t)(expanded_x1 - expanded_x);

    expanded_height =
        (uint8_t)(expanded_y1 - expanded_y);

    return
        Floppy144SiteRoomIntersectsRect(
            room,
            expanded_x,
            expanded_y,
            expanded_width,
            expanded_height
        );
}

static bool Floppy144Site2DBuildCamera(
    Floppy144RoomId room,
    const Floppy144RunState *run_state,
    const Floppy144Surface *surface,
    Floppy144SiteCamera2D *camera
)
{
    Floppy144SiteRegion world_bounds;
    Floppy144SiteRect world_rect;
    Floppy144SiteRect view_rect;

    int32_t player_view_x16;
    int32_t player_view_y16;

    int32_t room_x0;
    int32_t room_y0;
    int32_t room_x1;
    int32_t room_y1;

    int32_t room_width16;
    int32_t room_height16;

    int32_t visible_width16;
    int32_t visible_height16;

    int32_t minimum;
    int32_t maximum;

    if(
        run_state == NULL ||
        surface == NULL ||
        camera == NULL
    )
    {
        return false;
    }

    if(!Floppy144SiteRoomBounds(room, &world_bounds))
    {
        world_bounds.x = 0U;
        world_bounds.y = 0U;
        world_bounds.width = FLOPPY144_SITE_SIZE_UNITS;
        world_bounds.height = FLOPPY144_SITE_SIZE_UNITS;
    }

    world_rect.type =
        (uint8_t)FLOPPY144_SITE_FLOOR_A;

    world_rect.room =
        (uint8_t)room;

    world_rect.x = world_bounds.x;
    world_rect.y = world_bounds.y;
    world_rect.width = world_bounds.width;
    world_rect.height = world_bounds.height;

    Floppy144SiteViewRect(
        &world_rect,
        &view_rect
    );

    room_x0 =
        (int32_t)view_rect.x *
        FLOPPY144_SITE_FIXED_ONE;

    room_y0 =
        (int32_t)view_rect.y *
        FLOPPY144_SITE_FIXED_ONE;

    room_x1 =
        ((int32_t)view_rect.x + (int32_t)view_rect.width) *
        FLOPPY144_SITE_FIXED_ONE;

    room_y1 =
        ((int32_t)view_rect.y + (int32_t)view_rect.height) *
        FLOPPY144_SITE_FIXED_ONE;

    /*
     * Keep a one-unit visual gutter beyond the room perimeter so the room
     * border remains visible when the camera reaches an outer clamp.
     */
    room_x0 -= FLOPPY144_SITE_2D_CAMERA_GUTTER_X16;
    room_y0 -= FLOPPY144_SITE_2D_CAMERA_TOP_GUTTER_X16;
    room_x1 += FLOPPY144_SITE_2D_CAMERA_GUTTER_X16;
    room_y1 += FLOPPY144_SITE_2D_CAMERA_GUTTER_X16;

    room_width16 = room_x1 - room_x0;
    room_height16 = room_y1 - room_y0;

    camera->screen_x = FLOPPY144_SITE_2D_VIEWPORT_X;
    camera->screen_y = FLOPPY144_SITE_2D_VIEWPORT_Y;
    camera->width = FLOPPY144_SITE_2D_VIEWPORT_WIDTH;
    camera->height = FLOPPY144_SITE_2D_VIEWPORT_HEIGHT;

    visible_width16 =
        (camera->width * FLOPPY144_SITE_FIXED_ONE) /
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT;

    visible_height16 =
        (camera->height * FLOPPY144_SITE_FIXED_ONE) /
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT;

    Floppy144SiteViewPoint(
        run_state->player_site_x,
        run_state->player_site_y,
        &player_view_x16,
        &player_view_y16
    );

    /*
     * Keep the player centred while there is room to scroll. When the camera
     * reaches an edge, the camera stops and the player walks away from centre.
     */
    if(room_width16 <= visible_width16)
    {
        camera->x16 =
            room_x0 -
            (visible_width16 - room_width16) / 2;
    }
    else
    {
        minimum = room_x0;
        maximum = room_x1 - visible_width16;

        camera->x16 =
            Floppy144Site2DClamp(
                player_view_x16 - visible_width16 / 2,
                minimum,
                maximum
            );
    }

    if(room_height16 <= visible_height16)
    {
        camera->y16 =
            room_y0 -
            (visible_height16 - room_height16) / 2;
    }
    else
    {
        minimum = room_y0;
        maximum = room_y1 - visible_height16;

        camera->y16 =
            Floppy144Site2DClamp(
                player_view_y16 - visible_height16 / 2,
                minimum,
                maximum
            );
    }

    return true;
}

static int32_t Floppy144Site2DProjectViewX16(
    const Floppy144SiteCamera2D *camera,
    int32_t view_x16
)
{
    if(camera == NULL)
    {
        return 0;
    }

    return
        camera->screen_x +
        ((view_x16 - camera->x16) *
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT) /
        FLOPPY144_SITE_FIXED_ONE;
}

static int32_t Floppy144Site2DProjectViewY16(
    const Floppy144SiteCamera2D *camera,
    int32_t view_y16
)
{
    if(camera == NULL)
    {
        return 0;
    }

    return
        camera->screen_y +
        ((view_y16 - camera->y16) *
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT) /
        FLOPPY144_SITE_FIXED_ONE;
}

static void Floppy144Site2DProjectPoint(
    const Floppy144SiteCamera2D *camera,
    int32_t world_x16,
    int32_t world_y16,
    int32_t *screen_x,
    int32_t *screen_y
)
{
    int32_t view_x16;
    int32_t view_y16;

    Floppy144SiteViewPoint(
        world_x16,
        world_y16,
        &view_x16,
        &view_y16
    );

    if(screen_x != NULL)
    {
        *screen_x =
            Floppy144Site2DProjectViewX16(
                camera,
                view_x16
            );
    }

    if(screen_y != NULL)
    {
        *screen_y =
            Floppy144Site2DProjectViewY16(
                camera,
                view_y16
            );
    }
}

static bool Floppy144Site2DProjectRect(
    const Floppy144SiteCamera2D *camera,
    const Floppy144SiteRect *rect,
    Floppy144SiteScreenRect *screen_rect
)
{
    Floppy144SiteRect view_rect;

    if(
        camera == NULL ||
        rect == NULL ||
        screen_rect == NULL
    )
    {
        return false;
    }

    Floppy144SiteViewRect(
        rect,
        &view_rect
    );

    screen_rect->x =
        Floppy144Site2DProjectViewX16(
            camera,
            (int32_t)view_rect.x *
            FLOPPY144_SITE_FIXED_ONE
        );

    screen_rect->y =
        Floppy144Site2DProjectViewY16(
            camera,
            (int32_t)view_rect.y *
            FLOPPY144_SITE_FIXED_ONE
        );

    screen_rect->width =
        (int32_t)view_rect.width *
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT;

    screen_rect->height =
        (int32_t)view_rect.height *
        FLOPPY144_SITE_2D_PIXELS_PER_UNIT;

    return
        screen_rect->width > 0 &&
        screen_rect->height > 0;
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
        Floppy144Site2DClipRect(
            surface,
            &x,
            &y,
            &width,
            &height
        )
    )
    {
        Floppy144DrawRect(
            surface,
            (uint32_t)x,
            (uint32_t)y,
            (uint32_t)width,
            (uint32_t)height,
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

static void Floppy144Site2DDrawWorldEdge(
    Floppy144Surface *surface,
    const Floppy144SiteCamera2D *camera,
    int32_t world_x0,
    int32_t world_y0,
    int32_t world_x1,
    int32_t world_y1,
    uint32_t colour
)
{
    const int32_t thickness =
        FLOPPY144_SITE_2D_ROOM_BORDER_PIXELS;
    int32_t screen_x0;
    int32_t screen_y0;
    int32_t screen_x1;
    int32_t screen_y1;
    int32_t minimum;
    int32_t length;

    Floppy144Site2DProjectPoint(
        camera,
        world_x0 * FLOPPY144_SITE_FIXED_ONE,
        world_y0 * FLOPPY144_SITE_FIXED_ONE,
        &screen_x0,
        &screen_y0
    );

    Floppy144Site2DProjectPoint(
        camera,
        world_x1 * FLOPPY144_SITE_FIXED_ONE,
        world_y1 * FLOPPY144_SITE_FIXED_ONE,
        &screen_x1,
        &screen_y1
    );

    if(screen_x0 == screen_x1)
    {
        minimum = screen_y0 < screen_y1 ? screen_y0 : screen_y1;
        length = screen_y0 < screen_y1 ? screen_y1 - screen_y0 : screen_y0 - screen_y1;

        Floppy144Site2DFill(
            surface,
            screen_x0 - thickness / 2,
            minimum,
            thickness,
            length + 1,
            colour
        );
    }
    else if(screen_y0 == screen_y1)
    {
        minimum = screen_x0 < screen_x1 ? screen_x0 : screen_x1;
        length = screen_x0 < screen_x1 ? screen_x1 - screen_x0 : screen_x0 - screen_x1;

        Floppy144Site2DFill(
            surface,
            minimum,
            screen_y0 - thickness / 2,
            length + 1,
            thickness,
            colour
        );
    }
}

/*
 * Draw the exposed perimeter of the active room's FLOOR_* union.
 *
 * Each edge is tested against adjacent floor ownership, so multi-rectangle
 * rooms such as the corridor do not acquire false internal seams. Doors are
 * drawn after this border and therefore punch clean visual openings through it.
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

        for(x = x0; x < x1; ++x)
        {
            if(!Floppy144Site2DRoomOwnsCell(room, x, y0 - 1))
            {
                Floppy144Site2DDrawWorldEdge(
                    surface,
                    camera,
                    x,
                    y0,
                    x + 1,
                    y0,
                    border_colour
                );
            }

            if(!Floppy144Site2DRoomOwnsCell(room, x, y1))
            {
                Floppy144Site2DDrawWorldEdge(
                    surface,
                    camera,
                    x,
                    y1,
                    x + 1,
                    y1,
                    border_colour
                );
            }
        }

        for(y = y0; y < y1; ++y)
        {
            if(!Floppy144Site2DRoomOwnsCell(room, x0 - 1, y))
            {
                Floppy144Site2DDrawWorldEdge(
                    surface,
                    camera,
                    x0,
                    y,
                    x0,
                    y + 1,
                    border_colour
                );
            }

            if(!Floppy144Site2DRoomOwnsCell(room, x1, y))
            {
                Floppy144Site2DDrawWorldEdge(
                    surface,
                    camera,
                    x1,
                    y,
                    x1,
                    y + 1,
                    border_colour
                );
            }
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

    Floppy144Site2DFill(
        surface,
        screen_rect->x + 3,
        screen_rect->y + 3,
        screen_rect->width,
        screen_rect->height,
        shadow_colour
    );

    Floppy144Site2DFill(
        surface,
        screen_rect->x,
        screen_rect->y,
        screen_rect->width,
        screen_rect->height,
        body_colour
    );

    Floppy144Site2DOutline(
        surface,
        screen_rect->x,
        screen_rect->y,
        screen_rect->width,
        screen_rect->height,
        edge_colour
    );

    if(
        screen_rect->width >= 8 &&
        screen_rect->height >= 8
    )
    {
        Floppy144Site2DFill(
            surface,
            screen_rect->x + 2,
            screen_rect->y + 2,
            screen_rect->width - 4,
            2,
            highlight_colour
        );
    }
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

static void Floppy144Site2DDrawFurnitureDetails(
    Floppy144Surface *surface,
    Floppy144SiteElement element,
    const Floppy144SiteScreenRect *screen_rect,
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

    if(screen_rect == NULL)
    {
        return;
    }

    inset_x = screen_rect->x + screen_rect->width / 8;
    inset_y = screen_rect->y + screen_rect->height / 8;
    inset_w = screen_rect->width * 3 / 4;
    inset_h = screen_rect->height * 3 / 4;

    switch(element)
    {
        case FLOPPY144_SITE_STANDARD_DESK:
        {
            Floppy144Site2DFill(
                surface,
                inset_x,
                inset_y,
                screen_rect->width / 4,
                screen_rect->height / 3,
                detail_colour
            );

            Floppy144Site2DOutline(
                surface,
                inset_x,
                inset_y,
                screen_rect->width / 4,
                screen_rect->height / 3,
                edge_colour
            );

            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width * 5 / 8,
                screen_rect->y + screen_rect->height / 5,
                screen_rect->width / 5,
                3,
                paper_colour
            );

            break;
        }

        case FLOPPY144_SITE_TERMINAL_DESK:
        {
            int32_t monitor_w = screen_rect->width / 2;
            int32_t monitor_h = screen_rect->height / 2;

            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width / 8,
                screen_rect->y + screen_rect->height / 8,
                monitor_w,
                monitor_h,
                detail_colour
            );

            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width / 8 + 4,
                screen_rect->y + screen_rect->height / 8 + 4,
                monitor_w - 8,
                monitor_h - 8,
                screen_colour
            );

            Floppy144Site2DFill(
                surface,
                screen_rect->x + screen_rect->width / 4,
                screen_rect->y + screen_rect->height * 3 / 4,
                screen_rect->width / 2,
                3,
                edge_colour
            );

            break;
        }

        case FLOPPY144_SITE_CHAIR:
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
                screen_rect->x + 4,
                screen_rect->y + 4,
                screen_rect->width - 8,
                3,
                light_colour
            );

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
            Floppy144Site2DFill(
                surface,
                inset_x,
                screen_rect->y + screen_rect->height / 2,
                inset_w,
                2,
                detail_colour
            );

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

    Floppy144Site2DDrawFurnitureBase(
        surface,
        &screen_rect,
        style->colour,
        edge_colour,
        highlight_colour,
        shadow_colour
    );

    Floppy144Site2DDrawFurnitureDetails(
        surface,
        element,
        &screen_rect,
        edge_colour
    );
}

static void Floppy144Site2DDrawPlayer(
    Floppy144Surface *surface,
    const Floppy144SiteCamera2D *camera,
    const Floppy144RunState *run_state
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

    int32_t foot_x;
    int32_t foot_y;

    int32_t sprite_width;
    int32_t sprite_height;

    int32_t sprite_y;

    int32_t head_size;
    int32_t torso_x;
    int32_t torso_y;
    int32_t torso_width;
    int32_t torso_height;
    int32_t leg_width;
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

    sprite_width = (FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16 * FLOPPY144_SITE_2D_PIXELS_PER_UNIT) / FLOPPY144_SITE_FIXED_ONE;

    sprite_height = (FLOPPY144_SITE_2D_PLAYER_VISUAL_HEIGHT_X16 * FLOPPY144_SITE_2D_PIXELS_PER_UNIT) / FLOPPY144_SITE_FIXED_ONE;

    collision_shadow_width = (FLOPPY144_SITE_PLAYER_COLLISION_X16 * FLOPPY144_SITE_2D_PIXELS_PER_UNIT) / FLOPPY144_SITE_FIXED_ONE;

    sprite_y =
        foot_y - sprite_height;

    head_size =
        sprite_width * 2 / 5;

    if(head_size < 8)
    {
        head_size = 8;
    }

    torso_width =
        sprite_width * 3 / 4;

    torso_x =
        foot_x - torso_width / 2;

    torso_y =
        sprite_y + head_size - 2;

    torso_height =
        sprite_height - head_size - sprite_height / 4;

    leg_width =
        torso_width / 3;

    Floppy144Site2DFill(
        surface,
        foot_x - collision_shadow_width / 2,
        foot_y - 3,
        collision_shadow_width,
        6,
        shadow_colour
    );

    Floppy144Site2DFill(
        surface,
        torso_x,
        torso_y,
        torso_width,
        torso_height,
        body_colour
    );

    Floppy144Site2DOutline(
        surface,
        torso_x,
        torso_y,
        torso_width,
        torso_height,
        edge_colour
    );

    Floppy144Site2DFill(
        surface,
        torso_x + 4,
        torso_y + 5,
        5,
        5,
        shirt_light
    );

    Floppy144Site2DFill(
        surface,
        foot_x - head_size / 2,
        sprite_y,
        head_size,
        head_size,
        skin_colour
    );

    Floppy144Site2DOutline(
        surface,
        foot_x - head_size / 2,
        sprite_y,
        head_size,
        head_size,
        edge_colour
    );

    Floppy144Site2DFill(
        surface,
        foot_x - head_size / 2,
        sprite_y,
        head_size,
        head_size / 5,
        trouser_colour
    );

    Floppy144Site2DFill(
        surface,
        torso_x + 2,
        torso_y + torso_height,
        leg_width,
        foot_y - (torso_y + torso_height),
        trouser_colour
    );

    Floppy144Site2DFill(
        surface,
        torso_x + torso_width - leg_width - 2,
        torso_y + torso_height,
        leg_width,
        foot_y - (torso_y + torso_height),
        trouser_colour
    );
}

static const char *Floppy144Site2DInteractionPrompt(
    const Floppy144RunState *run_state
)
{
    Floppy144ObjectId object;
    const Floppy144ObjectDefinition *definition;
    const Floppy144ObjectInteractionDefinition *interaction;

    const char *prompt =
        "WASD OR ARROWS TO MOVE";

    if(run_state == NULL)
    {
        return prompt;
    }

    object =
        Floppy144SiteInteractionTarget(
            run_state
        );

    definition =
        Floppy144ObjectGet(
            object
        );

    interaction =
        definition != NULL
            ? definition->interaction
            : NULL;

    if(
        interaction == NULL ||
        interaction->prompt == NULL
    )
    {
        return prompt;
    }

    prompt =
        interaction->prompt;

    if(
        interaction->alternate_prompt != NULL &&
        interaction->alternate_prompt_collection !=
            FLOPPY144_COLLECTION_COUNT &&
        Floppy144RunStateCollectionRestored(
            run_state,
            interaction->alternate_prompt_collection
        )
    )
    {
        prompt =
            interaction->alternate_prompt;
    }

    return prompt;
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

void Floppy144Site2DDraw(
    F144Runtime *runtime,
    const Floppy144RunState *run_state,
    const char *notice
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

    const uint32_t green =
        FLOPPY144_RGB(100, 156, 111);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    Floppy144Surface surface;
    Floppy144RoomId active_room;
    Floppy144SiteCamera2D camera;

    const char *room_label;
    const char *prompt;
    char status_text[16];

    uint32_t index;
    uint32_t rect_count;
    bool room_reconstructed;

    if(
        runtime == NULL ||
        run_state == NULL ||
        runtime->backbuffer.data == NULL
    )
    {
        return;
    }

    surface.pixels =
        (uint32_t *)runtime->backbuffer.data;

    surface.width =
        runtime->backbuffer.width;

    surface.height =
        runtime->backbuffer.height;

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

    if(notice != NULL)
    {
        room_label = notice;
    }

    prompt =
        Floppy144Site2DInteractionPrompt(
            run_state
        );

    snprintf(
        status_text,
        sizeof(status_text),
        "STATUS %02u%%",
        (unsigned)Floppy144RunStateReconstructionPercent(
            run_state
        )
    );

    /*
     * Persistent Site Exploration shell. The scrolling room view is clipped to
     * the central 576 x 252 window, leaving the reconstruction HUD untouched.
     */
    Floppy144DrawClear(
        &surface,
        background
    );

    Floppy144DrawText(
        &surface,
        10U,
        5U,
        "GDR SITE RECONSTRUCTION",
        1U,
        muted
    );

    Floppy144DrawText(
        &surface,
        526U,
        5U,
        status_text,
        1U,
        green
    );

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

        /* Room floors first. */
        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            Floppy144SiteElement element;

            if(
                rect == NULL ||
                !Floppy144Site2DRectVisibleInRoom(
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

        Floppy144Site2DDrawRoomBorder(
            &surface,
            &camera,
            active_room
        );

        /* Door thresholds above the room border. */
        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            if(
                rect == NULL ||
                rect->type != (uint8_t)FLOPPY144_SITE_DOOR ||
                !Floppy144Site2DRectVisibleInRoom(
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

        /* Structure, furniture and fixtures above the floor. */
        for(index = 0U; index < rect_count; ++index)
        {
            const Floppy144SiteRect *rect =
                Floppy144SiteRectAt(index);

            Floppy144SiteElement element;

            if(
                rect == NULL ||
                !Floppy144Site2DRectVisibleInRoom(
                    active_room,
                    rect
                ) ||
                !Floppy144SiteObjectGeometryVisible(
                    run_state,
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
                element == FLOPPY144_SITE_DOOR
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

        Floppy144Site2DDrawPlayer(
            &surface,
            &camera,
            run_state
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
        notice != NULL || !room_reconstructed
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
        526U,
        322U,
        "ESC RECOVERY",
        1U,
        muted
    );
}
