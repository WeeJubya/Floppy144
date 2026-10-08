/*
 * Floppy//144 - Site-space object interaction bridge
 */

#include "floppy144_site_object.h"
#include "floppy144_site_rooms.h"

#include "floppy144_object_registry.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_game_data.h"
#include "floppy144_site.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define FLOPPY144_ARRAY_COUNT(values) \
    ((uint32_t)(sizeof(values) / sizeof((values)[0])))

#define FLOPPY144_SITE_TERMINAL_INTERACTION_RANGE 2U

/*
 * Player-facing Site menus are deliberately local. Inspect/Access becomes
 * available only within half a Site unit of the player's collision footprint.
 * Labels and actions share that exact range and the same winning parent.
 */
#define FLOPPY144_SITE_DATA_INTERACTION_RANGE_X16 \
    (FLOPPY144_SITE_FIXED_ONE / 2)

/*
 * Labels and contextual actions deliberately share one range and one winning
 * parent, preventing neighbouring objects from contributing mixed controls.
 */
#define FLOPPY144_SITE_CONTEXT_LABEL_RANGE_X16 \
    FLOPPY144_SITE_DATA_INTERACTION_RANGE_X16

/*
 * Transitional Site-space location used only by the old technical-slice
 * interaction-target helper later in this file.
 *
 * Current Stage 3B Site inspection and geometry visibility are generated-data
 * driven. These coordinates are retained only so the dormant legacy helper is
 * internally coherent until it is removed.
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
        66U,
        90U,
        1U,
        4U,
        3U
    },

    {
        FLOPPY144_OBJECT_DESK_ONE,
        86U,
        78U,
        6U,
        4U,
        3U
    },

    /*
     * Legacy DESK_FOUR behaviour belongs to canonical Desk 02.
     */

    {
        FLOPPY144_OBJECT_DESK_FOUR,
        86U,
        84U,
        6U,
        4U,
        3U
    },

    {
        FLOPPY144_OBJECT_DESK_FOUR_PERSONNEL_FORMS,
        86U,
        84U,
        6U,
        4U,
        3U
    },

    /*
     * P-033 Final Isolation Register.
     *
     * Interaction rectangle sits inside Facilities Shelving Bay 3:
     * parent geometry = x47 y97 w8 h2.
     *
     * It deliberately does not exactly match the shelving rectangle, so
     * hiding the document cannot hide the shelving itself.
     */

    {
        FLOPPY144_OBJECT_P033_FINAL_ISOLATION_REGISTER,
        14U,
        57U,
        1U,
        4U,
        2U
    },

    /*
     * P-014 Site Closure Handover Folder.
     *
     * Interaction rectangle sits on Main Office Desk 06:
     * parent geometry = x85 y22 w4 h6.
     */

    {
        FLOPPY144_OBJECT_P014_SITE_CLOSURE_HANDOVER_FOLDER,
        74U,
        84U,
        2U,
        4U,
        2U
    }
};

/*
 * RunState equivalent of the old effective-visibility test.
 *
 * An object is usable only when it and every parent are visible, and every
 * required collection in that parent chain has been restored.
 */

/*
 * Determine whether a room owns an operational GDR terminal.
 *
 * Terminal identity is data-defined. Physical geometry remains authoritative
 * in site_layout.jsonc and reaches runtime through the generated Site store.
 */
static bool Floppy144SiteRoomHasGdrTerminal(
    Floppy144RoomId room
)
{
    switch(room)
    {
        #define FLOPPY144_TERMINAL(symbol, room_value, display_name) \
        case room_value:                                    \
        {                                                   \
            return true;                                    \
        }

        #include "floppy144_terminals.def"

        #undef FLOPPY144_TERMINAL

        default:
        {
            return false;
        }
    }
}

/*
 * Test whether the player is close enough to the GDR terminal in their
 * current room.
 *
 * The interaction range extends 2 whole Site units outward from the physical
 * terminal-desk rectangle. Terminals in adjoining rooms cannot be targeted
 * through walls because only the player's current room is considered.
 */
static bool Floppy144SiteTerminalInRange(
    const Floppy144RunState *state
)
{
    Floppy144RoomId room;

    uint32_t rect_index;

    const int32_t interaction_range_x16 =
    (int32_t)FLOPPY144_SITE_TERMINAL_INTERACTION_RANGE *
    FLOPPY144_SITE_FIXED_ONE;

    const uint32_t interaction_range_squared =
    (uint32_t)(
        interaction_range_x16 *
        interaction_range_x16
    );

    if(state == NULL)
    {
        return false;
    }

    room =
    Floppy144SiteRoomAtPosition(
        state->player_site_x,
        state->player_site_y
    );

    if(
        (uint32_t)room >=
        (uint32_t)FLOPPY144_ROOM_COUNT ||
        !Floppy144SiteRoomHasGdrTerminal(
            room
        )
    )
    {
        return false;
    }

    for(
        rect_index = 0U;
    rect_index < Floppy144SiteRectCount();
    ++rect_index
    )
    {
        const Floppy144SiteRect *rect =
        Floppy144SiteRectAt(
            rect_index
        );

        int32_t rectangle_x0;
        int32_t rectangle_x1;
        int32_t rectangle_y0;
        int32_t rectangle_y1;

        int32_t distance_x =
        0;

        int32_t distance_y =
        0;

        uint32_t distance_squared;

        if(
            rect == NULL ||
            rect->type !=
            (uint8_t)FLOPPY144_SITE_TERMINAL_DESK ||
            rect->room !=
            (uint8_t)room
        )
        {
            continue;
        }

        rectangle_x0 =
        (int32_t)rect->x *
        FLOPPY144_SITE_FIXED_ONE;

        rectangle_x1 =
        (
            (int32_t)rect->x +
            (int32_t)rect->width
        ) *
        FLOPPY144_SITE_FIXED_ONE;

        rectangle_y0 =
        (int32_t)rect->y *
        FLOPPY144_SITE_FIXED_ONE;

        rectangle_y1 =
        (
            (int32_t)rect->y +
            (int32_t)rect->height
        ) *
        FLOPPY144_SITE_FIXED_ONE;

        if(
            state->player_site_x <
            rectangle_x0
        )
        {
            distance_x =
            rectangle_x0 -
            state->player_site_x;
        }
        else if(
            state->player_site_x >
            rectangle_x1
        )
        {
            distance_x =
            state->player_site_x -
            rectangle_x1;
        }

        if(
            state->player_site_y <
            rectangle_y0
        )
        {
            distance_y =
            rectangle_y0 -
            state->player_site_y;
        }
        else if(
            state->player_site_y >
            rectangle_y1
        )
        {
            distance_y =
            state->player_site_y -
            rectangle_y1;
        }

        distance_squared =
        (uint32_t)(
            distance_x * distance_x +
            distance_y * distance_y
        );

        if(
            distance_squared <=
            interaction_range_squared
        )
        {
            return true;
        }
    }

    return false;
}

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
 * Resolve a generated furniture/fixture object hook to the small legacy
 * object registry.
 *
 * Stage 3B still retains a handful of legacy object bits for migrated
 * interactions, but their physical coordinates must never be authoritative.
 * The canonical JSON already emits each furniture/fixture object_id into the
 * flat game-data registry. Resolve that symbol here instead of maintaining a
 * second coordinate table for visibility.
 */
