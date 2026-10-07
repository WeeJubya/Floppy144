/*
 * F144 Win32 wake/clock adapter.
 *
 * Game cadence semantics live in floppy144_timing.c. This file only supplies
 * monotonic time and a periodic native wake-up for the Win32 message loop.
 */

#pragma once

#include "f144_platform.h"

#include <stdbool.h>
#include <stdint.h>

uint64_t f144Win32MonotonicMs(
    F144Platform *platform
);

bool f144Win32TimingStartWake(
    F144Platform *platform,
    uint32_t interval_ms
);

void f144Win32TimingStopWake(
    F144Platform *platform
);

bool f144Win32TimingIsWakeMessage(
    uint32_t message,
    uintptr_t parameter
);
