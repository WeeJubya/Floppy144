/* Review-only candidates must remain deterministic and outside the game. */
#define F144_SFX_REVIEW
#include "../src/f144sfx.c"
#include <stdio.h>
#include "../game/src/floppy144_audio.h"
int main(void)
{
    static F144Voice a,b;
    unsigned effect,variant,i,n,m;
    static const unsigned semantic_ids[16]={
        FLOPPY144_AUDIO_TERMINAL_TYPE,FLOPPY144_AUDIO_ACCEPT,FLOPPY144_AUDIO_ERROR,
        FLOPPY144_AUDIO_RELAY,FLOPPY144_AUDIO_CABINET,FLOPPY144_AUDIO_CRT_WAKE,
        FLOPPY144_AUDIO_POWER_FAIL,FLOPPY144_AUDIO_FOOTSTEP,
        FLOPPY144_AUDIO_FILE_RESTORE,FLOPPY144_AUDIO_DOOR_BLOCKED,
        FLOPPY144_AUDIO_EVIDENCE_FOUND,FLOPPY144_AUDIO_DOOR_OPEN,
        FLOPPY144_AUDIO_FLOPPY_INSERT,FLOPPY144_AUDIO_TERMINAL_BACKSPACE,
        FLOPPY144_AUDIO_RESTORE_TICK,FLOPPY144_AUDIO_RESTORE_TICK_HIGH
    };
    if(F144_SFX_COUNT!=16U)return 13;
    for(i=0U;i<F144_SFX_COUNT;++i)if(semantic_ids[i]!=i+1U)return 14;
    for(effect=0U;effect<F144_SFX_COUNT;effect++)
        for(variant=0U;variant<3U;variant++) {
            g_volume=0.8f;g_noise=0xF14451F1U;
            n=generate_review(&a,(F144SFX)effect,variant,0U);
            g_noise=0xF14451F1U;
            m=generate_review(&b,(F144SFX)effect,variant,0U);
            if(n==0U||n>F144_MAX_SAMPLES||n!=m)return 1;
            if(memcmp(a.samples,b.samples,n*sizeof(int16_t)))return 2;
            for(i=0U;i<n;i++)if(a.samples[i]>=32760||a.samples[i]<=-32760)return 3;
            g_volume=0.0f;g_noise=0xF14451F1U;
            m=generate_review(&b,(F144SFX)effect,variant,0U);
            for(i=0U;i<m;i++)if(b.samples[i])return 4;
        }
    g_volume=0.8f;g_noise=0xF14451F1U;
    n=generate_review(&a,F144_SFX_FOOTSTEP,1U,0U);
    g_noise=0xF14451F1U;
    m=generate_review(&b,F144_SFX_FOOTSTEP,1U,1U);
    if(n!=m||!memcmp(a.samples,b.samples,n*sizeof(int16_t)))return 5;
    /* Shipping footstep is exactly the approved Revised A review generator. */
    g_volume=0.8f;g_noise=0xF14451F1U;g_footstep_side=0U;
    n=generate(&a,F144_SFX_FOOTSTEP);
    g_noise=0xF14451F1U;
    m=generate_review(&b,F144_SFX_FOOTSTEP,1U,0U);
    if(n!=m||memcmp(a.samples,b.samples,n*sizeof(int16_t)))return 10;
    /* The historic ORIGINAL must remain available in the comparison tool. */
    g_noise=0xF14451F1U;
    n=make_footstep_original(&a);
    g_noise=0xF14451F1U;
    m=generate_review(&b,F144_SFX_FOOTSTEP,0U,0U);
    if(n!=m||memcmp(a.samples,b.samples,n*sizeof(int16_t)))return 11;
    /* Original and approved footfalls must not alias one another. */
    g_noise=0xF14451F1U;
    n=generate(&a,F144_SFX_FOOTSTEP);
    g_noise=0xF14451F1U;
    m=generate_review(&b,F144_SFX_FOOTSTEP,0U,0U);
    if(n==m&&!memcmp(a.samples,b.samples,n*sizeof(int16_t)))return 12;
    /* Synthesis gain is independent; no global device gain cross-talk. */
    g_volume=0.8f;g_noise=0xF14451F1U;
    n=generate_review(&a,F144_SFX_FOOTSTEP,1U,0U);
    g_volume=0.4f;g_noise=0xF14451F1U;
    m=generate_review(&b,F144_SFX_FOOTSTEP,1U,0U);
    if(n!=m)return 6;
    for(i=0U;i<n;i++) {
        int difference=(int)a.samples[i]-2*(int)b.samples[i];
        if(difference>2||difference<-2)return 7;
    }
    /* Saturated playback retains the established bounded drop policy. */
    for(i=0U;i<F144_VOICES;i++){g_voice[i].prepared=1;g_voice[i].hdr.dwFlags=0;}
    if(find_voice()!=NULL)return 8;
    for(i=0U;i<F144_VOICES;i++)g_voice[i].prepared=0;
    if(find_voice()==NULL)return 9;
    puts("BF24 16 active cues deterministic, bounded, unclipped and muted; approved footstep A matches gameplay, original retained, alternating feet PASS");
    return 0;
}
