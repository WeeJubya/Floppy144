/*
 * FLOPPY//144 - compact pseudo-isometric Inspection parent renderer.
 *
 * Presentation only. It consumes generated parent metadata and the matching
 * projection-neutral Site rectangle without mutating either.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_game_data.h"
#include "floppy144_site.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Draw one parent into the left-hand Inspection presentation area.
 *
 * Returns true when the parent matched a known furniture/fixture family.
 * Unknown/future variants are still rendered using the generic 2.5D fallback
 * and return false so regression tests can exercise that path explicitly.
 */
bool Floppy144Cabinet25DDraw(
    Floppy144Surface *surface,
    const Floppy144DataRecord *parent,
    const Floppy144SiteRect *site_rect,
    uint32_t content_count,
    uint32_t selected_content
);
