/*
 * FLOPPY//144 Stage 4C operator-name entry/edit regression.
 */

#include "floppy144_profile.h"
#include "floppy144_profile_edit.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

typedef struct TestGuardedEditor
{
    Floppy144ProfileNameEditState editor;
    uint32_t guard;
}
TestGuardedEditor;

/*
 * Record one operator-name regression expectation.
 */
static void Expect(
    bool condition,
    const char *label
)
{
    if(!condition)
    {
        ++failures;

        printf(
            "FAIL: %s\n",
            label
        );
    }
}

/*
 * Satisfy profile.c's run-state link dependencies. Name-edit tests never use
 * collection/evidence merge behaviour.
 */
bool Floppy144RunStateCollectionRestored(
    const Floppy144RunState *state,
    Floppy144CollectionId collection
)
{
    (void)state;
    (void)collection;

    return false;
}

/*
 * Satisfy profile.c's evidence query dependency for this focused harness.
 */
bool Floppy144RunStateEvidenceEstablished(
    const Floppy144RunState *state,
    Floppy144EvidenceId evidence
)
{
    (void)state;
    (void)evidence;

    return false;
}

/*
 * Satisfy profile.c's completion-size dependency for this focused harness.
 */
uint32_t Floppy144RunStateRecoveredKb(
    const Floppy144RunState *state
)
{
    (void)state;

    return 0U;
}

/*
 * Feed one ordinary C string through the same codepoint-level editor API used
 * by the platform-neutral text-input route.
 */
static bool TypeText(
    Floppy144ProfileNameEditState *editor,
    const char *text
)
{
    size_t index;

    if(
        editor == NULL ||
        text == NULL
    )
    {
        return false;
    }

    for(
        index = 0U;
        text[index] != '\0';
        ++index
    )
    {
        if(
            !Floppy144ProfileNameEditInputCodepoint(
                editor,
                (uint32_t)(unsigned char)text[index]
            )
        )
        {
            return false;
        }
    }

    return true;
}

/*
 * Verify persistent storage and entry validation rules.
 */
static void TestStorageRules(void)
{
    Floppy144DiscoveryProfile profile;
    char overlong[
        FLOPPY144_PROFILE_NAME_CAPACITY +
        2U
    ];
    uint32_t index;

    _Static_assert(
        FLOPPY144_PROFILE_NAME_CAPACITY == 32U,
        "profile name field size changed"
    );

    _Static_assert(
        FLOPPY144_PROFILE_NAME_MAX_LENGTH == 31U,
        "profile name maximum must reserve one NUL byte"
    );

    Expect(
        Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'G'
        ),
        "uppercase letter is supported"
    );

    Expect(
        Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'y'
        ),
        "lowercase letter is supported"
    );

    Expect(
        Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'7'
        ),
        "digit is supported"
    );

    Expect(
        Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)' '
        ) &&
        Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'\''
        ) &&
        Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'-'
        ) &&
        Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'.'
        ),
        "name punctuation supported by the font is accepted"
    );

    Expect(
        !Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'@'
        ) &&
        !Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            (uint32_t)'\n'
        ) &&
        !Floppy144DiscoveryProfileOperatorNameCharacterSupported(
            0x00E9U
        ),
        "unsupported/control/non-ASCII input is rejected"
    );

    Expect(
        Floppy144DiscoveryProfileOperatorNameValid(
            "Glynn Williams"
        ),
        "mixed-case spaced name is valid"
    );

    Expect(
        Floppy144DiscoveryProfileOperatorNameValid(
            "O'Neil-Smith Jr."
        ),
        "apostrophe hyphen and period are valid"
    );

    Expect(
        !Floppy144DiscoveryProfileOperatorNameValid(
            ""
        ) &&
        !Floppy144DiscoveryProfileOperatorNameValid(
            "   "
        ) &&
        !Floppy144DiscoveryProfileOperatorNameValid(
            "Name@Home"
        ),
        "empty blank and unsupported names are invalid"
    );

    for(
        index = 0U;
        index <
            (uint32_t)sizeof(overlong) -
            1U;
        ++index
    )
    {
        overlong[index] =
            'A';
    }

    overlong[
        sizeof(overlong) -
        1U
    ] =
        '\0';

    Expect(
        !Floppy144DiscoveryProfileOperatorNameValid(
            overlong
        ),
        "overlong name is invalid"
    );

    Floppy144DiscoveryProfileReset(
        &profile
    );

    Expect(
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            "Glynn Williams"
        ),
        "mixed-case operator name stores"
    );

    Expect(
        strcmp(
            Floppy144DiscoveryProfileOperatorName(
                &profile
            ),
            "Glynn Williams"
        ) == 0,
        "storage preserves mixed case"
    );

    Expect(
        Floppy144DiscoveryProfileHasOperatorName(
            &profile
        ),
        "stored name marks profile assigned"
    );

    Expect(
        !Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            "Name@Home"
        ) &&
        strcmp(
            Floppy144DiscoveryProfileOperatorName(
                &profile
            ),
            "Glynn Williams"
        ) == 0,
        "invalid setter input leaves original name unchanged"
    );
}

