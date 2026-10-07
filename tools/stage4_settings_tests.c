#include "f144_platform.h"
#include "floppy144_draw.h"
#include "floppy144_settings.h"
#include "floppy144_settings_runtime.h"
#include "floppy144_settings_view.h"
#include "floppy144_timing.h"
#include <stdio.h>
#include <string.h>

static int failures;
static uint32_t pixels[640U*360U];

static void Expect(int condition,const char *label)
{
    if(!condition){++failures;printf("FAIL: %s\n",label);}
}

static uint32_t Hash(const uint32_t *data,uint32_t count)
{
    uint32_t h=2166136261U,i;
    for(i=0U;i<count;++i){h^=data[i];h*=16777619U;}
    return h;
}

static void TestDefaultsAndRanges(void)
{
    Floppy144Settings s;
    Floppy144SettingsReset(&s);
    Expect(s.crt_mode==(uint8_t)FLOPPY144_CRT_FULL,"default CRT");
    Expect(s.text_speed==(uint8_t)FLOPPY144_TEXT_SPEED_NORMAL,"default text speed");
    Expect(s.music_volume==10U&&s.sfx_volume==10U,"default volumes");
    Expect(s.autosave_mode==(uint8_t)FLOPPY144_AUTOSAVE_5_MINUTES,"default autosave");
    Expect(Floppy144SettingsSetCrtMode(&s,FLOPPY144_CRT_REDUCED),"CRT reduced");
    Expect(Floppy144SettingsSetCrtMode(&s,FLOPPY144_CRT_OFF),"CRT off");
    Expect(!Floppy144SettingsSetCrtMode(&s,FLOPPY144_CRT_COUNT),"invalid CRT");
    Expect(Floppy144SettingsSetTextSpeed(&s,FLOPPY144_TEXT_SPEED_FAST),"fast text");
    Expect(Floppy144SettingsSetTextSpeed(&s,FLOPPY144_TEXT_SPEED_INSTANT),"instant text");
    Expect(!Floppy144SettingsSetTextSpeed(&s,FLOPPY144_TEXT_SPEED_COUNT),"invalid text");
    Expect(Floppy144SettingsSetMusicVolume(&s,0U),"music min");
    Expect(Floppy144SettingsSetMusicVolume(&s,10U),"music max");
    Expect(!Floppy144SettingsSetMusicVolume(&s,11U),"music invalid");
    Expect(Floppy144SettingsSetSfxVolume(&s,0U),"SFX min");
    Expect(Floppy144SettingsSetSfxVolume(&s,10U),"SFX max");
    Expect(!Floppy144SettingsSetSfxVolume(&s,11U),"SFX invalid");
    Expect(Floppy144SettingsSetAutosaveMode(&s,FLOPPY144_AUTOSAVE_10_MINUTES),"autosave 10");
    Expect(Floppy144SettingsSetAutosaveMode(&s,FLOPPY144_AUTOSAVE_30_MINUTES),"autosave 30");
    Expect(Floppy144SettingsSetAutosaveMode(&s,FLOPPY144_AUTOSAVE_OFF),"autosave off");
}

static void TestRuntime(void)
{
    Floppy144Settings s;
    Floppy144TimingState timing;
    Floppy144TimingEvents events;
    uint32_t sample[32],hashes[3],mode,i;
    Floppy144Surface small={sample,8U,4U};

    Floppy144SettingsReset(&s);
    Expect(Floppy144SettingsTextElapsedMs(&s,1000U)==1000U,"normal text clock");
    s.text_speed=(uint8_t)FLOPPY144_TEXT_SPEED_FAST;
    Expect(Floppy144SettingsTextElapsedMs(&s,1000U)==2000U,"fast text clock");
    s.text_speed=(uint8_t)FLOPPY144_TEXT_SPEED_INSTANT;
    Expect(Floppy144SettingsTextElapsedMs(&s,1U)==UINT32_MAX,"instant text clock");

    for(mode=0U;mode<3U;++mode)
    {
        for(i=0U;i<32U;++i)sample[i]=FLOPPY144_RGB(160,120,80);
        s.crt_mode=(uint8_t)mode;
        Floppy144SettingsApplyCrtFilter(&small,&s);
        hashes[mode]=Hash(sample,32U);
    }
    Expect(hashes[0]!=hashes[1]&&hashes[1]!=hashes[2]&&hashes[0]!=hashes[2],"CRT modes differ");

    Floppy144TimingReset(&timing,1000ULL,300000U);
    Floppy144TimingSetAutosaveInterval(&timing,2000ULL,600000U);
    events=Floppy144TimingAdvance(&timing,601999ULL);
    Expect(events.autosave_due==0U,"rearmed autosave not early");
    events=Floppy144TimingAdvance(&timing,602000ULL);
    Expect(events.autosave_due!=0U,"rearmed autosave due");
    Floppy144TimingSetAutosaveInterval(&timing,700000ULL,0U);
    events=Floppy144TimingAdvance(&timing,5000000ULL);
    Expect(events.autosave_due==0U,"autosave off cancels deadline");
}

static void TestView(void)
{
    Floppy144Settings s;
    Floppy144Surface surface={pixels,640U,360U};
    uint32_t option,first=0U,current;
    Floppy144SettingsReset(&s);

    for(option=0U;option<(uint32_t)FLOPPY144_SETTINGS_OPTION_COUNT;++option)
    {
        memset(pixels,0,sizeof(pixels));
        Floppy144SettingsViewDraw(&surface,&s,(Floppy144SettingsOption)option,NULL);
        current=Hash(pixels,640U*360U);
        if(option==0U){first=current;Expect(first!=0U,"settings renders");}
        else Expect(current!=first,"selection renders distinctly");
    }
}

int main(void)
{
    TestDefaultsAndRanges();
    TestRuntime();
    TestView();
    if(failures!=0){printf("STAGE 4C SETTINGS TESTS: FAIL (%d)\n",failures);return 1;}
    printf("STAGE 4C SETTINGS TESTS: PASS\n");
    return 0;
}
