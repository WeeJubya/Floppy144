/* Portable semantic boundary, intentionally free of Win32 or WinMM calls. */
#include "floppy144_audio.h"

void Floppy144AudioPlay(F144Platform *platform,Floppy144AudioSound effect)
{
    if ((unsigned)effect<1U || (unsigned)effect>22U) return;
    /* The semantic registry order matches the approved original F144SFX
       entries followed by eight newly-authored procedural game cues. */
    (void)f144PlatformPlaySfx(platform,(F144SfxCueId)((unsigned)effect-1U));
}

void Floppy144AudioMusicSet(F144Platform *platform,Floppy144AudioMusic cue)
{
    if(cue==FLOPPY144_AUDIO_MUSIC_NONE) (void)f144PlatformStopMusic(platform);
    else if((unsigned)cue<=FLOPPY144_AUDIO_MUSIC_ACT_III)
        (void)f144PlatformPlayMusic(platform,(F144MusicCueId)cue);
}

void Floppy144AudioTick(F144Platform *platform,uint64_t now_ms,uint32_t recovered_percent)
{
    f144PlatformAudioUpdate(platform,now_ms,recovered_percent);
}