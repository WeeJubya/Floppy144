/*
 * FLOPPY//144 transient operator-name editor.
 */

#include "floppy144_profile_edit.h"

#include <stddef.h>
#include <string.h>

/*
 * Reset the transient editor to a known inactive state.
 */
void Floppy144ProfileNameEditReset(
    Floppy144ProfileNameEditState *state
)
{
    if(state == NULL)
    {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );
}

/*
 * Begin an edit by copying, never aliasing, the persistent name.
 */
void Floppy144ProfileNameEditBegin(
    Floppy144ProfileNameEditState *state,
    const Floppy144DiscoveryProfile *profile,
    bool first_time_setup
)
{
    const char *name;
    size_t length;

    if(state == NULL)
    {
        return;
    }

    Floppy144ProfileNameEditReset(
        state
    );

    name =
        Floppy144DiscoveryProfileOperatorName(
            profile
        );

    length =
        strlen(name);

    if(
        length >
        (size_t)FLOPPY144_PROFILE_NAME_MAX_LENGTH
    )
    {
        length =
            (size_t)FLOPPY144_PROFILE_NAME_MAX_LENGTH;
    }

    memcpy(
        state->text,
        name,
        length
    );

    state->text[length] =
        '\0';

    state->length =
        (uint32_t)length;

    state->active =
        1U;

    state->first_time_setup =
        first_time_setup
            ? 1U
            : 0U;

    state->notice =
        FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE;
}

/*
 * Cancel the transient edit while leaving the profile unchanged.
 */
void Floppy144ProfileNameEditCancel(
    Floppy144ProfileNameEditState *state
)
{
    if(state == NULL)
    {
        return;
    }

    state->active =
        0U;

    state->first_time_setup =
        0U;

    state->notice =
        FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE;
}

/*
 * Report whether the transient editor is active.
 */
bool Floppy144ProfileNameEditActive(
    const Floppy144ProfileNameEditState *state
)
{
    return
        state != NULL &&
        state->active != 0U;
}

/*
 * Return the current edit buffer, or an empty string for a missing state.
 */
const char *Floppy144ProfileNameEditText(
    const Floppy144ProfileNameEditState *state
)
{
    if(state == NULL)
    {
        return "";
    }

    return state->text;
}

/*
 * Append one supported ASCII codepoint without overrunning the persistent
 * profile-name capacity.
 */
bool Floppy144ProfileNameEditInputCodepoint(
    Floppy144ProfileNameEditState *state,
    uint32_t codepoint
)
{
    if(
        state == NULL ||
        state->active == 0U
    )
    {
        return false;
    }

    if(
        !Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            codepoint
        )
    )
    {
        state->notice =
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_UNSUPPORTED;

        return false;
    }

    if(
        state->length >=
        FLOPPY144_PROFILE_NAME_MAX_LENGTH
    )
    {
        state->notice =
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_FULL;

        return false;
    }

    state->text[state->length] =
        (char)codepoint;

    ++state->length;

    state->text[state->length] =
        '\0';

    state->notice =
        FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE;

    return true;
}

/*
 * Remove one byte from the ASCII edit buffer.
 */
bool Floppy144ProfileNameEditBackspace(
    Floppy144ProfileNameEditState *state
)
{
    if(
        state == NULL ||
        state->active == 0U ||
        state->length == 0U
    )
    {
        return false;
    }

    --state->length;

    state->text[state->length] =
        '\0';

    state->notice =
        FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE;

    return true;
}

/*
 * Validate the candidate before the application mutates/persists the profile.
 */
bool Floppy144ProfileNameEditReadyToSave(
    Floppy144ProfileNameEditState *state
)
{
    uint32_t index;
    bool has_alphanumeric;

    if(
        state == NULL ||
        state->active == 0U
    )
    {
        return false;
    }

    has_alphanumeric =
        false;

    for(
        index = 0U;
        index < state->length;
        ++index
    )
    {
        uint32_t codepoint =
            (uint32_t)(unsigned char)state->text[index];

        if(
            (
                codepoint >= (uint32_t)'A' &&
                codepoint <= (uint32_t)'Z'
            ) ||
            (
                codepoint >= (uint32_t)'a' &&
                codepoint <= (uint32_t)'z'
            ) ||
            (
                codepoint >= (uint32_t)'0' &&
                codepoint <= (uint32_t)'9'
            )
        )
        {
            has_alphanumeric =
                true;

            break;
        }
    }

    if(
        state->length == 0U ||
        !has_alphanumeric
    )
    {
        state->notice =
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_EMPTY;

        return false;
    }

    if(
        !Floppy144DiscoveryProfileOperatorNameValid(
            state->text
        )
    )
    {
        state->notice =
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_UNSUPPORTED;

        return false;
    }

    state->notice =
        FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE;

    return true;
}

/*
 * Keep the edit active and visible after a profile-file write failure.
 */
void Floppy144ProfileNameEditMarkSaveFailed(
    Floppy144ProfileNameEditState *state
)
{
    if(
        state == NULL ||
        state->active == 0U
    )
    {
        return;
    }

    state->notice =
        FLOPPY144_PROFILE_NAME_EDIT_NOTICE_SAVE_FAILED;
}
