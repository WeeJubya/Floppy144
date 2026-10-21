/*
 * Win32 monotonic clock and timer wake source for the independent window host.
 * Game cadence remains exclusively in Floppy144TimingAdvance.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_win32_timing.h"
#include "floppy144_win32_host.h"

#define FLOPPY144_NATIVE_WAKE_TIMER 1440U

uint64_t f144Win32MonotonicMs(F144Platform *platform)
{
    (void)platform;
    return (uint64_t)GetTickCount64();
}

bool f144Win32TimingStartWake(
    F144Platform *platform,
    uint32_t interval_ms
)
{
    HWND window = Floppy144Win32HostWindow(platform);
    if(window == NULL || interval_ms == 0U)
    {
        return false;
    }
    return SetTimer(window, FLOPPY144_NATIVE_WAKE_TIMER,
                    (UINT)interval_ms, NULL) != 0U;
}

void f144Win32TimingStopWake(F144Platform *platform)
{
    HWND window = Floppy144Win32HostWindow(platform);
    if(window != NULL)
    {
        (void)KillTimer(window, FLOPPY144_NATIVE_WAKE_TIMER);
    }
}

bool f144Win32TimingIsWakeMessage(uint32_t message, uintptr_t parameter)
{
    return message == (uint32_t)WM_TIMER &&
           parameter == (uintptr_t)FLOPPY144_NATIVE_WAKE_TIMER;
}
