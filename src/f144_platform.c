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

bool f144PlatformAudioInit(
    F144Platform *platform
)
{
    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->audio_init == NULL
    )
    {
        return false;
    }

    if(platform->audio_initialized != 0U)
    {
        return true;
    }

    if(!platform->api->audio_init(platform))
    {
        return false;
    }

    platform->audio_initialized =
        1U;
    platform->music_volume =
        F144_AUDIO_VOLUME_MAX;
    platform->sfx_volume =
        F144_AUDIO_VOLUME_MAX;

    return true;
}

void f144PlatformAudioShutdown(
    F144Platform *platform
)
{
    if(
        platform == NULL ||
        platform->audio_initialized == 0U
    )
    {
        return;
    }

    if(
        platform->api != NULL &&
        platform->api->audio_shutdown != NULL
    )
    {
        platform->api->audio_shutdown(platform);
    }

    platform->audio_initialized =
        0U;
}

bool f144PlatformPlayMusic(
    F144Platform *platform,
    F144MusicCueId cue
)
{
    if(
        platform == NULL ||
        platform->audio_initialized == 0U ||
        platform->api == NULL ||
        platform->api->music_play == NULL
    )
    {
        return false;
    }

    platform->api->music_play(
        platform,
        cue
    );

    return true;
}

bool f144PlatformStopMusic(
    F144Platform *platform
)
{
    if(
        platform == NULL ||
        platform->audio_initialized == 0U ||
        platform->api == NULL ||
        platform->api->music_stop == NULL
    )
    {
        return false;
    }

    platform->api->music_stop(platform);

    return true;
}

bool f144PlatformPlaySfx(
    F144Platform *platform,
    F144SfxCueId cue
)
{
    if(
        platform == NULL ||
        platform->audio_initialized == 0U ||
        platform->api == NULL ||
        platform->api->sfx_play == NULL
    )
    {
        return false;
    }

    platform->api->sfx_play(
        platform,
        cue
    );

    return true;
}

bool f144PlatformSetMusicVolume(
    F144Platform *platform,
    uint8_t volume
)
{
    if(
        platform == NULL ||
        platform->audio_initialized == 0U ||
        volume > F144_AUDIO_VOLUME_MAX ||
        platform->api == NULL ||
        platform->api->music_volume == NULL
    )
    {
        return false;
    }

    platform->api->music_volume(
        platform,
        volume
    );

    platform->music_volume =
        volume;

    return true;
}

bool f144PlatformSetSfxVolume(
    F144Platform *platform,
    uint8_t volume
)
{
    if(
        platform == NULL ||
        platform->audio_initialized == 0U ||
        volume > F144_AUDIO_VOLUME_MAX ||
        platform->api == NULL ||
        platform->api->sfx_volume == NULL
    )
    {
        return false;
    }

    platform->api->sfx_volume(
        platform,
        volume
    );

    platform->sfx_volume =
        volume;

    return true;
}

uint64_t f144PlatformMonotonicMs(
    F144Platform *platform
)
{
    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->monotonic_ms == NULL
    )
    {
        return 0U;
    }

    return platform->api->monotonic_ms(
        platform
    );
}

void f144PlatformQuit(
    F144Platform *platform
)
{
    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->quit == NULL
    )
    {
        return;
    }

    platform->api->quit(
        platform
    );
}
