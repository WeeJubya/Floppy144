/* Review-only candidates must remain deterministic and outside the game. */
#define F144_SFX_REVIEW
#include "../src/f144sfx.c"
#include <stdio.h>
int main(void)
{
    static F144Voice a,b;
    unsigned effect,variant,i,n,m;
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
    puts("BF24 all ORIGINAL/A/B cues deterministic, bounded, unclipped, muted; alternating feet PASS");
    return 0;
}
