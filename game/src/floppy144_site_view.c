/*
 * Floppy//144 - shared Site presentation transform
 *
 * Stage 3B.3 geometry-orientation migration:
 * the canonical JSON now stores the player-facing Site orientation directly.
 * These helpers therefore deliberately perform identity transforms. Keeping
 * the facade avoids churn in camera/input call sites while removing the old
 * hidden 90-degree coordinate conversion from runtime behaviour.
 */

#include "floppy144_site_view.h"

#include <stddef.h>

void Floppy144SiteViewPoint(
    int32_t world_x16,
    int32_t world_y16,
    int32_t *view_x16,
    int32_t *view_y16
)
{
    if(view_x16 != NULL)
    {
        *view_x16 = world_x16;
    }

    if(view_y16 != NULL)
    {
        *view_y16 = world_y16;
    }
}

void Floppy144SiteViewRect(
    const Floppy144SiteRect *world_rect,
    Floppy144SiteRect *view_rect
)
{
    if(world_rect == NULL || view_rect == NULL)
    {
        return;
    }

    /*
     * Canonical Site coordinates are already presentation coordinates.
     * Copy the complete record so boundary endpoints and authored rotation are
     * preserved as well as the rectangle dimensions.
     */
    *view_rect = *world_rect;
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
        *world_delta_x16 = view_delta_x16;
    }

    if(world_delta_y16 != NULL)
    {
        *world_delta_y16 = view_delta_y16;
    }
}
