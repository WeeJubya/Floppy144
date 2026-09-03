/*
 * Floppy//144 - Site-space object interaction bridge
 */

#include "floppy144_site_object.h"

#include "floppy144_object_registry.h"
#include "floppy144_site.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FLOPPY144_ARRAY_COUNT(values) \
((uint32_t)(sizeof(values) / sizeof((values)[0])))

/*
 * Site-space location of one migrated interactable object.
 *
 * x/y/width/height are whole Site units from Office Dimensions.xlsx.
 * interaction_range is measured outward from the object's rectangle.
 */

typedef struct Floppy144SiteObjectLocation
{
    Floppy144ObjectId object;

    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;

    uint8_t interaction_range;
}
Floppy144SiteObjectLocation;

/*
 * Technical-slice interactions migrated to the exact Main Office.
 *
 * DESK_FOUR and DESK_FOUR_PERSONNEL_FORMS are legacy internal identifiers.
 * Their authored behaviour describes the temporary IT Support workstation.
 * The canonical Setup assigns that workstation to Desk 02, therefore both
 * are deliberately positioned on the physical Desk 02 rectangle here.
 *
 * The identifiers can be renamed after the old pixel Office has been retired
 * without changing their persisted bit positions.
 */

static const Floppy144SiteObjectLocation
floppy144_site_object_locations[] =
{
    {
        FLOPPY144_OBJECT_SUPPRESSION_CONTROL_PANEL,
        90U,
        32U,
        6U,
        1U,
        3U
    },

    {
        FLOPPY144_OBJECT_ARCHIVE_TERMINAL,
        94U,
        1U,
        5U,
        4U,
        3U
    },

    {
        FLOPPY144_OBJECT_DESK_ONE,
        80U,
        8U,
        4U,
        6U,
        3U
    },

    /*
     * Legacy DESK_FOUR behaviour belongs to canonical Desk 02.
     */
    {
        FLOPPY144_OBJECT_DESK_FOUR,
        85U,
        8U,
        4U,
        6U,
        3U
    },

    {
        FLOPPY144_OBJECT_DESK_FOUR_PERSONNEL_FORMS,
        85U,
        8U,
        4U,
        6U,
        3U
    }
};

/*
 * RunState equivalent of the old effective-visibility test.
 *
 * An object is usable only when it and every parent are visible, and every
 * required collection in that parent chain has been restored.
 */

static bool Floppy144SiteObjectEffectivelyVisible(
    const Floppy144RunState *state,
    Floppy144ObjectId object
)
{
    Floppy144ObjectId current =
    object;

    uint32_t depth =
    0U;

    if(state == NULL)
    {
        return false;
    }

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
        Floppy144ObjectGet(
            current
        );

        if(
            definition == NULL ||
            !Floppy144RunStateObjectVisible(
                state,
                current
            )
        )
        {
            return false;
        }

        if(
            definition->required_collection !=
            FLOPPY144_COLLECTION_COUNT &&
            !Floppy144RunStateCollectionRestored(
                state,
                definition->required_collection
            )
        )
        {
            return false;
        }

        current =
        definition->parent;

        ++depth;
    }

    return true;
}

/*
 * Connect the small existing object registry to generated Site geometry.
 *
 * The JSONC "object" strings are intentionally only future hooks at this
 * stage. The current playable registry therefore remains bridged through the
 * same compact Site-location table used for interaction targeting.
 *
 * Only top-level registered objects own a Site furniture/fixture rectangle.
 * Child objects such as DESK_FOUR_PERSONNEL_FORMS share their parent's
 * physical location but must not make the desk itself disappear.
 */
bool Floppy144SiteObjectGeometryVisible(
    const Floppy144RunState *state,
    const Floppy144SiteRect *rect
)
{
    uint32_t location_index;

    if(
        state == NULL ||
        rect == NULL
    )
    {
        return false;
    }

    for(
        location_index = 0U;
        location_index <
            FLOPPY144_ARRAY_COUNT(
                floppy144_site_object_locations
            );
        ++location_index
    )
    {
        const Floppy144SiteObjectLocation *location =
            &floppy144_site_object_locations[location_index];

        const Floppy144ObjectDefinition *definition =
            Floppy144ObjectGet(location->object);

        if(
            definition == NULL ||
            definition->parent != FLOPPY144_OBJECT_NONE
        )
        {
            continue;
        }

        if(
            rect->x == location->x &&
            rect->y == location->y &&
            rect->width == location->width &&
            rect->height == location->height
        )
        {
            return
                Floppy144SiteObjectEffectivelyVisible(
                    state,
                    location->object
                );
        }
    }

    return true;
}

