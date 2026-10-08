/*
 * Floppy//144 - recovery screen implementation
 *
 * Builds the boot-like recovery interface entirely from drawing primitives.
 * This screen introduces Disk 144, APS-12 and the partial reconstruction.
 */

#include "floppy144_recovery.h"

#include "floppy144_draw.h"

#include <stdio.h>

/*
 * Centred-text helper
 *
 * Measures a string and converts its width into a horizontal start point.
 */

static void Floppy144RecoveryTextCentred(
    Floppy144Surface *surface,
    uint32_t y,
    const char *text,
    uint32_t scale,
    uint32_t colour
)
{
    uint32_t text_width =
        Floppy144DrawTextWidth(text, scale);

    uint32_t x =
        (surface->width - text_width) / 2;

    Floppy144DrawText(
        surface,
        x,
        y,
        text,
        scale,
        colour
    );
}

static const char *const floppy144_main_menu_labels[] =
{
    "INITIATE NEW RECOVERY SESSION",
    "RETURN TO ACTIVE SITE",
    "RECORD CURRENT SESSION",
    "REINSTATE RECORDED SESSION",
    "OPERATOR PROFILE",
    "SETTINGS",
    "TERMINATE RECOVERY ENVIRONMENT"
};

/*
 * Report whether one session-control option is available.
 */

void Floppy144RecoveryFormatCapacity(
    const Floppy144RunState *pState,
    char *pszBuffer,
    uint32_t uBufferCapacity
)
{
    Floppy144RunStateFormatCapacity(
        pState,
        pszBuffer,
        uBufferCapacity
    );
}

bool Floppy144MainMenuOptionEnabled(
    Floppy144MainMenuOption option,
    bool active_session,
    bool recorded_session_available
)
{
    switch(option)
    {
        case FLOPPY144_MAIN_MENU_INITIATE_SESSION:
        case FLOPPY144_MAIN_MENU_OPERATOR_PROFILE:
        case FLOPPY144_MAIN_MENU_SETTINGS:
        case FLOPPY144_MAIN_MENU_TERMINATE:
        {
            return true;
        }

        case FLOPPY144_MAIN_MENU_RETURN_TO_SITE:
        case FLOPPY144_MAIN_MENU_RECORD_SESSION:
        {
            return active_session;
        }

        case FLOPPY144_MAIN_MENU_REINSTATE_SESSION:
        {
            return recorded_session_available;
        }

        case FLOPPY144_MAIN_MENU_OPTION_COUNT:
        {
            return false;
        }
    }

    return false;
}

/*
 * Draw the GDR session-control menu.
 */

