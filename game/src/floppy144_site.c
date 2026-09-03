/*
 * Floppy//144 - projection-neutral Site model implementation
 *
 * Runtime Site geometry is generated from site_layout.jsonc by the
 * development-only Site compiler. The game never parses JSONC at runtime.
 */

#include "floppy144_site.h"

#include "floppy144_room.h"

#include <stddef.h>

/*
 * Pull only the physical rectangles from the generated Site definition.
 * Room metadata/topology macros deliberately expand to nothing here.
 */
static const Floppy144SiteRect floppy144_site_rects[] =
{
    #define SITE_META(site_width, site_height, spawn_x16, spawn_y16, spawn_room)
    #define SITE_ROOM_REGION(room, x, y, width, height)
    #define SITE_ROOM_DEF(room, first_region, region_count)

    #define SITE_GEOMETRY(room, type, x, y, width, height) \
        { (uint8_t)(type), (uint8_t)(room), (uint8_t)(x), (uint8_t)(y), \
          (uint8_t)(width), (uint8_t)(height) },

    #define SITE_ROTATED_GEOMETRY(room, type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation) \
        { (uint8_t)(type), (uint8_t)(room), (uint8_t)(x), (uint8_t)(y), \
          (uint8_t)(width), (uint8_t)(height) },

    #define SITE_DOOR(from_room, to_room, x, y, width, height) \
        { (uint8_t)FLOPPY144_SITE_DOOR, (uint8_t)FLOPPY144_SITE_ROOM_SHARED, \
          (uint8_t)(x), (uint8_t)(y), (uint8_t)(width), (uint8_t)(height) },

    #define SITE_SHARED(type, x, y, width, height) \
        { (uint8_t)(type), (uint8_t)FLOPPY144_SITE_ROOM_SHARED, \
          (uint8_t)(x), (uint8_t)(y), (uint8_t)(width), (uint8_t)(height) },

    #define SITE_ROTATED_SHARED(type, x, y, width, height, centre_x16, centre_y16, width16, height16, rotation) \
        { (uint8_t)(type), (uint8_t)FLOPPY144_SITE_ROOM_SHARED, \
          (uint8_t)(x), (uint8_t)(y), (uint8_t)(width), (uint8_t)(height) },

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

#define FLOPPY144_SITE_RECT_COUNT \
((uint32_t)(sizeof(floppy144_site_rects) / sizeof(floppy144_site_rects[0])))

/* Pull the compiled spawn point from the same generated source. */
#define SITE_META(site_width, site_height, spawn_x16, spawn_y16, spawn_room) \
    static const int32_t floppy144_site_spawn_x16 = (int32_t)(spawn_x16); \
    static const int32_t floppy144_site_spawn_y16 = (int32_t)(spawn_y16);
#define SITE_ROOM_REGION(room, x, y, width, height)
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

#define FLOPPY144_SITE_MIN_X16 0
#define FLOPPY144_SITE_MIN_Y16 0
#define FLOPPY144_SITE_MAX_X16 \
    (FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE)
#define FLOPPY144_SITE_MAX_Y16 \
    (FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE)

uint32_t Floppy144SiteRectCount(
    void
)
{
    return FLOPPY144_SITE_RECT_COUNT;
}

const Floppy144SiteRect *Floppy144SiteRectAt(
    uint32_t index
)
{
    if(index >= FLOPPY144_SITE_RECT_COUNT)
    {
        return NULL;
    }

    return &floppy144_site_rects[index];
}

bool Floppy144SiteElementBlocksMovement(
    Floppy144SiteElement element
)
{
    if((uint32_t)element >= (uint32_t)FLOPPY144_SITE_ELEMENT_COUNT)
    {
        return true;
    }

    return element > FLOPPY144_SITE_DOOR;
}

static bool Floppy144SiteElementIsWalkableGround(
    uint8_t type
)
{
    return
        (type >= (uint8_t)FLOPPY144_SITE_FLOOR_A &&
         type <= (uint8_t)FLOPPY144_SITE_FLOOR_D) ||
        type == (uint8_t)FLOPPY144_SITE_DOOR;
}

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

static bool Floppy144SiteCellHasWalkableGround(
    uint8_t x,
    uint8_t y
)
{
    uint32_t index;

    for(index = 0U; index < FLOPPY144_SITE_RECT_COUNT; ++index)
    {
        const Floppy144SiteRect *rect = &floppy144_site_rects[index];

        if(
            Floppy144SiteElementIsWalkableGround(rect->type) &&
            Floppy144SiteRectContainsCell(rect, x, y)
        )
        {
            return true;
        }
    }

    return false;
}

bool Floppy144SitePositionBlocked(
    int32_t centre_x16,
    int32_t centre_y16
)
{
    const int32_t half_player =
        FLOPPY144_SITE_PLAYER_COLLISION_X16 / 2;

    const int32_t player_x0 = centre_x16 - half_player;
    const int32_t player_x1 = centre_x16 + half_player;
    const int32_t player_y0 = centre_y16 - half_player;
    const int32_t player_y1 = centre_y16 + half_player;

    uint32_t index;
    int32_t cell_x0;
    int32_t cell_x1;
    int32_t cell_y0;
    int32_t cell_y1;
    int32_t cell_x;
    int32_t cell_y;

    if(
        player_x0 < FLOPPY144_SITE_MIN_X16 ||
        player_y0 < FLOPPY144_SITE_MIN_Y16 ||
        player_x1 > FLOPPY144_SITE_MAX_X16 ||
        player_y1 > FLOPPY144_SITE_MAX_Y16
    )
    {
        return true;
    }

    /*
     * Empty JSONC space is not walkable. The entire player footprint must be
     * supported by generated FLOOR_* or DOOR geometry. This makes authored
     * floors the physical room interior as well as the room-ownership source,
     * while shared door rectangles provide the only legal wall crossings.
     */
    cell_x0 = player_x0 / FLOPPY144_SITE_FIXED_ONE;
    cell_y0 = player_y0 / FLOPPY144_SITE_FIXED_ONE;
    cell_x1 = (player_x1 - 1) / FLOPPY144_SITE_FIXED_ONE;
    cell_y1 = (player_y1 - 1) / FLOPPY144_SITE_FIXED_ONE;

    for(cell_y = cell_y0; cell_y <= cell_y1; ++cell_y)
    {
        for(cell_x = cell_x0; cell_x <= cell_x1; ++cell_x)
        {
            if(
                !Floppy144SiteCellHasWalkableGround(
                    (uint8_t)cell_x,
                    (uint8_t)cell_y
                )
            )
            {
                return true;
            }
        }
    }

    for(index = 0U; index < FLOPPY144_SITE_RECT_COUNT; ++index)
    {
        const Floppy144SiteRect *rect = &floppy144_site_rects[index];
        int32_t rect_x0;
        int32_t rect_x1;
        int32_t rect_y0;
        int32_t rect_y1;

        if(
            !Floppy144SiteElementBlocksMovement(
                (Floppy144SiteElement)rect->type
            )
        )
        {
            continue;
        }

        rect_x0 = (int32_t)rect->x * FLOPPY144_SITE_FIXED_ONE;
        rect_x1 =
            ((int32_t)rect->x + (int32_t)rect->width) *
            FLOPPY144_SITE_FIXED_ONE;

        rect_y0 = (int32_t)rect->y * FLOPPY144_SITE_FIXED_ONE;
        rect_y1 =
            ((int32_t)rect->y + (int32_t)rect->height) *
            FLOPPY144_SITE_FIXED_ONE;

        if(
            player_x1 > rect_x0 &&
            player_x0 < rect_x1 &&
            player_y1 > rect_y0 &&
            player_y0 < rect_y1
        )
        {
            return true;
        }
    }

    return false;
}

void Floppy144SiteSpawnPosition(
    int32_t *x16,
    int32_t *y16
)
{
    if(x16 != NULL)
    {
        *x16 = floppy144_site_spawn_x16;
    }

    if(y16 != NULL)
    {
        *y16 = floppy144_site_spawn_y16;
    }
}

bool Floppy144SiteMovePosition(
    int32_t *x16,
    int32_t *y16,
    int32_t delta_x16,
    int32_t delta_y16
)
{
    int32_t next;
    bool moved = false;

    if(x16 == NULL || y16 == NULL)
    {
        return false;
    }

    next = *x16 + delta_x16;

    if(!Floppy144SitePositionBlocked(next, *y16))
    {
        if(next != *x16)
        {
            *x16 = next;
            moved = true;
        }
    }

    next = *y16 + delta_y16;

    if(!Floppy144SitePositionBlocked(*x16, next))
    {
        if(next != *y16)
        {
            *y16 = next;
            moved = true;
        }
    }

    return moved;
}
