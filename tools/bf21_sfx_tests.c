/* BF21 native source synthesis tests, no audio device required. */
#include <stdio.h>
#include <stdint.h>
#include "../src/f144sfx.c"

int main(void)
{
    static F144Voice normal;
    static F144Voice backspace;
    unsigned i,n,m;
    unsigned voiced=0U;
    g_volume=0.80f;g_noise=0xA14451F1u;
    n=make_terminal_key(&normal);
    g_noise=0xA14451F1u;
    m=make_terminal_backspace(&backspace);
    if(n==0U||n!=m||n!=ms_samples(42U))
    {
        puts("FAIL: reversed typing cue length");
        return 1;
    }
    for(i=0U;i<n;++i)
    {
        if(backspace.samples[i]!=normal.samples[n-1U-i])
        {
            puts("FAIL: reverse PCM must match approved key sample exactly");
            return 2;
        }
        if(backspace.samples[i]!=0)++voiced;
    }
    if(voiced<50U){puts("FAIL: reversed typing inaudible");return 3;}
    g_volume=0.0f;g_noise=0xA14451F1u;
    m=make_terminal_backspace(&backspace);
    for(i=0U;i<m;++i)
        if(backspace.samples[i]!=0){puts("FAIL: SFX mute not respected");return 4;}
    g_volume=0.5f;g_noise=0xA14451F1u;
    m=make_terminal_backspace(&backspace);
    if(m!=n||backspace.samples[0]==normal.samples[0])
    {
        puts("FAIL: reverse/volume regression");
        return 5;
    }
    puts("BF21: 42ms approved PCM reversed exactly, volume/mute, determinism PASS");
    return 0;
}
