/* Development-only WinMM listening harness; never linked into Floppy144.exe. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "f144audio.h"
#include "f144sfx.h"

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
    int rkey=0,ekey=0;
    unsigned cue=0U,i;
    int midi=F144_AudioInit(),sfx=F144_SFXInit();
    if(!midi && !sfx) { puts("No MIDI or waveOut device available."); return 1; }
    F144_MusicSetRestoration(0U);
    if(midi) F144_MusicRestart();
    puts("F144 original-sound review: ESC quit | 1-4 Acts | F1-F7: 0,5,10,15,20,25,29m");
    puts("N/B next/previous SFX | SPACE play cue | UP/DOWN music | LEFT/RIGHT SFX");
    puts("Restoration corruption: R increase by 10, E decrease by 10");
    printf("Current cue %u/%u (see src/f144sfx.h for names)\n",cue,F144_SFX_COUNT-1U);
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    for(;;) {
        double dt;
        QueryPerformanceCounter(&now);
        dt=(double)(now.QuadPart-last.QuadPart)/(double)freq.QuadPart;
        last=now;
        F144_AudioUpdate(dt);
        for(i=0;i<4;i++) {
            if(pressed('1'+(int)i,&number_keys[i])) {
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
        if(pressed('N',&nextkey)) { cue=(cue+1U)%F144_SFX_COUNT; printf("SFX %u\n",cue); }
        if(pressed('B',&prevkey)) { cue=(cue+F144_SFX_COUNT-1U)%F144_SFX_COUNT; printf("SFX %u\n",cue); }
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