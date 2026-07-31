/*
 * Floppy//144 - immutable scene-object registry implementation
 */

#include "floppy144_object_registry.h"

#include <stddef.h>

#define FLOPPY144_ARRAY_COUNT(values)                              \
    ((uint32_t)(sizeof(values) / sizeof((values)[0])))

/*
 * Suppression control panel visual recipe
 *
 * Every coordinate is relative to the object's registered position.
 */

static const Floppy144ObjectPrimitive
    floppy144_suppression_control_panel_primitives[] =
{
    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 0, 34, 34,
        FLOPPY144_OBJECT_COLOUR_BODY,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        0, 0, 34, 34,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        5, 5, 24, 11,
        FLOPPY144_OBJECT_COLOUR_SCREEN,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        5, 5, 24, 11,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_TEXT,
        6, 8, 0, 0,
        FLOPPY144_OBJECT_COLOUR_WARNING,
        "HALON"
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        6, 21, 5, 5,
        FLOPPY144_OBJECT_COLOUR_WARNING,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        15, 21, 13, 5,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        3, 34, 2, 8,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    }
};

/*
 * Generate immutable object metadata from the master declaration file.
 */

static const Floppy144ObjectDefinition
    floppy144_object_definitions[FLOPPY144_OBJECT_COUNT] =
{
#define FLOPPY144_OBJECT(                                          \
    symbol,                                                        \
    scene_value,                                                   \
    parent_value,                                                  \
    local_x_value,                                                 \
    local_y_value,                                                 \
    collision_x_value,                                             \
    collision_y_value,                                             \
    collision_width_value,                                         \
    collision_height_value,                                        \
    flags_value,                                                   \
    initially_visible_value,                                       \
    primitive_values                                               \
)                                                                  \
    {                                                              \
        FLOPPY144_OBJECT_##symbol,                                 \
        scene_value,                                               \
        parent_value,                                              \
        local_x_value,                                             \
        local_y_value,                                             \
        collision_x_value,                                         \
        collision_y_value,                                         \
        collision_width_value,                                     \
        collision_height_value,                                    \
        flags_value,                                               \
        initially_visible_value,                                   \
        primitive_values,                                          \
        FLOPPY144_ARRAY_COUNT(primitive_values)                    \
    },

#include "floppy144_objects.def"

#undef FLOPPY144_OBJECT
};

/*
 * Retrieve immutable object metadata.
 */

const Floppy144ObjectDefinition *Floppy144ObjectGet(
    Floppy144ObjectId object
)
{
    if(
        object < 0 ||
        (uint32_t)object >=
            (uint32_t)FLOPPY144_OBJECT_COUNT
    )
    {
        return NULL;
    }

    return
        &floppy144_object_definitions[object];
}

/*
 * Resolve parent-relative coordinates.
 *
 * The depth limit prevents malformed data from creating an infinite loop.
 */

bool Floppy144ObjectWorldPosition(
    Floppy144ObjectId object,
    int32_t *world_x,
    int32_t *world_y
)
{
    Floppy144ObjectId current =
        object;

    uint32_t depth =
        0U;

    if(
        world_x == NULL ||
        world_y == NULL
    )
    {
        return false;
    }

    *world_x =
        0;

    *world_y =
        0;

    while(current != FLOPPY144_OBJECT_NONE)
    {
        const Floppy144ObjectDefinition *definition;

        if(
            depth >=
                (uint32_t)FLOPPY144_OBJECT_COUNT
        )
        {
            return false;
        }

        definition =
            Floppy144ObjectGet(current);

        if(definition == NULL)
        {
            return false;
        }

        *world_x +=
            definition->local_x;

        *world_y +=
            definition->local_y;

        current =
            definition->parent;

        ++depth;
    }

    return true;
}
