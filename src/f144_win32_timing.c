/*
 * F144 Win32 wake/clock adapter.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_runtime.h"
#include "f144_win32_timing.h"

#define F144_WIN32_WAKE_TIMER_ID 1440U

uint64_t f144Win32MonotonicMs(
    F144Platform *platform
)
{
    (void)platform;

    return (uint64_t)GetTickCount64();
}

bool f144Win32TimingStartWake(
    F144Platform *platform,
    uint32_t interval_ms
)
{
    F144Runtime *runtime;

    if(
        platform == NULL ||
        interval_ms == 0U
    )
    {
        return false;
    }

    runtime =
        (F144Runtime *)platform->state;

    if(
        runtime == NULL ||
        runtime->window == NULL
    )
    {
        return false;
    }

    return
        SetTimer(
            runtime->window,
            F144_WIN32_WAKE_TIMER_ID,
            (UINT)interval_ms,
            NULL
        ) != 0U;
}

void f144Win32TimingStopWake(
    F144Platform *platform
)
{
    F144Runtime *runtime;

    if(platform == NULL)
    {
        return;
    }

    runtime =
        (F144Runtime *)platform->state;

    if(
        runtime != NULL &&
        runtime->window != NULL
    )
    {
        (void)KillTimer(
            runtime->window,
            F144_WIN32_WAKE_TIMER_ID
        );
    }
}

bool f144Win32TimingIsWakeMessage(
    uint32_t message,
    uintptr_t parameter
)
{
    return
        message == (uint32_t)WM_TIMER &&
        parameter == (uintptr_t)F144_WIN32_WAKE_TIMER_ID;
}