/*
 * Squared distance from a Site point to an object's rectangular footprint.
 *
 * A point inside the rectangle has distance zero. Avoiding square roots keeps
 * the test compact and deterministic.
 */

static uint32_t Floppy144SiteObjectDistanceSquared(
    const Floppy144RunState *state,
    const Floppy144SiteObjectLocation *location
)
{
    int32_t rectangle_x0;
    int32_t rectangle_x1;
    int32_t rectangle_y0;
    int32_t rectangle_y1;

    int32_t distance_x =
    0;

    int32_t distance_y =
    0;

    rectangle_x0 =
    (int32_t)location->x *
    FLOPPY144_SITE_FIXED_ONE;

    rectangle_x1 =
    (
        (int32_t)location->x +
        (int32_t)location->width
    ) *
    FLOPPY144_SITE_FIXED_ONE;

    rectangle_y0 =
    (int32_t)location->y *
    FLOPPY144_SITE_FIXED_ONE;

    rectangle_y1 =
    (
        (int32_t)location->y +
        (int32_t)location->height
    ) *
    FLOPPY144_SITE_FIXED_ONE;

    if(state->player_site_x < rectangle_x0)
    {
        distance_x =
        rectangle_x0 -
        state->player_site_x;
    }
    else if(state->player_site_x > rectangle_x1)
    {
        distance_x =
        state->player_site_x -
        rectangle_x1;
    }

    if(state->player_site_y < rectangle_y0)
    {
        distance_y =
        rectangle_y0 -
        state->player_site_y;
    }
    else if(state->player_site_y > rectangle_y1)
    {
        distance_y =
        state->player_site_y -
        rectangle_y1;
    }

    return
    (uint32_t)(
        distance_x * distance_x +
        distance_y * distance_y
    );
}

/*
 * Resolve the best Site-space interaction.
 *
 * Registered interaction priority retains its old meaning. When two objects
 * have equal priority, the physically nearer object wins.
 */

Floppy144ObjectId Floppy144SiteInteractionTarget(
    const Floppy144RunState *state
)
{
    Floppy144ObjectId best_object =
    FLOPPY144_OBJECT_NONE;

    uint32_t best_priority =
    0U;

    uint32_t best_distance =
    0U;

    uint32_t location_index;

    if(state == NULL)
    {
        return FLOPPY144_OBJECT_NONE;
    }

    for(
        location_index = 0U;
    location_index <
    FLOPPY144_ARRAY_COUNT(
        floppy144_site_object_locations
    );
    ++location_index
    )
    {
        const Floppy144SiteObjectLocation *location =
        &floppy144_site_object_locations[
            location_index
        ];

        const Floppy144ObjectDefinition *definition =
        Floppy144ObjectGet(
            location->object
        );

        const Floppy144ObjectInteractionDefinition *interaction;

        uint32_t distance_squared;
        uint32_t interaction_range_x16;
        uint32_t interaction_range_squared;

        if(
            definition == NULL ||
            definition->scene !=
            FLOPPY144_SCENE_OFFICE ||
            definition->interaction == NULL
        )
        {
            continue;
        }

        if(
            !Floppy144SiteObjectEffectivelyVisible(
                state,
                location->object
            )
        )
        {
            continue;
        }

        interaction =
        definition->interaction;

        /*
         * The old collection-level evidence prerequisite is retired.
         * Current migrated interactions do not use it. Fail closed if an
         * unmigrated interaction carrying that legacy prerequisite appears.
         */
        if(
            interaction->required_evidence_collection !=
            FLOPPY144_COLLECTION_COUNT
        )
        {
            continue;
        }

        distance_squared =
        Floppy144SiteObjectDistanceSquared(
            state,
            location
        );

        interaction_range_x16 =
        (uint32_t)location->interaction_range *
        FLOPPY144_SITE_FIXED_ONE;

        interaction_range_squared =
        interaction_range_x16 *
        interaction_range_x16;

        if(
            distance_squared >
            interaction_range_squared
        )
        {
            continue;
        }

        if(
            best_object ==
            FLOPPY144_OBJECT_NONE ||
            interaction->priority >
            best_priority ||
            (
                interaction->priority ==
                best_priority &&
                distance_squared <
                best_distance
            )
        )
        {
            best_object =
            location->object;

            best_priority =
            interaction->priority;

            best_distance =
            distance_squared;
        }
    }

    return best_object;
}
