/*
 * Floppy//144 - recovery screen implementation
 *
 * Builds the boot-like recovery interface entirely from drawing primitives.
 * This screen introduces Disk 144, APS-12 and the partial reconstruction.
 */

#include "floppy144_recovery.h"

#include "floppy144_draw.h"

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

    const char *status_text =
        !recovery_started
            ? "SITE RECONSTRUCTION STATUS: 00%"
            : Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
                ? "SITE RECONSTRUCTION STATUS: 12%"
                : "SITE RECONSTRUCTION STATUS: 04%";

    const char *prompt_text =
        recovery_started
            ? "OFFICE RECONSTRUCTION READY"
            : "PRESS ENTER TO BEGIN RECOVERY";

    const char *sub_prompt_text =
        recovery_started
            ? "PRESS ENTER TO ENTER SITE"
            : "ESC TO TERMINATE SESSION";

    uint32_t progress_width =
        !recovery_started
            ? 2U
            : Floppy144WorldCollectionRestored(world, FLOPPY144_COLLECTION_HR02)
                ? 63U
                : 20U;

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
    /* XX-01 is always available and supplies the minimum site reconstruction. */
        "MANDATORY COLLECTION XX-01: RESTORED",
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