static Floppy144ObjectId Floppy144SiteObjectFromGeneratedHook(
    const char *pszObjectHook
)
{
    if(pszObjectHook == NULL)
    {
        return FLOPPY144_OBJECT_NONE;
    }

    #define FLOPPY144_OBJECT(symbol, ...)                         \
        if(strcmp(pszObjectHook, #symbol) == 0)                  \
        {                                                        \
            return FLOPPY144_OBJECT_##symbol;                    \
        }

    #include "floppy144_objects.def"

    #undef FLOPPY144_OBJECT

    return FLOPPY144_OBJECT_NONE;
}

/*
 * Find the legacy object hook, if any, belonging to one generated Site
 * rectangle.
 *
 * Geometry, room ownership and object identity all come from generated game
 * data. This deliberately avoids the old hard-coded Site-location table, so a
 * spreadsheet/JSON coordinate migration cannot silently disconnect hidden
 * progression geometry from its visibility state.
 */
static Floppy144ObjectId Floppy144SiteGeneratedObjectForRect(
    const Floppy144SiteRect *pRect
)
{
    uint32_t uRecordIndex;

    if(pRect == NULL)
    {
        return FLOPPY144_OBJECT_NONE;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        Floppy144RoomId eRoom;
        Floppy144ObjectId eObject;

        if(
            pRecord == NULL ||
            (
                pRecord->eKind != FLOPPY144_DATA_FURNITURE &&
                pRecord->eKind != FLOPPY144_DATA_FIXTURE
            ) ||
            pRecord->pszD == NULL
        )
        {
            continue;
        }

        eRoom =
            Floppy144GameDataRoomId(
                pRecord->pszA
            );

        if(
            eRoom == FLOPPY144_ROOM_COUNT ||
            (uint8_t)eRoom != pRect->room ||
            pRecord->n0 != (int32_t)pRect->x ||
            pRecord->n1 != (int32_t)pRect->y ||
            pRecord->n2 != (int32_t)pRect->width ||
            pRecord->n3 != (int32_t)pRect->height
        )
        {
            continue;
        }

        eObject =
            Floppy144SiteObjectFromGeneratedHook(
                pRecord->pszD
            );

        if(eObject != FLOPPY144_OBJECT_NONE)
        {
            return eObject;
        }
    }

    return FLOPPY144_OBJECT_NONE;
}

/*
 * Connect the small existing object registry to generated Site geometry.
 *
 * Only a generated furniture/fixture record with a recognised legacy
 * object_id can make its complete rectangle progression-controlled. Ordinary
 * desks and shelving remain visible even when a hidden document sits on them.
 */
bool Floppy144SiteObjectGeometryVisible(
    const Floppy144RunState *state,
    const Floppy144SiteRect *rect
)
{
    Floppy144ObjectId eObject;
    const Floppy144ObjectDefinition *pDefinition;

    if(
        state == NULL ||
        rect == NULL
    )
    {
        return false;
    }

    eObject =
        Floppy144SiteGeneratedObjectForRect(
            rect
        );

    /*
     * Most Site geometry has no legacy object hook and is ordinary
     * reconstructed-room scenery.
     */
    if(eObject == FLOPPY144_OBJECT_NONE)
    {
        return true;
    }

    pDefinition =
        Floppy144ObjectGet(
            eObject
        );

    /*
     * Child objects share their parent's furniture footprint. A hidden child
     * document must not hide the parent desk/cabinet itself.
     */
    if(
        pDefinition == NULL ||
        pDefinition->parent != FLOPPY144_OBJECT_NONE
    )
    {
        return true;
    }

    return
        Floppy144SiteObjectEffectivelyVisible(
            state,
            eObject
        );
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

    /*
     * GDR terminals are Site-native infrastructure.
     *
     * Their physical position comes directly from generated Site geometry.
     * The legacy ARCHIVE_TERMINAL registry object is retained only as the
     * interaction-action proxy until the old object registry is retired.
     */
    if(
        Floppy144SiteTerminalInRange(
            state
        )
    )
    {
        return
        FLOPPY144_OBJECT_ARCHIVE_TERMINAL;
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

/******************************************************************************
 * Stage 3B.2 data-driven Site interaction
 *
 * The legacy object table above remains only for technical-slice geometry
 * visibility while Stage 3 is migrated. New physical gameplay is resolved
 * from the generated furniture/fixture/physical-item/interaction records.
 ******************************************************************************/

static const Floppy144DataRecord *Floppy144SiteParentRecordForRect(
    const Floppy144SiteRect *pRect
);

static bool Floppy144SiteDataStringEqual(
    const char *pszA,
    const char *pszB
)
{
    return
        pszA != NULL &&
        pszB != NULL &&
        strcmp(pszA, pszB) == 0;
}

/*
 * Locate the furniture/fixture which physically owns one canonical item.
 */
static const Floppy144DataRecord *Floppy144SitePhysicalParentRecord(
    const char *pszParentId
)
{
    const Floppy144DataRecord *pRecord;

    if(pszParentId == NULL)
    {
        return NULL;
    }

    pRecord =
        Floppy144GameDataFind(
            FLOPPY144_DATA_FURNITURE,
            pszParentId
        );

    if(pRecord != NULL)
    {
        return pRecord;
    }

    return
        Floppy144GameDataFind(
            FLOPPY144_DATA_FIXTURE,
            pszParentId
        );
}

/*
 * Any generated parent with recovered physical material may expose a contents
 * view. Windows remain non-interactive scenery and secure cupboards retain
 * their dedicated Access/keypad gate; ordinary doors may still expose mounted
 * notices and room plates through Inspect.
 */
static bool Floppy144SiteParentInspectable(
    const Floppy144DataRecord *pParent
)
{
    if(pParent == NULL || pParent->pszB == NULL)
    {
        return false;
    }

    if(
        Floppy144SiteDataStringEqual(
            pParent->pszB,
            "WINDOW"
        ) ||
        Floppy144SiteDataStringEqual(
            pParent->pszB,
            "SECURE_CABINET_HALF"
        ) ||
        Floppy144SiteDataStringEqual(
            pParent->pszB,
            "SECURE_CABINET_FULL"
        )
    )
    {
        return false;
    }

    return true;
}

/*
 * Physical children cannot exist independently of a progression-controlled
 * parent.  This mirrors Floppy144SiteObjectGeometryVisible(): ordinary
 * furniture with no legacy visibility hook is present as soon as its room is
 * reconstructed, while a top-level hooked object such as the suppression
 * panel must itself be visible before any of its contextual children can be
 * inspected.
 */
static bool Floppy144SitePhysicalParentVisible(
    const Floppy144RunState *pState,
    const Floppy144DataRecord *pParent
)
{
    Floppy144ObjectId eObject;
    const Floppy144ObjectDefinition *pDefinition;

    if(pState == NULL || pParent == NULL)
    {
        return false;
    }

    eObject =
        Floppy144SiteObjectFromGeneratedHook(
            pParent->pszD
        );

    if(eObject == FLOPPY144_OBJECT_NONE)
    {
        return true;
    }

    pDefinition =
        Floppy144ObjectGet(
            eObject
        );

    /*
     * A generated parent may carry a legacy child hook.  As with Site
     * geometry, a child hook must not make the furniture containing it vanish.
     */
    if(
        pDefinition == NULL ||
        pDefinition->parent != FLOPPY144_OBJECT_NONE
    )
    {
        return true;
    }

    return
        Floppy144SiteObjectEffectivelyVisible(
            pState,
            eObject
        );
}

/*
 * Runtime furniture records normally carry x/y/width/height in n0..n3.
 * Rotated centre-authored furniture carries centre_x/centre_y in n4/n5 and
 * sets b0. The Stage 3B.2 compiler emitter adds that small piece of metadata so
 * interaction targeting stays generic for the Director and Secretary desks.
 */
static uint32_t Floppy144SiteDataRecordDistanceSquared(
    const Floppy144RunState *pState,
    const Floppy144DataRecord *pRecord
)
{
    int32_t nRectangleX0;
    int32_t nRectangleX1;
    int32_t nRectangleY0;
    int32_t nRectangleY1;

    int32_t nPlayerX0;
    int32_t nPlayerX1;
    int32_t nPlayerY0;
    int32_t nPlayerY1;

    int32_t nDistanceX = 0;
    int32_t nDistanceY = 0;

    if(
        pState == NULL ||
        pRecord == NULL ||
        pRecord->n2 <= 0 ||
        pRecord->n3 <= 0
    )
    {
        return UINT32_MAX;
    }

    if(pRecord->b0 != 0U)
    {
        int32_t nCentreX16 =
            pRecord->n4 * FLOPPY144_SITE_FIXED_ONE;

        int32_t nCentreY16 =
            pRecord->n5 * FLOPPY144_SITE_FIXED_ONE;

        int32_t nHalfWidth16 =
            pRecord->n2 * FLOPPY144_SITE_FIXED_ONE / 2;

        int32_t nHalfHeight16 =
            pRecord->n3 * FLOPPY144_SITE_FIXED_ONE / 2;

        nRectangleX0 = nCentreX16 - nHalfWidth16;
        nRectangleX1 = nCentreX16 + nHalfWidth16;
        nRectangleY0 = nCentreY16 - nHalfHeight16;
        nRectangleY1 = nCentreY16 + nHalfHeight16;
    }
    else
    {
        nRectangleX0 =
            pRecord->n0 * FLOPPY144_SITE_FIXED_ONE;

        nRectangleX1 =
            (pRecord->n0 + pRecord->n2) *
            FLOPPY144_SITE_FIXED_ONE;

        nRectangleY0 =
            pRecord->n1 * FLOPPY144_SITE_FIXED_ONE;

        nRectangleY1 =
            (pRecord->n1 + pRecord->n3) *
            FLOPPY144_SITE_FIXED_ONE;
    }

    /*
     * Measure from the player's actual movement footprint. The previous point
     * test measured from the foot point, which made large desks become
     * inspectable several movement steps too early.
     */
    nPlayerX0 =
        pState->player_site_x -
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 / 2;

    nPlayerX1 =
        pState->player_site_x +
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 / 2;

    nPlayerY0 =
        pState->player_site_y -
        FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16;

    nPlayerY1 =
        pState->player_site_y;

    if(nPlayerX1 < nRectangleX0)
    {
        nDistanceX = nRectangleX0 - nPlayerX1;
    }
    else if(nPlayerX0 > nRectangleX1)
    {
        nDistanceX = nPlayerX0 - nRectangleX1;
    }

    if(nPlayerY1 < nRectangleY0)
    {
        nDistanceY = nRectangleY0 - nPlayerY1;
    }
    else if(nPlayerY0 > nRectangleY1)
    {
        nDistanceY = nPlayerY0 - nRectangleY1;
    }

    return
        (uint32_t)(
            nDistanceX * nDistanceX +
            nDistanceY * nDistanceY
        );
}

/*
 * A physical item mentioned by any REVEAL_PHYSICAL_ITEM effect is hidden until
 * the owning persistent trigger/interaction has completed. Items which are not
 * reveal-controlled are ordinary reconstructed room context and are visible as
 * soon as their room exists.
 */
static bool Floppy144SitePhysicalItemRevealControlled(
    const char *pszPhysicalItemId
)
{
    uint32_t uRecordIndex;

    if(pszPhysicalItemId == NULL)
    {
        return false;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        if(
            pRecord == NULL ||
            (
                pRecord->eKind !=
                    FLOPPY144_DATA_TRIGGER_EFFECT &&
                pRecord->eKind !=
                    FLOPPY144_DATA_INTERACTION_EFFECT
            )
        )
        {
            continue;
        }

        if(
            Floppy144SiteDataStringEqual(
                pRecord->pszA,
                "REVEAL_PHYSICAL_ITEM"
            ) &&
            Floppy144SiteDataStringEqual(
                pRecord->pszB,
                pszPhysicalItemId
            )
        )
        {
            return true;
        }
    }

    return false;
}

bool Floppy144SitePhysicalItemVisible(
    const Floppy144RunState *pState,
    const Floppy144DataRecord *pPhysicalItem
)
{
    const Floppy144DataRecord *pParent;

    if(
        pState == NULL ||
        pPhysicalItem == NULL ||
        pPhysicalItem->eKind !=
            FLOPPY144_DATA_PHYSICAL_ITEM
    )
    {
        return false;
    }

    pParent =
        Floppy144SitePhysicalParentRecord(
            pPhysicalItem->pszC
        );

    /*
     * A contextual child added during the 500-PI world pass must not expose a
     * progression-hidden parent.  The suppression panel is the canonical
     * example: P-200..P-206 remain unavailable until the panel itself exists.
     */
    if(
        !Floppy144SitePhysicalParentVisible(
            pState,
            pParent
        )
    )
    {
        return false;
    }

    /* Hathaway inspects the entire restored inventory, including children
       normally revealed only by later interactions. This is a RAM-only view
       and never grants the child in a production recovery. */
    if(pState->hathaway_inspection != 0U)
    {
        return true;
    }

    if(
        !Floppy144SitePhysicalItemRevealControlled(
            pPhysicalItem->pszId
        )
    )
    {
        return true;
    }

    return
        Floppy144GameDataPhysicalItemRevealed(
            pState,
            pPhysicalItem->pszId
        );
}

/*
 * Rank physical children on the same furniture/fixture.
 *
 * A currently runnable story interaction wins. An incomplete but blocked item
 * remains inspectable without firing, completed evidence remains readable, and
 * ordinary contextual material is still available when no progression item is
 * present. This lets one Inspect key serve all canonical physical data without
 * hard-coding individual P-/I- identifiers.
 */
static uint32_t Floppy144SitePhysicalItemPriority(
    const Floppy144RunState *pState,
    const Floppy144DataRecord *pPhysicalItem,
    Floppy144InteractionId *pInteraction,
    bool *pAvailable,
    bool *pCompleted
)
{
    Floppy144InteractionId eInteraction;
    bool bAvailable = false;
    bool bCompleted = false;

    if(
        pInteraction == NULL ||
        pAvailable == NULL ||
        pCompleted == NULL
    )
    {
        return 0U;
    }

    *pInteraction =
        FLOPPY144_INTERACTION_COUNT;

    *pAvailable = false;
    *pCompleted = false;

    if(
        pState == NULL ||
        pPhysicalItem == NULL ||
        pPhysicalItem->pszId == NULL
    )
    {
        return 0U;
    }

    eInteraction =
        Floppy144InteractionForPhysicalSource(
            pPhysicalItem->pszId
        );

    if(eInteraction == FLOPPY144_INTERACTION_COUNT)
    {
        return 200U;
    }

    bCompleted =
        Floppy144RunStateInteractionCompleted(
            pState,
            eInteraction
        );

    bAvailable =
        !bCompleted &&
        Floppy144InteractionCanRun(
            pState,
            eInteraction
        );

    *pInteraction = eInteraction;
    *pAvailable = bAvailable;
    *pCompleted = bCompleted;

    if(bAvailable)
    {
        return 400U;
    }

    if(!bCompleted)
    {
        return 300U;
    }

    return 250U;
}

bool Floppy144SiteDirectoryNearby(
    const Floppy144RunState *pState
)
{
    const char *pszFocusedParentId;
    const Floppy144DataRecord *pRecord;
    Floppy144RoomId eCurrentRoom;

    if(pState == NULL)
    {
        return false;
    }

    pszFocusedParentId = Floppy144SiteFocusedParentId(pState);
    if(pszFocusedParentId == NULL)
    {
        return false;
    }

    pRecord = Floppy144GameDataFind(
        FLOPPY144_DATA_FIXTURE,
        pszFocusedParentId
    );

    if(
        pRecord == NULL ||
        pRecord->pszA == NULL ||
        pRecord->pszC == NULL ||
        strcmp(pRecord->pszC, "SITE_DIRECTORY") != 0
    )
    {
        return false;
    }

    eCurrentRoom = Floppy144SiteRoomAtPosition(
        pState->player_site_x,
        pState->player_site_y
    );

    return
        eCurrentRoom < FLOPPY144_ROOM_COUNT &&
        Floppy144GameDataRoomId(pRecord->pszA) == eCurrentRoom;
}


bool Floppy144SiteAccessTerminalRoom(
    const Floppy144RunState *pState,
    Floppy144RoomId *pRoom
)
{
    const char *pszFocusedParentId;
    const Floppy144DataRecord *pRecord;
    Floppy144RoomId eCurrentRoom;

    if(pState == NULL || pRoom == NULL)
    {
        return false;
    }

    pszFocusedParentId = Floppy144SiteFocusedParentId(pState);
    if(pszFocusedParentId == NULL)
    {
        return false;
    }

    pRecord = Floppy144GameDataFind(
        FLOPPY144_DATA_FURNITURE,
        pszFocusedParentId
    );

    if(
        pRecord == NULL ||
        !Floppy144SiteDataStringEqual(pRecord->pszB, "TERMINAL_DESK")
    )
    {
        return false;
    }

    eCurrentRoom = Floppy144SiteRoomAtPosition(
        pState->player_site_x,
        pState->player_site_y
    );

    if(
        eCurrentRoom >= FLOPPY144_ROOM_COUNT ||
        !Floppy144RunStateRoomReconstructed(pState, eCurrentRoom) ||
        Floppy144GameDataRoomId(pRecord->pszA) != eCurrentRoom
    )
    {
        return false;
    }

    *pRoom = eCurrentRoom;
    return true;
}

bool Floppy144SiteResolveInspectionTarget(
    const Floppy144RunState *pState,
    Floppy144SiteInspectionTarget *pTarget
)
{
    Floppy144RoomId eCurrentRoom;
    const char *pszFocusedParentId;
    uint32_t uRecordIndex;
    uint32_t uBestDistance = UINT32_MAX;
    uint32_t uBestPriority = 0U;
    int32_t nBestSourceOrder = INT32_MAX;
    bool bFound = false;

    const uint32_t uRangeX16 =
        FLOPPY144_SITE_DATA_INTERACTION_RANGE_X16;

    const uint32_t uRangeSquared =
        uRangeX16 * uRangeX16;

    if(pState == NULL || pTarget == NULL)
    {
        return false;
    }

    pTarget->pszParentId = NULL;
    pTarget->pszPhysicalItemId = NULL;
    pTarget->pszPhysicalItemName = NULL;
    pTarget->eInteraction = FLOPPY144_INTERACTION_COUNT;
    pTarget->bInteractionAvailable = false;
    pTarget->bInteractionCompleted = false;

    eCurrentRoom =
        Floppy144SiteRoomAtPosition(
            pState->player_site_x,
            pState->player_site_y
        );

    if(
        (uint32_t)eCurrentRoom >=
            (uint32_t)FLOPPY144_ROOM_COUNT ||
        !Floppy144RunStateRoomReconstructed(
            pState,
            eCurrentRoom
        )
    )
    {
        return false;
    }

    pszFocusedParentId = Floppy144SiteFocusedParentId(pState);
    if(pszFocusedParentId == NULL)
    {
        return false;
    }

    /*
     * Iterate canonical physical items rather than a hand-authored Site table.
     * Each item already names its furniture/fixture parent in generated data.
     */
    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pPhysicalItem =
            Floppy144GameDataRecordAt(uRecordIndex);

        const Floppy144DataRecord *pParent;
        Floppy144RoomId eItemRoom;
        uint32_t uDistance;
        uint32_t uPriority = 50U;
        int32_t nSourceOrder;
        Floppy144InteractionId eInteraction =
            FLOPPY144_INTERACTION_COUNT;
        bool bAvailable = false;
        bool bCompleted = false;
        bool bVisible;

        if(
            pPhysicalItem == NULL ||
            pPhysicalItem->eKind !=
                FLOPPY144_DATA_PHYSICAL_ITEM ||
            pPhysicalItem->pszC == NULL ||
            !Floppy144SiteDataStringEqual(
                pPhysicalItem->pszC,
                pszFocusedParentId
            )
        )
        {
            continue;
        }

        eItemRoom =
            Floppy144GameDataRoomId(
                pPhysicalItem->pszB
            );

        if(eItemRoom != eCurrentRoom)
        {
            continue;
        }

        pParent =
            Floppy144SitePhysicalParentRecord(
                pPhysicalItem->pszC
            );

        if(
            !Floppy144SiteParentInspectable(
                pParent
            )
        )
        {
            continue;
        }

        uDistance =
            Floppy144SiteDataRecordDistanceSquared(
                pState,
                pParent
            );

        if(uDistance > uRangeSquared)
        {
            continue;
        }

        bVisible =
            Floppy144SitePhysicalItemVisible(
                pState,
                pPhysicalItem
            );

        /*
         * Hidden progression items do not create a phantom Inspect action.
         * The action appears only when a physical child is actually present
         * in the reconstructed Site.
         */
        if(!bVisible)
        {
            continue;
        }

        uPriority =
            Floppy144SitePhysicalItemPriority(
                pState,
                pPhysicalItem,
                &eInteraction,
                &bAvailable,
                &bCompleted
            );

        nSourceOrder = pPhysicalItem->n0;

        if(
            !bFound ||
            uDistance < uBestDistance ||
            (
                uDistance == uBestDistance &&
                uPriority > uBestPriority
            ) ||
            (
                uDistance == uBestDistance &&
                uPriority == uBestPriority &&
                nSourceOrder < nBestSourceOrder
            )
        )
        {
            pTarget->pszParentId =
                pPhysicalItem->pszC;

            pTarget->pszPhysicalItemId =
                pPhysicalItem->pszId;

            pTarget->pszPhysicalItemName =
                pPhysicalItem->pszA;

            pTarget->eInteraction =
                eInteraction;

            pTarget->bInteractionAvailable =
                bAvailable;

            pTarget->bInteractionCompleted =
                bCompleted;

            uBestDistance = uDistance;
            uBestPriority = uPriority;
            nBestSourceOrder = nSourceOrder;
            bFound = true;
        }
    }

    /*
     * Empty non-secure storage is still an Inspect target. Physical children
     * remain authoritative when present; this fallback runs only when no
     * visible item was found, so it cannot steal focus from evidence or other
     * contextual props.
     */
    if(!bFound)
    {
        for(
            uRecordIndex = 0U;
            uRecordIndex < Floppy144GameDataRecordCount();
            ++uRecordIndex
        )
        {
            const Floppy144DataRecord *pParent =
                Floppy144GameDataRecordAt(uRecordIndex);

            Floppy144RoomId eParentRoom;
            uint32_t uDistance;

            if(
                pParent == NULL ||
                pParent->eKind != FLOPPY144_DATA_FURNITURE ||
                pParent->pszId == NULL ||
                !Floppy144SiteDataStringEqual(
                    pParent->pszId,
                    pszFocusedParentId
                ) ||
                pParent->pszB == NULL ||
                !(
                    Floppy144SiteDataStringEqual(
                        pParent->pszB,
                        "BOOKCASE"
                    ) ||
                    Floppy144SiteDataStringEqual(
                        pParent->pszB,
                        "NONSECURE_CABINET"
                    ) ||
                    Floppy144SiteDataStringEqual(
                        pParent->pszB,
                        "SHELVING_FULL"
                    )
                ) ||
                !Floppy144SiteParentInspectable(pParent)
            )
            {
                continue;
            }

            eParentRoom =
                Floppy144GameDataRoomId(
                    pParent->pszA
                );

            if(eParentRoom != eCurrentRoom)
            {
                continue;
            }

            uDistance =
                Floppy144SiteDataRecordDistanceSquared(
                    pState,
                    pParent
                );

            if(
                uDistance > uRangeSquared ||
                uDistance >= uBestDistance
            )
            {
                continue;
            }

            pTarget->pszParentId = pParent->pszId;
            pTarget->pszPhysicalItemId = NULL;
            pTarget->pszPhysicalItemName = "Empty storage";
            pTarget->eInteraction = FLOPPY144_INTERACTION_COUNT;
            pTarget->bInteractionAvailable = false;
            pTarget->bInteractionCompleted = false;

            uBestDistance = uDistance;
            bFound = true;
        }
    }

    /*
     * Boundary doors are inspectable containers in their own right. A room
     * plate is authored on the Corridor side, so a player standing on the
     * opposite side of a locked door will not find that child in the
     * same-room item pass above. Keep the focused Door parent as the target so
     * Inspect opens the reusable Door container from either side.
     */
    if(!bFound)
    {
        const Floppy144DataRecord *pDoor =
            Floppy144GameDataFind(
                FLOPPY144_DATA_FIXTURE,
                pszFocusedParentId
            );

        if(
            pDoor != NULL &&
            Floppy144SiteDataStringEqual(
                pDoor->pszB,
                "DOOR"
            )
        )
        {
            pTarget->pszParentId = pDoor->pszId;
            pTarget->pszPhysicalItemId = NULL;
            pTarget->pszPhysicalItemName = "Door";
            pTarget->eInteraction = FLOPPY144_INTERACTION_COUNT;
            pTarget->bInteractionAvailable = false;
            pTarget->bInteractionCompleted = false;
            bFound = true;
        }
    }

    return bFound;
}

/*
 * Squared distance from the player's collision footprint to generated Site
 * geometry. Used by passive context labels, which should appear only when the
 * player is immediately beside an object.
 */
static uint32_t Floppy144SiteRectDistanceSquared(
    const Floppy144RunState *pState,
    const Floppy144SiteRect *pRect
)
{
    int32_t nRectX0;
    int32_t nRectX1;
    int32_t nRectY0;
    int32_t nRectY1;
    int32_t nPlayerX0;
    int32_t nPlayerX1;
    int32_t nPlayerY0;
    int32_t nPlayerY1;
    int32_t nDistanceX = 0;
    int32_t nDistanceY = 0;

    if(pState == NULL || pRect == NULL)
    {
        return UINT32_MAX;
    }

    nRectX0 = (int32_t)pRect->x * FLOPPY144_SITE_FIXED_ONE;
    nRectX1 =
        ((int32_t)pRect->x + (int32_t)pRect->width) *
        FLOPPY144_SITE_FIXED_ONE;
    nRectY0 = (int32_t)pRect->y * FLOPPY144_SITE_FIXED_ONE;
    nRectY1 =
        ((int32_t)pRect->y + (int32_t)pRect->height) *
        FLOPPY144_SITE_FIXED_ONE;

    nPlayerX0 =
        pState->player_site_x -
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 / 2;

    nPlayerX1 =
        pState->player_site_x +
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16 / 2;

    nPlayerY0 =
        pState->player_site_y -
        FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16;

    nPlayerY1 =
        pState->player_site_y;

    if(nPlayerX1 < nRectX0)
    {
        nDistanceX = nRectX0 - nPlayerX1;
    }
    else if(nPlayerX0 > nRectX1)
    {
        nDistanceX = nPlayerX0 - nRectX1;
    }

    if(nPlayerY1 < nRectY0)
    {
        nDistanceY = nRectY0 - nPlayerY1;
    }
    else if(nPlayerY0 > nRectY1)
    {
        nDistanceY = nPlayerY0 - nRectY1;
    }

    return
        (uint32_t)(
            nDistanceX * nDistanceX +
            nDistanceY * nDistanceY
        );
}

static bool Floppy144SiteEndpointReconstructed(
    const Floppy144RunState *pState,
    uint8_t uRoom
)
{
    if(uRoom == FLOPPY144_SITE_ROOM_OUTSIDE)
    {
        return true;
    }

    return
        pState != NULL &&
        uRoom < (uint8_t)FLOPPY144_ROOM_COUNT &&
        Floppy144RunStateRoomReconstructed(
            pState,
            (Floppy144RoomId)uRoom
        );
}

bool Floppy144SiteRectRuntimeVisible(
    const Floppy144RunState *pState,
    const Floppy144SiteRect *pRect
)
{
    bool bReconstructed;

    if(pState == NULL || pRect == NULL)
    {
        return false;
    }

    /*
     * Ordinary room geometry exists only after its owning room has been
     * reconstructed. Boundary geometry exists only after both authored
     * endpoints exist; OUTSIDE is treated as permanently reconstructed.
     */
    if(pRect->from_room == pRect->to_room)
    {
        bReconstructed =
            pRect->room < (uint8_t)FLOPPY144_ROOM_COUNT &&
            Floppy144RunStateRoomReconstructed(
                pState,
                (Floppy144RoomId)pRect->room
            );
    }
    else
    {
        bReconstructed =
            Floppy144SiteEndpointReconstructed(
                pState,
                pRect->from_room
            ) &&
            Floppy144SiteEndpointReconstructed(
                pState,
                pRect->to_room
            );
    }

    if(!bReconstructed)
    {
        return false;
    }

    /*
     * A reconstructed room may still contain progression-controlled geometry
     * which has not yet been revealed. Keep the renderer, collision system and
     * passive labels on the same visibility contract.
     */
    return
        Floppy144SiteObjectGeometryVisible(
            pState,
            pRect
        );
}

static uint8_t Floppy144SiteConnectionEndpoint(
    const char *pszRoomId
)
{
    Floppy144RoomId eRoom;

    if(pszRoomId == NULL)
    {
        return FLOPPY144_SITE_ROOM_SHARED;
    }

    if(strcmp(pszRoomId, "OUTSIDE") == 0)
    {
        return FLOPPY144_SITE_ROOM_OUTSIDE;
    }

    eRoom = Floppy144GameDataRoomId(pszRoomId);

    if((uint32_t)eRoom >= (uint32_t)FLOPPY144_ROOM_COUNT)
    {
        return FLOPPY144_SITE_ROOM_SHARED;
    }

    return (uint8_t)eRoom;
}

/*
 * Player-facing signage on the Corridor side of authored doors.
 *
 * The Secretary's Office is the Director's anteroom, so its public Corridor
 * plaque reads DIRECTOR. The internal Secretary/Director door remains a normal
 * door and is not part of this Corridor-signage contract.
 */
static const char *Floppy144SiteCorridorDoorLabelForRect(
    const Floppy144SiteRect *pRect
)
{
    uint8_t uOtherRoom;

    if(
        pRect == NULL ||
        pRect->type != (uint8_t)FLOPPY144_SITE_DOOR ||
        (
            pRect->from_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR &&
            pRect->to_room != (uint8_t)FLOPPY144_ROOM_CORRIDOR
        )
    )
    {
        return NULL;
    }

    uOtherRoom =
        pRect->from_room == (uint8_t)FLOPPY144_ROOM_CORRIDOR
        ? pRect->to_room
        : pRect->from_room;

    if(uOtherRoom == FLOPPY144_SITE_ROOM_OUTSIDE)
        return "EMERGENCY EXIT";

    switch((Floppy144RoomId)uOtherRoom)
    {
        case FLOPPY144_ROOM_RECEPTION:         return "RECEPTION";
        case FLOPPY144_ROOM_RECORDS_OFFICE:    return "RECORDS OFFICE";
        case FLOPPY144_ROOM_MAIN_OFFICE:       return "MAIN OFFICE";
        case FLOPPY144_ROOM_SECURITY:          return "SECURITY";
        case FLOPPY144_ROOM_IT_SUPPORT:        return "IT SUPPORT";
        case FLOPPY144_ROOM_SECRETARY_OFFICE:  return "DIRECTOR";
        case FLOPPY144_ROOM_STAFF_ROOM:        return "STAFF ROOM";
        case FLOPPY144_ROOM_FACILITIES:        return "FACILITIES";
        default:                               return NULL;
    }
}

/*
 * Boundary rectangles do not carry connection IDs at runtime, but the pair of
 * endpoints uniquely determines current lock state in the authored Site. The
 * two parallel exterior entrance/emergency doors share the same state, so the
 * endpoint match remains unambiguous for player-facing labels.
 */
static bool Floppy144SiteDoorLocked(
    const Floppy144RunState *pState,
    const Floppy144SiteRect *pRect
)
{
    uint32_t uRecordIndex;
    bool bMatched = false;

    if(
        pState == NULL ||
        pRect == NULL ||
        pRect->type != (uint8_t)FLOPPY144_SITE_DOOR
    )
    {
        return false;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pConnection =
            Floppy144GameDataRecordAt(uRecordIndex);
        uint8_t uFrom;
        uint8_t uTo;
        bool bEndpointsMatch;

        if(
            pConnection == NULL ||
            pConnection->eKind != FLOPPY144_DATA_CONNECTION
        )
        {
            continue;
        }

        uFrom = Floppy144SiteConnectionEndpoint(pConnection->pszA);
        uTo = Floppy144SiteConnectionEndpoint(pConnection->pszB);

        bEndpointsMatch =
            (
                uFrom == pRect->from_room &&
                uTo == pRect->to_room
            ) ||
            (
                uFrom == pRect->to_room &&
                uTo == pRect->from_room
            );

        if(!bEndpointsMatch)
        {
            continue;
        }

        bMatched = true;

        if(
            pConnection->pszId != NULL &&
            !Floppy144GameDataConnectionUnlocked(
                pState,
                pConnection->pszId
            )
        )
        {
            return true;
        }
    }

    (void)bMatched;
    return false;
}

static const char *Floppy144SiteFurnitureLabel(
    Floppy144SiteElement eElement
)
{
    switch(eElement)
    {
        case FLOPPY144_SITE_STANDARD_DESK:        return "DESK";
        case FLOPPY144_SITE_TERMINAL_DESK:        return "TERMINAL DESK";
        case FLOPPY144_SITE_WALL_MOUNTED_ITEM:    return "WALL HANGING";
        case FLOPPY144_SITE_CHAIR:                return "CHAIR";
        case FLOPPY144_SITE_NONSECURE_CABINET:
        case FLOPPY144_SITE_SECURE_CABINET_HALF:
        case FLOPPY144_SITE_SECURE_CABINET_FULL:  return "CUPBOARD";
        case FLOPPY144_SITE_BOOKCASE:             return "BOOKCASE";
        case FLOPPY144_SITE_FRIDGE:               return "FRIDGE";
        case FLOPPY144_SITE_WORKTOP:              return "WORKTOP";
        case FLOPPY144_SITE_SINK:                 return "SINK";
        case FLOPPY144_SITE_COFFEE_MAKER:         return "COFFEE MAKER";
        case FLOPPY144_SITE_SOFA:                 return "SOFA";
        case FLOPPY144_SITE_SERVER:               return "SERVER";
        case FLOPPY144_SITE_SHELVING_FULL:        return "SHELVING";
        case FLOPPY144_SITE_TROLLEY:              return "TROLLEY";
        case FLOPPY144_SITE_TABLE:                return "TABLE";
        default:                                  return NULL;
    }
}

static const char *Floppy144SiteWallHangingLabel(
    const Floppy144SiteRect *pRect
)
{
    uint32_t uRecordIndex;

    if(
        pRect == NULL ||
        pRect->type != (uint8_t)FLOPPY144_SITE_WALL_MOUNTED_ITEM
    )
    {
        return NULL;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        const char *pszVariant;

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_FIXTURE ||
            pRecord->pszA == NULL ||
            Floppy144GameDataRoomId(pRecord->pszA) !=
                (Floppy144RoomId)pRect->room ||
            pRecord->n0 != (int32_t)pRect->x ||
            pRecord->n1 != (int32_t)pRect->y ||
            pRecord->n2 != (int32_t)pRect->width ||
            pRecord->n3 != (int32_t)pRect->height
        )
        {
            continue;
        }

        pszVariant = pRecord->pszC;

        if(pszVariant == NULL)
            break;

        if(strcmp(pszVariant, "SITE_DIRECTORY") == 0)
            return "SITE DIRECTORY";
        if(strcmp(pszVariant, "SUPPRESSION_PANEL") == 0)
            return "SUPPRESSION PANEL";
        if(strcmp(pszVariant, "PATCH_PANEL") == 0)
            return "PATCH PANEL";
        if(strcmp(pszVariant, "SHELVING") == 0)
            return "WALL SHELVING";
        if(strcmp(pszVariant, "NOTICEBOARD") == 0)
            return "NOTICEBOARD";
        if(strcmp(pszVariant, "KEY_CABINET") == 0)
            return "KEY CABINET";
        if(strcmp(pszVariant, "FIRST_AID_KIT") == 0)
            return "FIRST AID KIT";
        if(strcmp(pszVariant, "MONITOR_BANK") == 0)
            return "MONITOR BANK";
        if(strcmp(pszVariant, "CABLE_RISER") == 0)
            return "CABLE RISER";

        break;
    }

    return "WALL HANGING";
}

const char *Floppy144SiteCorridorDoorLabelNearby(
    const Floppy144RunState *pState,
    bool *pLocked
)
{
    const char *pszFocusedParentId;
    const Floppy144DataRecord *pFixture;
    uint32_t uRectIndex;

    if(pLocked != NULL)
    {
        *pLocked = false;
    }

    if(
        pState == NULL ||
        Floppy144SiteRoomAtPosition(
            pState->player_site_x,
            pState->player_site_y
        ) != FLOPPY144_ROOM_CORRIDOR
    )
    {
        return NULL;
    }

    pszFocusedParentId = Floppy144SiteFocusedParentId(pState);
    if(pszFocusedParentId == NULL)
    {
        return NULL;
    }

    pFixture = Floppy144GameDataFind(
        FLOPPY144_DATA_FIXTURE,
        pszFocusedParentId
    );

    if(
        pFixture == NULL ||
        !Floppy144SiteDataStringEqual(pFixture->pszB, "DOOR")
    )
    {
        return NULL;
    }

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);
        const Floppy144DataRecord *pParent;

        if(
            pRect == NULL ||
            pRect->type != (uint8_t)FLOPPY144_SITE_DOOR
        )
        {
            continue;
        }

        pParent = Floppy144SiteParentRecordForRect(pRect);

        if(
            pParent == NULL ||
            pParent->pszId == NULL ||
            strcmp(pParent->pszId, pszFocusedParentId) != 0
        )
        {
            continue;
        }

        if(pLocked != NULL)
        {
            *pLocked = Floppy144SiteDoorLocked(pState, pRect);
        }

        return Floppy144SiteCorridorDoorLabelForRect(pRect);
    }

    return NULL;
}

