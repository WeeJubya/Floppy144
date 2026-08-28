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

/*
 * Draw the recovery screen
 *
 * The function derives dynamic labels first, wraps the F144 backbuffer
 * as a Floppy144Surface, then paints the interface from back to front.
 */

/*
 * Draw the opening Floppy//144 splash
 *
 * The disk uses only rectangles and the embedded font. Its ascent uses an
 * integer ease-out followed by a small settling movement.
 */

void Floppy144SplashDraw(
    F144Runtime *engine,
    uint32_t elapsed_milliseconds
)
{
    const uint32_t background =
        FLOPPY144_RGB(7, 10, 12);

    const uint32_t text =
        FLOPPY144_RGB(202, 211, 205);

    const uint32_t muted =
        FLOPPY144_RGB(118, 133, 132);

    const uint32_t amber =
        FLOPPY144_RGB(194, 153, 76);

    const uint32_t disk_body =
        FLOPPY144_RGB(38, 43, 46);

    const uint32_t disk_inner =
        FLOPPY144_RGB(25, 29, 31);

    const uint32_t disk_edge =
        FLOPPY144_RGB(104, 113, 115);

    const uint32_t disk_label =
        FLOPPY144_RGB(191, 193, 183);

    const uint32_t disk_label_text =
        FLOPPY144_RGB(49, 54, 53);

    const uint32_t disk_metal =
        FLOPPY144_RGB(151, 158, 157);

    const uint32_t disk_metal_light =
        FLOPPY144_RGB(193, 198, 195);

    const char *title =
        "FLOPPY//144";

    const char *question =
        "WHAT VERSION OF THE TRUTH ARE YOU WILLING TO ACCEPT?";

    const uint32_t disk_x =
        256U;

    const uint32_t disk_start_y =
        372U;

    const uint32_t disk_target_y =
        48U;

    const uint32_t disk_width =
        128U;

    const uint32_t disk_height =
        112U;

    const uint32_t disk_flight_start =
        0U;

    const uint32_t disk_flight_duration =
        1000U;

    const uint32_t title_start =
        1100U;

    const uint32_t title_duration =
        700U;

    const uint32_t media_start =
        1950U;

    const uint32_t question_start =
        2300U;

    const uint32_t question_duration =
        1200U;

    const uint32_t prompt_start =
        3600U;

    uint32_t disk_y =
        disk_start_y;

    uint32_t title_x;
    uint32_t title_target_x;
    uint32_t title_width;

    uint32_t question_x;
    uint32_t question_target_x;
    uint32_t question_width;

    Floppy144Surface surface =
    {
        (uint32_t *)engine->backbuffer.data,
        engine->backbuffer.width,
        engine->backbuffer.height
    };

    /*
     * Smoothstep disk flight.
     *
     * This starts gently, reaches full speed near the centre and eases into
     * position without the abrupt initial leap produced by the old ease-out.
     */

    if(
        elapsed_milliseconds >=
            disk_flight_start &&
        elapsed_milliseconds <
            disk_flight_start +
            disk_flight_duration
    )
    {
        uint64_t time =
            (uint64_t)(
                elapsed_milliseconds -
                disk_flight_start
            );

        uint64_t duration =
            (uint64_t)disk_flight_duration;

        uint64_t smooth_time =
            time *
            time *
            (
                3U * duration -
                2U * time
            );

        uint64_t duration_cube =
            duration *
            duration *
            duration;

        uint32_t travelled =
            (uint32_t)(
                (
                    (uint64_t)(
                        disk_start_y -
                        disk_target_y
                    ) *
                    smooth_time
                ) /
                duration_cube
            );

        disk_y =
            disk_start_y -
            travelled;
    }
    else if(
        elapsed_milliseconds >=
            disk_flight_start +
            disk_flight_duration
    )
    {
        disk_y =
            disk_target_y;
    }

    /*
     * Title roll-in.
     */

    title_width =
        Floppy144DrawTextWidth(
            title,
            4U
        );

    title_target_x =
        (
            surface.width -
            title_width
        ) /
        2U;

    title_x =
        0U;

    if(elapsed_milliseconds >= title_start)
    {
        uint32_t title_time =
            elapsed_milliseconds -
            title_start;

        if(title_time >= title_duration)
        {
            title_x =
                title_target_x;
        }
        else
        {
            uint64_t time =
                (uint64_t)title_time;

            uint64_t duration =
                (uint64_t)title_duration;

            uint64_t smooth_time =
                time *
                time *
                (
                    3U * duration -
                    2U * time
                );

            uint64_t duration_cube =
                duration *
                duration *
                duration;

            title_x =
                (uint32_t)(
                    (
                        (uint64_t)title_target_x *
                        smooth_time
                    ) /
                    duration_cube
                );
        }
    }

    /*
     * Final question roll-in.
     */

    question_width =
        Floppy144DrawTextWidth(
            question,
            1U
        );

    question_target_x =
        (
            surface.width -
            question_width
        ) /
        2U;

    question_x =
        0U;

    if(elapsed_milliseconds >= question_start)
    {
        uint32_t question_time =
            elapsed_milliseconds -
            question_start;

        if(question_time >= question_duration)
        {
            question_x =
                question_target_x;
        }
        else
        {
            uint64_t time =
                (uint64_t)question_time;

            uint64_t duration =
                (uint64_t)question_duration;

            uint64_t smooth_time =
                time *
                time *
                (
                    3U * duration -
                    2U * time
                );

            uint64_t duration_cube =
                duration *
                duration *
                duration;

            question_x =
                (uint32_t)(
                    (
                        (uint64_t)question_target_x *
                        smooth_time
                    ) /
                    duration_cube
                );
        }
    }

    Floppy144DrawClear(
        &surface,
        background
    );

    /*
     * Reversed 3.5-inch disk shell.
     *
     * The single clipped corner is at the lower left. The casing apertures are
     * at the top, opposite the metal shutter.
     */

    Floppy144DrawFillRect(
        &surface,
        disk_x,
        disk_y,
        disk_width,
        disk_height,
        disk_body
    );

    /*
     * Single stepped lower-left cut.
     */

    Floppy144DrawFillRect(
        &surface,
        disk_x,
        disk_y + 100U,
        4U,
        12U,
        background
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 4U,
        disk_y + 104U,
        4U,
        8U,
        background
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 8U,
        disk_y + 108U,
        4U,
        4U,
        background
    );

    /*
     * Outer casing edges.
     */

    Floppy144DrawFillRect(
        &surface,
        disk_x,
        disk_y,
        disk_width,
        2U,
        disk_edge
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x,
        disk_y,
        2U,
        100U,
        disk_edge
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 126U,
        disk_y,
        2U,
        disk_height,
        disk_edge
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 12U,
        disk_y + 110U,
        116U,
        2U,
        disk_edge
    );

    /*
     * Stepped border around the clipped corner.
     */

    Floppy144DrawFillRect(
        &surface,
        disk_x + 2U,
        disk_y + 100U,
        4U,
        2U,
        disk_edge
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 6U,
        disk_y + 104U,
        4U,
        2U,
        disk_edge
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 10U,
        disk_y + 108U,
        4U,
        2U,
        disk_edge
    );

    /*
     * Recessed body panel.
     */

    Floppy144DrawRect(
        &surface,
        disk_x + 10U,
        disk_y + 10U,
        108U,
        94U,
        disk_inner
    );

    /*
     * Top casing apertures.
     */

    Floppy144DrawFillRect(
        &surface,
        disk_x + 8U,
        disk_y + 7U,
        9U,
        8U,
        disk_inner
    );

    Floppy144DrawRect(
        &surface,
        disk_x + 8U,
        disk_y + 7U,
        9U,
        8U,
        disk_edge
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 112U,
        disk_y + 7U,
        8U,
        8U,
        disk_label
    );

    Floppy144DrawRect(
        &surface,
        disk_x + 112U,
        disk_y + 7U,
        8U,
        8U,
        disk_edge
    );

    /*
     * Upper paper label.
     */

    Floppy144DrawFillRect(
        &surface,
        disk_x + 17U,
        disk_y + 20U,
        94U,
        38U,
        disk_label
    );

    Floppy144DrawRect(
        &surface,
        disk_x + 17U,
        disk_y + 20U,
        94U,
        38U,
        disk_edge
    );

    Floppy144DrawText(
        &surface,
        disk_x + 27U,
        disk_y + 28U,
        "DISK 144",
        1U,
        disk_label_text
    );

    Floppy144DrawText(
        &surface,
        disk_x + 27U,
        disk_y + 44U,
        "GDR RECOVERY",
        1U,
        disk_label_text
    );

    /*
     * Lower metal shutter.
     */

    Floppy144DrawFillRect(
        &surface,
        disk_x + 20U,
        disk_y + 70U,
        88U,
        32U,
        disk_metal
    );

    Floppy144DrawRect(
        &surface,
        disk_x + 20U,
        disk_y + 70U,
        88U,
        32U,
        disk_metal_light
    );

    Floppy144DrawFillRect(
        &surface,
        disk_x + 75U,
        disk_y + 75U,
        16U,
        22U,
        disk_inner
    );

    Floppy144DrawRect(
        &surface,
        disk_x + 75U,
        disk_y + 75U,
        16U,
        22U,
        disk_edge
    );
    /*
     * Staged text sequence.
     */

    if(elapsed_milliseconds >= title_start)
    {
        Floppy144DrawText(
            &surface,
            title_x,
            184U,
            title,
            4U,
            text
        );
    }

    if(elapsed_milliseconds >= media_start)
    {
        Floppy144RecoveryTextCentred(
            &surface,
            232U,
            "RECOVERY MEDIA DETECTED",
            1U,
            muted
        );
    }

    if(elapsed_milliseconds >= question_start)
    {
        Floppy144DrawText(
            &surface,
            question_x,
            286U,
            question,
            1U,
            amber
        );
    }

    if(elapsed_milliseconds >= prompt_start)
    {
        Floppy144RecoveryTextCentred(
            &surface,
            328U,
            "PRESS ENTER",
            1U,
            muted
        );
    }
}
static const char *const floppy144_main_menu_labels[] =
{
    "INITIATE NEW RECOVERY SESSION",
    "RETURN TO ACTIVE SITE",
    "RECORD CURRENT SESSION",
    "REINSTATE RECORDED SESSION",
    "TERMINATE RECOVERY ENVIRONMENT"
};

