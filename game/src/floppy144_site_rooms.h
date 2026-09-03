/*
 * Floppy//144 - Site room classification
 *
 * FLOOR_* geometry owns room membership. Authored room regions are retained
 * separately for camera/view extents and may overlap on shared boundaries.
 * Both are generated from the same site_layout.jsonc source.
 */

#pragma once

#include "floppy144_room.h"
#include "floppy144_site.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144SiteRegion
{
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
}
Floppy144SiteRegion;

typedef struct Floppy144SiteRoomDefinition
{
    uint8_t first_region;
    uint8_t region_count;
}
Floppy144SiteRoomDefinition;

const Floppy144SiteRoomDefinition *Floppy144SiteRoomGet(
    Floppy144RoomId room
);

const Floppy144SiteRegion *Floppy144SiteRoomRegionGet(
    uint8_t region_index
);

uint8_t Floppy144SiteRoomRegionCount(
    void
);

/* FLOOR_* ownership query. */
bool Floppy144SiteRoomContainsCell(
    Floppy144RoomId room,
    uint8_t x,
    uint8_t y
);

/* FLOOR_* ownership, with no inference from overlapping view regions. */
Floppy144RoomId Floppy144SiteRoomAtCell(
    uint8_t x,
    uint8_t y
);

/*
 * Position-based room query. Floor ownership is authoritative. Door threshold
 * positions are resolved against generated door topology and adjacent floors.
 */
Floppy144RoomId Floppy144SiteRoomAtPosition(
    int32_t x16,
    int32_t y16
);

/* Camera/view bounding box around all authored regions belonging to a room. */
bool Floppy144SiteRoomBounds(
    Floppy144RoomId room,
    Floppy144SiteRegion *bounds
);

/* View-region intersection query used for shared boundary visibility. */
bool Floppy144SiteRoomIntersectsRect(
    Floppy144RoomId room,
    uint8_t rect_x,
    uint8_t rect_y,
    uint8_t rect_width,
    uint8_t rect_height
);
