/*
 * Floppy//144 independent Win32 platform service binding.
 *
 * Only public game-owned service declarations and the B2-02 contracts inform
 * this adapter. This file contains no legacy runtime or image abstractions.
 */
#include "floppy144_win32_host.h"

#include "f144_win32_audio.h"
#include "f144_win32_calendar.h"
#include "f144_win32_storage.h"
#include "f144_win32_timing.h"

#include <string.h>

static Floppy144Win32Host *HostState(const F144Platform *platform)
{
    return platform != NULL ? (Floppy144Win32Host *)platform->state : NULL;
}

static Floppy144Surface *HostFramebuffer(F144Platform *platform)
{
    Floppy144Win32Host *host = HostState(platform);
    if(host == NULL || host->display == NULL)
    {
        return NULL;
    }
    return &platform->surface;
}

static void HostPresent(F144Platform *platform)
{
    Floppy144Win32Host *host = HostState(platform);
    RECT client;

    if(host == NULL || host->window == NULL ||
       host->display == NULL || host->paint_dc == NULL)
    {
        return;
    }

    if(!GetClientRect(host->window, &client))
    {
        return;
    }

    (void)Floppy144Win32DisplayPresent(
        host->display,
        host->paint_dc,
        (int64_t)client.right - (int64_t)client.left,
        (int64_t)client.bottom - (int64_t)client.top
    );
}

static void HostQuit(F144Platform *platform)
{
    Floppy144Win32Host *host = HostState(platform);
    if(host == NULL || host->closing)
    {
        return;
    }

    host->closing = true;
    if(host->window != NULL)
    {
        (void)DestroyWindow(host->window);
    }
}

/*
 * All fifteen slots are explicit. Procedural music, persistence, clock
 * and calendar services are reused, without altering any stored data.
 */
static const F144PlatformApi host_api =
{
    HostFramebuffer,
    HostPresent,
    f144Win32PlatformPersistencePath,
    f144Win32PlatformLegacyPersistencePath,
    f144Win32AudioInit,
    f144Win32AudioShutdown,
    f144Win32AudioPlayMusic,
    f144Win32AudioStopMusic,
    f144Win32AudioPlaySfx,
    f144Win32AudioSetMusicVolume,
    f144Win32AudioSetSfxVolume,
    f144Win32MonotonicMs,
    HostQuit,
    f144Win32LocalCalendarDate,
    f144Win32AudioUpdate
};

bool Floppy144Win32HostBind(
    F144Platform *platform,
    Floppy144Win32Host *host
)
{
    if(platform == NULL || host == NULL)
    {
        return false;
    }

    memset(platform, 0, sizeof(*platform));
    memset(host, 0, sizeof(*host));
    platform->api = &host_api;
    platform->state = host;
    return true;
}

bool Floppy144Win32HostStart(
    F144Platform *platform,
    HWND window
)
{
    Floppy144Win32Host *host = HostState(platform);
    Floppy144Surface *surface;

    if(host == NULL || window == NULL || host->display != NULL ||
       host->closing)
    {
        return false;
    }

    host->display = Floppy144Win32DisplayCreate();
    if(host->display == NULL)
    {
        return false;
    }

    surface = Floppy144Win32DisplayFramebuffer(host->display);
    if(surface == NULL || surface->pixels == NULL ||
       surface->width != FLOPPY144_DISPLAY_WIDTH ||
       surface->height != FLOPPY144_DISPLAY_HEIGHT)
    {
        Floppy144Win32DisplayDestroy(&host->display);
        return false;
    }

    host->window = window;
    platform->surface = *surface;
    return true;
}

void Floppy144Win32HostPaintTarget(
    F144Platform *platform,
    HDC paint_dc
)
{
    Floppy144Win32Host *host = HostState(platform);
    if(host != NULL)
    {
        host->paint_dc = paint_dc;
    }
}

void Floppy144Win32HostWindowDestroyed(
    F144Platform *platform,
    HWND window
)
{
    Floppy144Win32Host *host = HostState(platform);
    if(host != NULL && host->window == window)
    {
        host->window = NULL;
        host->paint_dc = NULL;
        host->closing = true;
        /* The owning window has finished its native destruction sequence. */
        PostQuitMessage(0);
    }
}

HWND Floppy144Win32HostWindow(
    const F144Platform *platform
)
{
    Floppy144Win32Host *host = HostState(platform);
    return host != NULL ? host->window : NULL;
}

void Floppy144Win32HostStop(
    F144Platform *platform
)
{
    Floppy144Win32Host *host = HostState(platform);
    if(host == NULL)
    {
        return;
    }

    Floppy144Win32DisplayDestroy(&host->display);
    memset(&platform->surface, 0, sizeof(platform->surface));
    host->paint_dc = NULL;
    host->window = NULL;
    host->closing = true;
}
