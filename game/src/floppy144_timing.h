/*
 * FLOPPY//144 deterministic game timing.
 *
 * Native platforms only provide a monotonic millisecond clock and periodic
 * opportunities to update. This module owns the actual game cadences.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_SPLASH_FRAME_MS       16U
#define FLOPPY144_TERMINAL_RESTORE_MS   50U
#define FLOPPY144_TERMINAL_CURSOR_MS   500U

typedef struct Floppy144TimingState
{
    uint64_t splash_started_ms;
    uint64_t next_splash_frame_ms;
    uint64_t next_terminal_restore_ms;
    uint64_t next_terminal_cursor_ms;
    uint64_t next_autosave_ms;
    uint64_t last_update_ms;

    uint32_t autosave_interval_ms;
    uint8_t splash_active;
} Floppy144TimingState;

typedef struct Floppy144TimingEvents
{
    uint32_t terminal_restore_elapsed_ms;
    uint8_t splash_frame_due;
    uint8_t terminal_cursor_toggle;
    uint8_t autosave_due;
} Floppy144TimingEvents;

void Floppy144TimingReset(
    Floppy144TimingState *timing,
    uint64_t now_ms,
    uint32_t autosave_interval_ms
);

void Floppy144TimingStopSplash(
    Floppy144TimingState *timing
);

uint32_t Floppy144TimingSplashElapsedMs(
    const Floppy144TimingState *timing,
    uint64_t now_ms
);

Floppy144TimingEvents Floppy144TimingAdvance(
    Floppy144TimingState *timing,
    uint64_t now_ms
);
