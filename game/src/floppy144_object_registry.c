/*
 * Floppy//144 - immutable scene-object registry implementation
 */

#include "floppy144_object_registry.h"

#include <stddef.h>

#define FLOPPY144_ARRAY_COUNT(values)                              \
    ((uint32_t)(sizeof(values) / sizeof((values)[0])))

/*
 * Object interactions
 *
 * Priority resolves overlapping interaction zones. Eligibility requirements,
 * prompts and resulting notices live beside the objects that use them.
 */

static const Floppy144ObjectInteractionDefinition
    floppy144_archive_terminal_interaction =
{
    FLOPPY144_OBJECT_ACTION_OPEN_TERMINAL,
    100U,
    FLOPPY144_COLLECTION_COUNT,
    "PRESS E TO ACCESS ARCHIVE TERMINAL",
    FLOPPY144_COLLECTION_HR02,
    "PRESS E TO REVIEW RESTORED COLLECTIONS",
    NULL
};

static const Floppy144ObjectInteractionDefinition
    floppy144_desk_one_interaction =
{
    FLOPPY144_OBJECT_ACTION_SHOW_NOTICE,
    50U,
    FLOPPY144_COLLECTION_HR02,
    "PRESS E TO INSPECT RECONSTRUCTED DESK",
    FLOPPY144_COLLECTION_COUNT,
    NULL,
    "DESK 01: MUG IS NOT AN ARCHIVE ITEM. FORM AR-7 NOT REQUIRED."
};

static const Floppy144ObjectInteractionDefinition
    floppy144_desk_four_interaction =
{
    FLOPPY144_OBJECT_ACTION_SHOW_NOTICE,
    50U,
    FLOPPY144_COLLECTION_HR02,
    "PRESS E TO INSPECT RECONSTRUCTED DESK",
    FLOPPY144_COLLECTION_COUNT,
    NULL,
    "DESK 04: IT SUPPORT MOVED HERE PENDING TERMINAL CABLE REPLACEMENT."
};
/*
 * Desk 04 personnel-form investigation
 *
 * Reading the HR-02 memorandum reveals these forms. Inspecting them confirms
 * the collection evidence through the same generic effect system used by
 * authored documents.
 */

static const Floppy144Effect
    floppy144_personnel_forms_effects[] =
{
    {
        FLOPPY144_EFFECT_FIND_COLLECTION_EVIDENCE,
        (uint32_t)FLOPPY144_COLLECTION_HR02
    }
};

static const Floppy144ObjectInteractionDefinition
    floppy144_personnel_forms_interaction =
{
    FLOPPY144_OBJECT_ACTION_SHOW_NOTICE,
    100U,
    FLOPPY144_COLLECTION_COUNT,
    "PRESS E TO INSPECT PERSONNEL FORMS",
    FLOPPY144_COLLECTION_COUNT,
    NULL,
    "PERSONNEL FORMS CONFIRM DESK 04 WAS REALLOCATED TO IT SUPPORT.",
    floppy144_personnel_forms_effects,
    1U
};
/*
 * FA-03 suppression-panel investigation
 *
 * The recovered service note reveals this fixture. Physical inspection then
 * confirms the collection evidence through the generic interaction effects.
 */

static const Floppy144Effect
    floppy144_suppression_panel_effects[] =
{
    {
        FLOPPY144_EFFECT_FIND_COLLECTION_EVIDENCE,
        (uint32_t)FLOPPY144_COLLECTION_FA03
    }
};

static const Floppy144ObjectInteractionDefinition
    floppy144_suppression_panel_interaction =
{
    FLOPPY144_OBJECT_ACTION_SHOW_NOTICE,
    100U,
    FLOPPY144_COLLECTION_COUNT,
    "PRESS E TO INSPECT SUPPRESSION PANEL",
    FLOPPY144_COLLECTION_COUNT,
    NULL,
    "SUPPRESSION PANEL: MANUAL DISCHARGE INPUT REMAINS CONNECTED.",
    floppy144_suppression_panel_effects,
    1U
};
/*
 * Object-attached labels
 */

static const Floppy144ObjectLabelDefinition
    floppy144_archive_terminal_label =
{
    12,
    -10,
    FLOPPY144_OBJECT_COLOUR_LABEL,
    "ARCHIVE TERMINAL",
    FLOPPY144_COLLECTION_COUNT,
    NULL
};

static const Floppy144ObjectLabelDefinition
    floppy144_desk_one_label =
{
    45,
    10,
    FLOPPY144_OBJECT_COLOUR_EDGE,
    "DESK 01",
    FLOPPY144_COLLECTION_HR02,
    "SENIOR ARCHIVIST"
};

static const Floppy144ObjectLabelDefinition
    floppy144_desk_two_label =
{
    45,
    10,
    FLOPPY144_OBJECT_COLOUR_EDGE,
    "DESK 02",
    FLOPPY144_COLLECTION_HR02,
    "RECORDS OFFICER"
};

static const Floppy144ObjectLabelDefinition
    floppy144_desk_three_label =
{
    45,
    10,
    FLOPPY144_OBJECT_COLOUR_EDGE,
    "DESK 03",
    FLOPPY144_COLLECTION_HR02,
    "ADMINISTRATOR"
};

static const Floppy144ObjectLabelDefinition
    floppy144_desk_four_label =
{
    45,
    10,
    FLOPPY144_OBJECT_COLOUR_EDGE,
    "DESK 04",
    FLOPPY144_COLLECTION_HR02,
    "IT SUPPORT"
};

/*
 * Archive terminal visual recipe
 */

