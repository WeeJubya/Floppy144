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
 * Test visible solid objects from the reconstructed-object registry
 *
 * Rendering and collision consume the same object definition. Hidden objects
 * do not block movement. Parent-relative coordinates are resolved by the
 * object registry before collision bounds are applied.
 */

static bool Floppy144OfficeRegisteredObjectBlocks(
    const Floppy144WorldState *world,
    int32_t x,
    int32_t y
)
{
    uint32_t object_index;

    if(world == NULL)
    {
        return false;
    }

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

        if(
            definition == NULL ||
            definition->scene !=
                FLOPPY144_SCENE_OFFICE ||
            (
                definition->flags &
                FLOPPY144_OBJECT_FLAG_SOLID
            ) == 0U ||
            !Floppy144WorldObjectEffectivelyVisible(
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

        if(
            definition->collision_width <= 0 ||
            definition->collision_height <= 0
        )
        {
            continue;
        }

        if(
            Floppy144RectsOverlap(
                x,
                y,
                FLOPPY144_PLAYER_WIDTH,
                FLOPPY144_PLAYER_HEIGHT,
                object_x +
                    definition->collision_x,
                object_y +
                    definition->collision_y,
                definition->collision_width,
                definition->collision_height
            )
        )
        {
            return true;
        }
    }

    return false;
}

/*
 * Validate a proposed player position
 *
 * First keeps the player inside the office interior, then rejects overlap with
 * permanent furniture and visible registered objects marked as solid.
 */

static bool Floppy144OfficePositionValid(
    const Floppy144WorldState *world,
    int32_t x,
    int32_t y
)
{
    if(
        x < 32 ||
        y < 32 ||
        x + FLOPPY144_PLAYER_WIDTH > 608 ||
        y + FLOPPY144_PLAYER_HEIGHT > 284
    )
    {
        return false;
    }

    return
        !Floppy144OfficeRegisteredObjectBlocks(
            world,
            x,
            y
        );
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
 * Horizontal and vertical movement are tested separately. Permanent furniture
 * and currently visible solid reconstructed objects share the same validation
 * path.
 */

void Floppy144OfficeMove(
    Floppy144Player *player,
    const Floppy144WorldState *world,
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
            world,
            candidate_x,
            player->y
        )
    )
    {
        player->x = candidate_x;
    }

    if(
        Floppy144OfficePositionValid(
            world,
            player->x,
            candidate_y
        )
    )
    {
        player->y = candidate_y;
    }
}
/*
 * Test whether the player is inside one registered object's interaction zone
 *
 * The player centre is tested against the object's parent-resolved interaction
 * rectangle. Hidden and non-interactable objects are ignored.
 */

static bool Floppy144OfficeNearObject(
    const Floppy144WorldState *world,
    const Floppy144Player *player,
    Floppy144ObjectId object
)
{
    const Floppy144ObjectDefinition *definition =
        Floppy144ObjectGet(object);

    int32_t object_x;
    int32_t object_y;

    int32_t player_centre_x;
    int32_t player_centre_y;

    if(
        player == NULL ||
        definition == NULL ||
        definition->scene !=
            FLOPPY144_SCENE_OFFICE ||
        definition->interaction == NULL ||
        definition->interaction_width <= 0 ||
        definition->interaction_height <= 0 ||
        !Floppy144WorldObjectEffectivelyVisible(
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
        return false;
    }

    player_centre_x =
        player->x +
        FLOPPY144_PLAYER_WIDTH / 2;

    player_centre_y =
        player->y +
        FLOPPY144_PLAYER_HEIGHT / 2;

    return
        player_centre_x >=
            object_x +
            definition->interaction_x &&
        player_centre_x <
            object_x +
            definition->interaction_x +
            definition->interaction_width &&
        player_centre_y >=
            object_y +
            definition->interaction_y &&
        player_centre_y <
            object_y +
            definition->interaction_y +
            definition->interaction_height;
}
/*
 * Find the highest-priority eligible interaction at the player's position.
 */

Floppy144ObjectId Floppy144OfficeInteractionTarget(
    const Floppy144WorldState *world,
    const Floppy144Player *player
)
{
    Floppy144ObjectId best_object =
        FLOPPY144_OBJECT_NONE;

    uint32_t best_priority =
        0U;

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

        const Floppy144ObjectInteractionDefinition *interaction;

        if(
            definition == NULL ||
            definition->interaction == NULL
        )
        {
            continue;
        }

        interaction =
            definition->interaction;

        if(
            interaction->required_evidence_collection !=
                FLOPPY144_COLLECTION_COUNT &&
            !Floppy144WorldCollectionEvidenceFound(
                world,
                interaction->required_evidence_collection
            )
        )
        {
            continue;
        }

        if(
            !Floppy144OfficeNearObject(
                world,
                player,
                object
            )
        )
        {
            continue;
        }

        if(
            best_object == FLOPPY144_OBJECT_NONE ||
            interaction->priority > best_priority
        )
        {
            best_object =
                object;

            best_priority =
                interaction->priority;
        }
    }

    return best_object;
}
/*
 * Resolve an object palette role into an office colour.
 */

