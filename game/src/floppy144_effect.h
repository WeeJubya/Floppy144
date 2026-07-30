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
 * The technical slice initially needs only collection evidence. Additional
 * effect types can later reveal objects, change variants, unlock routes and
 * record final-report facts without changing document-handling code.
 */

typedef enum Floppy144EffectType
{
    FLOPPY144_EFFECT_NONE = 0,
    FLOPPY144_EFFECT_FIND_COLLECTION_EVIDENCE
} Floppy144EffectType;

/*
 * One data-driven world-state operation
 *
 * collection identifies the collection affected by the operation. Future
 * effect types may interpret this field differently or add compact arguments.
 */

typedef struct Floppy144Effect
{
    Floppy144EffectType type;
    Floppy144CollectionId collection;
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