static const Floppy144ObjectPrimitive
    floppy144_archive_terminal_primitives[] =
{
    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 0, 92, 36,
        FLOPPY144_OBJECT_COLOUR_FURNITURE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        0, 0, 92, 36,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        18, 5, 56, 20,
        FLOPPY144_OBJECT_COLOUR_SCREEN,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        18, 5, 56, 20,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_TEXT,
        25, 12, 0, 0,
        FLOPPY144_OBJECT_COLOUR_WARNING,
        "READY"
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        34, 28, 24, 3,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    }
};

/*
 * Shared desk visual recipe
 */

static const Floppy144ObjectPrimitive
    floppy144_desk_primitives[] =
{
    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 0, 132, 42,
        FLOPPY144_OBJECT_COLOUR_FURNITURE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        0, 0, 132, 42,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        8, 7, 28, 18,
        FLOPPY144_OBJECT_COLOUR_DETAIL,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        8, 7, 28, 18,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        45, 27, 74, 2,
        FLOPPY144_OBJECT_COLOUR_DETAIL,
        NULL
    }
};

/*
 * Desk 01 mug
 *
 * The object origin is the highest point of the steam. All coordinates are
 * relative to Desk 01 through the parent relationship.
 */

static const Floppy144ObjectPrimitive
    floppy144_desk_one_mug_primitives[] =
{
    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 7, 12, 12,
        FLOPPY144_OBJECT_COLOUR_WARNING,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        0, 7, 12, 12,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        12, 10, 5, 7,
        FLOPPY144_OBJECT_COLOUR_WARNING,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        2, 9, 8, 2,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        3, 2, 2, 3,
        FLOPPY144_OBJECT_COLOUR_PAPER,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        7, 0, 2, 4,
        FLOPPY144_OBJECT_COLOUR_PAPER,
        NULL
    }
};

/*
 * Desk 04 personnel forms
 *
 * Three overlapping sheets and their recovered text markings are positioned
 * relative to Desk 04.
 */

static const Floppy144ObjectPrimitive
    floppy144_desk_four_personnel_forms_primitives[] =
{
    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 8, 30, 15,
        FLOPPY144_OBJECT_COLOUR_PAPER,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        0, 8, 30, 15,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        4, 4, 30, 15,
        FLOPPY144_OBJECT_COLOUR_PAPER,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        4, 4, 30, 15,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        8, 0, 30, 15,
        FLOPPY144_OBJECT_COLOUR_PAPER,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        8, 0, 30, 15,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        12, 4, 20, 1,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        12, 8, 16, 1,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        12, 12, 18, 1,
        FLOPPY144_OBJECT_COLOUR_BACKGROUND,
        NULL
    }
};
/*
 * Filing cabinet visual recipes
 *
 * The west and east cabinets have slightly different widths, so each side
 * shares one immutable recipe.
 */

static const Floppy144ObjectPrimitive
    floppy144_west_cabinet_primitives[] =
{
    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 0, 34, 60,
        FLOPPY144_OBJECT_COLOUR_BODY,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        0, 0, 34, 60,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 15, 34, 1,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 30, 34, 1,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 45, 34, 1,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        14, 7, 6, 2,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        14, 22, 6, 2,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        14, 37, 6, 2,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    }
};

static const Floppy144ObjectPrimitive
    floppy144_east_cabinet_primitives[] =
{
    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 0, 30, 60,
        FLOPPY144_OBJECT_COLOUR_BODY,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_RECT,
        0, 0, 30, 60,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 15, 30, 1,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 30, 30, 1,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        0, 45, 30, 1,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        12, 7, 6, 2,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        12, 22, 6, 2,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    },

    {
        FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT,
        12, 37, 6, 2,
        FLOPPY144_OBJECT_COLOUR_EDGE,
        NULL
    }
};
/*
 * Suppression control panel visual recipe
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

static const Floppy144ObjectDefinition
    floppy144_object_definitions[FLOPPY144_OBJECT_COUNT] =
{
#define FLOPPY144_OBJECT(                                          \
    symbol,                                                        \
    name_value,                                                    \
    scene_value,                                                   \
    parent_value,                                                  \
    local_x_value,                                                 \
    local_y_value,                                                 \
    draw_layer_value,                                              \
    collision_x_value,                                             \
    collision_y_value,                                             \
    collision_width_value,                                         \
    collision_height_value,                                        \
    interaction_x_value,                                           \
    interaction_y_value,                                           \
    interaction_width_value,                                       \
    interaction_height_value,                                      \
    required_collection_value,                                     \
    flags_value,                                                   \
    initially_visible_value,                                       \
    label_value,                                                   \
    interaction_value,                                             \
    primitive_values                                               \
)                                                                  \
    {                                                              \
        FLOPPY144_OBJECT_##symbol,                                 \
        name_value,                                                \
        scene_value,                                               \
        parent_value,                                              \
        local_x_value,                                             \
        local_y_value,                                             \
        draw_layer_value,                                          \
        collision_x_value,                                         \
        collision_y_value,                                         \
        collision_width_value,                                     \
        collision_height_value,                                    \
        interaction_x_value,                                       \
        interaction_y_value,                                       \
        interaction_width_value,                                   \
        interaction_height_value,                                  \
        required_collection_value,                                 \
        flags_value,                                               \
        initially_visible_value,                                   \
        label_value,                                               \
        interaction_value,                                         \
        primitive_values,                                          \
        FLOPPY144_ARRAY_COUNT(primitive_values)                    \
    },

#include "floppy144_objects.def"

#undef FLOPPY144_OBJECT
};

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
