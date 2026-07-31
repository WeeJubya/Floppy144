/*
 * Floppy//144 - immutable scene-object registry
 *
 * Defines where reconstructed objects belong, how they are drawn and which
 * bounds are consumed by collision and interaction systems.
 */

#pragma once

#include "floppy144_object.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Scene ownership
 */

typedef enum Floppy144SceneId
{
    FLOPPY144_SCENE_OFFICE = 0
} Floppy144SceneId;

/*
 * Object behaviour flags
 */

typedef enum Floppy144ObjectFlags
{
    FLOPPY144_OBJECT_FLAG_NONE =
        0U,

    FLOPPY144_OBJECT_FLAG_SOLID =
        1U << 0,

    FLOPPY144_OBJECT_FLAG_INTERACTABLE =
        1U << 1,

    FLOPPY144_OBJECT_FLAG_INSPECTABLE =
        1U << 2
} Floppy144ObjectFlags;

/*
 * Scene drawing layers
 *
 * Parenting controls relative position and inherited visibility. It does not
 * dictate drawing order.
 */

typedef enum Floppy144ObjectLayer
{
    FLOPPY144_OBJECT_LAYER_BACKGROUND = 0,
    FLOPPY144_OBJECT_LAYER_FIXTURE = 10,
    FLOPPY144_OBJECT_LAYER_FURNITURE = 20,
    FLOPPY144_OBJECT_LAYER_CONTENT = 30,
    FLOPPY144_OBJECT_LAYER_FOREGROUND = 40,

    FLOPPY144_OBJECT_LAYER_MAX =
        FLOPPY144_OBJECT_LAYER_FOREGROUND
} Floppy144ObjectLayer;

/*
 * Asset-free drawing primitives
 */

typedef enum Floppy144ObjectPrimitiveType
{
    FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT = 0,
    FLOPPY144_OBJECT_PRIMITIVE_RECT,
    FLOPPY144_OBJECT_PRIMITIVE_TEXT
} Floppy144ObjectPrimitiveType;

/*
 * Palette roles are converted into actual colours by the owning scene.
 */

typedef enum Floppy144ObjectColourRole
{
    FLOPPY144_OBJECT_COLOUR_BODY = 0,
    FLOPPY144_OBJECT_COLOUR_FURNITURE,
    FLOPPY144_OBJECT_COLOUR_EDGE,
    FLOPPY144_OBJECT_COLOUR_SCREEN,
    FLOPPY144_OBJECT_COLOUR_WARNING,
    FLOPPY144_OBJECT_COLOUR_LABEL
} Floppy144ObjectColourRole;
typedef struct Floppy144ObjectPrimitive
{
    Floppy144ObjectPrimitiveType type;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;

    Floppy144ObjectColourRole colour_role;

    const char *text;
} Floppy144ObjectPrimitive;

/*
 * Immutable object definition
 *
 * Local coordinates are relative to the parent. Objects without a parent are
 * positioned relative to their owning scene.
 */

typedef struct Floppy144ObjectDefinition
{
    Floppy144ObjectId id;

    const char *name;

    Floppy144SceneId scene;
    Floppy144ObjectId parent;

    int32_t local_x;
    int32_t local_y;

    int32_t draw_layer;

    int32_t collision_x;
    int32_t collision_y;
    int32_t collision_width;
    int32_t collision_height;

    int32_t interaction_x;
    int32_t interaction_y;
    int32_t interaction_width;
    int32_t interaction_height;

    uint32_t flags;
    bool initially_visible;

    const Floppy144ObjectPrimitive *primitives;
    uint32_t primitive_count;
} Floppy144ObjectDefinition;

/*
 * Registry access
 */

const Floppy144ObjectDefinition *Floppy144ObjectGet(
    Floppy144ObjectId object
);

/*
 * Resolve parent-relative coordinates into scene coordinates.
 *
 * Returns false for an invalid ID or a circular parent chain.
 */

bool Floppy144ObjectWorldPosition(
    Floppy144ObjectId object,
    int32_t *world_x,
    int32_t *world_y
);
