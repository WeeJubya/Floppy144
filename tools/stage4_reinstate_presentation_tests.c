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
        true,true,NULL,NULL,NULL,NULL,false,false,slide,NULL);
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
/* Verify the ACCOUNT field renders guest when no persisted operator is
 * assigned, changes for a named profile, and uses the same main panel. */
static uint32_t AccountFrame(const char *operator_name,
    const char *file_name)
{
    static uint32_t pixels[TEST_PIXEL_COUNT];
    Floppy144Surface surface={pixels,TEST_WIDTH,TEST_HEIGHT};
    const char *capture=getenv("F144_BF28_SLIDE_CAPTURE_DIR");
    uint32_t x,y;
    Floppy144MainMenuSlide slide;
    Floppy144MainMenuSlideReset(&slide);
    Floppy144MainMenuDrawSliding(&surface,
        FLOPPY144_MAIN_MENU_INITIATE_SESSION,false,false,
        NULL,NULL,NULL,NULL,false,false,&slide,operator_name);
    if(capture!=NULL&&capture[0]!='\0')
    {
        char path[512];
        FILE *out=NULL;
        (void)snprintf(path,sizeof(path),"%s/%s.ppm",capture,file_name);
        if(fopen_s(&out,path,"wb")!=0||out==NULL)
            Expect(0,"BF28 account capture opened");
        else
        {
            (void)fprintf(out,"P6\n640 360\n255\n");
            for(y=0U;y<360U;++y)for(x=0U;x<640U;++x)
            {
                uint32_t pixel=pixels[y*640U+x];
                (void)fputc((int)((pixel>>16U)&255U),out);
                (void)fputc((int)((pixel>>8U)&255U),out);
                (void)fputc((int)(pixel&255U),out);
            }
            (void)fclose(out);
        }
    }
    return PixelHash(pixels,TEST_PIXEL_COUNT);
}
static void TestMenuAccount(void)
{
    uint32_t guest=AccountFrame(NULL,"bf28_guest_account");
    uint32_t empty=AccountFrame("","bf28_blank_account");
    uint32_t named=AccountFrame("WEEJUBYA","bf28_named_account");
    Expect(guest==empty,"missing or empty operator uses GUEST");
    Expect(guest!=named,"persistent name changes menu ACCOUNT presentation");
}
/* BF37: focus, disabled-row filtering, wrapping and switching all
 * follow the same visible-action mapping used by the renderer. */
