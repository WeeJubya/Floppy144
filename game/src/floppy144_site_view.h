/*
 * Floppy//144 - shared Site presentation transform
 *
 * The authoritative Office Dimensions world remains unrotated.
 *
 * Player-facing Site views are rotated 90 degrees clockwise so the
 * source-plan northern entrance is presented on the eastern side.
 *
 * This transform sits between canonical Site coordinates and any
 * projection-specific renderer.
 */

#pragma once

#include "floppy144_site.h"

#include <stdint.h>

/*
 * Transform one fixed-point canonical Site position into the
 * player-facing clockwise-rotated view.
 */

void Floppy144SiteViewPoint(
    int32_t world_x16,
    int32_t world_y16,
    int32_t *view_x16,
    int32_t *view_y16
);

/*
 * Rotate one whole-unit Site rectangle clockwise.
 *
 * Source and destination may point to the same object.
 */

void Floppy144SiteViewRect(
    const Floppy144SiteRect *world_rect,
    Floppy144SiteRect *view_rect
);

/*
 * Convert movement expressed relative to the screen back into
 * canonical Site movement.
 *
 * Clockwise display transform:
 *
 *   screen right -> canonical north
 *   screen left  -> canonical south
 *   screen up    -> canonical west
 *   screen down  -> canonical east
 */

void Floppy144SiteViewMovementToWorld(
    int32_t view_delta_x16,
    int32_t view_delta_y16,
    int32_t *world_delta_x16,
    int32_t *world_delta_y16
);
