/*
 * Floppy//144 - reconstructed office implementation
 *
 * Defines room geometry, collision, interaction ranges and all procedural office
 * art. World-state flags decide which reconstructed details are visible.
 */

#include "floppy144_office.h"

#include <stdio.h>

#include "floppy144_draw.h"
#include "floppy144_object_registry.h"

#include <stddef.h>

/*
 * Player and screen geometry
 *
 * All dimensions use the 640x360 logical canvas. The Win32 window doubles
 * that canvas without changing collision coordinates.
 */

#define FLOPPY144_PLAYER_WIDTH  18
#define FLOPPY144_PLAYER_HEIGHT 22

enum Floppy144OfficeLayout
{
    FLOPPY144_HEADER_TOP = 0,
    FLOPPY144_HEADER_HEIGHT = 20,

    FLOPPY144_OFFICE_TOP = 20,
    FLOPPY144_OFFICE_BOTTOM = 296,
    FLOPPY144_OFFICE_HEIGHT = 276,

    FLOPPY144_INTERIOR_TOP = 32,
    FLOPPY144_INTERIOR_BOTTOM = 284,

    FLOPPY144_LABEL_TOP = 296,
    FLOPPY144_LABEL_TEXT_Y = 300,
    FLOPPY144_LABEL_HEIGHT = 16,

    FLOPPY144_FOOTER_TOP = 312,
    FLOPPY144_FOOTER_TEXT_Y = 323,
    FLOPPY144_FOOTER_HEIGHT = 32,

    FLOPPY144_BOTTOM_SHIM_TOP = 344,
    FLOPPY144_BOTTOM_SHIM_HEIGHT = 16
};

/*
 * Collision rectangles
 *
 * A rectangle represents the solid area of furniture. The obstacle array order
 * is significant because Desk 01 and Desk 04 proximity checks use indices 1 and 4.
 */

typedef struct Floppy144Rect
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} Floppy144Rect;

/*
 * Solid furniture layout
 *
 * Order: terminal, four desks, two left cabinets, two right cabinets.
 */

static const Floppy144Rect floppy144_obstacles[] =
/* Terminal */
{
    {274, 46, 92, 36},

    /* Desks 01-04 */
    {92, 92, 132, 42},
    {416, 92, 132, 42},
    {92, 212, 132, 42},
    {416, 212, 132, 42},

    /* West-wall filing cabinets */
    {42, 68, 34, 60},
    {42, 140, 34, 60},

    /* East-wall filing cabinets */
    {564, 68, 30, 60},
    {564, 140, 30, 60}
};

/*
 * Axis-aligned rectangle overlap test
 *
 * Used by both collision and interaction proximity. Touching edges are not
 * considered overlap, which lets the player stand immediately beside furniture.
 */

static bool Floppy144RectsOverlap(
    int32_t first_x,
    int32_t first_y,
    int32_t first_width,
    int32_t first_height,
    int32_t second_x,
    int32_t second_y,
    int32_t second_width,
    int32_t second_height
)
{
    return
        first_x < second_x + second_width &&
        first_x + first_width > second_x &&
        first_y < second_y + second_height &&
        first_y + first_height > second_y;
}

/*
 * Test an interaction halo around furniture
 *
 * The margin expands a solid obstacle only for proximity checks. It does not
 * make the physical collision box larger.
 */

static bool Floppy144OfficeNearObstacle(
    const Floppy144Player *player,
    size_t obstacle_index,
    int32_t margin
)
{
    const Floppy144Rect *obstacle =
        &floppy144_obstacles[obstacle_index];

    return Floppy144RectsOverlap(
        player->x,
        player->y,
        FLOPPY144_PLAYER_WIDTH,
        FLOPPY144_PLAYER_HEIGHT,
        obstacle->x - margin,
        obstacle->y - margin,
        obstacle->width + margin * 2,
        obstacle->height + margin * 2
    );
}

/*
 * Validate a proposed player position
 *
 * First keeps the player inside the office interior, then rejects overlap with
 * every solid item in the obstacle table.
 */

