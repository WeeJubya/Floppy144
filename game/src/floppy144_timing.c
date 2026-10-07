/*
 * FLOPPY//144 deterministic game timing.
 */

#include "floppy144_timing.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

static uint64_t Floppy144TimingDueCount(
    uint64_t now_ms,
    uint64_t next_ms,
    uint32_t interval_ms
)
{
    if(
        interval_ms == 0U ||
        now_ms < next_ms
    )
    {
        return 0U;
    }

    return
        ((now_ms - next_ms) / (uint64_t)interval_ms) + 1U;
}

void Floppy144TimingReset(
    Floppy144TimingState *timing,
    uint64_t now_ms,
    uint32_t autosave_interval_ms
)
{
    if(timing == NULL)
    {
        return;
    }

    memset(timing,0,sizeof(*timing));

    timing->splash_started_ms =
        now_ms;
    timing->next_splash_frame_ms =
        now_ms + FLOPPY144_SPLASH_FRAME_MS;
    timing->next_terminal_restore_ms =
        now_ms + FLOPPY144_TERMINAL_RESTORE_MS;
    timing->next_terminal_cursor_ms =
        now_ms + FLOPPY144_TERMINAL_CURSOR_MS;
    timing->last_update_ms =
        now_ms;
    timing->autosave_interval_ms =
        autosave_interval_ms;
    timing->splash_active =
        1U;

    if(autosave_interval_ms != 0U)
    {
        timing->next_autosave_ms =
            now_ms + autosave_interval_ms;
    }
}

void Floppy144TimingStopSplash(
    Floppy144TimingState *timing
)
{
    if(timing != NULL)
    {
        timing->splash_active =
            0U;
    }
}

uint32_t Floppy144TimingSplashElapsedMs(
    const Floppy144TimingState *timing,
    uint64_t now_ms
)
{
    uint64_t elapsed;

    if(
        timing == NULL ||
        now_ms <= timing->splash_started_ms
    )
    {
        return 0U;
    }

    elapsed =
        now_ms - timing->splash_started_ms;

    if(elapsed > UINT32_MAX)
    {
        return UINT32_MAX;
    }

    return (uint32_t)elapsed;
}

Floppy144TimingEvents Floppy144TimingAdvance(
    Floppy144TimingState *timing,
    uint64_t now_ms
)
{
    Floppy144TimingEvents events = {0};
    uint64_t due;

    if(timing == NULL)
    {
        return events;
    }

    /*
     * The platform contract promises monotonic time. Clamp a faulty/test clock
     * rather than allowing deadlines to move backwards.
     */
    if(now_ms < timing->last_update_ms)
    {
        now_ms =
            timing->last_update_ms;
    }

    timing->last_update_ms =
        now_ms;

    if(timing->splash_active != 0U)
    {
        due =
            Floppy144TimingDueCount(
                now_ms,
                timing->next_splash_frame_ms,
                FLOPPY144_SPLASH_FRAME_MS
            );

        if(due != 0U)
        {
            events.splash_frame_due =
                1U;
            timing->next_splash_frame_ms +=
                due * FLOPPY144_SPLASH_FRAME_MS;
        }
    }

    due =
        Floppy144TimingDueCount(
            now_ms,
            timing->next_terminal_restore_ms,
            FLOPPY144_TERMINAL_RESTORE_MS
        );

    if(due != 0U)
    {
        /*
         * Stage 3's Win32 SetTimer delivered one restoration quantum when the
         * message loop observed the timer; it did not replay every missed
         * timer message after a stall. Skip missed deadlines but emit one
         * fixed 50 ms quantum to preserve that behaviour.
         */
        events.terminal_restore_elapsed_ms =
            FLOPPY144_TERMINAL_RESTORE_MS;

        timing->next_terminal_restore_ms +=
            due * FLOPPY144_TERMINAL_RESTORE_MS;
    }

    due =
        Floppy144TimingDueCount(
            now_ms,
            timing->next_terminal_cursor_ms,
            FLOPPY144_TERMINAL_CURSOR_MS
        );

    if(due != 0U)
    {
        /*
         * As with WM_TIMER, a delayed message loop produces one visible toggle
         * rather than replaying all missed blink messages.
         */
        events.terminal_cursor_toggle =
            1U;

        timing->next_terminal_cursor_ms +=
            due * FLOPPY144_TERMINAL_CURSOR_MS;
    }

    if(
        timing->autosave_interval_ms != 0U &&
        timing->next_autosave_ms != 0U
    )
    {
        due =
            Floppy144TimingDueCount(
                now_ms,
                timing->next_autosave_ms,
                timing->autosave_interval_ms
            );

        if(due != 0U)
        {
            events.autosave_due =
                1U;

            timing->next_autosave_ms +=
                due * timing->autosave_interval_ms;
        }
    }

    return events;
}
