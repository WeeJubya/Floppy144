/*
 * Floppy//144 - generic game-effect implementation
 *
 * This module translates data-defined effects into persistent world-state
 * changes. Callers do not need to know how those facts are stored.
 */

#include "floppy144_effect.h"
#include <stddef.h>

/*
 * Apply one effect
 */

void Floppy144ApplyEffect(
    Floppy144WorldState *world,
    Floppy144RunState *run_state,
    const Floppy144Effect *effect
)
{
    if(
        world == NULL ||
        run_state == NULL ||
        effect == NULL
    )
    {
        return;
    }

    switch(effect->type)
    {
        case FLOPPY144_EFFECT_ESTABLISH_EVIDENCE:
        {
            Floppy144RunStateEstablishEvidence(
                run_state,
                (Floppy144EvidenceId)effect->target_id
            );

            break;
        }

        case FLOPPY144_EFFECT_REVEAL_OBJECT:
        {
            Floppy144ObjectId object =
            (Floppy144ObjectId)effect->target_id;

            /*
             * RunState is authoritative. WorldState mirrors visibility for
             * the currently hydrated runtime.
             */
            Floppy144RunStateRevealObject(
                run_state,
                object
            );

            Floppy144WorldRevealObject(
                world,
                object
            );

            break;
        }

        case FLOPPY144_EFFECT_RECONSTRUCT_ROOM:
        {
            Floppy144RunStateReconstructRoom(
                run_state,
                (Floppy144RoomId)effect->target_id
            );

            break;
        }

        case FLOPPY144_EFFECT_UNLOCK_OBJECT:
        {
            Floppy144RunStateSetObjectAccessState(
                run_state,
                (Floppy144ObjectId)effect->target_id,
                                                  FLOPPY144_OBJECT_ACCESS_UNLOCKED
            );

            break;
        }

        case FLOPPY144_EFFECT_OPEN_OBJECT:
        {
            Floppy144RunStateSetObjectAccessState(
                run_state,
                (Floppy144ObjectId)effect->target_id,
                                                  FLOPPY144_OBJECT_ACCESS_OPEN
            );

            break;
        }

        case FLOPPY144_EFFECT_RESTORE_COLLECTION:
        {
            Floppy144CollectionId collection =
            (Floppy144CollectionId)effect->target_id;

            /*
             * RunState remains authoritative while WorldState mirrors the
             * restored collection for the currently active runtime.
             */
            if(
                Floppy144RunStateRestoreCollection(
                    run_state,
                    collection
                )
            )
            {
                Floppy144WorldRestoreCollection(
                    world,
                    collection
                );
            }

            break;
        }

        case FLOPPY144_EFFECT_GRANT_CAPABILITY:
        {
            Floppy144RunStateGrantCapability(
                run_state,
                (Floppy144CapabilityId)effect->target_id
            );

            break;
        }

        case FLOPPY144_EFFECT_SET_PROJECTION:
        {
            Floppy144RunStateSetProjection(
                run_state,
                (Floppy144Projection)effect->target_id
            );

            break;
        }

        case FLOPPY144_EFFECT_SET_BRANCH:
        {
            Floppy144RunStateSetBranch(
                run_state,
                (Floppy144RunBranch)effect->target_id
            );

            break;
        }

        case FLOPPY144_EFFECT_COMPLETE_INTERACTION:
        {
            Floppy144RunStateCompleteInteraction(
                run_state,
                (Floppy144InteractionId)effect->target_id
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
    Floppy144RunState *run_state,
    const Floppy144Effect *effects,
    uint32_t effect_count
)
{
    uint32_t effect_index;

    if(
        world == 0 ||
        run_state == 0 ||
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
            run_state,
            &effects[effect_index]
        );
    }
}
