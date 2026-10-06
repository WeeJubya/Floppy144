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

bool f144PlatformPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
)
{
    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->persistence_path == NULL ||
        path == NULL ||
        path_capacity == 0U
    )
    {
        return false;
    }

    path[0] = '\0';

    return platform->api->persistence_path(
        platform,
        file,
        path,
        path_capacity
    );
}

bool f144PlatformLegacyPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    uint32_t candidate,
    char *path,
    uint32_t path_capacity
)
{
    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->legacy_persistence_path == NULL ||
        path == NULL ||
        path_capacity == 0U
    )
    {
        return false;
    }

    path[0] = '\0';

    return platform->api->legacy_persistence_path(
        platform,
        file,
        candidate,
        path,
        path_capacity
    );
}