static void TestMenuNavigation(void)
{
    Floppy144MainMenuSlide slide;
    Floppy144MainMenuOption row;
    uint32_t i;
    int active,recorded,swapped;

    /* In every availability/configuration combination, no step or repair
     * may land on a disabled row or on the non-row action enum values. */
    for(active=0;active<=1;++active)
    {
        for(recorded=0;recorded<=1;++recorded)
        {
            for(swapped=0;swapped<=1;++swapped)
            {
                for(i=0U;i<(uint32_t)FLOPPY144_MAIN_MENU_OPTION_COUNT;++i)
                {
                    int direction;
                    for(direction=-1;direction<=1;++direction)
                    {
                        row=Floppy144MainMenuSelectRow(
                            (Floppy144MainMenuOption)i,direction,
                            swapped!=0,active!=0,recorded!=0);
                        Expect(Floppy144MainMenuRowSelectable(row,
                            swapped!=0,active!=0,recorded!=0),
                            "BF37 navigation never lands on a disabled row");
                        if(direction==0 &&
                           Floppy144MainMenuRowSelectable(
                               (Floppy144MainMenuOption)i,
                               swapped!=0,active!=0,recorded!=0))
                        {
                            Expect(row==(Floppy144MainMenuOption)i,
                                "BF37 valid focus survives configuration sync");
                        }
                    }
                }
            }
        }
    }

    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_INITIATE_SESSION,1,false,false,false)==
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,
        "BF37 default configuration skips disabled Return");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_TERMINATE,1,false,false,false)==
        FLOPPY144_MAIN_MENU_INITIATE_SESSION,
        "BF37 default Down wraps to Initiate");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_INITIATE_SESSION,-1,false,false,false)==
        FLOPPY144_MAIN_MENU_TERMINATE,
        "BF37 default Up wraps to Terminate");

    Expect(!Floppy144MainMenuRowSelectable(
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,true,false,false) &&
        !Floppy144MainMenuRowSelectable(
        FLOPPY144_MAIN_MENU_SETTINGS,true,false,false),
        "BF37 Save and Load rows disabled when neither prerequisite exists");
    Expect(!Floppy144MainMenuRowSelectable(
        FLOPPY144_MAIN_MENU_RECORD_SESSION,false,true,true) &&
        !Floppy144MainMenuRowSelectable(
        FLOPPY144_MAIN_MENU_REINSTATE_SESSION,true,true,true),
        "BF37 action enum values cannot masquerade as row indices");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_INITIATE_SESSION,1,true,false,false)==
        FLOPPY144_MAIN_MENU_TERMINATE,
        "BF37 disabled Save/Load skipped rather than hidden Profile/Settings");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_TERMINATE,-1,true,false,false)==
        FLOPPY144_MAIN_MENU_INITIATE_SESSION,
        "BF37 Save/Load Up skips both disabled centre rows");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,0,true,false,false)==
        FLOPPY144_MAIN_MENU_TERMINATE,
        "BF37 switching off Profile repairs invalid row");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_SETTINGS,0,true,false,false)==
        FLOPPY144_MAIN_MENU_TERMINATE,
        "BF37 switching off Settings repairs invalid row");

    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_RETURN_TO_SITE,1,true,true,false)==
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,
        "BF37 active session makes visible Save selectable");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,1,true,true,false)==
        FLOPPY144_MAIN_MENU_TERMINATE,
        "BF37 disabled Load skipped when only Save is allowed");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_INITIATE_SESSION,1,true,false,true)==
        FLOPPY144_MAIN_MENU_SETTINGS,
        "BF37 recorded session makes Load selectable without Save");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,1,true,true,true)==
        FLOPPY144_MAIN_MENU_SETTINGS,
        "BF37 both Save and Load available participate in order");
    Expect(Floppy144MainMenuSelectRow(
        FLOPPY144_MAIN_MENU_SETTINGS,-1,true,true,true)==
        FLOPPY144_MAIN_MENU_OPERATOR_PROFILE,
        "BF37 both Save and Load available participate in reverse order");

    /* Rapid Up/Down while whisking, repeated blocked Left/Right, Enter's
     * completion of the animation, and later switches cannot desynchronise
     * the highlight from the selected visible action. */
    Floppy144MainMenuSlideReset(&slide);
    row=FLOPPY144_MAIN_MENU_OPERATOR_PROFILE;
    for(i=0U;i<128U;++i)
    {
        int32_t direction=(i&1U)!=0U?-1:1;
        Expect(Floppy144MainMenuSlideStart(&slide,direction),
            "BF37 switch begins from a settled menu");
        row=Floppy144MainMenuSelectRow(row,0,slide.save_load_visible,
            false,false);
        Expect(!Floppy144MainMenuSlideStart(&slide,-direction),
            "BF37 repeated switching blocked until animation completes");
        row=Floppy144MainMenuSelectRow(row,1,slide.save_load_visible,
            false,false);
        row=Floppy144MainMenuSelectRow(row,-1,slide.save_load_visible,
            false,false);
        Expect(Floppy144MainMenuRowSelectable(row,
            slide.save_load_visible,false,false),
            "BF37 movement during slide follows destination labels");
        (void)Floppy144MainMenuSlideAdvance(&slide,240U);
        Expect(!slide.animating &&
            Floppy144MainMenuOptionEnabled(
                Floppy144MainMenuVisibleOption(row,slide.save_load_visible),
                false,false),
            "BF37 Enter after slide cannot activate hidden or disabled option");
    }
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
    TestMenuAccount();
    TestMenuNavigation();

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
