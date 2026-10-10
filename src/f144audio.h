#ifndef F144_AUDIO_H
#define F144_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum F144Act {
    F144_ACT_PROLOGUE = 0,
    F144_ACT_I        = 1,
    F144_ACT_II       = 2,
    F144_ACT_III      = 3
} F144Act;

int  F144_AudioInit(void);
void F144_AudioShutdown(void);

void F144_MusicRestart(void);
void F144_MusicSeekBar(unsigned bar);
void F144_MusicStop(void);
void F144_MusicSetRestoration(unsigned percent);
unsigned F144_MusicGetRestoration(void);
unsigned F144_MusicGetDistortion(void);
void F144_AudioUpdate(double dt_seconds);

void  F144_MusicSetVolume(float volume);
float F144_MusicGetVolume(void);

void    F144_MusicSetAct(F144Act act);
F144Act F144_MusicGetAct(void);

void     F144_MusicSetSeed(uint32_t seed);
uint32_t F144_MusicGetSeed(void);

int         F144_AudioIsAvailable(void);
unsigned    F144_AudioDeviceCount(void);
const char *F144_AudioDeviceName(void);

#ifdef __cplusplus
}
#endif

#endif