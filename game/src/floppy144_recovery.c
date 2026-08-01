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
 * The function derives dynamic labels first, wraps the river2D backbuffer
 * as a Floppy144Surface, then paints the interface from back to front.
 */

/*
 * Draw the opening Floppy//144 splash
 *
 * The disk uses only rectangles and the embedded font. Its ascent uses an
 * integer ease-out followed by a small settling movement.
 */

void Floppy144SplashDraw(
    EngineData *engine,
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
void Floppy144RecoveryDraw(
    EngineData *engine,
    bool recovery_started,
    const Floppy144WorldState *world
)
{
    /*
     * Screen palette
     *
     * Colours are local constants so the compiler can fold them into the draw
     * calls without requiring a theme object.
     */

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

    /*
     * Dynamic recovery messages
     *
     * Text and progress width depend on whether recovery has begun and how
     * much of the site has been restored.
     */

    uint32_t reconstruction_percent =
        recovery_started
            ? Floppy144WorldReconstructionPercent(world)
            : 0U;

    char status_text[48];

    snprintf(
        status_text,
        sizeof(status_text),
        "SITE RECONSTRUCTION STATUS: %02u%%",
        (unsigned)reconstruction_percent
    );

    const char *prompt_text =
        recovery_started
            ? "OFFICE RECONSTRUCTION READY"
            : "PRESS ENTER TO BEGIN RECOVERY";

    const char *sub_prompt_text =
        recovery_started
            ? "PRESS ENTER TO ENTER SITE"
            : "ESC TO TERMINATE SESSION";

    uint32_t progress_width =
        recovery_started
            ? (
                522U *
                reconstruction_percent
              ) / 100U
            : 2U;

    /*
     * Backbuffer view
     *
     * river2D owns the memory. The game supplies width, height and pixel pointer
     * to the compact drawing layer.
     */

    Floppy144Surface surface =
    {
        (uint32_t *)engine->backbuffer.data,
        engine->backbuffer.width,
        engine->backbuffer.height
    };

    /* Paint order: background, system header, main panel, metadata, progress and prompts. */
    Floppy144DrawClear(
        &surface,
        background
    );

    Floppy144DrawFillRect(
        &surface,
        0,
        0,
        640,
        16,
        panel_dark
    );

    Floppy144DrawText(
        &surface,
        10,
        5,
        "GDR ARCHIVE RECOVERY SYSTEM",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        556,
        5,
        "APS-12",
        1,
        amber
    );

    Floppy144DrawFillRect(
        &surface,
        24,
        28,
        592,
        306,
        panel
    );

    Floppy144DrawRect(
        &surface,
        24,
        28,
        592,
        306,
        border
    );

    Floppy144RecoveryTextCentred(
        &surface,
        44,
        "GOVERNMENT DEPARTMENT OF RECORDS",
        2,
        text
    );

    Floppy144RecoveryTextCentred(
        &surface,
        66,
        "ARCHIVE RECOVERY ENVIRONMENT",
        1,
        muted
    );

    Floppy144DrawFillRect(
        &surface,
        48,
        88,
        544,
        1,
        border
    );

    Floppy144DrawText(
        &surface,
        56,
        106,
    /* Static metadata identifies the recovered disk and governing protocol. */
        "REMOVABLE MEDIA:",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        196,
        106,
        "DISK 144",
        1,
        text
    );

    Floppy144DrawText(
        &surface,
        56,
        126,
        "MEDIA CLASS:",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        196,
        126,
        "RECOVERY",
        1,
        text
    );

    Floppy144DrawText(
        &surface,
        56,
        146,
        "PROTOCOL:",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        196,
        146,
        "APS-12 PARTIAL SITE",
        1,
        amber
    );

    Floppy144DrawFillRect(
        &surface,
        48,
        174,
        544,
        1,
        border
    );

    Floppy144DrawText(
        &surface,
        56,
        190,
    /* DR-01 is always available and supplies the minimum site reconstruction. */
        "MANDATORY COLLECTION DR-01: RESTORED",
        1,
        green
    );

    Floppy144DrawText(
        &surface,
        56,
        210,
        status_text,
        1,
        text
    );

    Floppy144DrawFillRect(
        &surface,
        56,
        232,
        528,
        16,
        panel_dark
    );

    Floppy144DrawRect(
        &surface,
        56,
        232,
        528,
        16,
        border
    );

    Floppy144DrawFillRect(
        &surface,
        59,
        235,
        progress_width,
        10,
        green
    );

    Floppy144DrawFillRect(
        &surface,
        48,
        268,
        544,
        46,
        panel_dark
    );

    Floppy144DrawRect(
        &surface,
        48,
        268,
        544,
        46,
        border
    );

    Floppy144RecoveryTextCentred(
        &surface,
        278,
    /* The lower panel tells the player what Enter will do next. */
        prompt_text,
        1,
        recovery_started ? green : amber
    );

    Floppy144RecoveryTextCentred(
        &surface,
        296,
        sub_prompt_text,
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        34,
        320,
        "SESSION 144-REC",
        1,
        muted
    );

    Floppy144DrawText(
        &surface,
        529,
        320,
        "OFFLINE",
        1,
        muted
    );
}