bool Floppy144SiteLockedDoorNearby(
    const Floppy144RunState *pState
)
{
    const char *pszFocusedParentId;
    uint32_t uRectIndex;

    if(pState == NULL)
    {
        return false;
    }

    pszFocusedParentId = Floppy144SiteFocusedParentId(pState);
    if(pszFocusedParentId == NULL)
    {
        return false;
    }

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);
        const Floppy144DataRecord *pParent;

        if(
            pRect == NULL ||
            pRect->type != (uint8_t)FLOPPY144_SITE_DOOR ||
            !Floppy144SiteRectRuntimeVisible(pState, pRect)
        )
        {
            continue;
        }

        pParent = Floppy144SiteParentRecordForRect(pRect);

        if(
            pParent == NULL ||
            pParent->pszId == NULL ||
            strcmp(pParent->pszId, pszFocusedParentId) != 0
        )
        {
            continue;
        }

        return Floppy144SiteDoorLocked(pState, pRect);
    }

    return false;
}


static const Floppy144DataRecord *Floppy144SiteParentRecordForRect(
    const Floppy144SiteRect *pRect
)
{
    uint32_t uRecordIndex;

    if(pRect == NULL)
    {
        return NULL;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);
        Floppy144RoomId eRecordRoom;
        bool bGeometryMatch;

        if(
            pRecord == NULL ||
            (
                pRecord->eKind != FLOPPY144_DATA_FURNITURE &&
                pRecord->eKind != FLOPPY144_DATA_FIXTURE
            ) ||
            pRecord->pszId == NULL
        )
        {
            continue;
        }

        /*
         * Ordinary generated records store runtime x/y/width/height in n0..n3.
         * Rotated centre-authored furniture stores its unrotated width/height
         * in n2/n3, centre in n4/n5 and sets b0. Match both forms back to the
         * same runtime Site rectangle.
         */
        if(pRecord->b0 != 0U)
        {
            int32_t nCentreX16 =
                ((int32_t)pRect->x * FLOPPY144_SITE_FIXED_ONE) +
                ((int32_t)pRect->width * FLOPPY144_SITE_FIXED_ONE) / 2;
            int32_t nCentreY16 =
                ((int32_t)pRect->y * FLOPPY144_SITE_FIXED_ONE) +
                ((int32_t)pRect->height * FLOPPY144_SITE_FIXED_ONE) / 2;

            bGeometryMatch =
                pRecord->n4 * FLOPPY144_SITE_FIXED_ONE == nCentreX16 &&
                pRecord->n5 * FLOPPY144_SITE_FIXED_ONE == nCentreY16 &&
                pRecord->n2 * FLOPPY144_SITE_FIXED_ONE ==
                    (int32_t)pRect->authored_width16 &&
                pRecord->n3 * FLOPPY144_SITE_FIXED_ONE ==
                    (int32_t)pRect->authored_height16;
        }
        else
        {
            bGeometryMatch =
                pRecord->n0 == (int32_t)pRect->x &&
                pRecord->n1 == (int32_t)pRect->y &&
                pRecord->n2 == (int32_t)pRect->width &&
                pRecord->n3 == (int32_t)pRect->height;
        }

        if(!bGeometryMatch)
        {
            continue;
        }

        if(pRect->type != (uint8_t)FLOPPY144_SITE_DOOR)
        {
            eRecordRoom = Floppy144GameDataRoomId(pRecord->pszA);
            if(
                eRecordRoom >= FLOPPY144_ROOM_COUNT ||
                (uint8_t)eRecordRoom != pRect->room
            )
            {
                continue;
            }
        }
        else if(
            pRecord->eKind != FLOPPY144_DATA_FIXTURE ||
            !Floppy144SiteDataStringEqual(pRecord->pszB, "DOOR")
        )
        {
            continue;
        }

        return pRecord;
    }

    return NULL;
}

