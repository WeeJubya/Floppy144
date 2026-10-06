/*
 * F144 Win32 audio backend
 *
 * The verified Stage 4 baseline has no shipping audio playback implementation.
 * These callbacks deliberately preserve that silent behaviour while isolating
 * the platform seam for the later generated-audio feature.
 */

#pragma once

#include "f144_platform.h"

#include <stdbool.h>
#include <stdint.h>

bool f144Win32AudioInit(
    F144Platform *platform
);

void f144Win32AudioShutdown(
    F144Platform *platform
);

void f144Win32AudioPlayMusic(
    F144Platform *platform,
    F144MusicCueId cue
);

void f144Win32AudioStopMusic(
    F144Platform *platform
);

void f144Win32AudioPlaySfx(
    F144Platform *platform,
    F144SfxCueId cue
);

void f144Win32AudioSetMusicVolume(
    F144Platform *platform,
    uint8_t volume
);

void f144Win32AudioSetSfxVolume(
    F144Platform *platform,
    uint8_t volume
);
