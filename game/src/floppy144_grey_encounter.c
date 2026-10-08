/*
 * GREY DOOR REPUBLIC. A tiny procedural office, outside world coordinates.
 * Every pixel is drawn through the existing 640x360 software framebuffer.
 */
#include "floppy144_grey_encounter.h"
#include "floppy144_grey_door.h"
#include <stddef.h>
#include <string.h>

#define RGB(r,g,b) FLOPPY144_RGB((r),(g),(b))
#define FILL(s,x,y,w,h,c) Floppy144DrawFillRect((s),(x),(y),(w),(h),(c))
#define LINE(s,x,y,w,h,c) Floppy144DrawFillRect((s),(x),(y),(w),(h),(c))
#define TXT(s,x,y,t,c) Floppy144DrawText((s),(x),(y),(t),1U,(c))
#define DEVELOPER_X 455
#define DEVELOPER_Y 235

static uint32_t SceneLength(uint8_t phase)
{
    switch(phase)
    {
        case FLOPPY144_GREY_ENTERING: return 700U;
        case FLOPPY144_GREY_IDENTIFY: return 1050U;
        case FLOPPY144_GREY_TURN: return 1900U;
        case FLOPPY144_GREY_DIALOGUE: return 3200U;
        case FLOPPY144_GREY_CAPACITY: return 1800U;
        case FLOPPY144_GREY_GLITCH: return 950U;
        default: return 0U;
    }
}
bool Floppy144GreyEncounterBegin(
    Floppy144GreyEncounter *scene, const Floppy144RunState *run
)
{
    if(scene==NULL || run==NULL ||
       (run->grey_door_state!=(uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE &&
        run->hathaway_inspection==0U) ||
       !Floppy144GreyDoorNearby(run))
        return false;
    memset(scene,0,sizeof(*scene));
    scene->phase=(uint8_t)FLOPPY144_GREY_ENTERING;
    scene->local_x=115;
    scene->local_y=264;
    scene->return_x16=run->player_site_x;
    scene->return_y16=run->player_site_y;
    return true;
}
bool Floppy144GreyEncounterMove(
    Floppy144GreyEncounter *scene,int32_t dx,int32_t dy
)
{
    int32_t x,y;
    if(scene==NULL || scene->phase!=(uint8_t)FLOPPY144_GREY_EXPLORE)
        return false;
    x=scene->local_x+dx;
    y=scene->local_y+dy;
    if(x<65) x=65;
    if(x>548) x=548;
    if(y<235) y=235;
    if(y>292) y=292;
    if(x==scene->local_x && y==scene->local_y) return false;
    scene->local_x=x;
    scene->local_y=y;
    return true;
}
bool Floppy144GreyEncounterInspect(Floppy144GreyEncounter *scene)
{
    int32_t dx,dy;
    if(scene==NULL || scene->phase!=(uint8_t)FLOPPY144_GREY_EXPLORE)
        return false;
    dx=scene->local_x-DEVELOPER_X;
    dy=scene->local_y-DEVELOPER_Y;
    if(dx*dx+dy*dy>112*112) return false;
    scene->developer_inspected=1U;
    scene->phase=(uint8_t)FLOPPY144_GREY_IDENTIFY;
    scene->elapsed_ms=0U;
    return true;
}
bool Floppy144GreyEncounterAdvance(
    Floppy144GreyEncounter *scene,uint32_t elapsed_ms
)
{
    bool changed=false;
    if(scene==NULL || scene->phase==(uint8_t)FLOPPY144_GREY_DONE ||
       scene->phase==(uint8_t)FLOPPY144_GREY_EXPLORE)
        return false;
    /* Large suspend-frame deltas cannot skip directly to DONE without
       visiting the intended ordered presentation states. */
    if(elapsed_ms>100U) elapsed_ms=100U;
    scene->elapsed_ms+=elapsed_ms;
    changed=elapsed_ms!=0U;
    if(scene->elapsed_ms>=SceneLength(scene->phase))
    {
        scene->elapsed_ms=0U;
        ++scene->phase;
        changed=true;
    }
    return changed;
}
bool Floppy144GreyEncounterFinished(const Floppy144GreyEncounter *scene)
{
    return scene!=NULL && scene->phase==(uint8_t)FLOPPY144_GREY_DONE;
}
bool Floppy144GreyEncounterSaveAllowed(const Floppy144GreyEncounter *scene)
{
    /* A vignette is never serialised, including during the final glitch.
       The coordinator resumes run-state persistence only after return. */
    return scene==NULL || Floppy144GreyEncounterFinished(scene);
}
static void ModernDesk(Floppy144Surface *s,uint32_t x,uint32_t y,uint32_t w)
{
    FILL(s,x+9,y+17,9,48,RGB(79,88,104));
    FILL(s,x+w-19,y+17,9,48,RGB(79,88,104));
    FILL(s,x,y,w,18,RGB(205,198,183));
    FILL(s,x,y,w,3,RGB(240,235,225));
    FILL(s,x,y+18,w,3,RGB(131,137,146));
}
static void Poster(Floppy144Surface *s,uint32_t x,uint32_t y,uint32_t kind)
{
    uint32_t a=kind==0?RGB(56,97,134):kind==1?RGB(110,64,105):RGB(50,111,92);
    FILL(s,x,y,76,64,RGB(245,245,236));
    FILL(s,x+4,y+4,68,50,a);
    if(kind==0)
    {
        /* A drowned observatory beneath two luminous moons. */
        FILL(s,x+15,y+9,10,10,RGB(233,232,177));
        FILL(s,x+50,y+13,7,7,RGB(214,233,246));
        FILL(s,x+24,y+31,25,20,RGB(25,36,59));
        FILL(s,x+18,y+44,39,6,RGB(23,28,40));
    }
    else if(kind==1)
    {
        /* A crimson staircase into an impossible library. */
        uint32_t k;
        for(k=0;k<5;k++)
            FILL(s,x+9+k*10,y+39-k*7,16,4,RGB(229,173,128));
        FILL(s,x+53,y+9,9,32,RGB(36,26,54));
    }
    else
    {
        /* A one-eyed mechanical whale in a neon rain. */
        FILL(s,x+10,y+25,49,19,RGB(20,61,63));
        FILL(s,x+54,y+18,12,14,RGB(20,61,63));
        FILL(s,x+49,y+28,5,4,RGB(254,194,103));
        FILL(s,x+24,y+45,19,4,RGB(169,225,188));
    }
    FILL(s,x+8,y+58,54,2,RGB(129,142,147));
}
static void SourceMonitor(Floppy144Surface *s)
{
    FILL(s,245,111,167,105,RGB(30,43,53));
    FILL(s,250,115,157,94,RGB(12,23,34));
    FILL(s,322,216,12,16,RGB(76,83,90));
    FILL(s,305,232,46,4,RGB(80,88,97));
    TXT(s,257,124,"floppy144_run_state.c",RGB(130,207,229));
    TXT(s,257,141,"if (site->restored)",RGB(189,203,216));
    TXT(s,263,154,"draw_world(site);",RGB(172,213,167));
    TXT(s,257,170,"if (door->unseen)",RGB(189,203,216));
    TXT(s,263,183,"return false;",RGB(222,174,121));
    FILL(s,255,199,105,1,RGB(80,116,137));
}
static void Sandwich(Floppy144Surface *s)
{
    /* One corner has been bitten out. No explanation will follow. */
    FILL(s,478,226,71,33,RGB(235,236,225));
    FILL(s,487,230,53,23,RGB(206,146,73));
    FILL(s,491,237,45,10,RGB(98,145,81));
    FILL(s,493,241,41,7,RGB(215,99,85));
    FILL(s,519,229,22,12,RGB(235,236,225));
    FILL(s,532,240,12,14,RGB(235,236,225));
    FILL(s,503,265,4,3,RGB(197,146,92));
    FILL(s,519,268,3,2,RGB(197,146,92));
}
static void Developer(Floppy144Surface *s,const Floppy144GreyEncounter *scene)
{
    uint32_t turn=scene->phase==(uint8_t)FLOPPY144_GREY_TURN
        ? scene->elapsed_ms : scene->phase>(uint8_t)FLOPPY144_GREY_TURN?1900U:0U;
    uint32_t head_x=447U;
    uint32_t skin=RGB(179,142,118);
    /* Chair and figure. Shoulder and face progress through a slow turn. */
    FILL(s,435,208,42,56,RGB(46,67,84));
    FILL(s,443,263,25,16,RGB(44,53,60));
    FILL(s,438,220,36,31,RGB(63,85,108));
    if(turn>=680U) head_x=451U;
    if(turn>=1400U) head_x=455U;
    FILL(s,head_x,195,20,25,skin);
    FILL(s,head_x-2,190,25,11,RGB(50,45,41));
    if(turn<680U) FILL(s,head_x,203,20,12,RGB(49,41,37));
    else if(turn<1400U) FILL(s,head_x+14,204,5,4,RGB(40,47,54));
    else
    {
        FILL(s,head_x+4,207,3,3,RGB(37,43,48));
        FILL(s,head_x+15,207,3,3,RGB(37,43,48));
        FILL(s,head_x+9,216,5,2,RGB(99,66,57));
    }
}
static void Office(Floppy144Surface *s,const Floppy144GreyEncounter *scene)
{
    uint32_t i;
    Floppy144DrawClear(s,RGB(236,239,239));
    /* Crisp contemporary office panels and cool ambient lighting. */
    FILL(s,0,0,640,196,RGB(223,234,236));
    FILL(s,0,196,640,164,RGB(177,187,194));
    FILL(s,0,198,640,4,RGB(111,137,149));
    for(i=0U;i<640U;i+=40U)
        FILL(s,i,199,1,161,RGB(161,174,181));
    for(i=232U;i<360U;i+=28U)
        FILL(s,0,i,640,1,RGB(166,178,184));
    FILL(s,25,16,590,66,RGB(242,246,243));
    Floppy144DrawRect(s,25,16,590,66,RGB(179,193,196));
    Floppy144DrawText(s,94,34,"GREY DOOR REPUBLIC",2U,RGB(35,61,73));
    FILL(s,74,69,491,2,RGB(73,151,166));
    /* Windows glow, architectural light bands. */
    FILL(s,34,97,178,97,RGB(121,185,199));
    FILL(s,40,103,166,84,RGB(190,221,221));
    FILL(s,119,103,4,84,RGB(238,248,245));
    FILL(s,40,145,166,3,RGB(236,248,246));
    ModernDesk(s,226,214,202);
    ModernDesk(s,55,241,158);
    ModernDesk(s,445,229,151);
    SourceMonitor(s);
    /* The first two miniature concept boards are scattered across the
       near desk rather than hung as GDR-style wall notices. */
    Poster(s,60,245,0U);
    Poster(s,135,245,1U);
    Poster(s,501,107,2U);
    Sandwich(s);
    Developer(s,scene);
    /* Small local avatar, confined to this vignette only. */
    FILL(s,(uint32_t)(scene->local_x-8),(uint32_t)(scene->local_y-19),
         16,19,RGB(45,69,83));
    FILL(s,(uint32_t)(scene->local_x-5),(uint32_t)(scene->local_y-27),
         11,9,RGB(188,152,122));
    FILL(s,(uint32_t)(scene->local_x-7),(uint32_t)(scene->local_y),
         5,6,RGB(53,60,69));
    FILL(s,(uint32_t)(scene->local_x+3),(uint32_t)(scene->local_y),
         5,6,RGB(53,60,69));
    FILL(s,20,317,600,29,RGB(33,54,67));
    if(scene->phase==(uint8_t)FLOPPY144_GREY_EXPLORE)
    {
        int32_t dx=scene->local_x-DEVELOPER_X;
        int32_t dy=scene->local_y-DEVELOPER_Y;
        TXT(s,38,328,dx*dx+dy*dy<=112*112
            ?"I INSPECT    ARROWS MOVE":"ARROWS MOVE",RGB(226,238,241));
    }
    else if(scene->phase>=(uint8_t)FLOPPY144_GREY_IDENTIFY &&
            scene->phase<=(uint8_t)FLOPPY144_GREY_TURN)
        TXT(s,278,328,"DEVELOPER",RGB(248,234,207));
    else if(scene->phase==(uint8_t)FLOPPY144_GREY_DIALOGUE)
    {
        FILL(s,98,267,451,46,RGB(32,51,62));
        Floppy144DrawRect(s,98,267,451,46,RGB(97,174,181));
        TXT(s,112,285,"You're not supposed to be able to get in here.",
            RGB(245,248,243));
    }
    else if(scene->phase==(uint8_t)FLOPPY144_GREY_CAPACITY)
    {
        FILL(s,91,264,471,51,RGB(38,49,55));
        Floppy144DrawText(s,117,282,"RESTORATION CAPACITY: 144%",
            2U,RGB(245,192,105));
    }
}
static void Glitch(Floppy144Surface *s,uint32_t t)
{
    uint32_t y;
    uint32_t stripe;
    uint32_t buffer[640];
    if(s==NULL || s->pixels==NULL || s->width!=640U || s->height!=360U)
        return;
    for(stripe=0U;stripe<11U;++stripe)
    {
        uint32_t start=(stripe*39U+t/13U)%351U;
        uint32_t offset=((stripe*17U+t/37U)%31U)+3U;
        for(y=start;y<start+((stripe%3U)+1U)*3U && y<360U;++y)
        {
            uint32_t x;
            uint32_t *row=&s->pixels[(uint64_t)y*640U];
            memcpy(buffer,row,sizeof(buffer));
            for(x=0U;x<640U;++x)
            {
                uint32_t v=buffer[(x+offset)%640U];
                if((stripe%3U)==0U) v=(~v)&0x00ffffffU;
                row[x]=v;
            }
        }
    }
    if((t/90U)%2U==0U)
        TXT(s,72U,169U,"RESTORATION CAPACITY: 144%",RGB(253,250,247));
}
void Floppy144GreyEncounterDraw(
    Floppy144Surface *surface,const Floppy144GreyEncounter *scene
)
{
    uint32_t t;
    if(surface==NULL || surface->pixels==NULL || scene==NULL) return;
    if(scene->phase==(uint8_t)FLOPPY144_GREY_ENTERING)
    {
        Floppy144DrawClear(surface,RGB(9,18,23));
        t=scene->elapsed_ms;
        if(t>150U) FILL(surface,318,30,4,300,RGB(188,193,197));
        if(t>400U) FILL(surface,270,18,100,324,RGB(158,171,178));
        return;
    }
    Office(surface,scene);
    if(scene->phase==(uint8_t)FLOPPY144_GREY_GLITCH)
        Glitch(surface,scene->elapsed_ms);
}