static const Floppy144SiteRect *Floppy144SiteFocusedRect(
    const Floppy144RunState *pState,
    const char **ppszLabel
)
{
    Floppy144RoomId eCurrentRoom;
    uint32_t uRectIndex;
    uint32_t uBestDistance = UINT32_MAX;
    const Floppy144SiteRect *pBestRect = NULL;
    const char *pszBestLabel = NULL;
    const uint32_t uRangeSquared =
        FLOPPY144_SITE_CONTEXT_LABEL_RANGE_X16 *
        FLOPPY144_SITE_CONTEXT_LABEL_RANGE_X16;

    if(ppszLabel != NULL)
    {
        *ppszLabel = NULL;
    }

    if(pState == NULL)
    {
        return NULL;
    }

    eCurrentRoom = Floppy144SiteRoomAtPosition(
        pState->player_site_x,
        pState->player_site_y
    );

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);
        const char *pszLabel = NULL;
        uint32_t uDistance;

        if(
            pRect == NULL ||
            !Floppy144SiteRectRuntimeVisible(pState, pRect)
        )
        {
            continue;
        }

        /*
         * Wall-mounted fixtures may now occupy the wall cell itself. Ordinary
         * furniture/fixtures can only own focus from their authored room side;
         * boundary doors remain focusable from either endpoint room.
         */
        if(pRect->type == (uint8_t)FLOPPY144_SITE_DOOR)
        {
            if(
                eCurrentRoom >= FLOPPY144_ROOM_COUNT ||
                (
                    pRect->from_room != (uint8_t)eCurrentRoom &&
                    pRect->to_room != (uint8_t)eCurrentRoom
                )
            )
            {
                continue;
            }
        }
        else if(
            eCurrentRoom >= FLOPPY144_ROOM_COUNT ||
            pRect->room != (uint8_t)eCurrentRoom
        )
        {
            continue;
        }

        if(pRect->type == (uint8_t)FLOPPY144_SITE_DOOR)
        {
            if(eCurrentRoom == FLOPPY144_ROOM_CORRIDOR)
            {
                pszLabel = Floppy144SiteCorridorDoorLabelForRect(pRect);
            }

            if(
                pszLabel == NULL &&
                Floppy144SiteDoorLocked(pState, pRect)
            )
            {
                pszLabel = "LOCKED DOOR";
            }
        }
        else if(
            pRect->type ==
                (uint8_t)FLOPPY144_SITE_WALL_MOUNTED_ITEM
        )
        {
            pszLabel = Floppy144SiteWallHangingLabel(pRect);
        }
        else
        {
            pszLabel = Floppy144SiteFurnitureLabel(
                (Floppy144SiteElement)pRect->type
            );
        }

        if(pszLabel == NULL)
        {
            continue;
        }

        uDistance = Floppy144SiteRectDistanceSquared(pState, pRect);

        if(
            uDistance > uRangeSquared ||
            uDistance >= uBestDistance
        )
        {
            continue;
        }

        uBestDistance = uDistance;
        pBestRect = pRect;
        pszBestLabel = pszLabel;
    }

    if(ppszLabel != NULL)
    {
        *ppszLabel = pszBestLabel;
    }

    return pBestRect;
}

