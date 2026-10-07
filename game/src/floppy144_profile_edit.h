/*
 * FLOPPY//144 transient operator-name editor.
 *
 * This state is deliberately not persistent. It keeps an editable working
 * copy of the profile name so cancel can leave the stored identity untouched.
 */

#pragma once

#include "floppy144_profile.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum Floppy144ProfileNameEditNotice
{
    FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE = 0,
    FLOPPY144_PROFILE_NAME_EDIT_NOTICE_EMPTY,
    FLOPPY144_PROFILE_NAME_EDIT_NOTICE_FULL,
    FLOPPY144_PROFILE_NAME_EDIT_NOTICE_UNSUPPORTED,
    FLOPPY144_PROFILE_NAME_EDIT_NOTICE_SAVE_FAILED
}
Floppy144ProfileNameEditNotice;

typedef struct Floppy144ProfileNameEditState
{
    char text[
        FLOPPY144_PROFILE_NAME_CAPACITY
    ];

    uint32_t length;
    uint8_t active;
    uint8_t first_time_setup;
    uint8_t notice;
}
Floppy144ProfileNameEditState;

/*
 * Reset the transient editor to an inactive empty state.
 */
void Floppy144ProfileNameEditReset(
    Floppy144ProfileNameEditState *state
);

/*
 * Begin editing from the profile's current persistent name.
 */
void Floppy144ProfileNameEditBegin(
    Floppy144ProfileNameEditState *state,
    const Floppy144DiscoveryProfile *profile,
    bool first_time_setup
);

/*
 * Cancel the current edit without mutating the profile.
 */
void Floppy144ProfileNameEditCancel(
    Floppy144ProfileNameEditState *state
);

/*
 * Report whether name editing currently owns profile-screen input.
 */
bool Floppy144ProfileNameEditActive(
    const Floppy144ProfileNameEditState *state
);

/*
 * Return the current transient edit buffer.
 */
const char *Floppy144ProfileNameEditText(
    const Floppy144ProfileNameEditState *state
);

/*
 * Accept one platform-neutral text codepoint when it is safe for the current
 * bitmap font and there is room in the persistent profile field.
 */
bool Floppy144ProfileNameEditInputCodepoint(
    Floppy144ProfileNameEditState *state,
    uint32_t codepoint
);

/*
 * Delete one character from the transient edit buffer.
 */
bool Floppy144ProfileNameEditBackspace(
    Floppy144ProfileNameEditState *state
);

/*
 * Validate the transient buffer before the application commits it.
 */
bool Floppy144ProfileNameEditReadyToSave(
    Floppy144ProfileNameEditState *state
);

/*
 * Record a persistence failure without discarding the player's edit buffer.
 */
void Floppy144ProfileNameEditMarkSaveFailed(
    Floppy144ProfileNameEditState *state
);
