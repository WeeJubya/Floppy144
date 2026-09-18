/*
 * Floppy//144 - shared Site presentation facade
 *
 * The canonical JSON now stores the Site in its player-facing orientation.
 * Runtime rendering, collision and input all use the same coordinate system.
 *
 * These helpers remain as a stable boundary between the Site model and
 * projection/input modules, but they intentionally perform identity mapping.
 */

#pragma once

#include "floppy144_site.h"

#include <stdint.h>

/* Copy one fixed-point canonical Site position into presentation space. */
void Floppy144SiteViewPoint(
    int32_t world_x16,
    int32_t world_y16,
    int32_t *view_x16,
    int32_t *view_y16
);

/* Copy one whole-unit Site rectangle into presentation space. */
void Floppy144SiteViewRect(
    const Floppy144SiteRect *world_rect,
    Floppy144SiteRect *view_rect
);

/*
 * Input and canonical movement now share axes directly:
 *
 *   screen right -> Site +X
 *   screen left  -> Site -X
 *   screen up    -> Site -Y
 *   screen down  -> Site +Y
 */
void Floppy144SiteViewMovementToWorld(
    int32_t view_delta_x16,
    int32_t view_delta_y16,
    int32_t *world_delta_x16,
    int32_t *world_delta_y16
);
