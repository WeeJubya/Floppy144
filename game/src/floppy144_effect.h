/*
 * Floppy//144 - generic game effects
 *
 * Effects describe changes caused by documents, objects and other content.
 * Content declares what should happen, while this module applies the change
 * to persistent world state.
 */

#pragma once

#include "floppy144_world.h"
#include "floppy144_run_state.h"

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

    FLOPPY144_EFFECT_ESTABLISH_EVIDENCE,
    FLOPPY144_EFFECT_REVEAL_OBJECT,

    FLOPPY144_EFFECT_RECONSTRUCT_ROOM,
    FLOPPY144_EFFECT_UNLOCK_OBJECT,
    FLOPPY144_EFFECT_OPEN_OBJECT,
    FLOPPY144_EFFECT_RESTORE_COLLECTION,
    FLOPPY144_EFFECT_GRANT_CAPABILITY,
    FLOPPY144_EFFECT_SET_PROJECTION,
    FLOPPY144_EFFECT_SET_BRANCH,
    FLOPPY144_EFFECT_COMPLETE_INTERACTION,
}
Floppy144EffectType;

/*
 * target_id interpretation:
 *
 * ESTABLISH_EVIDENCE  -> Floppy144EvidenceId
 * REVEAL_OBJECT       -> Floppy144ObjectId
 * RECONSTRUCT_ROOM    -> Floppy144RoomId
 * UNLOCK_OBJECT       -> Floppy144ObjectId
 * OPEN_OBJECT         -> Floppy144ObjectId
 * RESTORE_COLLECTION  -> Floppy144CollectionId
 * GRANT_CAPABILITY    -> Floppy144CapabilityId
 * SET_PROJECTION      -> Floppy144Projection
 * SET_BRANCH          -> Floppy144RunBranch
 * COMPLETE_INTERACTION -> Floppy144InteractionId
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
    Floppy144RunState *run_state,
    const Floppy144Effect *effect
);

void Floppy144ApplyEffects(
    Floppy144WorldState *world,
    Floppy144RunState *run_state,
    const Floppy144Effect *effects,
    uint32_t effect_count
);