void Floppy144MainMenuDraw(
    Floppy144Surface *pSurface,
    Floppy144MainMenuOption selected_option,
    bool active_session,
    bool recorded_session_available,
    const Floppy144RunState *run_state,
    const Floppy144RunState *recorded_run_state,
    const char *persistence_warning,
    const char *menu_notice,
    bool menu_notice_is_warning,
    bool reinstate_confirmation
)
{
    const uint32_t background =
        FLOPPY144_RGB(17, 23, 28);

    const uint32_t panel =
        FLOPPY144_RGB(24, 33, 39);

    const uint32_t panel_dark =
        FLOPPY144_RGB(12, 17, 21);

    const uint32_t border =
        FLOPPY144_RGB(86, 103, 107);

    const uint32_t text =
        FLOPPY144_RGB(202, 211, 205);

    const uint32_t muted =
        FLOPPY144_RGB(118, 133, 132);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    const uint32_t green =
        FLOPPY144_RGB(100, 156, 111);

        const Floppy144RunState *display_state =
        active_session
        ? run_state
        : NULL;

        uint32_t reconstruction_percent =
        display_state != NULL
        ? Floppy144RunStateRecoveredPercent(
            display_state
        )
        : 0U;

        uint32_t progress_width =
        (
            522U *
            reconstruction_percent
        ) /
        100U;

        char session_status[80];
        char reconstruction_text[64];

    const char *status_text;
    uint32_t status_colour;
    uint32_t option_index;

    Floppy144Surface surface;

    if(
        pSurface == NULL ||
        pSurface->pixels == NULL
    )
    {
        return;
    }

    surface = *pSurface;

    if(
        selected_option < 0 ||
        selected_option >=
            FLOPPY144_MAIN_MENU_OPTION_COUNT
    )
    {
        selected_option =
            FLOPPY144_MAIN_MENU_INITIATE_SESSION;
    }

    if(active_session)
    {
        snprintf(
            session_status,
            sizeof(session_status),
                 "SESSION STATUS: ACTIVE"
        );
    }
    else if(
        recorded_session_available &&
        recorded_run_state != NULL
    )
    {
        snprintf(
            session_status,
            sizeof(session_status),
            "RECORDED SESSION: SEED %u",
            (unsigned)recorded_run_state->recovery_seed
        );
    }
    else
    {
        snprintf(
            session_status,
            sizeof(session_status),
                 "SESSION STATUS: NO ACTIVE SESSION"
        );
    }

    Floppy144RecoveryFormatCapacity(
        display_state,
        reconstruction_text,
        (uint32_t)sizeof(reconstruction_text)
    );

    if(menu_notice != NULL)
    {
        status_text =
            menu_notice;

        status_colour =
            menu_notice_is_warning
                ? amber
                : green;
    }
    else if(
        !active_session &&
        persistence_warning != NULL
    )
    {
        status_text =
            persistence_warning;

        status_colour =
            amber;
    }
    else
    {
        status_text =
            session_status;

        status_colour =
            active_session
                ? green
                : (
                    recorded_session_available
                        ? amber
                        : muted
                );
    }

    Floppy144DrawClear(
        &surface,
        background
    );

    Floppy144DrawFillRect(
        &surface,
        0U,
        0U,
        640U,
        16U,
        panel_dark
    );

    Floppy144DrawText(
        &surface,
        10U,
        5U,
        "GDR SESSION CONTROL SYSTEM",
        1U,
        muted
    );

    Floppy144DrawText(
        &surface,
        556U,
        5U,
        "APS-12",
        1U,
        amber
    );

    Floppy144DrawFillRect(
        &surface,
        24U,
        28U,
        592U,
        306U,
        panel
    );

    Floppy144DrawRect(
        &surface,
        24U,
        28U,
        592U,
        306U,
        border
    );

    Floppy144RecoveryTextCentred(
        &surface,
        40U,
        "GOVERNMENT DEPARTMENT OF RECORDS",
        2U,
        text
    );

    Floppy144RecoveryTextCentred(
        &surface,
        66U,
        "SITE RECONSTRUCTION ENVIRONMENT",
        1U,
        muted
    );

    Floppy144DrawFillRect(
        &surface,
        48U,
        88U,
        544U,
        1U,
        border
    );

    Floppy144DrawText(
        &surface,
        56U,
        104U,
        "REMOVABLE MEDIA:",
        1U,
        muted
    );

    Floppy144DrawText(
        &surface,
        196U,
        104U,
        "DISK 144",
        1U,
        text
    );

    Floppy144DrawText(
        &surface,
        56U,
        122U,
        "MEDIA CLASS:",
        1U,
        muted
    );

    Floppy144DrawText(
        &surface,
        196U,
        122U,
        "RECOVERY",
        1U,
        text
    );

    Floppy144DrawText(
        &surface,
        56U,
        140U,
        "PROTOCOL:",
        1U,
        muted
    );

    Floppy144DrawText(
        &surface,
        196U,
        140U,
        "APS-12 PARTIAL SITE",
        1U,
        amber
    );

    Floppy144DrawFillRect(
        &surface,
        48U,
        160U,
        544U,
        1U,
        border
    );

    Floppy144DrawText(
        &surface,
        56U,
        264U,
        status_text,
        1U,
        status_colour
    );

    Floppy144DrawText(
        &surface,
        56U,
        282U,
        reconstruction_text,
        1U,
        text
    );

    Floppy144DrawFillRect(
        &surface,
        56U,
        300U,
        528U,
        14U,
        panel_dark
    );

    Floppy144DrawRect(
        &surface,
        56U,
        300U,
        528U,
        14U,
        border
    );

    Floppy144DrawFillRect(
        &surface,
        59U,
        303U,
        progress_width,
        8U,
        green
    );

    Floppy144DrawFillRect(
        &surface,
        48U,
        252U,
        544U,
        1U,
        border
    );

    for(
        option_index = 0U;
        option_index <
            (uint32_t)FLOPPY144_MAIN_MENU_OPTION_COUNT;
        ++option_index
    )
    {
        Floppy144MainMenuOption option =
            (Floppy144MainMenuOption)option_index;

        const char *label =
            floppy144_main_menu_labels[option_index];

        bool enabled =
            Floppy144MainMenuOptionEnabled(
                option,
                active_session,
                recorded_session_available
            );

        bool selected =
            option == selected_option;

        uint32_t row_y =
            170U +
            option_index * 12U;

        uint32_t label_width =
            Floppy144DrawTextWidth(
                label,
                1U
            );

        uint32_t label_x =
            (
                surface.width -
                label_width
            ) /
            2U;

        if(selected && enabled)
        {
            Floppy144DrawFillRect(
                &surface,
                48U,
                row_y - 4U,
                544U,
                14U,
                panel_dark
            );

            Floppy144DrawText(
                &surface,
                label_x - 14U,
                row_y,
                ">",
                1U,
                amber
            );
        }

        Floppy144DrawText(
            &surface,
            label_x,
            row_y,
            label,
            1U,
            enabled
                ? (
                    selected
                        ? amber
                        : text
                  )
                : muted
        );

        if(!enabled)
        {
            Floppy144DrawText(
                &surface,
                514U,
                row_y,
                "UNAVAILABLE",
                1U,
                muted
            );
        }
    }

    Floppy144RecoveryTextCentred(
        &surface,
        FLOPPY144_UI_FORMAL_FOOTER_Y,
        "UP/DOWN SELECT   ENTER CONFIRM",
        1U,
        muted
    );

    if(reinstate_confirmation)
    {
        char restored_session_text[64];

        /*
         * Successful reinstate remains a state of Session Control rather than
         * a separate modal screen. Reuse the menu's inner margins, panel,
         * dividers and typography so the acknowledgement reads as part of the
         * same GDR system.
         *
         * The normal status/capacity strip is still deliberately hidden until
         * acknowledgement. The existing input handshake then paints one
         * ordinary active-session menu frame, including the loaded percentage,
         * before the matching key-up returns to gameplay.
         */
        if(run_state != NULL)
        {
            (void)snprintf(
                restored_session_text,
                sizeof(restored_session_text),
                "RECOVERY SEED: %u",
                (unsigned)run_state->recovery_seed
            );
        }
        else
        {
            (void)snprintf(
                restored_session_text,
                sizeof(restored_session_text),
                "%s",
                "RECOVERY STATE: REINSTATED"
            );
        }

        Floppy144DrawFillRect(
            &surface,
            48U,
            164U,
            544U,
            154U,
            panel_dark
        );

        Floppy144DrawRect(
            &surface,
            48U,
            164U,
            544U,
            154U,
            border
        );

        Floppy144DrawFillRect(
            &surface,
            56U,
            174U,
            528U,
            24U,
            panel
        );

        Floppy144DrawRect(
            &surface,
            56U,
            174U,
            528U,
            24U,
            green
        );

        Floppy144RecoveryTextCentred(
            &surface,
            180U,
            "SESSION RESTORED",
            2U,
            green
        );

        Floppy144RecoveryTextCentred(
            &surface,
            214U,
            "RECORDED RECOVERY STATE REINSTATED",
            1U,
            text
        );

        Floppy144RecoveryTextCentred(
            &surface,
            234U,
            restored_session_text,
            1U,
            muted
        );

        Floppy144DrawFillRect(
            &surface,
            56U,
            254U,
            528U,
            1U,
            border
        );

        Floppy144RecoveryTextCentred(
            &surface,
            270U,
            "RECOVERY STATUS AVAILABLE AFTER ACKNOWLEDGEMENT",
            1U,
            muted
        );

        Floppy144RecoveryTextCentred(
            &surface,
            296U,
            "PRESS ANY KEY TO CONTINUE",
            1U,
            amber
        );
    }
}
