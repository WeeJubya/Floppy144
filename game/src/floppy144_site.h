/*
 * Floppy//144 - projection-neutral Site model
 *
 * Defines the canonical 100 x 100 GDR Site compiled from site_layout.jsonc.
 * Geometry and collision live here in Site coordinates. Rendering belongs to
 * projection-specific modules.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * Site coordinate system
 *
 * One authored Site unit is one plan unit. Sixteen fixed-point ticks represent
 * one Site unit so half-unit movement and centred player positions require no
 * floating point at runtime.
 */

#define FLOPPY144_SITE_SIZE_UNITS              100
#define FLOPPY144_SITE_FIXED_ONE                16

#define FLOPPY144_SITE_WALL_HEIGHT_UNITS        12

/*
 * Visible player proportions and ground collision footprint are deliberately
 * separate. The standing character remains 3.5 units wide and 8 units high,
 * while movement uses a compact 2 x 2 unit footprint so doorways have
 * comfortable clearance.
 */
#define FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16   56
#define FLOPPY144_SITE_PLAYER_HEIGHT_UNITS        8
#define FLOPPY144_SITE_PLAYER_COLLISION_X16       32

#define FLOPPY144_SITE_MOVE_STEP_X16              8

/*
 * Room ownership marker used by generated Site rectangles that are shared
 * structure rather than room-owned geometry. Door topology uses the same
 * 0xff value for OUTSIDE in generated data.
 */
#define FLOPPY144_SITE_ROOM_SHARED              0xffU
#define FLOPPY144_SITE_ROOM_OUTSIDE             0xffU

/*
 * Site element types. The first four values are floor fills. FLOOR_* geometry
 * is also the authoritative source for room ownership.
 */

typedef enum Floppy144SiteElement
{
    FLOPPY144_SITE_FLOOR_A = 0,
    FLOPPY144_SITE_FLOOR_B,
    FLOPPY144_SITE_FLOOR_C,
    FLOPPY144_SITE_FLOOR_D,

    FLOPPY144_SITE_DOOR,
    FLOPPY144_SITE_WINDOW,
    FLOPPY144_SITE_STANDARD_DESK,
    FLOPPY144_SITE_CHAIR,
    FLOPPY144_SITE_NONSECURE_CABINET,
    FLOPPY144_SITE_SECURE_CABINET_HALF,
    FLOPPY144_SITE_SECURE_CABINET_FULL,
    FLOPPY144_SITE_WALL_MOUNTED_ITEM,
    FLOPPY144_SITE_BOOKCASE,
    FLOPPY144_SITE_TERMINAL_DESK,
    FLOPPY144_SITE_PARTITION_WALL,
    FLOPPY144_SITE_FRIDGE,
    FLOPPY144_SITE_WORKTOP,
    FLOPPY144_SITE_SINK,
    FLOPPY144_SITE_COFFEE_MAKER,
    FLOPPY144_SITE_SOFA,
    FLOPPY144_SITE_SERVER,
    FLOPPY144_SITE_SHELVING_FULL,
    FLOPPY144_SITE_TROLLEY,
    FLOPPY144_SITE_TABLE,

    FLOPPY144_SITE_ELEMENT_COUNT
}
Floppy144SiteElement;

/*
 * Compact generated-plan rectangle.
 *
 * room is a Floppy144RoomId value for room-owned geometry, or
 * FLOPPY144_SITE_ROOM_SHARED for shared structure/doors/windows.
 * Coordinates and dimensions are expressed in whole Site units. Rotated JSONC
 * placements currently expose their conservative generated bounds here while
 * the source rotation remains preserved in the generated definition.
 */

typedef struct Floppy144SiteRect
{
    uint8_t type;
    uint8_t room;
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
}
Floppy144SiteRect;

uint32_t Floppy144SiteRectCount(
    void
);

const Floppy144SiteRect *Floppy144SiteRectAt(
    uint32_t index
);

bool Floppy144SiteElementBlocksMovement(
    Floppy144SiteElement element
);

bool Floppy144SitePositionBlocked(
    int32_t centre_x16,
    int32_t centre_y16
);

void Floppy144SiteSpawnPosition(
    int32_t *x16,
    int32_t *y16
);

bool Floppy144SiteMovePosition(
    int32_t *x16,
    int32_t *y16,
    int32_t delta_x16,
    int32_t delta_y16
);
