/*
 * FLOPPY//144 platform-neutral application lifecycle state.
 */

#include "floppy144_lifecycle.h"

#include <stddef.h>
#include <string.h>

void Floppy144LifecycleReset(
    Floppy144LifecycleState *state
)
{
    if(state != NULL)
    {
        memset(state,0,sizeof(*state));
    }
}

void Floppy144LifecycleApply(
    Floppy144LifecycleState *state,
    const F144LifecycleEvent *event
)
{
    if(state == NULL || event == NULL)
    {
        return;
    }

    switch(event->type)
    {
        case F144_LIFECYCLE_START:
        {
            state->started = 1U;
            state->active = 1U;
            state->suspended = 0U;
            break;
        }

        case F144_LIFECYCLE_ACTIVE:
        {
            state->active = 1U;
            break;
        }

        case F144_LIFECYCLE_INACTIVE:
        {
            state->active = 0U;
            break;
        }

        case F144_LIFECYCLE_SUSPEND:
        {
            state->suspended = 1U;
            break;
        }

        case F144_LIFECYCLE_RESUME:
        {
            state->suspended = 0U;
            state->active = 1U;
            break;
        }

        case F144_LIFECYCLE_SHUTDOWN_REQUESTED:
        {
            state->shutdown_requested = 1U;
            break;
        }

        case F144_LIFECYCLE_SHUTDOWN:
        {
            state->shutdown_complete = 1U;
            state->active = 0U;
            break;
        }

        case F144_LIFECYCLE_NONE:
        case F144_LIFECYCLE_COUNT:
        default:
        {
            break;
        }
    }
}
