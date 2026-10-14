/* BF22 generated GM MIDI inspection, independent of a MIDI device.
 * Include the production engine with only midiOutShortMsg redirected. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static MMRESULT WINAPI bf22_capture(HMIDIOUT device,DWORD message);
#define midiOutShortMsg bf22_capture
#include "../src/f144audio.c"
#undef midiOutShortMsg

typedef struct Bf22Event { DWORD message; unsigned step; } Bf22Event;
static Bf22Event events[8192];
static unsigned used,at_step;
static int failures;
static MMRESULT WINAPI bf22_capture(HMIDIOUT device,DWORD message)
{
    (void)device;
    if(used<8192u) {events[used].message=message;events[used].step=at_step;++used;}
    else {++failures;puts("FAIL: MIDI capture overflow");}
    return MMSYSERR_NOERROR;
}
static void check(int condition,const char *label)
{
    if(!condition){++failures;printf("FAIL: %s\n",label);}
}
static int midi_seen(unsigned step,unsigned status,unsigned pitch)
{
    unsigned i;
    for(i=0u;i<used;++i)
        if(events[i].step==step &&
           (events[i].message&255u)==status &&
           ((events[i].message>>8)&255u)==pitch)return 1;
    return 0;
}
int main(void)
{
    unsigned i,voice_count;
    /* Fake a present, working WinMM device; only outgoing messages are mocked. */
    g_available=1;
    g_out=(HMIDIOUT)(uintptr_t)1u;
    g_playing=0;
    g_scene=F144_MUSIC_GAMEPLAY;
    F144_MusicSetScene(F144_MUSIC_MENU);
    check(F144_MusicGetScene()==F144_MUSIC_MENU,"menu selection");
    check(midi_seen(0u,0xc0u,12u),"GM #13 marimba");
    check(midi_seen(0u,0xc1u,32u),"GM #33 bass");
    check(midi_seen(0u,0xc2u,45u),"GM #46 pizzicato");
    check(midi_seen(0u,0xc4u,4u),"GM #5 electric piano");
    check(midi_seen(0u,0xc5u,8u),"GM #9 celesta");
    used=0u;
    at_step=0u;
    F144_MusicRestart();
    for(i=1u;i<F144_MENU_TOTAL_STEPS;++i)
    {
        at_step=i;
        process_step(i);
    }
    check(midi_seen(0u,0x91u,36u),"bar 1 bass C");
    check(midi_seen(0u,0x90u,72u),"bar 1 marimba motif");
    check(midi_seen(0u,0x94u,60u),"bar 1 C triad");
    check(midi_seen(0u,0x99u,42u),"metronomic GM hat");
    check(midi_seen(2u,0x92u,72u),"offbeat pizzicato");
    check(midi_seen(16u,0x94u,68u),"E7 secondary dominant G sharp");
    check(midi_seen(3u*16u,0x94u,56u),"Fm6 borrowed Ab");
    check(midi_seen(5u*16u,0x94u,63u),"Abmaj7 borrowed Eb");
    check(midi_seen(7u*16u,0x94u,68u),"G7 flat ninth");
    check(midi_seen(9u*16u+4u,0x95u,80u),"celesta reply in middle section");
    check(midi_seen(8u*16u+3u,0x90u,76u),"expanded middle melody");
    check(!midi_seen(24u*16u+3u,0x90u,76u),"closing melody is sparse");
    check(midi_seen(31u*16u+12u,0x91u,35u),"final B approaches initial C");
    check(midi_seen(31u*16u+14u,0x99u,76u),"last woodblock stamp");
    voice_count=0u;
    for(i=0u;i<used;++i)
        if(events[i].step<16u && (events[i].message&255u)==0x90u &&
           ((events[i].message>>16)&255u)!=0u)++voice_count;
    check(voice_count==3u,"opening melody is three notes");

    F144_MusicSeekBar(33u);
    check(g_step==16u,"menu seeks modulo 32 bars");
    F144_MusicSeekBar(0u);
    at_step=0u;
    used=0u;
    /* Drive the actual 110 BPM scheduler through a complete 512-step cycle. */
    for(i=0u;i<F144_MENU_TOTAL_STEPS;++i)
    {
        at_step=i+1u;
        F144_AudioUpdate((60.0/110.0/4.0)+0.000001);
    }
    check(g_step==0u,"exact 32-bar rollover");
    check(midi_seen(512u,0x91u,36u),"C bass re-enters exactly on wrap");
    check(midi_seen(512u,0xb1u,123u),"old notes released on wrap");

    at_step=513u;
    F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
    check(midi_seen(513u,0xc4u,48u),"original string pad restored");
    check(midi_seen(513u,0xc5u,71u),"original clarinet counter restored");
    check(F144_MusicGetScene()==F144_MUSIC_GAMEPLAY,"gameplay cue restored");
    check(g_step==0u,"gameplay restart on change");
    F144_MusicSeekBar(601u);
    check(g_step==16u,"gameplay still seeks modulo 600 bars");
    F144_MusicSetScene(F144_MUSIC_COMPLETION);
    F144_MusicSeekBar(17u);
    check(g_step==16u,"completion still seeks modulo 16 bars");
    check(midi_seen(513u,0xc0u,0u),"completion acoustic piano unchanged");
    g_playing=0;
    g_available=0;
    g_out=NULL;
    if(failures){printf("BF22 MENU MIDI TESTS: FAIL %d\n",failures);return 1;}
    puts("BF22 MENU MIDI TESTS: PASS");
    return 0;
}
