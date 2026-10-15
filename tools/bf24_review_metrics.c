/*
 * BF24 native Windows waveform audit for the ORIGINAL / Revised A / Revised B
 * synthesis already used by the existing f144_audio_review.exe harness.
 *
 * No playback device, file samples, or audio API calls are required. Each
 * row is a reproducible output of precisely the same procedural generator
 * with fixed gain and RNG seed as the listening harness.
 */
#define F144_SFX_REVIEW
#include "../src/f144sfx.c"
#include <math.h>
#include <stdio.h>

static double window_rms(const F144Voice *voice,unsigned begin,unsigned end)
{
    unsigned i;
    double sum=0.0;
    if(begin>=end)return 0.0;
    for(i=begin;i<end;++i)
    {
        double v=(double)voice->samples[i];
        sum+=v*v;
    }
    return sqrt(sum/(double)(end-begin))/32768.0;
}

static double dbfs(double x)
{
    return x>0.00000000001?20.0*log10(x):-120.0;
}
int main(void)
{
    static F144Voice voice;
    unsigned cue,variant,foot,n,i,peak;
    double energy,difference,previous,bright,rms;
    puts("effect_id,effect,variant,foot,samples,duration_ms,peak_dbfs,rms_dbfs,crest_db,difference_energy_ratio,zero_crossings,heel_0_32_dbfs,sole_32_72_dbfs,tail_72_end_dbfs");
    for(cue=0U;cue<F144_SFX_COUNT;++cue)
        for(variant=0U;variant<3U;++variant)
            for(foot=0U;foot<(cue==F144_SFX_FOOTSTEP?2U:1U);++foot)
            {
                unsigned crossings=0U;
                double maxamp;
                g_volume=0.8f;
                g_noise=0xF14451F1U;
                n=generate_review(&voice,(F144SFX)cue,variant,foot);
                if(n==0U||n>F144_MAX_SAMPLES)
                {
                    fprintf(stderr,"Invalid waveform for cue %u variant %u\n",cue,variant);
                    return 1;
                }
                energy=0.0;difference=0.0;previous=0.0;peak=0U;
                for(i=0U;i<n;++i)
                {
                    double v=(double)voice.samples[i];
                    unsigned amp=(unsigned)(v<0.0?-v:v);
                    if(amp>peak)peak=amp;
                    energy+=v*v;
                    if(i>0U)
                    {
                        double d=v-previous;
                        difference+=d*d;
                        if((v>0.0&&previous<0.0)||(v<0.0&&previous>0.0))
                            ++crossings;
                    }
                    previous=v;
                }
                rms=sqrt(energy/(double)n)/32768.0;
                maxamp=(double)peak/32768.0;
                bright=energy>0.0?difference/(4.0*energy):0.0;
                printf("%u,%s,%s,%u,%u,%.2f,%.2f,%.2f,%.2f,%.6f,%u,%.2f,%.2f,%.2f\n",
                    cue,F144_SFXReviewName((F144SFX)cue),
                    variant==0U?"ORIGINAL":variant==1U?"REVISED_A":"REVISED_B",
                    foot,n,(double)n*1000.0/F144_RATE,
                    dbfs(maxamp),dbfs(rms),rms>0.0?dbfs(maxamp/rms):-120.0,
                    bright,crossings,
                    dbfs(window_rms(&voice,0U,ms_samples(32U)<n?ms_samples(32U):n)),
                    dbfs(window_rms(&voice,ms_samples(32U)<n?ms_samples(32U):n,
                        ms_samples(72U)<n?ms_samples(72U):n)),
                    dbfs(window_rms(&voice,ms_samples(72U)<n?ms_samples(72U):n,n)));
            }
    return 0;
}