/*
 * Verify optional first-time setup, short names and profile-level identity.
 */
static void TestFirstTimeEntry(void)
{
    Floppy144DiscoveryProfile profile;
    Floppy144ProfileNameEditState editor;

    Floppy144DiscoveryProfileReset(
        &profile
    );

    Floppy144ProfileNameEditBegin(
        &editor,
        &profile,
        true
    );

    Expect(
        Floppy144ProfileNameEditActive(
            &editor
        ) &&
        editor.first_time_setup != 0U &&
        editor.length == 0U,
        "fresh profile begins optional first-time editor empty"
    );

    Expect(
        TypeText(
            &editor,
            "A"
        ),
        "short operator name types"
    );

    Expect(
        Floppy144ProfileNameEditReadyToSave(
            &editor
        ),
        "short operator name is confirmable"
    );

    Expect(
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            Floppy144ProfileNameEditText(
                &editor
            )
        ),
        "first-time name commits to profile"
    );

    Floppy144ProfileNameEditCancel(
        &editor
    );

    Expect(
        strcmp(
            profile.operator_name,
            "A"
        ) == 0,
        "first-time committed identity remains after editor closes"
    );

    Floppy144DiscoveryProfileBeginRecovery(
        &profile
    );

    Expect(
        strcmp(
            profile.operator_name,
            "A"
        ) == 0 &&
        profile.recovery_sessions_begun == 1U,
        "starting another recovery does not replace profile identity"
    );
}

/*
 * Verify spaces, deletion, editing, cancellation and repeated edits.
 */
static void TestEditingAndCancel(void)
{
    Floppy144DiscoveryProfile profile;
    Floppy144ProfileNameEditState editor;

    Floppy144DiscoveryProfileReset(
        &profile
    );

    Expect(
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            "Glynn Williams"
        ),
        "existing name prepared"
    );

    Floppy144ProfileNameEditBegin(
        &editor,
        &profile,
        false
    );

    Expect(
        strcmp(
            Floppy144ProfileNameEditText(
                &editor
            ),
            "Glynn Williams"
        ) == 0,
        "edit begins with existing name visible"
    );

    Expect(
        Floppy144ProfileNameEditBackspace(
            &editor
        ) &&
        Floppy144ProfileNameEditBackspace(
            &editor
        ),
        "backspace deletes existing characters"
    );

    Expect(
        strcmp(
            Floppy144ProfileNameEditText(
                &editor
            ),
            "Glynn Willia"
        ) == 0,
        "deleted characters leave corrected edit buffer"
    );

    Expect(
        TypeText(
            &editor,
            "m Jones"
        ),
        "editing existing name accepts spaces"
    );

    Expect(
        Floppy144ProfileNameEditReadyToSave(
            &editor
        ) &&
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            Floppy144ProfileNameEditText(
                &editor
            )
        ),
        "edited existing name commits"
    );

    Floppy144ProfileNameEditCancel(
        &editor
    );

    Expect(
        strcmp(
            profile.operator_name,
            "Glynn William Jones"
        ) == 0,
        "edited profile stores corrected value"
    );

    Floppy144ProfileNameEditBegin(
        &editor,
        &profile,
        false
    );

    Expect(
        Floppy144ProfileNameEditBackspace(
            &editor
        ),
        "cancel test changes transient buffer"
    );

    Floppy144ProfileNameEditCancel(
        &editor
    );

    Expect(
        strcmp(
            profile.operator_name,
            "Glynn William Jones"
        ) == 0,
        "cancel leaves original persistent name unchanged"
    );

    Floppy144ProfileNameEditBegin(
        &editor,
        &profile,
        false
    );

    while(editor.length > 0U)
    {
        (void)Floppy144ProfileNameEditBackspace(
            &editor
        );
    }

    Expect(
        TypeText(
            &editor,
            "Second Operator"
        ) &&
        Floppy144ProfileNameEditReadyToSave(
            &editor
        ) &&
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            Floppy144ProfileNameEditText(
                &editor
            )
        ),
        "repeated edit can replace the full name"
    );

    Floppy144ProfileNameEditCancel(
        &editor
    );

    Expect(
        strcmp(
            profile.operator_name,
            "Second Operator"
        ) == 0,
        "repeated edit persists the latest identity"
    );
}

