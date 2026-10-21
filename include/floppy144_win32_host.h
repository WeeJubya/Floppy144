/*
 * Floppy//144: native Win32 host for the independently authored display.
 *
 * Windows handles belong to this platform-only interface. The game-facing
 * F144PlatformApi retains its existing portable declarations and semantics.
 * The host is owned by WinMain; no heap allocation or engine image type here.
 */
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_platform.h"
#include "floppy144_win32_display.h"

#include <stdbool.h>

typedef struct Floppy144Win32Host
{
    HWND window;
    HDC paint_dc; /* Borrowed only between BeginPaint and EndPaint. */
    Floppy144Win32Display *display;
    bool closing;
} Floppy144Win32Host;

/* Called before Windows window creation; no graphics allocations yet. */
bool Floppy144Win32HostBind(
    F144Platform *platform,
    Floppy144Win32Host *host
);

/* Called once a real HWND exists; fails without changing the public surface. */
bool Floppy144Win32HostStart(
    F144Platform *platform,
    HWND window
);

/* Native WndProc borrows a paint DC for one F144PlatformApi.present call. */
void Floppy144Win32HostPaintTarget(
    F144Platform *platform,
    HDC paint_dc
);

/* The real HWND has been destroyed; stop using its native handle. */
void Floppy144Win32HostWindowDestroyed(
    F144Platform *platform,
    HWND window
);

/* Used by the separate Win32 timer adapter, never by portable game code. */
HWND Floppy144Win32HostWindow(
    const F144Platform *platform
);

/* Clear all borrowed/native state. Can be called repeatedly after shutdown. */
void Floppy144Win32HostStop(
    F144Platform *platform
);
