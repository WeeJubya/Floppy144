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
 * separate.
 *
 * The standing character is 3.5 units wide, but the movement footprint is
 * intentionally narrower than the sprite. A 2 x 2 unit ground footprint lets
 * the player pass through authored office clearances without the shoulders of
 * the visual sprite behaving like solid geometry. The shadow uses the same
 * collision width so the on-screen footprint matches movement clearance.
 */
#define FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16       56
#define FLOPPY144_SITE_PLAYER_HEIGHT_UNITS            8

#define FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16    32
#define FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16    32

#define FLOPPY144_SITE_MOVE_STEP_X16              8

/*
 * Boundary endpoints may reference OUTSIDE. Runtime rectangles themselves are
 * always owned by a real Site room; anonymous shared geometry is no longer
 * emitted by the Site compiler.
 *
 * FLOPPY144_SITE_ROOM_SHARED remains only as a legacy compatibility sentinel
 * for stale generated definitions and must not appear in newly generated data.
 */
#define FLOPPY144_SITE_ROOM_SHARED              0xfeU
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
 * room is the owning room. Ordinary room geometry has from_room == to_room ==
 * room. Boundary geometry such as a door or window retains its explicit
 * endpoints so renderers can decide whether that boundary belongs in the
 * current reconstruction without spatial guesswork or visibility halos.
 *
 * Coordinates and dimensions are expressed in whole Site units. rotation is
 * the authored clockwise rotation in degrees, normalised to 0..359. Rotated
 * placements still expose conservative generated bounds for collision, while
 * renderers may use rotation to orient directional furniture detail.
 */

typedef struct Floppy144SiteRect
{
    uint8_t type;
    uint8_t room;
    uint8_t from_room;
    uint8_t to_room;
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint16_t rotation;
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

/*
 * Optional collision filter.
 *
 * The Site model owns geometry and footprint maths, while RunState decides
 * whether progression-controlled geometry currently exists. Returning false
 * from the callback removes that blocking rectangle from the collision pass.
 */
typedef bool (*Floppy144SiteCollisionFilter)(
    const Floppy144SiteRect *rect,
    void *context
);

bool Floppy144SitePositionBlockedFiltered(
    int32_t centre_x16,
    int32_t centre_y16,
    Floppy144SiteCollisionFilter filter,
    void *context
);

bool Floppy144SitePositionBlocked(
    int32_t centre_x16,
    int32_t centre_y16
);

void Floppy144SiteSpawnPosition(
    int32_t *x16,
    int32_t *y16
);

/*
 * Return the exterior door crossed by one attempted movement step.
 *
 * This is a geometry-only query. It does not decide whether the door is
 * unlocked; progression state remains the responsibility of RunState.
 * NULL means the movement does not cross the Site boundary through a door.
 */
const Floppy144SiteRect *Floppy144SiteExteriorDoorForMove(
    int32_t x16,
    int32_t y16,
    int32_t delta_x16,
    int32_t delta_y16
);

bool Floppy144SiteMovePositionFiltered(
    int32_t *x16,
    int32_t *y16,
    int32_t delta_x16,
    int32_t delta_y16,
    Floppy144SiteCollisionFilter filter,
    void *context
);

bool Floppy144SiteMovePosition(
    int32_t *x16,
    int32_t *y16,
    int32_t delta_x16,
    int32_t delta_y16
);
