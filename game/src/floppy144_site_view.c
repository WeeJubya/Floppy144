/*
 * Floppy//144 - shared Site presentation transform
 */

#include "floppy144_site_view.h"

#include <stddef.h>

#define FLOPPY144_SITE_VIEW_MAX_X16 \
(FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE)

void Floppy144SiteViewPoint(
    int32_t world_x16,
    int32_t world_y16,
    int32_t *view_x16,
    int32_t *view_y16
)
{
    if(view_x16 != NULL)
    {
        *view_x16 =
        FLOPPY144_SITE_VIEW_MAX_X16 -
        world_y16;
    }

    if(view_y16 != NULL)
    {
        *view_y16 =
        world_x16;
    }
}

void Floppy144SiteViewRect(
    const Floppy144SiteRect *world_rect,
    Floppy144SiteRect *view_rect
)
{
    Floppy144SiteRect source;

    if(
        world_rect == NULL ||
        view_rect == NULL
    )
    {
        return;
    }

    /*
     * Copy first so an in-place transform is safe.
     */

    source =
    *world_rect;

    view_rect->type =
    source.type;

    view_rect->room =
    source.room;

    view_rect->x =
    (uint8_t)(
        FLOPPY144_SITE_SIZE_UNITS -
        (
            (uint32_t)source.y +
            (uint32_t)source.height
        )
    );

    view_rect->y =
    source.x;

    view_rect->width =
    source.height;

    view_rect->height =
    source.width;
}

void Floppy144SiteViewMovementToWorld(
    int32_t view_delta_x16,
    int32_t view_delta_y16,
    int32_t *world_delta_x16,
    int32_t *world_delta_y16
)
{
    if(world_delta_x16 != NULL)
    {
        *world_delta_x16 =
        view_delta_y16;
    }

    if(world_delta_y16 != NULL)
    {
        *world_delta_y16 =
        -view_delta_x16;
    }
}