/*
 * Report whether one session-control option is available.
 */

bool Floppy144MainMenuOptionEnabled(
    Floppy144MainMenuOption option,
    bool active_session,
    bool recorded_session_available
)
{
    switch(option)
    {
        case FLOPPY144_MAIN_MENU_INITIATE_SESSION:
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

static const char *Floppy144RecoveryActName(
    uint8_t act
)
{
    switch((Floppy144RunAct)act)
    {
        case FLOPPY144_RUN_ACT_I:
        {
            return "I";
        }

        case FLOPPY144_RUN_ACT_II:
        {
            return "II";
        }

        case FLOPPY144_RUN_ACT_III:
        {
            return "III";
        }

        case FLOPPY144_RUN_ACT_COMPLETE:
        {
            return "COMPLETE";
        }

        case FLOPPY144_RUN_ACT_PROLOGUE:
        default:
        {
            return "PROLOGUE";
        }
    }
}

void Floppy144MainMenuDraw(
    F144Runtime *engine,
    Floppy144MainMenuOption selected_option,
    bool active_session,
    bool recorded_session_available,
    const Floppy144RunState *run_state,
    const Floppy144RunState *recorded_run_state,
    const char *persistence_warning
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
        : (
            recorded_session_available
            ? recorded_run_state
            : NULL
        );

        uint32_t reconstruction_percent =
        display_state != NULL
        ? Floppy144RunStateReconstructionPercent(
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
        char reconstruction_text[48];

    uint32_t option_index;

    Floppy144Surface surface =
    {
        (uint32_t *)engine->backbuffer.data,
        engine->backbuffer.width,
        engine->backbuffer.height
    };

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
                 "RECORDED SESSION: ACT %s / SEED %u",
                 Floppy144RecoveryActName(
                     recorded_run_state->act
                 ),
                 (unsigned)
                 recorded_run_state->recovery_seed
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

    snprintf(
        reconstruction_text,
        sizeof(reconstruction_text),
        "SITE RECONSTRUCTION: %02u%%",
        (unsigned)reconstruction_percent
    );

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
        (
            !active_session &&
            persistence_warning != NULL
        )
        ? persistence_warning
        : session_status,
        1U,
        (
            !active_session &&
            persistence_warning != NULL
        )
        ? amber
        : (
            active_session
            ? green
            : (
                recorded_session_available
                ? amber
                : muted
            )
        )
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
            174U +
            option_index * 16U;

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
        321U,
        "ENTER SELECT",
        1U,
        muted
    );
}
