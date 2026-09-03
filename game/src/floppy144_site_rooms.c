/*
 * Floppy//144 - generated Site room classification
 *
 * site_layout.jsonc is the single authored source. FLOOR_* geometry owns room
 * membership; SITE_ROOM_REGION data exists only for camera/view extents.
 */

#include "floppy144_site_rooms.h"

#include <stddef.h>

#define FLOPPY144_ARRAY_COUNT(values) \
    ((uint8_t)(sizeof(values) / sizeof((values)[0])))

typedef struct Floppy144SiteDoorTopology
{
    Floppy144SiteRegion rect;
    uint8_t room_a;
    uint8_t room_b;
}
Floppy144SiteDoorTopology;

/* Generated camera/view regions. */
static const Floppy144SiteRegion floppy144_site_room_regions[] =
{
    #define SITE_META(site_width, site_height, spawn_x16, spawn_y16, spawn_room)
    #define SITE_ROOM_REGION(room, x, y, width, height) \
        { (uint8_t)(x), (uint8_t)(y), (uint8_t)(width), (uint8_t)(height) },
    #define SITE_ROOM_DEF(room, first_region, region_count)
    #define SITE_GEOMETRY(room, type, x, y, width, height)
    #define SITE_ROTATED_GEOMETRY(room, type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation)
    #define SITE_DOOR(from_room, to_room, x, y, width, height)
    #define SITE_SHARED(type, x, y, width, height)
    #define SITE_ROTATED_SHARED(type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation)

    #include "floppy144_site_generated.def"

    #undef SITE_ROTATED_SHARED
    #undef SITE_SHARED
    #undef SITE_DOOR
    #undef SITE_ROTATED_GEOMETRY
    #undef SITE_GEOMETRY
    #undef SITE_ROOM_DEF
    #undef SITE_ROOM_REGION
    #undef SITE_META
};

/*
 * Generated room definitions are emitted in canonical Floppy144RoomId order,
 * so no designated initializers or second hand-maintained room table are
 * required.
 */
static const Floppy144SiteRoomDefinition
floppy144_site_rooms[FLOPPY144_ROOM_COUNT] =
{
    #define SITE_META(site_width, site_height, spawn_x16, spawn_y16, spawn_room)
    #define SITE_ROOM_REGION(room, x, y, width, height)
    #define SITE_ROOM_DEF(room, first_region, region_count) \
        { (uint8_t)(first_region), (uint8_t)(region_count) },
    #define SITE_GEOMETRY(room, type, x, y, width, height)
    #define SITE_ROTATED_GEOMETRY(room, type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation)
    #define SITE_DOOR(from_room, to_room, x, y, width, height)
    #define SITE_SHARED(type, x, y, width, height)
    #define SITE_ROTATED_SHARED(type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation)

    #include "floppy144_site_generated.def"

    #undef SITE_ROTATED_SHARED
    #undef SITE_SHARED
    #undef SITE_DOOR
    #undef SITE_ROTATED_GEOMETRY
    #undef SITE_GEOMETRY
    #undef SITE_ROOM_DEF
    #undef SITE_ROOM_REGION
    #undef SITE_META
};

/* Generated physical doors and their bidirectional topology. */
static const Floppy144SiteDoorTopology floppy144_site_doors[] =
{
    #define SITE_META(site_width, site_height, spawn_x16, spawn_y16, spawn_room)
    #define SITE_ROOM_REGION(room, x, y, width, height)
    #define SITE_ROOM_DEF(room, first_region, region_count)
    #define SITE_GEOMETRY(room, type, x, y, width, height)
    #define SITE_ROTATED_GEOMETRY(room, type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation)
    #define SITE_DOOR(from_room, to_room, x, y, width, height) \
        { { (uint8_t)(x), (uint8_t)(y), (uint8_t)(width), (uint8_t)(height) }, \
          (uint8_t)(from_room), (uint8_t)(to_room) },
    #define SITE_SHARED(type, x, y, width, height)
    #define SITE_ROTATED_SHARED(type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation)

    #include "floppy144_site_generated.def"

    #undef SITE_ROTATED_SHARED
    #undef SITE_SHARED
    #undef SITE_DOOR
    #undef SITE_ROTATED_GEOMETRY
    #undef SITE_GEOMETRY
    #undef SITE_ROOM_DEF
    #undef SITE_ROOM_REGION
    #undef SITE_META
};

