#include "floppy144_office.h"

#include "floppy144_draw.h"

#include <stddef.h>

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

typedef struct Floppy144Rect
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} Floppy144Rect;

static const Floppy144Rect floppy144_obstacles[] =
{
    {274, 46, 92, 36},

    {92, 92, 132, 42},
    {416, 92, 132, 42},
    {92, 212, 132, 42},
    {416, 212, 132, 42},

    {42, 68, 34, 60},
    {42, 140, 34, 60},

    {564, 68, 30, 60},
    {564, 140, 30, 60}
};

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

void Floppy144OfficeReset(
    Floppy144Player *player
)
{
    player->x = 316;
    player->y = 168;
}

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

static void Floppy144OfficeDrawPersonnelDetails(
    Floppy144Surface *surface,
    const Floppy144WorldState *world,
    uint32_t paper_colour,
    uint32_t mug_colour,
    uint32_t edge_colour
)
{
    if(!world->hr02_restored)
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
void Floppy144OfficeDraw(
    EngineData *engine,
    const Floppy144Player *player,
    const Floppy144WorldState *world
)
{
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

    const char *prompt =
        Floppy144OfficeNearTerminal(player)
            ? world->hr02_restored
                ? "PRESS E TO REVIEW RESTORED COLLECTIONS"
                : "PRESS E TO ACCESS ARCHIVE TERMINAL"
            : "WASD OR ARROWS TO MOVE";

    const char *status_text =
        world->hr02_restored
            ? "STATUS 12%"
            : "STATUS 04%";

    const char *desk_one_label =
        world->hr02_restored
            ? "SENIOR ARCHIVIST"
            : "DESK 01";

    const char *desk_two_label =
        world->hr02_restored
            ? "RECORDS OFFICER"
            : "DESK 02";

    const char *desk_three_label =
        world->hr02_restored
            ? "ADMINISTRATOR"
            : "DESK 03";

    const char *desk_four_label =
        world->hr02_restored
            ? "IT SUPPORT"
            : "DESK 04";

    const char *room_label =
        world->hr02_restored
            ? "RECORDS OFFICE - PERSONNEL RECORDS RESTORED"
            : "RECORDS OFFICE - PARTIAL RECONSTRUCTION";

    uint32_t grid_x;
    uint32_t grid_y;

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

    Floppy144OfficeDrawTerminal(
        &surface,
        desk_colour,
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

    Floppy144OfficeDrawPersonnelDetails(
        &surface,
        world,
        text_colour,
        amber,
        background
    );

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

    Floppy144DrawText(
        &surface,
        32,
        322,
        prompt,
        1,
        Floppy144OfficeNearTerminal(player)
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







