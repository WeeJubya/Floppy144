/*
 * S4F-02 startup-intro regression vectors.
 */
#include "floppy144_intro.h"
#include "floppy144_settings.h"
#include "floppy144_settings_runtime.h"
#include "floppy144_timing.h"

#include <stdio.h>
#include <string.h>

#define WIDTH 640U
#define HEIGHT 360U

static uint32_t pixels[WIDTH * HEIGHT];

static int require_true(int condition,const char *message)
{
    if(!condition)
    {
        fprintf(stderr,"FAIL: %s\n",message);
        return 0;
    }

    return 1;
}

static uint64_t checksum(const uint32_t *data,uint32_t count)
{
    uint64_t value=1469598103934665603ULL;
    uint32_t index;

    for(index=0U;index<count;++index)
    {
        value^=(uint64_t)data[index];
        value*=1099511628211ULL;
    }

    return value;
}

static uint64_t render(
    uint32_t elapsed_ms,
    Floppy144TextSpeed speed,
    Floppy144CrtMode crt,
    uint8_t sfx_volume
)
{
    Floppy144Surface surface={pixels,WIDTH,HEIGHT};
    Floppy144Settings settings;

    Floppy144SettingsReset(&settings);
    settings.text_speed=(uint8_t)speed;
    settings.crt_mode=(uint8_t)crt;
    settings.sfx_volume=sfx_volume;

    memset(pixels,0,sizeof(pixels));
    Floppy144IntroDraw(
        &surface,
        elapsed_ms,
        Floppy144SettingsTextElapsedMs(&settings,1U)
    );
    Floppy144SettingsApplyCrtFilter(&surface,&settings);

    return checksum(pixels,WIDTH*HEIGHT);
}

int main(void)
{
    Floppy144TimingState timing;
    uint64_t normal_program;
    uint64_t fast_program;
    uint64_t instant_program;
    uint64_t muted_network;
    uint64_t audible_network;
    uint64_t full_crt;
    uint64_t reduced_crt;
    uint64_t off_crt;

    if(!require_true(Floppy144IntroBeatAt(0U)==FLOPPY144_INTRO_DISCOVERY,"0 ms discovery")) return 1;
    if(!require_true(Floppy144IntroBeatAt(2599U)==FLOPPY144_INTRO_DISCOVERY,"discovery end")) return 2;
    if(!require_true(Floppy144IntroBeatAt(2600U)==FLOPPY144_INTRO_INSERTION,"insertion start")) return 3;
    if(!require_true(Floppy144IntroBeatAt(4799U)==FLOPPY144_INTRO_INSERTION,"insertion end")) return 4;
    if(!require_true(Floppy144IntroBeatAt(4800U)==FLOPPY144_INTRO_PROGRAM_START,"program start")) return 5;
    if(!require_true(Floppy144IntroBeatAt(6999U)==FLOPPY144_INTRO_PROGRAM_START,"program end")) return 6;
    if(!require_true(Floppy144IntroBeatAt(7000U)==FLOPPY144_INTRO_GDR_CONNECTION,"network start")) return 7;
    if(!require_true(Floppy144IntroBeatAt(10399U)==FLOPPY144_INTRO_GDR_CONNECTION,"network end")) return 8;
    if(!require_true(Floppy144IntroBeatAt(10400U)==FLOPPY144_INTRO_COMPLETE,"intro completion")) return 9;

    if(!require_true(Floppy144IntroActionSkips(F144_ACTION_CONFIRM),"confirm skips")) return 10;
    if(!require_true(Floppy144IntroActionSkips(F144_ACTION_BACK),"back skips")) return 11;
    if(!require_true(Floppy144IntroActionSkips(F144_ACTION_MENU),"menu skips")) return 12;
    if(!require_true(!Floppy144IntroActionSkips(F144_ACTION_MOVE_UP),"movement does not skip")) return 13;

    if(!require_true(render(1200U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"discovery renders")) return 14;
    if(!require_true(render(3600U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"insertion renders")) return 15;
    if(!require_true(render(5600U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"program renders")) return 16;
    if(!require_true(render(8500U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"network renders")) return 17;

    normal_program=render(5550U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    fast_program=render(5550U,FLOPPY144_TEXT_SPEED_FAST,FLOPPY144_CRT_OFF,10U);
    instant_program=render(5550U,FLOPPY144_TEXT_SPEED_INSTANT,FLOPPY144_CRT_OFF,10U);
    if(!require_true(normal_program!=fast_program,"fast text changes staged reveal")) return 18;
    if(!require_true(fast_program!=instant_program,"instant text changes staged reveal")) return 19;

    muted_network=render(9000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,0U);
    audible_network=render(9000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    if(!require_true(muted_network==audible_network,"audio volume cannot alter intro visuals")) return 20;

    full_crt=render(9000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_FULL,10U);
    reduced_crt=render(9000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_REDUCED,10U);
    off_crt=render(9000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    if(!require_true(full_crt!=reduced_crt,"full/reduced CRT differ")) return 21;
    if(!require_true(reduced_crt!=off_crt,"reduced/off CRT differ")) return 22;

    Floppy144TimingReset(&timing,1000U,300000U);
    if(!require_true(Floppy144TimingSplashElapsedMs(&timing,1500U)==500U,"launch intro elapsed")) return 23;
    Floppy144TimingStopSplash(&timing);
    Floppy144TimingStartSplash(&timing,5000U);
    if(!require_true(timing.splash_active!=0U,"replay rearms intro")) return 24;
    if(!require_true(Floppy144TimingSplashElapsedMs(&timing,5250U)==250U,"replay restarts elapsed")) return 25;
    if(!require_true(timing.next_autosave_ms==301000U,"replay preserves autosave deadline")) return 26;

    puts("S4F-02 INTRO VECTORS: PASS");
    puts("beats=discovery,insertion,program,gdr duration=10400ms");
    puts("skip=confirm,back,menu replay=timing-rearm settings=text+crt audio=silent-safe");
    return 0;
}