#define FLOPPY144_SITE_DOOR_COUNT \
    ((uint32_t)(sizeof(floppy144_site_doors) / sizeof(floppy144_site_doors[0])))

static bool Floppy144SiteRectContainsCell(
    const Floppy144SiteRect *rect,
    uint8_t x,
    uint8_t y
)
{
    uint16_t max_x;
    uint16_t max_y;

    if(rect == NULL)
    {
        return false;
    }

    max_x = (uint16_t)rect->x + (uint16_t)rect->width;
    max_y = (uint16_t)rect->y + (uint16_t)rect->height;

    return
        (uint16_t)x >= (uint16_t)rect->x &&
        (uint16_t)x < max_x &&
        (uint16_t)y >= (uint16_t)rect->y &&
        (uint16_t)y < max_y;
}

static bool Floppy144SiteElementIsFloor(
    uint8_t type
)
{
    return
        type >= (uint8_t)FLOPPY144_SITE_FLOOR_A &&
        type <= (uint8_t)FLOPPY144_SITE_FLOOR_D;
}

static Floppy144RoomId Floppy144SiteFloorRoomAtCellSigned(
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
        return FLOPPY144_ROOM_COUNT;
    }

    return Floppy144SiteRoomAtCell((uint8_t)x, (uint8_t)y);
}

static bool Floppy144SiteDoorContainsPosition(
    const Floppy144SiteDoorTopology *door,
    int32_t x16,
    int32_t y16
)
{
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;

    if(door == NULL)
    {
        return false;
    }

    x0 = (int32_t)door->rect.x * FLOPPY144_SITE_FIXED_ONE;
    y0 = (int32_t)door->rect.y * FLOPPY144_SITE_FIXED_ONE;
    x1 =
        ((int32_t)door->rect.x + (int32_t)door->rect.width) *
        FLOPPY144_SITE_FIXED_ONE;
    y1 =
        ((int32_t)door->rect.y + (int32_t)door->rect.height) *
        FLOPPY144_SITE_FIXED_ONE;

    return x16 >= x0 && x16 < x1 && y16 >= y0 && y16 < y1;
}

