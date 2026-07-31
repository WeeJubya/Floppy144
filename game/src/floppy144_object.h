/*
 * Floppy//144 - reconstructed world-object identifiers
 *
 * IDs are generated from floppy144_objects.def and provide stable array
 * indices for persistent object state.
 */

#pragma once

typedef enum Floppy144ObjectId
{
    FLOPPY144_OBJECT_NONE = -1,

#define FLOPPY144_OBJECT(                                          \
    symbol,                                                        \
    scene,                                                         \
    parent,                                                        \
    local_x,                                                       \
    local_y,                                                       \
    collision_x,                                                   \
    collision_y,                                                   \
    collision_width,                                               \
    collision_height,                                              \
    flags,                                                         \
    initially_visible,                                             \
    primitives                                                    \
)                                                                  \
    FLOPPY144_OBJECT_##symbol,

#include "floppy144_objects.def"

#undef FLOPPY144_OBJECT

    FLOPPY144_OBJECT_COUNT
} Floppy144ObjectId;
