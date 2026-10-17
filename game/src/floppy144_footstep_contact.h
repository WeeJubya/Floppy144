#ifndef FLOPPY144_FOOTSTEP_CONTACT_H
#define FLOPPY144_FOOTSTEP_CONTACT_H

#include <stdbool.h>

/* One immediate shoe contact per uninterrupted successful movement sequence.
 * Later contacts remain owned by the unchanged PlayerVisualAdvance() gait.
 * Keep policy independent of Win32/audio for focused regression tests. */
typedef struct Floppy144FootstepContactState
{
    bool initial_contact_played;
} Floppy144FootstepContactState;

static void Floppy144FootstepContactReset(
    Floppy144FootstepContactState *state
)
{
    if(state!=NULL) state->initial_contact_played=false;
}

static bool Floppy144FootstepContactOnAttempt(
    Floppy144FootstepContactState *state,
    bool displaced
)
{
    if(state==NULL) return false;
    if(!displaced)
    {
        Floppy144FootstepContactReset(state);
        return false;
    }
    if(state->initial_contact_played) return false;
    state->initial_contact_played=true;
    return true;
}

#endif
