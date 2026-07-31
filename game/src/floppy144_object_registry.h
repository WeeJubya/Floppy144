/*
 * Floppy//144 - immutable scene-object registry
 *
 * Defines where reconstructed objects belong, how they are drawn and which
 * bounds are consumed by collision and interaction systems.
 */

#pragma once

#include "floppy144_collection.h"
#include "floppy144_object.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum Floppy144SceneId
{
    FLOPPY144_SCENE_OFFICE = 0
} Floppy144SceneId;

typedef enum Floppy144ObjectFlags
{
    FLOPPY144_OBJECT_FLAG_NONE =
        0U,

    FLOPPY144_OBJECT_FLAG_SOLID =
        1U << 0
} Floppy144ObjectFlags;

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

typedef enum Floppy144ObjectAction
{
    FLOPPY144_OBJECT_ACTION_NONE = 0,
    FLOPPY144_OBJECT_ACTION_OPEN_TERMINAL,
    FLOPPY144_OBJECT_ACTION_SHOW_NOTICE
} Floppy144ObjectAction;

typedef struct Floppy144ObjectInteractionDefinition
{
    Floppy144ObjectAction action;

    uint32_t priority;

    Floppy144CollectionId required_evidence_collection;

    const char *prompt;

    Floppy144CollectionId alternate_prompt_collection;
    const char *alternate_prompt;

    const char *notice;
} Floppy144ObjectInteractionDefinition;
typedef enum Floppy144ObjectPrimitiveType
{
    FLOPPY144_OBJECT_PRIMITIVE_FILL_RECT = 0,
    FLOPPY144_OBJECT_PRIMITIVE_RECT,
    FLOPPY144_OBJECT_PRIMITIVE_TEXT
} Floppy144ObjectPrimitiveType;

typedef enum Floppy144ObjectColourRole
{
    FLOPPY144_OBJECT_COLOUR_BODY = 0,
    FLOPPY144_OBJECT_COLOUR_FURNITURE,
    FLOPPY144_OBJECT_COLOUR_DETAIL,
    FLOPPY144_OBJECT_COLOUR_PAPER,
    FLOPPY144_OBJECT_COLOUR_EDGE,
    FLOPPY144_OBJECT_COLOUR_SCREEN,
    FLOPPY144_OBJECT_COLOUR_BACKGROUND,
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
 * Object-attached label
 *
 * The optional restored text replaces the default text when its associated
 * collection has been restored. Position remains relative to the object.
 */

typedef struct Floppy144ObjectLabelDefinition
{
    int32_t x;
    int32_t y;

    Floppy144ObjectColourRole colour_role;

    const char *default_text;

    Floppy144CollectionId restored_collection;
    const char *restored_text;
} Floppy144ObjectLabelDefinition;

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

    Floppy144CollectionId required_collection;

    uint32_t flags;
    bool initially_visible;

    const Floppy144ObjectLabelDefinition *label;

    const Floppy144ObjectInteractionDefinition *interaction;

    const Floppy144ObjectPrimitive *primitives;
    uint32_t primitive_count;
} Floppy144ObjectDefinition;

const Floppy144ObjectDefinition *Floppy144ObjectGet(
    Floppy144ObjectId object
);

bool Floppy144ObjectWorldPosition(
    Floppy144ObjectId object,
    int32_t *world_x,
    int32_t *world_y
);
