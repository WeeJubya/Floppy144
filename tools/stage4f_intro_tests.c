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

/* Exercise the same profile-aware drawing entry point used by Windows. */
static uint64_t render_login(uint32_t elapsed_ms,const char *name,bool has_profile)
{
    Floppy144Surface surface={pixels,WIDTH,HEIGHT};
    memset(pixels,0,sizeof(pixels));
    Floppy144IntroDrawForProfile(&surface,elapsed_ms,1U,name,false,has_profile);
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
    if(!require_true(Floppy144IntroBeatAt(6399U)==FLOPPY144_INTRO_INSERTION,"insertion end")) return 4;
    if(!require_true(Floppy144IntroBeatAt(6400U)==FLOPPY144_INTRO_PROGRAM_START,"program start")) return 5;
    if(!require_true(Floppy144IntroBeatAt(14299U)==FLOPPY144_INTRO_PROGRAM_START,"program end")) return 6;
    if(!require_true(Floppy144IntroBeatAt(14300U)==FLOPPY144_INTRO_GDR_CONNECTION,"network start")) return 7;
    if(!require_true(Floppy144IntroBeatAt(17999U)==FLOPPY144_INTRO_GDR_CONNECTION,"network end")) return 8;
    if(!require_true(Floppy144IntroBeatAt(18000U)==FLOPPY144_INTRO_COMPLETE,"intro completion")) return 9;

    if(!require_true(!Floppy144IntroActionSkips(F144_ACTION_CONFIRM),"confirm cannot skip essential intro")) return 10;
    if(!require_true(!Floppy144IntroActionSkips(F144_ACTION_BACK),"back cannot skip")) return 11;
    if(!require_true(!Floppy144IntroActionSkips(F144_ACTION_MENU),"menu cannot skip")) return 12;
    if(!require_true(!Floppy144IntroActionSkips(F144_ACTION_MOVE_UP),"movement does not skip")) return 13;

    if(!require_true(render(1400U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"discovery renders")) return 14;
    if(!require_true(render(5200U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"insertion renders")) return 15;
    if(!require_true(render(12150U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"program renders")) return 16;
    if(!require_true(render(17000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=0U,"network renders")) return 17;

    /* BF20: discovery disk and grey foreground both meet the wall/floor line.
     * Pixel assertions exercise the actual renderer rather than only vector data.
     * Reveal at t=0 must reproduce discovery exactly: no first-frame jump. */
    if(!require_true(
        render(2599U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)==
        render(2600U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U),
        "discovery-to-reveal alignment has no frame jump")) return 30;
    (void)render(1400U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    if(!require_true(
        pixels[306U*WIDTH+100U]==0xb09d8bU &&
        pixels[307U*WIDTH+100U]==0x5d452eU,
        "rear wall terminates at the floor baseline")) return 31;
    if(!require_true(
        pixels[307U*WIDTH+176U]==0x131313U &&
        pixels[308U*WIDTH+176U]==0x5d452eU,
        "discovery disk touches the dark floor strip")) return 32;
    if(!require_true(
        pixels[307U*WIDTH+250U]==0xa6a6a6U &&
        pixels[308U*WIDTH+250U]==0x5d452eU,
        "discovery grey panel touches the dark floor strip")) return 33;

    /* New credits print progressively, then leave with the zoom. */
    if(!require_true(render(50U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=
        render(2000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U),
        "GDR/PRESENTS title is typed, not immediately complete")) return 34;
    if(!require_true(render(2600U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U)!=
        render(3350U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U),
        "GDR credits leave during animation")) return 35;
    /* A black vector-guide stroke used to cross the drive at (460,174). */
    (void)render(6400U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    if(!require_true(pixels[174U*WIDTH+460U]==0xd2ceb6U,
        "motion guide strokes are not rendered into computer scene"))return 36;

    /* During insertion, the floppy retains its blank paper label.
     * Inspect pixels close to the drive, where lettering previously appeared. */
    (void)render(FLOPPY144_INTRO_INSERTION_END_MS+1800U,
        FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    {
        uint32_t x,y,nonblank=0U;
        for(y=171U;y<180U;++y)
            for(x=325U;x<395U;++x)
                if(pixels[y*WIDTH+x]!=0xf2f2f2U)++nonblank;
        if(!require_true(nonblank==0U,
            "floppy label is unlettered near the drive"))return 43;
    }

    /* The grey cabinet base follows the moving wall/floor seam mid-zoom. */
    (void)render(FLOPPY144_INTRO_DISCOVERY_END_MS+1000U,
        FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    if(!require_true(
        pixels[336U*WIDTH+450U]==0xa6a6a6U &&
        pixels[337U*WIDTH+450U]==0x5d452eU,
        "grey cabinet stays anchored to the zooming floor"))return 37;
    /* Initialising is amber even after later lines become active. */
    (void)render(FLOPPY144_INTRO_PROGRAM_END_MS+50U,
        FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    {
        uint32_t x,y,amber=0U;
        for(y=135U;y<143U;++y)
            for(x=30U;x<320U;++x)
                if(pixels[y*WIDTH+x]==0xc19c59U)++amber;
        if(!require_true(amber>0U,
            "spaced INITIALISING dots are printed in amber"))return 38;
    }

    if(!require_true(render_login(FLOPPY144_INTRO_DURATION_MS,
            "OPERATOR TWELVE",true)!=
        render_login(FLOPPY144_INTRO_DURATION_MS+2200U,
            "OPERATOR TWELVE",true),
        "persisted profile name types in instead of appearing at once"))return 39;
    if(!require_true(
        render_login(FLOPPY144_INTRO_DURATION_MS+800U,"",false)!=
        render_login(FLOPPY144_INTRO_DURATION_MS+1180U,"",false),
        "guest login cursor flashes while awaiting operator input"))return 40;
    (void)render_login(FLOPPY144_INTRO_DURATION_MS+800U,"",false);
    {
        uint32_t x,y,green=0U;
        for(y=302U;y<311U;++y)
            for(x=30U;x<260U;++x)
                if(pixels[y*WIDTH+x]==0x84d399U)++green;
        if(!require_true(green>0U,
            "guest access guidance appears with empty operator profile"))return 41;
    }

    normal_program=render(12550U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    fast_program=render(12550U,FLOPPY144_TEXT_SPEED_FAST,FLOPPY144_CRT_OFF,10U);
    instant_program=render(12550U,FLOPPY144_TEXT_SPEED_INSTANT,FLOPPY144_CRT_OFF,10U);
    if(!require_true(normal_program!=fast_program,"fast text changes staged reveal")) return 18;
    if(!require_true(fast_program!=instant_program,"instant text changes staged reveal")) return 19;

    muted_network=render(18000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,0U);
    audible_network=render(18000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    if(!require_true(muted_network==audible_network,"audio volume cannot alter intro visuals")) return 20;

    full_crt=render(18000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_FULL,10U);
    reduced_crt=render(18000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_REDUCED,10U);
    off_crt=render(18000U,FLOPPY144_TEXT_SPEED_NORMAL,FLOPPY144_CRT_OFF,10U);
    if(!require_true(full_crt!=reduced_crt,"full/reduced CRT differ")) return 21;
    if(!require_true(reduced_crt!=off_crt,"reduced/off CRT differ")) return 22;

    Floppy144TimingReset(&timing,1000U,300000U);
    if(!require_true(Floppy144TimingSplashElapsedMs(&timing,1500U)==500U,"launch intro elapsed")) return 23;
    Floppy144TimingStopSplash(&timing);
    Floppy144TimingStartSplash(&timing,5000U);
    if(!require_true(timing.splash_active!=0U,"replay rearms intro")) return 24;
    if(!require_true(Floppy144TimingSplashElapsedMs(&timing,5250U)==250U,"replay restarts elapsed")) return 25;
    if(!require_true(timing.next_autosave_ms==301000U,"replay preserves autosave deadline")) return 26;

    if(!require_true(!Floppy144IntroLoginReady(17999U) &&
        Floppy144IntroLoginReady(18000U),"LOGIN is gated until full boot"))return 27;

    if(!require_true(!Floppy144IntroHasPersonalLogin(NULL) &&
        !Floppy144IntroHasPersonalLogin("") &&
        !Floppy144IntroHasPersonalLogin("GUEST") &&
        Floppy144IntroHasPersonalLogin("OPERATOR TWELVE"),
        "GUEST fallback must prompt, while named operators can auto-login"))return 42;

    if(!require_true(!Floppy144IntroAutoLoginReady(20499U,true) &&
        Floppy144IntroAutoLoginReady(20500U,true),"existing profile waits for typewriter completion"))return 28;
    if(!require_true(!Floppy144IntroAutoLoginReady(99999U,false),
        "first profile still waits for explicit LOGIN"))return 29;
    puts("S4F-02 INTRO VECTORS: PASS");
    puts("beats=cabinet,disk-reveal,computer-crt,full-terminal duration=18000ms");
    puts("no intro skip; persisted LOGIN when ready; replay=timing-rearm");
    return 0;
}
