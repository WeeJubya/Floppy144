#define _CRT_SECURE_NO_WARNINGS 1
/*
 * FLOPPY//144 S4C-07 restored-session presentation regression.
 *
 * This is deliberately a renderer-only test. Persistence/load semantics stay
 * in the established Stage 3A/3B and Stage 4 persistence suites.
 */

#include "floppy144_recovery.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_WIDTH 640U
#define TEST_HEIGHT 360U
#define TEST_PIXEL_COUNT (TEST_WIDTH * TEST_HEIGHT)

static uint32_t guarded_pixels[TEST_PIXEL_COUNT + 2U];
static int failures;

static void Expect(int condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

/*
 * Recovery rendering needs only these two RunState display queries.
 * Stubs keep this test focused on presentation instead of duplicating the
 * complete gameplay dependency graph.
 */
uint32_t Floppy144RunStateRecoveredPercent(
    const Floppy144RunState *state
)
{
    return state != NULL ? 42U : 0U;
}

void Floppy144RunStateFormatCapacity(
    const Floppy144RunState *state,
    char *buffer,
    uint32_t capacity
)
{
    if(buffer == NULL || capacity == 0U)
    {
        return;
    }

    (void)snprintf(
        buffer,
        capacity,
        state != NULL
            ? "RESTORATION CAPACITY: 42%%"
            : "RESTORATION CAPACITY: --"
    );
}

static uint32_t PixelHash(const uint32_t *pixels,uint32_t count)
{
    uint32_t hash=2166136261U;
    uint32_t index;

    for(index=0U;index<count;++index)
    {
        hash^=pixels[index];
        hash*=16777619U;
    }

    return hash;
}

static uint32_t DrawMenu(
    bool active,
    bool recorded,
    bool confirmation,
    Floppy144RunState *run_state,
    Floppy144RunState *recorded_state
)
{
    Floppy144Surface surface;

    memset(guarded_pixels,0,sizeof(guarded_pixels));
    guarded_pixels[0]=0x13579BDFU;
    guarded_pixels[TEST_PIXEL_COUNT+1U]=0x2468ACE0U;

    surface.pixels=&guarded_pixels[1];
    surface.width=TEST_WIDTH;
    surface.height=TEST_HEIGHT;

    Floppy144MainMenuDraw(
        &surface,
        FLOPPY144_MAIN_MENU_RETURN_TO_SITE,
        active,
        recorded,
        run_state,
        recorded_state,
        NULL,
        NULL,
        false,
        confirmation
    );

    Expect(
        guarded_pixels[0]==0x13579BDFU &&
        guarded_pixels[TEST_PIXEL_COUNT+1U]==0x2468ACE0U,
        "menu renderer stays inside the 640x360 framebuffer"
    );

    return PixelHash(
        &guarded_pixels[1],
        TEST_PIXEL_COUNT
    );
}


/* BF28 animated row test with real 640x360 renderer pixels. Unlike a
 * source-pattern audit, this verifies the in-between frame and invariant
 * pixel rows outside the two moving entries. */
static uint32_t SlideFrame(Floppy144MainMenuSlide *slide,
    const char *filename)
{
    static uint32_t screen[TEST_PIXEL_COUNT];
    Floppy144Surface surface={screen,640U,360U};
    uint32_t result,y,x;
    const char *dir=getenv("F144_BF28_SLIDE_CAPTURE_DIR");
    memset(screen,0,sizeof(screen));
    Floppy144MainMenuDrawSliding(&surface,
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,
        true,true,NULL,NULL,NULL,NULL,false,false,slide);
    result=PixelHash(screen,TEST_PIXEL_COUNT);
    if(filename!=NULL&&dir!=NULL&&dir[0]!='\0')
    {
        char path[512];
        FILE *file=NULL;
        (void)snprintf(path,sizeof(path),"%s/%s.ppm",dir,filename);
        if(fopen_s(&file,path,"wb")!=0||file==NULL)
            Expect(0,"BF28 frame capture opens");
        else
        {
            (void)fprintf(file,"P6\n640 360\n255\n");
            for(y=0U;y<360U;++y)
                for(x=0U;x<640U;++x)
                {
                    uint32_t pixel=screen[y*640U+x];
                    (void)fputc((int)((pixel>>16U)&255U),file);
                    (void)fputc((int)((pixel>>8U)&255U),file);
                    (void)fputc((int)(pixel&255U),file);
                }
            (void)fclose(file);
        }
    }
    return result;
}
static void TestMenuSlide(void)
{
    Floppy144MainMenuSlide slide;
    uint32_t base,half,complete,reverse_half,returned;
    Floppy144MainMenuSlideReset(&slide);
    base=SlideFrame(&slide,"bf28_default_profile_settings");
    Expect(!Floppy144MainMenuSlideStart(&slide,0),
        "neutral horizontal input does nothing");
    Expect(Floppy144MainMenuSlideStart(&slide,1),
        "Right begins outward rightward slide");
    Expect(!Floppy144MainMenuSlideStart(&slide,-1),
        "cannot double-toggle during animation");
    Expect(slide.direction==1&&slide.save_load_visible,
        "Right switches to Save / Load and exits right");
    Expect(Floppy144MainMenuSlideAdvance(&slide,120U),
        "half animation tick advances");
    half=SlideFrame(&slide,"bf28_switch_right_halfway");
    Expect(half!=base,"middle labels differ at 50% slide");
    Expect(Floppy144MainMenuSlideAdvance(&slide,120U)&&!slide.animating,
        "240ms animation completes");
    complete=SlideFrame(&slide,"bf28_save_load");
    Expect(complete!=base&&complete!=half,
        "Save / Load labels arrive at original two row positions");
    Expect(!Floppy144MainMenuSlideAdvance(&slide,10U),
        "completed slide cannot animate again");
    Expect(Floppy144MainMenuSlideStart(&slide,-1)&&
        slide.direction==-1&&!slide.save_load_visible,
        "Left makes Save / Load exit left and Profile / Settings enter");
    (void)Floppy144MainMenuSlideAdvance(&slide,120U);
    reverse_half=SlideFrame(&slide,"bf28_switch_left_halfway");
    Expect(reverse_half!=complete,"reverse transition is not static");
    (void)Floppy144MainMenuSlideAdvance(&slide,150U);
    returned=SlideFrame(&slide,"bf28_profile_settings_restored");
    Expect(returned==base,
        "original Profile / Settings frame restored pixel-identically");
    Floppy144MainMenuSlideReset(&slide);
    Expect(!slide.animating&&!slide.save_load_visible,
        "new menu session resets to original five rows");
}
int main(void)
{
    Floppy144RunState restored;
    Floppy144RunState before_draw;
    Floppy144RunState recorded;
    uint32_t no_save_hash;
    uint32_t recorded_hash;
    uint32_t restored_confirmation_hash;
    uint32_t restored_active_hash;

    memset(&restored,0,sizeof(restored));
    memset(&recorded,0,sizeof(recorded));

    restored.recovery_seed=144U;
    recorded.recovery_seed=144U;
    before_draw=restored;

    Expect(
        !Floppy144MainMenuOptionEnabled(
            FLOPPY144_MAIN_MENU_REINSTATE_SESSION,
            false,
            false
        ),
        "reinstate remains unavailable when no recorded session exists"
    );

    Expect(
        Floppy144MainMenuOptionEnabled(
            FLOPPY144_MAIN_MENU_REINSTATE_SESSION,
            false,
            true
        ),
        "reinstate remains available when a recorded session exists"
    );

    /* The two centre rows are a carousel, not direct save/load arrows. */
    Expect(FLOPPY144_MAIN_MENU_OPTION_COUNT==5,
        "five visible menu rows retained");
    Expect(Floppy144MainMenuVisibleOption(
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,false)==
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,
        "default third row opens Profile");
    Expect(Floppy144MainMenuVisibleOption(
        FLOPPY144_MAIN_MENU_SETTINGS,false)==
        FLOPPY144_MAIN_MENU_SETTINGS,
        "default fourth row opens Settings");
    Expect(Floppy144MainMenuVisibleOption(
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,true)==
        FLOPPY144_MAIN_MENU_RECORD_SESSION,
        "switched third row opens manual Save");
    Expect(Floppy144MainMenuVisibleOption(
        FLOPPY144_MAIN_MENU_SETTINGS,true)==
        FLOPPY144_MAIN_MENU_REINSTATE_SESSION,
        "switched fourth row opens recorded-session Load");
    Expect(Floppy144MainMenuVisibleOption(
        FLOPPY144_MAIN_MENU_TERMINATE,true)==
        FLOPPY144_MAIN_MENU_TERMINATE,
        "Terminate never changes on slide");
    TestMenuSlide();

    no_save_hash=DrawMenu(
        false,
        false,
        false,
        &restored,
        &recorded
    );

    recorded_hash=DrawMenu(
        false,
        true,
        false,
        &restored,
        &recorded
    );

    restored_confirmation_hash=DrawMenu(
        true,
        true,
        true,
        &restored,
        &recorded
    );

    Expect(
        memcmp(&restored,&before_draw,sizeof(restored))==0,
        "restored-session presentation does not mutate RunState"
    );

    restored_active_hash=DrawMenu(
        true,
        true,
        false,
        &restored,
        &recorded
    );

    Expect(no_save_hash!=0U,"no-save Session Control renders");
    Expect(recorded_hash!=no_save_hash,"recorded-session availability changes menu presentation");
    Expect(
        restored_confirmation_hash!=restored_active_hash,
        "SESSION RESTORED acknowledgement is visually distinct from the revealed active-session frame"
    );
    Expect(
        restored_confirmation_hash!=recorded_hash,
        "successful restore confirmation is visually distinct from pre-restore recorded-session menu"
    );

    if(failures!=0)
    {
        printf(
            "STAGE 4C RESTORED-SESSION PRESENTATION TESTS: FAIL (%d)\n",
            failures
        );
        return 1;
    }

    printf("STAGE 4C RESTORED-SESSION PRESENTATION TESTS: PASS\n");
    return 0;
}
