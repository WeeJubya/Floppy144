/* BF22 completion companion: intercept actual production MIDI messages.
 * No MIDI hardware is required; WinMM's short-message output is captured. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>

static MMRESULT WINAPI capture_completion(HMIDIOUT output,DWORD message);
#define midiOutShortMsg capture_completion
#include "../src/f144audio.c"
#undef midiOutShortMsg

typedef struct MidiEvent {DWORD message;unsigned step;} MidiEvent;
static MidiEvent received[8192];
static unsigned count,clock_step;
static int failures;
static MMRESULT WINAPI capture_completion(HMIDIOUT output,DWORD message)
{
    (void)output;
    if(count<8192u)
    {
        received[count].message=message;
        received[count].step=clock_step;
        ++count;
    }
    else {++failures;puts("FAIL: completion MIDI capture overflow");}
    return MMSYSERR_NOERROR;
}
static void expect(int yes,const char *name)
{
    if(!yes){++failures;printf("FAIL: %s\n",name);}
}
static int has(unsigned step,unsigned status,unsigned data1)
{
    unsigned i;
    for(i=0u;i<count;++i)
        if(received[i].step==step&&
           (received[i].message&255u)==status&&
           ((received[i].message>>8)&255u)==data1)return 1;
    return 0;
}
static int has_any_note(unsigned step,unsigned channel)
{
    unsigned i;
    for(i=0u;i<count;++i)
        if(received[i].step==step&&
           (received[i].message&255u)==(0x90u|channel)&&
           ((received[i].message>>16)&255u)>0u)return 1;
    return 0;
}
int main(void)
{
    unsigned i;
    const double one_step=60.0/110.0/4.0;
    g_available=1;
    g_out=(HMIDIOUT)(uintptr_t)1u;
    g_scene=F144_MUSIC_MENU;
    g_playing=0;
    F144_MusicSetRestoration(100u);
    F144_MusicSetAct(F144_ACT_III);
    F144_MusicSetTempoPercent(46u);
    F144_MusicSetScene(F144_MUSIC_COMPLETION);
    expect(g_completion_intro_pending==1u,"entry arms approval only once");
    expect(has(0u,0xc0u,4u),"electric piano GM #5");
    expect(has(0u,0xc1u,32u),"acoustic bass GM #33");
    expect(has(0u,0xc2u,45u),"pizzicato GM #46");
    expect(has(0u,0xc3u,10u),"music box GM #11");
    expect(has(0u,0xc4u,48u),"string ensemble GM #49");
    expect(has(0u,0xc5u,8u),"celesta GM #9");
    expect(has(0u,0xc6u,12u),"marimba GM #13");
    count=0u;
    clock_step=0u;
    F144_MusicRestart();
    for(i=1u;i<F144_COMPLETION_TOTAL_STEPS;++i)
    {
        clock_step=i;
        process_step(i);
    }
    expect(has(0u,0x90u,72u),"intro C5 piano");
    expect(has(2u,0x90u,76u),"intro E5 piano");
    expect(has(4u,0x95u,79u),"intro G5 celesta");
    expect(has(6u,0x93u,84u),"intro C6 music box");
    expect(has(8u,0x99u,37u),"intro approval stamp");
    expect(!has_any_note(4u,CH_BASS),"half-time bass leaves beat two empty");
    expect(has(8u,0x91u,43u),"half-time bass fifth on third beat");
    expect(has(0u,0x94u,74u),"Cmaj9 extension D");
    expect(has(16u,0x94u,56u),"E7/G# secondary dominant bass");
    expect(has(2u*16u,0x94u,71u),"Am9 extension B");
    expect(has(3u*16u,0x94u,56u),"Fm6 borrowed minor iv");
    expect(has(5u*16u,0x94u,63u),"Abmaj7 borrowed bVI");
    expect(has(6u*16u+8u,0x94u,65u),"G7sus4 second-half suspended fourth");
    expect(has(7u*16u,0x94u,74u),"C6/9 true resolution");
    expect(has_any_note(8u*16u+4u,CH_EXTRA_PIZZ),"marimba middle-section reply");
    expect(has_any_note(9u*16u+4u,CH_CLARINET),"music box middle-section reply");
    expect(has_any_note(8u*16u+4u,CH_MARIMBA),"piano middle-section expansion");
    expect(!has_any_note(24u*16u+4u,CH_MARIMBA),"piano closing section sparse");
    expect(has(31u*16u+1u,0x93u,79u),"final G5 ascent");
    expect(has(31u*16u+5u,0x93u,81u),"final A5 ascent");
    expect(has(31u*16u+9u,0x93u,83u),"final B5 ascent");
    expect(has(31u*16u+12u,0x93u,84u),"final C6 ascent");
    expect(has(31u*16u+12u,0x99u,37u),"final quiet office stamp");
    expect(g_completion_intro_pending==0u,"intro unarmed before first wrap");
    count=0u;
    F144_MusicSeekBar(33u);
    expect(g_step==16u,"completion seek uses 32 bars");
    F144_MusicSeekBar(0u);
    expect(g_completion_intro_pending==0u,"seek never replays approval flourish");
    count=0u;
    /* Drive actual scheduler through full 32-bar loop at 110, even if
       gameplay tempo was previously 46% and restoration 100%. */
    for(i=0u;i<F144_COMPLETION_TOTAL_STEPS;++i)
    {
        clock_step=i+1u;
        F144_AudioUpdate(one_step+0.000001);
    }
    expect(g_step==0u,"completion wraps at 512 steps");
    expect(has(512u,0x94u,74u),"completion Cmaj9 re-enters on wrap");
    expect(has(512u,0xb4u,123u),"previous harmony silenced on wrap");
    expect(!has(512u,0x95u,79u),"approval celesta does not repeat on wrap");
    expect(g_completion_intro_pending==0u,"loop never rearms approval flourish");
    clock_step=513u;
    F144_MusicSetScene(F144_MUSIC_MENU);
    expect(has(513u,0xc0u,12u),"menu marimba restored");
    expect(has(513u,0xc4u,4u),"menu electric-piano harmony restored");
    expect(g_step==0u,"menu starts afresh");
    clock_step=514u;
    F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
    expect(has(514u,0xc0u,12u),"gameplay marimba restored");
    expect(has(514u,0xc4u,48u),"gameplay strings restored");
    F144_MusicSeekBar(601u);
    expect(g_step==16u,"approved gameplay still seeks modulo 600");
    clock_step=515u;
    F144_MusicSetScene(F144_MUSIC_COMPLETION);
    expect(g_completion_intro_pending==1u,"re-entering completion rearms flourish");
    expect(has(515u,0x90u,72u),"re-entry plays opening approval piano");
    g_playing=0;
    g_available=0;
    g_out=NULL;
    if(failures){printf("BF22 COMPLETION MIDI TESTS: FAIL %d\n",failures);return 1;}
    puts("BF22 COMPLETION MIDI TESTS: PASS");
    return 0;
}