static Floppy144RoomId Floppy144SiteResolveDoorRoom(
    const Floppy144SiteDoorTopology *door,
    int32_t x16,
    int32_t y16
)
{
    Floppy144RoomId side_a;
    Floppy144RoomId side_b;
    int32_t midpoint16;
    int32_t cell_x;
    int32_t cell_y;

    if(door == NULL)
    {
        return FLOPPY144_ROOM_COUNT;
    }

    if(door->room_a == FLOPPY144_SITE_ROOM_OUTSIDE)
    {
        return
            door->room_b < (uint8_t)FLOPPY144_ROOM_COUNT
                ? (Floppy144RoomId)door->room_b
                : FLOPPY144_ROOM_COUNT;
    }

    if(door->room_b == FLOPPY144_SITE_ROOM_OUTSIDE)
    {
        return
            door->room_a < (uint8_t)FLOPPY144_ROOM_COUNT
                ? (Floppy144RoomId)door->room_a
                : FLOPPY144_ROOM_COUNT;
    }

    if(door->room_a == door->room_b)
    {
        return
            door->room_a < (uint8_t)FLOPPY144_ROOM_COUNT
                ? (Floppy144RoomId)door->room_a
                : FLOPPY144_ROOM_COUNT;
    }

    cell_x = x16 / FLOPPY144_SITE_FIXED_ONE;
    cell_y = y16 / FLOPPY144_SITE_FIXED_ONE;

    if(door->rect.width >= door->rect.height)
    {
        side_a =
            Floppy144SiteFloorRoomAtCellSigned(
                cell_x,
                (int32_t)door->rect.y - 1
            );

        side_b =
            Floppy144SiteFloorRoomAtCellSigned(
                cell_x,
                (int32_t)door->rect.y + (int32_t)door->rect.height
            );

        midpoint16 =
            (int32_t)door->rect.y * FLOPPY144_SITE_FIXED_ONE +
            ((int32_t)door->rect.height * FLOPPY144_SITE_FIXED_ONE) / 2;

        if(side_a != FLOPPY144_ROOM_COUNT && side_b != FLOPPY144_ROOM_COUNT)
        {
            return y16 < midpoint16 ? side_a : side_b;
        }

        if(side_a != FLOPPY144_ROOM_COUNT)
        {
            return side_a;
        }

        if(side_b != FLOPPY144_ROOM_COUNT)
        {
            return side_b;
        }
    }
    else
    {
        side_a =
            Floppy144SiteFloorRoomAtCellSigned(
                (int32_t)door->rect.x - 1,
                cell_y
            );

        side_b =
            Floppy144SiteFloorRoomAtCellSigned(
                (int32_t)door->rect.x + (int32_t)door->rect.width,
                cell_y
            );

        midpoint16 =
            (int32_t)door->rect.x * FLOPPY144_SITE_FIXED_ONE +
            ((int32_t)door->rect.width * FLOPPY144_SITE_FIXED_ONE) / 2;

        if(side_a != FLOPPY144_ROOM_COUNT && side_b != FLOPPY144_ROOM_COUNT)
        {
            return x16 < midpoint16 ? side_a : side_b;
        }

        if(side_a != FLOPPY144_ROOM_COUNT)
        {
            return side_a;
        }

        if(side_b != FLOPPY144_ROOM_COUNT)
        {
            return side_b;
        }
    }

    /* Topology fallback if authored floors are not immediately adjacent. */
    if(door->room_a < (uint8_t)FLOPPY144_ROOM_COUNT)
    {
        return (Floppy144RoomId)door->room_a;
    }

    if(door->room_b < (uint8_t)FLOPPY144_ROOM_COUNT)
    {
        return (Floppy144RoomId)door->room_b;
    }

    return FLOPPY144_ROOM_COUNT;
}

const Floppy144SiteRoomDefinition *Floppy144SiteRoomGet(
    Floppy144RoomId room
)
{
    if((uint32_t)room >= (uint32_t)FLOPPY144_ROOM_COUNT)
    {
        return NULL;
    }

    return &floppy144_site_rooms[(uint32_t)room];
}

const Floppy144SiteRegion *Floppy144SiteRoomRegionGet(
    uint8_t region_index
)
{
    if(region_index >= FLOPPY144_ARRAY_COUNT(floppy144_site_room_regions))
    {
        return NULL;
    }

    return &floppy144_site_room_regions[region_index];
}

uint8_t Floppy144SiteRoomRegionCount(
    void
)
{
    return FLOPPY144_ARRAY_COUNT(floppy144_site_room_regions);
}

bool Floppy144SiteRoomContainsCell(
    Floppy144RoomId room,
    uint8_t x,
    uint8_t y
)
{
    return Floppy144SiteRoomAtCell(x, y) == room;
}

Floppy144RoomId Floppy144SiteRoomAtCell(
    uint8_t x,
    uint8_t y
)
{
    uint32_t index;
    uint32_t count;

    if(
        x >= FLOPPY144_SITE_SIZE_UNITS ||
        y >= FLOPPY144_SITE_SIZE_UNITS
    )
    {
        return FLOPPY144_ROOM_COUNT;
    }

    count = Floppy144SiteRectCount();

    for(index = 0U; index < count; ++index)
    {
        const Floppy144SiteRect *rect = Floppy144SiteRectAt(index);

        if(
            rect == NULL ||
            !Floppy144SiteElementIsFloor(rect->type) ||
            rect->room >= (uint8_t)FLOPPY144_ROOM_COUNT
        )
        {
            continue;
        }

        if(Floppy144SiteRectContainsCell(rect, x, y))
        {
            return (Floppy144RoomId)rect->room;
        }
    }

    return FLOPPY144_ROOM_COUNT;
}