/*
 * Verify the 31-character capacity and unsupported input cannot overrun state.
 */
static void TestCapacityAndInvalidInput(void)
{
    TestGuardedEditor guarded;
    Floppy144DiscoveryProfile profile;
    const char *maximum_name =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ12345";
    uint32_t index;

    Floppy144DiscoveryProfileReset(
        &profile
    );

    memset(
        &guarded,
        0,
        sizeof(guarded)
    );

    guarded.guard =
        0xA144BEEFU;

    Floppy144ProfileNameEditBegin(
        &guarded.editor,
        &profile,
        true
    );

    Expect(
        strlen(maximum_name) ==
            (size_t)FLOPPY144_PROFILE_NAME_MAX_LENGTH,
        "maximum-length test name is exactly 31 characters"
    );

    Expect(
        TypeText(
            &guarded.editor,
            maximum_name
        ),
        "maximum-length name fills the editor"
    );

    Expect(
        guarded.editor.length ==
            FLOPPY144_PROFILE_NAME_MAX_LENGTH &&
        guarded.editor.text[
            FLOPPY144_PROFILE_NAME_MAX_LENGTH
        ] == '\0',
        "maximum-length name remains NUL terminated"
    );

    for(
        index = 0U;
        index < 64U;
        ++index
    )
    {
        (void)Floppy144ProfileNameEditInputCodepoint(
            &guarded.editor,
            (uint32_t)'Z'
        );
    }

    Expect(
        guarded.editor.length ==
            FLOPPY144_PROFILE_NAME_MAX_LENGTH &&
        guarded.editor.notice ==
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_FULL &&
        guarded.guard ==
            0xA144BEEFU,
        "input beyond capacity is rejected without buffer overrun"
    );

    Expect(
        Floppy144ProfileNameEditReadyToSave(
            &guarded.editor
        ) &&
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            Floppy144ProfileNameEditText(
                &guarded.editor
            )
        ),
        "maximum-length name can be committed"
    );

    Expect(
        strlen(
            profile.operator_name
        ) ==
            (size_t)FLOPPY144_PROFILE_NAME_MAX_LENGTH,
        "maximum-length stored name remains bounded"
    );

    Floppy144ProfileNameEditBegin(
        &guarded.editor,
        &profile,
        false
    );

    Expect(
        !Floppy144ProfileNameEditInputCodepoint(
            &guarded.editor,
            (uint32_t)'@'
        ) &&
        guarded.editor.notice ==
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_UNSUPPORTED,
        "unsupported printable character is ignored safely"
    );

    Expect(
        !Floppy144ProfileNameEditInputCodepoint(
            &guarded.editor,
            0x00E9U
        ) &&
        guarded.guard ==
            0xA144BEEFU,
        "unsupported non-ASCII codepoint is ignored safely"
    );

    while(guarded.editor.length > 0U)
    {
        (void)Floppy144ProfileNameEditBackspace(
            &guarded.editor
        );
    }

    Expect(
        !Floppy144ProfileNameEditReadyToSave(
            &guarded.editor
        ) &&
        guarded.editor.notice ==
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_EMPTY,
        "empty edited name cannot be confirmed"
    );

    Expect(
        TypeText(
            &guarded.editor,
            "   "
        ),
        "spaces are accepted as editable characters"
    );

    Expect(
        !Floppy144ProfileNameEditReadyToSave(
            &guarded.editor
        ),
        "whitespace-only name cannot be confirmed"
    );
}

/*
 * Run the complete Stage 4C operator-name regression.
 */
int main(void)
{
    TestStorageRules();
    TestFirstTimeEntry();
    TestEditingAndCancel();
    TestCapacityAndInvalidInput();

    if(failures != 0)
    {
        printf(
            "STAGE 4C OPERATOR NAME TESTS: FAIL (%d)\n",
            failures
        );

        return 1;
    }

    printf(
        "STAGE 4C OPERATOR NAME TESTS: PASS\n"
    );

    return 0;
}
