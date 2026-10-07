/*
 * FLOPPY//144 persistent operator-profile screen.
 */

#include "floppy144_profile_view.h"

#include <stddef.h>
#include <stdio.h>

/*
 * Return the player-facing label for the stored operator body configuration.
 */
static const char *Floppy144ProfileViewBodyStyleText(
    const Floppy144DiscoveryProfile *profile
)
{
    if(profile == NULL)
    {
        return "UNASSIGNED";
    }

    switch((Floppy144OperatorBodyStyle)profile->body_style)
    {
        case FLOPPY144_OPERATOR_BODY_STYLE_A:
        {
            return "TYPE A";
        }

        case FLOPPY144_OPERATOR_BODY_STYLE_B:
        {
            return "TYPE B";
        }

        case FLOPPY144_OPERATOR_BODY_STYLE_COUNT:
        {
            break;
        }
    }

    return "UNASSIGNED";
}

/*
 * Return the player-facing outcome text for the latest completed recovery.
 */
static const char *Floppy144ProfileViewCompletionOutcomeText(
    const Floppy144DiscoveryProfile *profile
)
{
    uint8_t flags;

    if(
        profile == NULL ||
        profile->completed_recoveries == 0U
    )
    {
        return "NO COMPLETED RECOVERY ON FILE";
    }

    flags =
        profile->latest_completion_flags;

    if(
        (
            flags &
            FLOPPY144_PROFILE_COMPLETION_EVIDENCE_RESOLVED
        ) != 0U &&
        (
            flags &
            FLOPPY144_PROFILE_COMPLETION_CAPACITY_EXHAUSTED
        ) != 0U
    )
    {
        return "EVIDENCE + CAPACITY";
    }

    if(
        (
            flags &
            FLOPPY144_PROFILE_COMPLETION_EVIDENCE_RESOLVED
        ) != 0U
    )
    {
        return "EVIDENCE RESOLVED";
    }

    if(
        (
            flags &
            FLOPPY144_PROFILE_COMPLETION_CAPACITY_EXHAUSTED
        ) != 0U
    )
    {
        return "CAPACITY EXHAUSTED";
    }

    return "RECOVERY CLOSED";
}

/*
 * Return the editor/status line shown above the profile footer.
 */
static const char *Floppy144ProfileViewNameEditStatusText(
    const Floppy144ProfileNameEditState *name_edit
)
{
    if(
        name_edit == NULL ||
        !Floppy144ProfileNameEditActive(
            name_edit
        )
    )
    {
        return "PROFILE HISTORY IS RETAINED BETWEEN RECOVERIES";
    }

    switch((Floppy144ProfileNameEditNotice)name_edit->notice)
    {
        case FLOPPY144_PROFILE_NAME_EDIT_NOTICE_EMPTY:
        {
            return "NAME REQUIRED - TYPE A NAME OR PRESS ESC TO CANCEL";
        }

        case FLOPPY144_PROFILE_NAME_EDIT_NOTICE_FULL:
        {
            return "NAME LIMIT REACHED - MAXIMUM 31 CHARACTERS";
        }

        case FLOPPY144_PROFILE_NAME_EDIT_NOTICE_UNSUPPORTED:
        {
            return "USE LETTERS, NUMBERS, SPACE, APOSTROPHE, HYPHEN OR PERIOD";
        }

        case FLOPPY144_PROFILE_NAME_EDIT_NOTICE_SAVE_FAILED:
        {
            return "PROFILE COULD NOT BE SAVED - ENTER RETRIES / ESC CANCELS";
        }

        case FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE:
        {
            return
                name_edit->first_time_setup != 0U
                    ? "NEW OPERATOR SETUP - ENTER SAVES / ESC CANCELS"
                    : "EDIT OPERATOR NAME - ENTER SAVES / ESC CANCELS";
        }
    }

    return "";
}

/*
 * Draw the persistent GDR operator record without mutating profile history.
 */
