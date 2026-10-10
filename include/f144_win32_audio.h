#pragma once
#include "f144_platform.h"
#include <stdbool.h>
#include <stdint.h>
bool f144Win32AudioInit(F144Platform *platform);
void f144Win32AudioShutdown(F144Platform *platform);
void f144Win32AudioPlayMusic(F144Platform *platform,F144MusicCueId cue);
void f144Win32AudioStopMusic(F144Platform *platform);
void f144Win32AudioPlaySfx(F144Platform *platform,F144SfxCueId cue);
void f144Win32AudioSetMusicVolume(F144Platform *platform,uint8_t volume);
void f144Win32AudioSetSfxVolume(F144Platform *platform,uint8_t volume);
void f144Win32AudioUpdate(F144Platform *platform,uint64_t monotonic_ms,uint32_t recovered_percent);