static bool Floppy144OfficePositionValid(
    int32_t x,
    int32_t y
)
{
    size_t obstacle_index;

    if(
        x < 32 ||
        y < 32 ||
        x + FLOPPY144_PLAYER_WIDTH > 608 ||
        y + FLOPPY144_PLAYER_HEIGHT > 284
    )
    {
        return false;
    }

    for(
        obstacle_index = 0;
        obstacle_index <
            sizeof(floppy144_obstacles) /
            sizeof(floppy144_obstacles[0]);
        ++obstacle_index
    )
    {
        const Floppy144Rect *obstacle =
            &floppy144_obstacles[obstacle_index];

        if(
            Floppy144RectsOverlap(
                x,
                y,
                FLOPPY144_PLAYER_WIDTH,
                FLOPPY144_PLAYER_HEIGHT,
                obstacle->x,
                obstacle->y,
                obstacle->width,
                obstacle->height
            )
        )
        {
            return false;
        }
    }

    return true;
}

/*
 * Place the player at the office spawn point
 */

void Floppy144OfficeReset(
    Floppy144Player *player
)
{
    player->x = 316;
    player->y = 168;
}

/*
 * Move with sliding collision
 *
 * Horizontal and vertical movement are tested separately. If one axis is blocked,
 * the other can still move, allowing the player to slide around furniture.
 */

void Floppy144OfficeMove(
    Floppy144Player *player,
    int32_t movement_x,
    int32_t movement_y
)
{
    int32_t candidate_x =
        player->x + movement_x;

    int32_t candidate_y =
        player->y + movement_y;

    if(
        Floppy144OfficePositionValid(
            candidate_x,
            player->y
        )
    )
    {
        player->x = candidate_x;
    }

    if(
        Floppy144OfficePositionValid(
            player->x,
            candidate_y
        )
    )
    {
        player->y = candidate_y;
    }
}

/*
 * Terminal interaction zone
 *
 * Uses the player centre and a hand-tuned rectangle in front of the terminal.
 */

bool Floppy144OfficeNearTerminal(
    const Floppy144Player *player
)
{
    int32_t player_centre_x =
        player->x + FLOPPY144_PLAYER_WIDTH / 2;

    int32_t player_centre_y =
        player->y + FLOPPY144_PLAYER_HEIGHT / 2;

    return
        player_centre_x >= 254 &&
        player_centre_x < 386 &&
        player_centre_y >= 80 &&
        player_centre_y < 108;
}

/*
 * Evidence-driven desk interaction zones
 *
 * These use the corresponding collision rectangle plus an eight-pixel halo.
 */

bool Floppy144OfficeNearDeskOne(
    const Floppy144Player *player
)
{
    return Floppy144OfficeNearObstacle(
        player,
        1,
        8
    );
}
bool Floppy144OfficeNearDeskFour(
    const Floppy144Player *player
)
{
    return Floppy144OfficeNearObstacle(
        player,
        4,
        8
    );
}
/*
 * Procedural furniture and player drawing helpers
 *
 * Simple rectangles build desks, cabinets, the archive terminal and the player.
 * No external image files are required.
 */

static void Floppy144OfficeDrawDesk(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    const char *label,
    uint32_t desk_colour,
    uint32_t edge_colour,
    uint32_t detail_colour
)
{
    Floppy144DrawFillRect(
        surface,
        x,
        y,
        132,
        42,
        desk_colour
    );

    Floppy144DrawRect(
        surface,
        x,
        y,
        132,
        42,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        x + 8,
        y + 7,
        28,
        18,
        detail_colour
    );

    Floppy144DrawRect(
        surface,
        x + 8,
        y + 7,
        28,
        18,
        edge_colour
    );

    Floppy144DrawText(
        surface,
        x + 45,
        y + 10,
        label,
        1,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        x + 45,
        y + 27,
        74,
        2,
        detail_colour
    );
}

/*
 * Draw one filing cabinet
 *
 * A loop adds repeated drawer separators and handles.
 */

