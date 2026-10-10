/* F144 native WinMM boundary: music, waveOut and device lifetime. */
#include "f144_win32_audio.h"
#include "f144audio.h"
#include "f144sfx.h"

static uint64_t previous_update_ms;
static unsigned music_cue;
static int midi_ready;
static int sfx_ready;

bool f144Win32AudioInit(F144Platform *platform)
{
    (void)platform;
    midi_ready=F144_AudioInit();
    sfx_ready=F144_SFXInit();
    previous_update_ms=0U;
    music_cue=0U;
    return midi_ready!=0 || sfx_ready!=0;
}

void f144Win32AudioShutdown(F144Platform *platform)
{
    (void)platform;
    f144Win32AudioStopMusic(platform);
    F144_SFXShutdown();
    F144_AudioShutdown();
    midi_ready=0;
    sfx_ready=0;
    previous_update_ms=0U;
}

void f144Win32AudioPlayMusic(F144Platform *platform,F144MusicCueId cue)
{
    (void)platform;
    if(cue==0U) {f144Win32AudioStopMusic(platform);return;}
    if(cue>4U) return;
    if(!midi_ready) return;
    if(music_cue==0U) F144_MusicRestart();
    F144_MusicSetAct((F144Act)(cue-1U));
    music_cue=cue;
}

void f144Win32AudioStopMusic(F144Platform *platform)
{
    (void)platform;
    if(midi_ready) F144_MusicStop();
    music_cue=0U;
}

void f144Win32AudioPlaySfx(F144Platform *platform,F144SfxCueId cue)
{
    (void)platform;
    if(sfx_ready && cue<F144_SFX_COUNT) F144_SFXPlay((F144SFX)cue);
}

void f144Win32AudioSetMusicVolume(F144Platform *platform,uint8_t volume)
{
    (void)platform;
    F144_MusicSetVolume((float)volume/10.0f);
}

void f144Win32AudioSetSfxVolume(F144Platform *platform,uint8_t volume)
{
    (void)platform;
    F144_SFXSetVolume((float)volume/10.0f);
}

void f144Win32AudioUpdate(F144Platform *platform,
                           uint64_t monotonic_ms,uint32_t recovered_percent)
{
    double dt=0.0;
    (void)platform;
    F144_MusicSetRestoration((unsigned)recovered_percent);
    if(previous_update_ms!=0U && monotonic_ms>=previous_update_ms)
        dt=(double)(monotonic_ms-previous_update_ms)/1000.0;
    previous_update_ms=monotonic_ms;
    if(midi_ready) F144_AudioUpdate(dt);
}