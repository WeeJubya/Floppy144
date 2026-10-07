/*
 * Floppy//144 - fixed-point Site 2D camera transform
 */

#include "floppy144_site_2d_camera.h"

#include "floppy144_site_rooms.h"
#include "floppy144_site_view.h"

#include <stddef.h>

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

bool Floppy144Site2DBuildCameraAtScale(
    Floppy144RoomId room,
    const Floppy144RunState *run_state,
    const Floppy144Surface *surface,
    int32_t pixels_per_unit,
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

    int32_t minimum;
    int32_t maximum;

    if(
        run_state == NULL ||
        surface == NULL ||
        camera == NULL ||
        pixels_per_unit <= 0
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

    world_rect.from_room =
        (uint8_t)room;

    world_rect.to_room =
        (uint8_t)room;

    world_rect.rotation =
        0U;

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
     * Preserve the established room-edge gutters. The larger top gutter keeps
     * the standing player sprite visible when the camera reaches a top wall.
     */
    room_x0 -=
        FLOPPY144_SITE_2D_CAMERA_GUTTER_X16;

    room_y0 -=
        FLOPPY144_SITE_2D_CAMERA_TOP_GUTTER_X16;

    room_x1 +=
        FLOPPY144_SITE_2D_CAMERA_GUTTER_X16;

    room_y1 +=
        FLOPPY144_SITE_2D_CAMERA_GUTTER_X16;

    room_width16 =
        room_x1 - room_x0;

    room_height16 =
        room_y1 - room_y0;

    camera->screen_x =
        FLOPPY144_SITE_2D_VIEWPORT_X;

    camera->screen_y =
        FLOPPY144_SITE_2D_VIEWPORT_Y;

    camera->width =
        FLOPPY144_SITE_2D_VIEWPORT_WIDTH;

    camera->height =
        FLOPPY144_SITE_2D_VIEWPORT_HEIGHT;

    camera->pixels_per_unit =
        pixels_per_unit;

    camera->visible_width16 =
        (camera->width * FLOPPY144_SITE_FIXED_ONE) /
        pixels_per_unit;

    camera->visible_height16 =
        (camera->height * FLOPPY144_SITE_FIXED_ONE) /
        pixels_per_unit;

    Floppy144SiteViewPoint(
        run_state->player_site_x,
        run_state->player_site_y,
        &player_view_x16,
        &player_view_y16
    );

    /*
     * Keep the player centred while there is room to scroll. When the camera
     * reaches an edge, stop the camera and allow the player to move off-centre.
     */
    if(room_width16 <= camera->visible_width16)
    {
        camera->x16 =
            room_x0 -
            (camera->visible_width16 - room_width16) / 2;
    }
    else
    {
        minimum =
            room_x0;

        maximum =
            room_x1 - camera->visible_width16;

        camera->x16 =
            Floppy144Site2DClamp(
                player_view_x16 -
                    camera->visible_width16 / 2,
                minimum,
                maximum
            );
    }

    if(room_height16 <= camera->visible_height16)
    {
        camera->y16 =
            room_y0 -
            (camera->visible_height16 - room_height16) / 2;
    }
    else
    {
        minimum =
            room_y0;

        maximum =
            room_y1 - camera->visible_height16;

        camera->y16 =
            Floppy144Site2DClamp(
                player_view_y16 -
                    camera->visible_height16 / 2,
                minimum,
                maximum
            );
    }

    return true;
}

bool Floppy144Site2DBuildCamera(
    Floppy144RoomId room,
    const Floppy144RunState *run_state,
    const Floppy144Surface *surface,
    Floppy144SiteCamera2D *camera
)
{
    return
        Floppy144Site2DBuildCameraAtScale(
            room,
            run_state,
            surface,
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT,
            camera
        );
}

int32_t Floppy144Site2DProjectViewX16(
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
        (
            (view_x16 - camera->x16) *
            camera->pixels_per_unit
        ) /
        FLOPPY144_SITE_FIXED_ONE;
}

int32_t Floppy144Site2DProjectViewY16(
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
        (
            (view_y16 - camera->y16) *
            camera->pixels_per_unit
        ) /
        FLOPPY144_SITE_FIXED_ONE;
}

void Floppy144Site2DProjectPoint(
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

bool Floppy144Site2DProjectRect(
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
        camera->pixels_per_unit;

    screen_rect->height =
        (int32_t)view_rect.height *
        camera->pixels_per_unit;

    return
        screen_rect->width > 0 &&
        screen_rect->height > 0;
}