static void Floppy144OfficeDrawCabinet(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t cabinet_colour,
    uint32_t edge_colour
)
{
    uint32_t drawer_y;

    Floppy144DrawFillRect(
        surface,
        x,
        y,
        width,
        60,
        cabinet_colour
    );

    Floppy144DrawRect(
        surface,
        x,
        y,
        width,
        60,
        edge_colour
    );

    for(drawer_y = y + 15;
        drawer_y < y + 60;
        drawer_y += 15)
    {
        Floppy144DrawFillRect(
            surface,
            x,
            drawer_y,
            width,
            1,
            edge_colour
        );

        Floppy144DrawFillRect(
            surface,
            x + width / 2 - 3,
            drawer_y - 8,
            6,
            2,
            edge_colour
        );
    }
}

/*
 * Draw the archive terminal desk and READY screen
 */

static void Floppy144OfficeDrawTerminal(
    Floppy144Surface *surface,
    uint32_t desk_colour,
    uint32_t edge_colour,
    uint32_t screen_colour,
    uint32_t amber
)
{
    Floppy144DrawFillRect(
        surface,
        274,
        46,
        92,
        36,
        desk_colour
    );

    Floppy144DrawRect(
        surface,
        274,
        46,
        92,
        36,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        292,
        51,
        56,
        20,
        screen_colour
    );

    Floppy144DrawRect(
        surface,
        292,
        51,
        56,
        20,
        edge_colour
    );

    Floppy144DrawText(
        surface,
        299,
        58,
        "READY",
        1,
        amber
    );

    Floppy144DrawFillRect(
        surface,
        308,
        74,
        24,
        3,
        edge_colour
    );
}

/*
 * Resolve an object palette role into an office colour.
 */

