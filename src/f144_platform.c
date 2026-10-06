/*
 * F144 platform contract dispatch
 *
 * Keeps game-facing callers independent of the active native implementation.
 */

#include "f144_platform.h"

Floppy144Surface *f144PlatformFramebuffer(
    F144Platform *platform
)
{
    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->framebuffer == NULL
    )
    {
        return NULL;
    }

    return platform->api->framebuffer(platform);
}

void f144PlatformPresent(
    F144Platform *platform
)
{
    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->present == NULL
    )
    {
        return;
    }

    platform->api->present(platform);
}
