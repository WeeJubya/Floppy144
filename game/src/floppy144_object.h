/*
 * Floppy//144 - reconstructed world-object identifiers
 *
 * IDs are generated from floppy144_objects.def and provide stable array
 * indices for persistent object state.
 */

#pragma once

typedef enum Floppy144ObjectId
{
#define FLOPPY144_OBJECT(symbol)                                   \
    FLOPPY144_OBJECT_##symbol,

#include "floppy144_objects.def"

#undef FLOPPY144_OBJECT

    FLOPPY144_OBJECT_COUNT
} Floppy144ObjectId;
