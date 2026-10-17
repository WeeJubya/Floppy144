/* BF24 initial footfall must follow real displacement, not attempted input. */
#include <stdio.h>
#include "floppy144_footstep_contact.h"

int main(void)
{
    Floppy144FootstepContactState state={false};
    if(Floppy144FootstepContactOnAttempt(&state,false)) return 1; /* wall */
    if(!Floppy144FootstepContactOnAttempt(&state,true)) return 2; /* first press */
    if(Floppy144FootstepContactOnAttempt(&state,true)) return 3; /* held repeat */
    if(Floppy144FootstepContactOnAttempt(&state,true)) return 4; /* changed direction */
    Floppy144FootstepContactReset(&state); /* all movement keys released */
    if(!Floppy144FootstepContactOnAttempt(&state,true)) return 5; /* restart */
    if(Floppy144FootstepContactOnAttempt(&state,false)) return 6; /* blocked */
    if(!Floppy144FootstepContactOnAttempt(&state,true)) return 7; /* move again */
    if(Floppy144FootstepContactOnAttempt(NULL,true)) return 8;
    Floppy144FootstepContactReset(NULL);
    puts("BF24 INITIAL FOOTSTEP: immediate displacement; no repeat; release and collision reset PASS");
    return 0;
}