static uint32_t Floppy144OfficeObjectColour(
    Floppy144ObjectColourRole role,
    uint32_t body_colour,
    uint32_t edge_colour,
    uint32_t screen_colour,
    uint32_t warning_colour
)
{
    switch(role)
    {
        case FLOPPY144_OBJECT_COLOUR_BODY:
        {
            return body_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_EDGE:
        {
            return edge_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_SCREEN:
        {
            return screen_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_WARNING:
        {
            return warning_colour;
        }
    }

    return edge_colour;
}

/*
 * Draw every visible registered object belonging to the office scene.
 *
 * Geometry and visual primitives come from the object registry. The office
 * supplies only its palette and the current persistent object state.
 */

static void Floppy144OfficeDrawRegisteredObjects(
    Floppy144Surface *surface,
    const Floppy144WorldState *world,
    uint32_t body_colour,
    uint32_t edge_colour,
    uint32_t screen_colour,
    uint32_t warning_colour
)
{
    uint32_t object_index;

    for(
        object_index = 0U;
        object_index <
            (uint32_t)FLOPPY144_OBJECT_COUNT;
        ++object_index
    )
    {
        Floppy144ObjectId object =
            (Floppy144ObjectId)object_index;

        const Floppy144ObjectDefinition *definition =
            Floppy144ObjectGet(object);

        int32_t object_x;
        int32_t object_y;

        uint32_t primitive_index;

        if(
            definition == NULL ||
            definition->scene !=
                FLOPPY144_SCENE_OFFICE ||
            !Floppy144WorldObjectVisible(
                world,
                object
            ) ||
            !Floppy144ObjectWorldPosition(
                object,
                &object_x,
                &object_y
            )
        )
        {
            continue;
        }

        for(
            primitive_index = 0U;
            primitive_index <
                definition->primitive_count;
            ++primitive_index
        )
        {
            const Floppy144ObjectPrimitive *primitive =
                &definition->primitives[primitive_index];

            uint32_t colour =
                Floppy144OfficeObjectColour(
                    primitive->colour_role,
                    body_colour,
                    edge_colour,
                    screen_colour,
                    warning_colour
                );

            switch(primitive->type)
            {
                case FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT:
                {
                    Floppy144DrawFillRect(
                        surface,
                        (uint32_t)(
                            object_x +
                            primitive->x
                        ),
                        (uint32_t)(
                            object_y +
                            primitive->y
                        ),
                        (uint32_t)primitive->width,
                        (uint32_t)primitive->height,
                        colour
                    );

                    break;
                }

                case FLOPPY144_OBJECT_PRIMITIVE_RECT:
                {
                    Floppy144DrawRect(
                        surface,
                        (uint32_t)(
                            object_x +
                            primitive->x
                        ),
                        (uint32_t)(
                            object_y +
                            primitive->y
                        ),
                        (uint32_t)primitive->width,
                        (uint32_t)primitive->height,
                        colour
                    );

                    break;
                }

                case FLOPPY144_OBJECT_PRIMITIVE_TEXT:
                {
                    Floppy144DrawText(
                        surface,
                        (uint32_t)(
                            object_x +
                            primitive->x
                        ),
                        (uint32_t)(
                            object_y +
                            primitive->y
                        ),
                        primitive->text,
                        1,
                        colour
                    );

                    break;
                }
            }
        }
    }
}
/*
 * Draw the player as a compact top-down human figure
 */

static void Floppy144OfficeDrawPlayer(
    Floppy144Surface *surface,
    const Floppy144Player *player,
    uint32_t body_colour,
    uint32_t edge_colour
)
{
    uint32_t player_x =
        (uint32_t)player->x;

    uint32_t player_y =
        (uint32_t)player->y;

    Floppy144DrawFillRect(
        surface,
        player_x + 6,
        player_y,
        6,
        6,
        body_colour
    );

    Floppy144DrawRect(
        surface,
        player_x + 6,
        player_y,
        6,
        6,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        player_x + 4,
        player_y + 5,
        10,
        11,
        body_colour
    );

    Floppy144DrawRect(
        surface,
        player_x + 4,
        player_y + 5,
        10,
        11,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        player_x + 1,
        player_y + 7,
        3,
        8,
        body_colour
    );

    Floppy144DrawFillRect(
        surface,
        player_x + 14,
        player_y + 7,
        3,
        8,
        body_colour
    );

    Floppy144DrawFillRect(
        surface,
        player_x + 4,
        player_y + 15,
        4,
        7,
        body_colour
    );

    Floppy144DrawFillRect(
        surface,
        player_x + 10,
        player_y + 15,
        4,
        7,
        body_colour
    );
}

/*
 * Draw HR-02 restoration details
 *
 * Before HR-02 is restored this function exits immediately. Afterwards it adds
 * the Desk 01 mug and Desk 04 personnel forms referenced by record 038.
 */

static void Floppy144OfficeDrawPersonnelDetails(
    Floppy144Surface *surface,
    const Floppy144WorldState *world,
    uint32_t paper_colour,
    uint32_t mug_colour,
    uint32_t edge_colour
)
{
    if(!Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02))
    {
        return;
    }

    /*
     * Desk 01: mug.
     */

    Floppy144DrawFillRect(
        surface,
        198,
        116,
        12,
        12,
        mug_colour
    );

    Floppy144DrawRect(
        surface,
        198,
        116,
        12,
        12,
        edge_colour
    );

    Floppy144DrawRect(
        surface,
        210,
        119,
        5,
        7,
        mug_colour
    );

    Floppy144DrawFillRect(
        surface,
        200,
        118,
        8,
        2,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        201,
        111,
        2,
        3,
        paper_colour
    );

    Floppy144DrawFillRect(
        surface,
        205,
        109,
        2,
        4,
        paper_colour
    );

    /*
     * Desk 04: personnel forms.
     */

    Floppy144DrawFillRect(
        surface,
        496,
        232,
        30,
        15,
        paper_colour
    );

    Floppy144DrawRect(
        surface,
        496,
        232,
        30,
        15,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        500,
        228,
        30,
        15,
        paper_colour
    );

    Floppy144DrawRect(
        surface,
        500,
        228,
        30,
        15,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        504,
        224,
        30,
        15,
        paper_colour
    );

    Floppy144DrawRect(
        surface,
        504,
        224,
        30,
        15,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        508,
        228,
        20,
        1,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        508,
        232,
        16,
        1,
        edge_colour
    );

    Floppy144DrawFillRect(
        surface,
        508,
        236,
        18,
        1,
        edge_colour
    );
}
/*
 * Draw the complete reconstructed office
 *
 * Derives prompts and labels from world state, paints the room shell and furniture,
 * adds restored evidence details, then draws the player and footer last.
 */

void Floppy144OfficeDraw(
    EngineData *engine,
    const Floppy144Player *player,
    const Floppy144WorldState *world,
    const char *notice
)
{
    /*
     * Office palette
     *
     * Muted institutional colours are defined locally and compiled as constants.
     */

    const uint32_t background =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t floor_colour =
        FLOPPY144_RGB(31, 39, 42);

    const uint32_t floor_line =
        FLOPPY144_RGB(36, 45, 48);

    const uint32_t wall_colour =
        FLOPPY144_RGB(74, 82, 81);

    const uint32_t wall_edge =
        FLOPPY144_RGB(113, 124, 120);

    const uint32_t desk_colour =
        FLOPPY144_RGB(76, 65, 51);

    const uint32_t desk_detail =
        FLOPPY144_RGB(42, 47, 47);

    const uint32_t cabinet_colour =
        FLOPPY144_RGB(62, 72, 73);

    const uint32_t text_colour =
        FLOPPY144_RGB(201, 210, 203);

    const uint32_t muted_colour =
        FLOPPY144_RGB(116, 132, 130);

    const uint32_t green =
        FLOPPY144_RGB(100, 156, 111);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    const uint32_t player_colour =
        FLOPPY144_RGB(177, 190, 181);

    /*
     * Context-sensitive interface state
     *
     * Evidence must be read before desk inspection prompts appear. A temporary notice
     * replaces the normal room label until the player moves again.
     */

    bool near_evidence_desk =
        Floppy144WorldCollectionEvidenceFound(world, FLOPPY144_COLLECTION_HR02) &&
        (
            Floppy144OfficeNearDeskOne(player) ||
            Floppy144OfficeNearDeskFour(player)
        );

    const char *prompt =
        Floppy144OfficeNearTerminal(player)
            ? Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
                ? "PRESS E TO REVIEW RESTORED COLLECTIONS"
                : "PRESS E TO ACCESS ARCHIVE TERMINAL"
            : near_evidence_desk
                ? "PRESS E TO INSPECT RECONSTRUCTED DESK"
                : "WASD OR ARROWS TO MOVE";

    char status_text[16];

    snprintf(
        status_text,
        sizeof(status_text),
        "STATUS %02u%%",
        (unsigned)Floppy144WorldReconstructionPercent(world)
    );

    const char *desk_one_label =
        Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
            ? "SENIOR ARCHIVIST"
            : "DESK 01";

    const char *desk_two_label =
        Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
            ? "RECORDS OFFICER"
            : "DESK 02";

    const char *desk_three_label =
        Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
            ? "ADMINISTRATOR"
            : "DESK 03";

    const char *desk_four_label =
        Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
            ? "IT SUPPORT"
            : "DESK 04";

    const char *default_room_label =
        Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
            ? "RECORDS OFFICE - PERSONNEL RECORDS RESTORED"
            : "RECORDS OFFICE - PARTIAL RECONSTRUCTION";

    const char *room_label =
        notice
            ? notice
            : default_room_label;

    uint32_t grid_x;
    uint32_t grid_y;

    /*
     * Room paint order
     *
     * Draw from back to front: background, floor grid, walls, fixtures, furniture,
     * restored details, player and interface footer.
     */

    Floppy144Surface surface =
    {
        (uint32_t *)engine->backbuffer.data,
        engine->backbuffer.width,
        engine->backbuffer.height
    };

    Floppy144DrawClear(
        &surface,
        background
    );

    Floppy144DrawFillRect(
        &surface,
        0,
        0,
        640,
        16,
        background
    );

    Floppy144DrawText(
        &surface,
        10,
        5,
        "GDR SITE RECONSTRUCTION",
        1,
        muted_colour
    );

    Floppy144DrawText(
        &surface,
        526,
        5,
        status_text,
        1,
        green
    );

    Floppy144DrawFillRect(
        &surface,
        32,
        32,
        576,
        252,
        floor_colour
    );

    /* Draw a subtle floor grid to make the room shape and movement easier to read. */
    for(grid_x = 32;
        grid_x < 608;
        grid_x += 24)
    {
        Floppy144DrawFillRect(
            &surface,
            grid_x,
            32,
            1,
            260,
            floor_line
        );
    }

    for(grid_y = 32;
        grid_y < 284;
        grid_y += 24)
    {
        Floppy144DrawFillRect(
            &surface,
            32,
            grid_y,
            576,
            1,
            floor_line
        );
    }

    Floppy144DrawFillRect(
        &surface,
        20,
        20,
        600,
        12,
        wall_colour
    );

    Floppy144DrawFillRect(
        &surface,
        20,
        284,
        600,
        12,
        wall_colour
    );

    Floppy144DrawFillRect(
        &surface,
        20,
        20,
        12,
        276,
        wall_colour
    );

    Floppy144DrawFillRect(
        &surface,
        608,
        20,
        12,
        276,
        wall_colour
    );

    Floppy144DrawRect(
        &surface,
        20,
        20,
        600,
        276,
        wall_edge
    );

    Floppy144DrawFillRect(
        &surface,
        500,
        20,
        72,
        12,
        background
    );

    Floppy144DrawRect(
        &surface,
        500,
        20,
        72,
        12,
        amber
    );

    Floppy144DrawText(
        &surface,
        506,
        23,
        "SEALED EXIT",
        1,
        amber
    );

    /* Place permanent fixtures and furniture at the same coordinates as collision. */
    Floppy144OfficeDrawTerminal(
        &surface,
        desk_colour,
        wall_edge,
        background,
        amber
    );
    Floppy144OfficeDrawRegisteredObjects(
        &surface,
        world,
        cabinet_colour,
        wall_edge,
        background,
        amber
    );

    Floppy144DrawText(
        &surface,
        286,
        36,
        "ARCHIVE TERMINAL",
        1,
        muted_colour
    );

    Floppy144OfficeDrawDesk(
        &surface,
        92,
        92,
        desk_one_label,
        desk_colour,
        wall_edge,
        desk_detail
    );

    Floppy144OfficeDrawDesk(
        &surface,
        416,
        92,
        desk_two_label,
        desk_colour,
        wall_edge,
        desk_detail
    );

    Floppy144OfficeDrawDesk(
        &surface,
        92,
        212,
        desk_three_label,
        desk_colour,
        wall_edge,
        desk_detail
    );

    Floppy144OfficeDrawDesk(
        &surface,
        416,
        212,
        desk_four_label,
        desk_colour,
        wall_edge,
        desk_detail
    );

    Floppy144OfficeDrawCabinet(
        &surface,
        42,
        68,
        34,
        cabinet_colour,
        wall_edge
    );

    Floppy144OfficeDrawCabinet(
        &surface,
        42,
        140,
        34,
        cabinet_colour,
        wall_edge
    );

    Floppy144OfficeDrawCabinet(
        &surface,
        564,
        68,
        30,
        cabinet_colour,
        wall_edge
    );

    Floppy144OfficeDrawCabinet(
        &surface,
        564,
        140,
        30,
        cabinet_colour,
        wall_edge
    );

    Floppy144DrawText(
        &surface,
        36,
        300,
        room_label,
        1,
        muted_colour
    );

    /* Collection-specific overlays are drawn after the base furniture. */
    Floppy144OfficeDrawPersonnelDetails(
        &surface,
        world,
        text_colour,
        amber,
        background
    );

    /* Draw the player above room objects and evidence overlays. */
    Floppy144OfficeDrawPlayer(
        &surface,
        player,
        player_colour,
        background
    );

    Floppy144DrawFillRect(
        &surface,
        20,
        312,
        600,
        28,
        background
    );

    Floppy144DrawRect(
        &surface,
        20,
        312,
        600,
        28,
        wall_edge
    );

    /* Footer shows the action available at the player's current position. */
    Floppy144DrawText(
        &surface,
        32,
        322,
        prompt,
        1,
        (
            Floppy144OfficeNearTerminal(player) ||
            near_evidence_desk
        )
            ? amber
            : text_colour
    );

    Floppy144DrawText(
        &surface,
        526,
        322,
        "ESC RECOVERY",
        1,
        muted_colour
    );
}