const char *Floppy144SiteContextLabel(
    const Floppy144RunState *pState
)
{
    const char *pszLabel = NULL;

    (void)Floppy144SiteFocusedRect(pState, &pszLabel);
    return pszLabel;
}

const char *Floppy144SiteFocusedParentId(
    const Floppy144RunState *pState
)
{
    const Floppy144SiteRect *pRect;
    const Floppy144DataRecord *pParent;

    pRect = Floppy144SiteFocusedRect(pState, NULL);
    if(pRect == NULL)
    {
        return NULL;
    }

    pParent = Floppy144SiteParentRecordForRect(pRect);

    return pParent != NULL ? pParent->pszId : NULL;
}

const Floppy144SiteRect *Floppy144SiteRectForParentId(
    const char *pszParentId
)
{
    uint32_t uRectIndex;

    if(pszParentId == NULL)
    {
        return NULL;
    }

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);

        const Floppy144DataRecord *pParent =
            Floppy144SiteParentRecordForRect(pRect);

        if(
            pParent != NULL &&
            pParent->pszId != NULL &&
            strcmp(pParent->pszId, pszParentId) == 0
        )
        {
            return pRect;
        }
    }

    return NULL;
}


bool Floppy144SiteAccessInteractionNearby(
    const Floppy144RunState *pState,
    Floppy144InteractionId *pInteraction
)
{
    const char *pszFocusedParentId;
    const Floppy144DataRecord *pParent;
    const Floppy144DataRecord *pInteractionRecord;
    Floppy144InteractionId eInteraction;

    if(pState == NULL || pInteraction == NULL)
    {
        return false;
    }

    pszFocusedParentId = Floppy144SiteFocusedParentId(pState);
    if(pszFocusedParentId == NULL)
    {
        return false;
    }

    pParent = Floppy144GameDataFind(
        FLOPPY144_DATA_FIXTURE,
        pszFocusedParentId
    );

    if(
        pParent == NULL ||
        !Floppy144SiteDataStringEqual(pParent->pszB, "DOOR")
    )
    {
        return false;
    }

    eInteraction = Floppy144InteractionForPhysicalSource(
        pszFocusedParentId
    );

    if(eInteraction == FLOPPY144_INTERACTION_COUNT)
    {
        return false;
    }

    pInteractionRecord = Floppy144InteractionRecord(eInteraction);

    if(
        pInteractionRecord == NULL ||
        !Floppy144SiteDataStringEqual(
            pInteractionRecord->pszA,
            "Access"
        ) ||
        !Floppy144InteractionCanRun(
            pState,
            eInteraction
        )
    )
    {
        return false;
    }

    *pInteraction = eInteraction;
    return true;
}

uint32_t Floppy144SiteAvailableActions(
    const Floppy144RunState *pState
)
{
    uint32_t uActions = 0U;
    Floppy144RoomId eRoom;
    Floppy144InteractionId eAccessInteraction;
    Floppy144SiteInspectionTarget sTarget;

    if(pState == NULL)
    {
        return 0U;
    }

    if(
        Floppy144SiteAccessTerminalRoom(
            pState,
            &eRoom
        ) ||
        Floppy144SiteAccessInteractionNearby(
            pState,
            &eAccessInteraction
        )
    )
    {
        uActions |=
            FLOPPY144_SITE_ACTION_ACCESS;
    }

    if(
        Floppy144SiteDirectoryNearby(pState) ||
        Floppy144SiteCorridorDoorLabelNearby(pState, NULL) != NULL ||
        Floppy144SiteLockedDoorNearby(pState) ||
        Floppy144SiteResolveInspectionTarget(
            pState,
            &sTarget
        )
    )
    {
        uActions |=
            FLOPPY144_SITE_ACTION_INSPECT;
    }

    return uActions;
}

