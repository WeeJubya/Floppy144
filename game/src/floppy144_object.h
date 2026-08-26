/*
 * Floppy//144 - reconstructed world-object identifiers
 *
 * IDs are generated from floppy144_objects.def and provide stable array
 * indices for persistent object state.
 */

#pragma once

typedef enum Floppy144ObjectAccessState
{
    FLOPPY144_OBJECT_ACCESS_NONE = 0,
    FLOPPY144_OBJECT_ACCESS_LOCKED,
    FLOPPY144_OBJECT_ACCESS_UNLOCKED,
    FLOPPY144_OBJECT_ACCESS_OPEN
}
Floppy144ObjectAccessState;

typedef enum Floppy144ObjectId
{
    FLOPPY144_OBJECT_NONE = -1,

#define FLOPPY144_OBJECT(                                          \
    symbol,                                                        \
    name,                                                          \
    scene,                                                         \
    parent,                                                        \
    local_x,                                                       \
    local_y,                                                       \
    draw_layer,                                                    \
    collision_x,                                                   \
    collision_y,                                                   \
    collision_width,                                               \
    collision_height,                                              \
    interaction_x,                                                 \
    interaction_y,                                                 \
    interaction_width,                                             \
    interaction_height,                                            \
    required_collection,                                          \
    flags,                                                         \
    initially_visible,                                             \
    label,                                                         \
    interaction,                                                   \
    primitives                                                     \
)                                                                  \
    FLOPPY144_OBJECT_##symbol,

#include "floppy144_objects.def"

#undef FLOPPY144_OBJECT

    FLOPPY144_OBJECT_COUNT
} Floppy144ObjectId;
