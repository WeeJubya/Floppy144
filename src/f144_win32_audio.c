/*
 * F144 Win32 audio backend
 *
 * S4B-04 is an abstraction pass. The audited Stage 4 baseline contains no
 * WinMIDI/WinMM/PlaySound/waveOut/MCI backend and no shipping music/SFX data.
 * Keep this backend intentionally silent so current behaviour is unchanged.
 *
 * When the separately approved generated-audio feature arrives, native device
 * calls belong here. Portable composition/event generation must remain outside
 * this file and supply semantic cue IDs/data through the platform boundary.
 */

#include "f144_win32_audio.h"

bool f144Win32AudioInit(
    F144Platform *platform
)
{
    (void)platform;
    return true;
}

void f144Win32AudioShutdown(
    F144Platform *platform
)
{
    (void)platform;
}

void f144Win32AudioPlayMusic(
    F144Platform *platform,
    F144MusicCueId cue
)
{
    (void)platform;
    (void)cue;
}

void f144Win32AudioStopMusic(
    F144Platform *platform
)
{
    (void)platform;
}

void f144Win32AudioPlaySfx(
    F144Platform *platform,
    F144SfxCueId cue
)
{
    (void)platform;
    (void)cue;
}

void f144Win32AudioSetMusicVolume(
    F144Platform *platform,
    uint8_t volume
)
{
    (void)platform;
    (void)volume;
}

void f144Win32AudioSetSfxVolume(
    F144Platform *platform,
    uint8_t volume
)
{
    (void)platform;
    (void)volume;
}