Floppy144RoomId Floppy144SiteRoomAtPosition(
    int32_t x16,
    int32_t y16
)
{
    int32_t max_position =
        FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE;
    Floppy144RoomId room;
    uint32_t door_index;

    if(
        x16 < 0 || y16 < 0 ||
        x16 >= max_position || y16 >= max_position
    )
    {
        return FLOPPY144_ROOM_COUNT;
    }

    room =
        Floppy144SiteRoomAtCell(
            (uint8_t)(x16 / FLOPPY144_SITE_FIXED_ONE),
            (uint8_t)(y16 / FLOPPY144_SITE_FIXED_ONE)
        );

    if(room != FLOPPY144_ROOM_COUNT)
    {
        return room;
    }

    for(door_index = 0U; door_index < FLOPPY144_SITE_DOOR_COUNT; ++door_index)
    {
        const Floppy144SiteDoorTopology *door =
            &floppy144_site_doors[door_index];

        if(Floppy144SiteDoorContainsPosition(door, x16, y16))
        {
            return Floppy144SiteResolveDoorRoom(door, x16, y16);
        }
    }

    return FLOPPY144_ROOM_COUNT;
}

bool Floppy144SiteRoomBounds(
    Floppy144RoomId room,
    Floppy144SiteRegion *bounds
)
{
    const Floppy144SiteRoomDefinition *definition;
    const Floppy144SiteRegion *first;
    uint16_t min_x;
    uint16_t min_y;
    uint16_t max_x;
    uint16_t max_y;
    uint8_t offset;

    if(bounds == NULL)
    {
        return false;
    }

    definition = Floppy144SiteRoomGet(room);

    if(definition == NULL || definition->region_count == 0U)
    {
        return false;
    }

    first = Floppy144SiteRoomRegionGet(definition->first_region);

    if(first == NULL)
    {
        return false;
    }

    min_x = first->x;
    min_y = first->y;
    max_x = (uint16_t)first->x + (uint16_t)first->width;
    max_y = (uint16_t)first->y + (uint16_t)first->height;

    for(offset = 1U; offset < definition->region_count; ++offset)
    {
        const Floppy144SiteRegion *region =
            Floppy144SiteRoomRegionGet(
                (uint8_t)(definition->first_region + offset)
            );
        uint16_t region_max_x;
        uint16_t region_max_y;

        if(region == NULL)
        {
            continue;
        }

        region_max_x = (uint16_t)region->x + (uint16_t)region->width;
        region_max_y = (uint16_t)region->y + (uint16_t)region->height;

        if(region->x < min_x) min_x = region->x;
        if(region->y < min_y) min_y = region->y;
        if(region_max_x > max_x) max_x = region_max_x;
        if(region_max_y > max_y) max_y = region_max_y;
    }

    bounds->x = (uint8_t)min_x;
    bounds->y = (uint8_t)min_y;
    bounds->width = (uint8_t)(max_x - min_x);
    bounds->height = (uint8_t)(max_y - min_y);

    return true;
}

bool Floppy144SiteRoomIntersectsRect(
    Floppy144RoomId room,
    uint8_t rect_x,
    uint8_t rect_y,
    uint8_t rect_width,
    uint8_t rect_height
)
{
    const Floppy144SiteRoomDefinition *definition;
    uint16_t rect_x1;
    uint16_t rect_y1;
    uint8_t offset;

    if(rect_width == 0U || rect_height == 0U)
    {
        return false;
    }

    definition = Floppy144SiteRoomGet(room);

    if(definition == NULL)
    {
        return false;
    }

    rect_x1 = (uint16_t)rect_x + (uint16_t)rect_width;
    rect_y1 = (uint16_t)rect_y + (uint16_t)rect_height;

    for(offset = 0U; offset < definition->region_count; ++offset)
    {
        const Floppy144SiteRegion *region =
            Floppy144SiteRoomRegionGet(
                (uint8_t)(definition->first_region + offset)
            );
        uint16_t region_x1;
        uint16_t region_y1;

        if(region == NULL)
        {
            continue;
        }

        region_x1 = (uint16_t)region->x + (uint16_t)region->width;
        region_y1 = (uint16_t)region->y + (uint16_t)region->height;

        if(
            (uint16_t)rect_x < region_x1 &&
            rect_x1 > (uint16_t)region->x &&
            (uint16_t)rect_y < region_y1 &&
            rect_y1 > (uint16_t)region->y
        )
        {
            return true;
        }
    }

    return false;
}
