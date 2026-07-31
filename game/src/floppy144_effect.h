/*
 * Floppy//144 - generic game effects
 *
 * Effects describe changes caused by documents, objects and other content.
 * Content declares what should happen, while this module applies the change
 * to persistent world state.
 */

#pragma once

#include "floppy144_world.h"

#include <stdint.h>

/*
 * Supported effect operations
 *
 * The effect type determines how target_id is interpreted. This keeps effect
 * declarations compact while allowing collections, objects and later world
 * systems to share the same processing path.
 */

typedef enum Floppy144EffectType
{
    FLOPPY144_EFFECT_NONE = 0,
    FLOPPY144_EFFECT_FIND_COLLECTION_EVIDENCE,
    FLOPPY144_EFFECT_REVEAL_OBJECT
} Floppy144EffectType;

/*
 * One data-driven world-state operation
 *
 * FIND_COLLECTION_EVIDENCE interprets target_id as Floppy144CollectionId.
 * REVEAL_OBJECT interprets target_id as Floppy144ObjectId.
 */

typedef struct Floppy144Effect
{
    Floppy144EffectType type;
    uint32_t target_id;
} Floppy144Effect;

/*
 * Apply one effect or a sequence of effects.
 */

void Floppy144ApplyEffect(
    Floppy144WorldState *world,
    const Floppy144Effect *effect
);

void Floppy144ApplyEffects(
    Floppy144WorldState *world,
    const Floppy144Effect *effects,
    uint32_t effect_count
);