static uint32_t Floppy144OfficeObjectColour(
    Floppy144ObjectColourRole role,
    uint32_t body_colour,
    uint32_t furniture_colour,
    uint32_t detail_colour,
    uint32_t paper_colour,
    uint32_t edge_colour,
    uint32_t screen_colour,
    uint32_t background_colour,
    uint32_t warning_colour,
    uint32_t label_colour
)
{
    switch(role)
    {
        case FLOPPY144_OBJECT_COLOUR_BODY:
        {
            return body_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_FURNITURE:
        {
            return furniture_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_DETAIL:
        {
            return detail_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_PAPER:
        {
            return paper_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_EDGE:
        {
            return edge_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_SCREEN:
        {
            return screen_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_BACKGROUND:
        {
            return background_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_WARNING:
        {
            return warning_colour;
        }

        case FLOPPY144_OBJECT_COLOUR_LABEL:
        {
            return label_colour;
        }
    }

    return edge_colour;
}

/*
 * Draw every effectively visible registered object in scene-layer order.
 */

static void Floppy144OfficeDrawRegisteredObjects(
    Floppy144Surface *surface,
    const Floppy144WorldState *world,
    uint32_t body_colour,
    uint32_t furniture_colour,
    uint32_t detail_colour,
    uint32_t paper_colour,
    uint32_t edge_colour,
    uint32_t screen_colour,
    uint32_t background_colour,
    uint32_t warning_colour,
    uint32_t label_colour
)
{
    int32_t draw_layer;

    for(
        draw_layer = 0;
        draw_layer <=
            FLOPPY144_OBJECT_LAYER_MAX;
        ++draw_layer
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
                definition->draw_layer !=
                    draw_layer ||
                !Floppy144WorldObjectEffectivelyVisible(
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
                        furniture_colour,
                        detail_colour,
                        paper_colour,
                        edge_colour,
                        screen_colour,
                        background_colour,
                        warning_colour,
                        label_colour
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

            if(
                definition->label != NULL &&
                definition->label->default_text != NULL
            )
            {
                const char *label_text =
                    definition->label->default_text;

                uint32_t label_colour_value;

                if(
                    definition->label->restored_text != NULL &&
                    Floppy144WorldCollectionRestored(
                        world,
                        definition->label->restored_collection
                    )
                )
                {
                    label_text =
                        definition->label->restored_text;
                }

                label_colour_value =
                    Floppy144OfficeObjectColour(
                        definition->label->colour_role,
                        body_colour,
                        furniture_colour,
                        detail_colour,
                        paper_colour,
                        edge_colour,
                        screen_colour,
                        background_colour,
                        warning_colour,
                        label_colour
                    );

                Floppy144DrawText(
                    surface,
                    (uint32_t)(
                        object_x +
                        definition->label->x
                    ),
                    (uint32_t)(
                        object_y +
                        definition->label->y
                    ),
                    label_text,
                    1,
                    label_colour_value
                );
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

    Floppy144ObjectId interaction_object =
        Floppy144OfficeInteractionTarget(
            world,
            player
        );

    const Floppy144ObjectDefinition *interaction_definition =
        Floppy144ObjectGet(
            interaction_object
        );

    const Floppy144ObjectInteractionDefinition *interaction =
        interaction_definition != NULL
            ? interaction_definition->interaction
            : NULL;

    const char *prompt =
        "WASD OR ARROWS TO MOVE";

    if(
        interaction != NULL &&
        interaction->prompt != NULL
    )
    {
        prompt =
            interaction->prompt;

        if(
            interaction->alternate_prompt != NULL &&
            interaction->alternate_prompt_collection !=
                FLOPPY144_COLLECTION_COUNT &&
            Floppy144WorldCollectionRestored(
                world,
                interaction->alternate_prompt_collection
            )
        )
        {
            prompt =
                interaction->alternate_prompt;
        }
    }
    char status_text[16];

    snprintf(
        status_text,
        sizeof(status_text),
        "STATUS %02u%%",
        (unsigned)Floppy144WorldReconstructionPercent(world)
    );

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
    Floppy144OfficeDrawRegisteredObjects(
        &surface,
        world,
        cabinet_colour,
        desk_colour,
        desk_detail,
        text_colour,
        wall_edge,
        background,
        background,
        amber,
        muted_colour
    );
    Floppy144DrawText(
        &surface,
        36,
        300,
        room_label,
        1,
        muted_colour
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
        interaction != NULL
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

