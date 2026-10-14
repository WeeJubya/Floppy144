/* Development-only WinMM listening harness; never linked into Floppy144.exe. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "f144audio.h"
#include "f144sfx.h"
#include "floppy144_restore_presentation.h"

static int pressed(int vk,int *previous)
{
    int down=(GetAsyncKeyState(vk)&0x8000)!=0;
    int next=down && !*previous;
    *previous=down;
    return next;
}

int main(void)
{
    static const unsigned seek_bars[]={0U,100U,200U,300U,400U,500U,580U};
    LARGE_INTEGER freq,last,now;
    int number_keys[4]={0},seek_keys[7]={0};
    int nextkey=0,prevkey=0,playkey=0,up=0,down=0,svup=0,svdown=0;
    int rkey=0,ekey=0,menu_key=0,ending_key=0,game_key=0;
    int original=0,authentic=0,retro=0,restore_key=0,cancel_key=0;
    unsigned cue=0U,i,variant=0U,restore_ms=0U,restore_progress=0U;
    int restoring=0;
    double restore_accum=0.0;
    int midi=F144_AudioInit(),sfx=F144_SFXInit();
    if(!midi && !sfx) { puts("No MIDI or waveOut device available."); return 1; }
    F144_MusicSetRestoration(0U);
    if(midi) F144_MusicRestart();
    puts("F144 original-sound review: ESC quit | 1-4 Acts | F1-F7: 0,5,10,15,20,25,29m");
    puts("N/B next/previous SFX | SPACE play cue | UP/DOWN music | LEFT/RIGHT SFX");
    puts("Restoration corruption: R increase by 10, E decrease by 10");
    puts("BF22 REVIEW: M = PLEASE HOLD (menu), C = YOUR CONTRIBUTION HAS BEEN NOTED (ending), G = gameplay");
    printf("Current cue %u/%u (see src/f144sfx.h for names)\n",cue,F144_SFX_COUNT-1U);
    puts("BF24: O ORIGINAL | A REVISED A: PHYSICALLY AUTHENTIC | S REVISED B: STYLISED RETRO");
    puts("T: BF23 3-second restoration demonstration | X: interrupt and restore tempo");
    printf("Cue: %s / ORIGINAL\n",F144_SFXReviewName((F144SFX)cue));
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    for(;;) {
        double dt;
        QueryPerformanceCounter(&now);
        dt=(double)(now.QuadPart-last.QuadPart)/(double)freq.QuadPart;
        last=now;
        if(pressed('T',&restore_key)) {
            restoring=1;restore_ms=0U;restore_progress=0U;restore_accum=0.0;
            F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
            puts("RESTORING...");
        }
        if(pressed('X',&cancel_key)) {restoring=0;F144_MusicSetTempoPercent(100U);puts("Interrupted / tempo returning to normal");}
        if(restoring) {
            unsigned next;
            restore_accum+=dt*1000.0;
            restore_ms=(unsigned)restore_accum;
            next=Floppy144RestoreProgress(restore_ms,3000U);
            F144_MusicSetTempoPercent(Floppy144RestoreTempo(next));
            if(next!=restore_progress) {
                printf("Restoration %u%% / tempo %u%%\n",next,Floppy144RestoreTempo(next));
                F144_SFXPlay(next==100U?F144_SFX_FILE_RESTORE:
                    (next>=97U?F144_SFX_RESTORE_TICK_HIGH:F144_SFX_RESTORE_TICK));
                restore_progress=next;
            }
            if(next==100U)restoring=0;
        }
        F144_AudioUpdate(dt);
        for(i=0;i<4;i++) {
            if(pressed('1'+(int)i,&number_keys[i])) {
                F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
                F144_MusicSetAct((F144Act)i);
                printf("Act %u\n",i);
            }
        }
        for(i=0;i<7;i++) {
            if(pressed(VK_F1+(int)i,&seek_keys[i])) {
                F144_MusicSeekBar(seek_bars[i]);
                printf("Seek %u:00\n",seek_bars[i]/20U);
            }
        }
        if(pressed('M',&menu_key)){
            F144_MusicSetScene(F144_MUSIC_MENU);
            puts("BF22 MENU: PLEASE HOLD / 70 BPM / looping");
        }
        if(pressed('C',&ending_key)){
            F144_MusicSetScene(F144_MUSIC_COMPLETION);
            puts("BF22 COMPLETION: YOUR CONTRIBUTION HAS BEEN NOTED / 65 BPM / looping");
        }
        if(pressed('G',&game_key)){
            F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
            puts("BF22 GAMEPLAY: original approved 80 BPM composition");
        }
        if(pressed('O',&original)){variant=0U;puts("ORIGINAL");}
        if(pressed('A',&authentic)){variant=1U;puts("REVISED A: PHYSICALLY AUTHENTIC / awaiting review");}
        if(pressed('S',&retro)){variant=2U;puts("REVISED B: STYLISED RETRO / awaiting review");}
        F144_SFXReviewVariant(variant);
        if(pressed('N',&nextkey)) { cue=(cue+1U)%F144_SFX_COUNT; printf("SFX %u: %s\n",cue,F144_SFXReviewName((F144SFX)cue)); }
        if(pressed('B',&prevkey)) { cue=(cue+F144_SFX_COUNT-1U)%F144_SFX_COUNT; printf("SFX %u: %s\n",cue,F144_SFXReviewName((F144SFX)cue)); }
        if(pressed(VK_SPACE,&playkey)) F144_SFXPlay((F144SFX)cue);
        if(pressed(VK_UP,&up)) F144_MusicSetVolume(F144_MusicGetVolume()+0.1f);
        if(pressed(VK_DOWN,&down)) F144_MusicSetVolume(F144_MusicGetVolume()-0.1f);
        if(pressed(VK_RIGHT,&svup)) F144_SFXSetVolume(F144_SFXGetVolume()+0.1f);
        if(pressed(VK_LEFT,&svdown)) F144_SFXSetVolume(F144_SFXGetVolume()-0.1f);
        if(pressed('R',&rkey)) F144_MusicSetRestoration(F144_MusicGetRestoration()+10U);
        if(pressed('E',&ekey)) {
            unsigned restoration=F144_MusicGetRestoration();
            F144_MusicSetRestoration(restoration>10U?restoration-10U:0U);
        }
        if(GetAsyncKeyState(VK_ESCAPE)&0x8000) break;
        Sleep(4);
    }
    F144_SFXShutdown();
    F144_AudioShutdown();
    return 0;
}