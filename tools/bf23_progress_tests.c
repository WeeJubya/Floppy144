#include "floppy144_restore_presentation.h"
#ifndef BF23_PORTABLE
#include "f144audio.h"
#endif
#include <stdio.h>
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void)
{
    unsigned p,last=0U,ms,arrivals[101]={0};
    CHECK(Floppy144RestoreProgress(1500U,3000U)>50U);
    CHECK(Floppy144RestoreProgress(2999U,3000U)==99U);
    CHECK(Floppy144RestoreProgress(3000U,3000U)==100U);
    CHECK(Floppy144RestoreProgress(1U,0U)==0U);
    for(ms=0U;ms<=3000U;ms++) {
        p=Floppy144RestoreProgress(ms,3000U);
        CHECK(p>=last && p<=last+1U);
        if(p>last)arrivals[p]=ms;
        last=p;
    }
    CHECK(arrivals[80]-arrivals[79]<arrivals[60]-arrivals[59]);
    CHECK(arrivals[94]-arrivals[93]>arrivals[90]-arrivals[89]);
    CHECK(arrivals[99]-arrivals[98]>arrivals[96]-arrivals[95]);
    CHECK(arrivals[100]-arrivals[99]>arrivals[99]-arrivals[98]);
    CHECK(Floppy144RestoreTempo(89U)==100U);
    CHECK(Floppy144RestoreTempo(99U)<Floppy144RestoreTempo(95U));
    CHECK(Floppy144RestoreTempo(100U)==100U);
#ifndef BF23_PORTABLE
    F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
    F144_MusicSetTempoPercent(46U);F144_AudioUpdate(0.3);
    CHECK(F144_MusicGetTempoScale()<0.5f);
    F144_MusicSetTempoPercent(100U);F144_AudioUpdate(0.05);
    CHECK(F144_MusicGetTempoScale()>0.5f && F144_MusicGetTempoScale()<1.0f);
    F144_AudioUpdate(0.3);CHECK(F144_MusicGetTempoScale()==1.0f);
    F144_MusicSetTempoPercent(46U);F144_AudioUpdate(0.3);
    F144_MusicSetScene(F144_MUSIC_MENU);F144_AudioUpdate(0.3);
    CHECK(F144_MusicGetTempoScale()==1.0f);
    F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
    F144_MusicSetTempoPercent(0U);F144_AudioUpdate(0.3);
    CHECK(F144_MusicGetTempoScale()==1.0f);
    F144_MusicSetVolume(0.0f);F144_MusicSetTempoPercent(46U);F144_AudioUpdate(0.3);
    CHECK(F144_MusicGetVolume()==0.0f);
    F144_MusicSetTempoPercent(100U);F144_AudioUpdate(0.3);
    CHECK(F144_MusicGetTempoScale()==1.0f);
#endif
    puts("BF23 progress timing / scheduler tempo tests PASS");return 0;
}
