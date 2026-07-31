/*
 * Floppy//144 - generic game-effect implementation
 *
 * This module translates data-defined effects into persistent world-state
 * changes. Callers do not need to know how those facts are stored.
 */

#include "floppy144_effect.h"

/*
 * Apply one effect
 */

void Floppy144ApplyEffect(
    Floppy144WorldState *world,
    const Floppy144Effect *effect
)
{
    if(
        world == 0 ||
        effect == 0
    )
    {
        return;
    }

    switch(effect->type)
    {
        case FLOPPY144_EFFECT_FIND_COLLECTION_EVIDENCE:
        {
            Floppy144WorldSetCollectionEvidenceFound(
                world,
                (Floppy144CollectionId)effect->target_id,
                true
            );

            break;
        }

        case FLOPPY144_EFFECT_REVEAL_OBJECT:
        {
            Floppy144WorldRevealObject(
                world,
                (Floppy144ObjectId)effect->target_id
            );

            break;
        }

        case FLOPPY144_EFFECT_NONE:
        default:
        {
            break;
        }
    }
}

/*
 * Apply every effect in a registered sequence
 */

void Floppy144ApplyEffects(
    Floppy144WorldState *world,
    const Floppy144Effect *effects,
    uint32_t effect_count
)
{
    uint32_t effect_index;

    if(
        world == 0 ||
        effects == 0
    )
    {
        return;
    }

    for(
        effect_index = 0;
        effect_index < effect_count;
        ++effect_index
    )
    {
        Floppy144ApplyEffect(
            world,
            &effects[effect_index]
        );
    }
}