void Floppy144ProfileViewDraw(
    Floppy144Surface *surface,
    const Floppy144DiscoveryProfile *profile,
    const Floppy144ProfileNameEditState *name_edit
)
{
    const uint32_t background =
        FLOPPY144_RGB(
            17,
            23,
            28
        );

    const uint32_t panel =
        FLOPPY144_RGB(
            24,
            33,
            39
        );

    const uint32_t panel_dark =
        FLOPPY144_RGB(
            12,
            17,
            21
        );

    const uint32_t border =
        FLOPPY144_RGB(
            86,
            103,
            107
        );

    const uint32_t text =
        FLOPPY144_RGB(
            202,
            211,
            205
        );

    const uint32_t muted =
        FLOPPY144_RGB(
            118,
            133,
            132
        );

    const uint32_t amber =
        FLOPPY144_RGB(
            194,
            153,
            76
        );

    const uint32_t green =
        FLOPPY144_RGB(
            100,
            156,
            111
        );

    const char *operator_name;
    const char *body_style;
    const char *completion_outcome;
    const char *edit_status;
    bool editing_name;
    uint32_t collection_count;
    uint32_t evidence_count;
    uint32_t title_width;
    char edit_name[FLOPPY144_PROFILE_NAME_CAPACITY + 2U];
    char line[96];

    if(
        surface == NULL ||
        surface->pixels == NULL ||
        profile == NULL
    )
    {
        return;
    }

    editing_name =
        Floppy144ProfileNameEditActive(
            name_edit
        );

    if(editing_name)
    {
        (void)snprintf(
            edit_name,
            sizeof(edit_name),
            "%s_",
            Floppy144ProfileNameEditText(
                name_edit
            )
        );

        operator_name =
            edit_name;
    }
    else
    {
        operator_name =
            Floppy144DiscoveryProfileHasOperatorName(
                profile
            )
                ? Floppy144DiscoveryProfileOperatorName(
                    profile
                )
                : "UNASSIGNED";
    }

    edit_status =
        Floppy144ProfileViewNameEditStatusText(
            name_edit
        );

    body_style =
        Floppy144ProfileViewBodyStyleText(
            profile
        );

    completion_outcome =
        Floppy144ProfileViewCompletionOutcomeText(
            profile
        );

    collection_count =
        Floppy144DiscoveryProfileCollectionsEverRestoredCount(
            profile
        );

    evidence_count =
        Floppy144DiscoveryProfileEvidenceEverEstablishedCount(
            profile
        );

    Floppy144DrawClear(
        surface,
        background
    );

    Floppy144DrawFillRect(
        surface,
        0U,
        0U,
        640U,
        16U,
        panel_dark
    );

    Floppy144DrawText(
        surface,
        10U,
        5U,
        "GDR PERSONNEL RECORD",
        1U,
        muted
    );

    Floppy144DrawText(
        surface,
        556U,
        5U,
        "APS-12",
        1U,
        amber
    );

    Floppy144DrawFillRect(
        surface,
        24U,
        28U,
        592U,
        306U,
        panel
    );

    Floppy144DrawRect(
        surface,
        24U,
        28U,
        592U,
        306U,
        border
    );

    title_width =
        Floppy144DrawTextWidth(
            "GDR OPERATOR PROFILE",
            2U
        );

    Floppy144DrawText(
        surface,
        (
            surface->width -
            title_width
        ) /
        2U,
        40U,
        "GDR OPERATOR PROFILE",
        2U,
        text
    );

    Floppy144DrawFillRect(
        surface,
        48U,
        72U,
        544U,
        1U,
        border
    );

    Floppy144DrawText(
        surface,
        56U,
        86U,
        "OPERATOR",
        1U,
        muted
    );

    if(editing_name)
    {
        Floppy144DrawRect(
            surface,
            50U,
            96U,
            246U,
            18U,
            amber
        );
    }

    Floppy144DrawText(
        surface,
        56U,
        102U,
        operator_name,
        1U,
        editing_name ||
        Floppy144DiscoveryProfileHasOperatorName(
            profile
        )
            ? amber
            : muted
    );

    if(!editing_name)
    {
        Floppy144DrawText(
            surface,
            56U,
            114U,
            "ENTER  EDIT NAME",
            1U,
            amber
        );
    }

    Floppy144DrawText(
        surface,
        340U,
        86U,
        "BODY CONFIGURATION",
        1U,
        muted
    );

    Floppy144DrawText(
        surface,
        340U,
        102U,
        body_style,
        1U,
        text
    );

    Floppy144DrawFillRect(
        surface,
        48U,
        124U,
        544U,
        1U,
        border
    );

    Floppy144DrawText(
        surface,
        56U,
        138U,
        "RECOVERY HISTORY",
        1U,
        amber
    );

    (void)snprintf(
        line,
        sizeof(line),
        "SESSIONS UNDERTAKEN ........ %u",
        (unsigned)profile->recovery_sessions_begun
    );

    Floppy144DrawText(
        surface,
        56U,
        156U,
        line,
        1U,
        text
    );

    (void)snprintf(
        line,
        sizeof(line),
        "COLLECTIONS RESTORED ....... %u / %u",
        (unsigned)collection_count,
        (unsigned)FLOPPY144_COLLECTION_COUNT
    );

    Floppy144DrawText(
        surface,
        56U,
        172U,
        line,
        1U,
        text
    );

    (void)snprintf(
        line,
        sizeof(line),
        "EVIDENCE ESTABLISHED ....... %u / %u",
        (unsigned)evidence_count,
        (unsigned)FLOPPY144_EVIDENCE_COUNT
    );

    Floppy144DrawText(
        surface,
        56U,
        188U,
        line,
        1U,
        text
    );

    (void)snprintf(
        line,
        sizeof(line),
        "COMPLETED RECOVERIES ....... %u",
        (unsigned)profile->completed_recoveries
    );

    Floppy144DrawText(
        surface,
        56U,
        204U,
        line,
        1U,
        text
    );

    Floppy144DrawFillRect(
        surface,
        48U,
        226U,
        544U,
        1U,
        border
    );

    Floppy144DrawText(
        surface,
        56U,
        240U,
        "LATEST COMPLETION",
        1U,
        amber
    );

    if(profile->completed_recoveries == 0U)
    {
        Floppy144DrawText(
            surface,
            56U,
            260U,
            completion_outcome,
            1U,
            muted
        );
    }
    else
    {
        (void)snprintf(
            line,
            sizeof(line),
            "EVIDENCE ESTABLISHED ....... %u%%",
            (unsigned)profile->latest_completion_evidence_percent
        );

        Floppy144DrawText(
            surface,
            56U,
            258U,
            line,
            1U,
            text
        );

        (void)snprintf(
            line,
            sizeof(line),
            "RECOVERED DATA ............. %u KB",
            (unsigned)profile->latest_completion_recovered_kb
        );

        Floppy144DrawText(
            surface,
            56U,
            274U,
            line,
            1U,
            text
        );

        (void)snprintf(
            line,
            sizeof(line),
            "OUTCOME .................... %s",
            completion_outcome
        );

        Floppy144DrawText(
            surface,
            56U,
            290U,
            line,
            1U,
            green
        );
    }

    Floppy144DrawText(
        surface,
        56U,
        316U,
        edit_status,
        1U,
        editing_name &&
        name_edit != NULL &&
        name_edit->notice !=
            FLOPPY144_PROFILE_NAME_EDIT_NOTICE_NONE
            ? amber
            : muted
    );

    Floppy144DrawText(
        surface,
        10U,
        346U,
        editing_name
            ? "ENTER  SAVE   ESC  CANCEL   BACKSPACE  DELETE"
            : "ENTER  EDIT NAME   BACKSPACE  BACK",
        1U,
        muted
    );
}
