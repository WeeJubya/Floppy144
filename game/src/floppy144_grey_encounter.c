/*
 * GREY DOOR REPUBLIK. A tiny procedural office, outside world coordinates.
 * Every pixel is drawn through the existing 640x360 software framebuffer.
 */
#include "floppy144_grey_encounter.h"
#include "floppy144_grey_door.h"
#include "floppy144_player_visual.h"
#include "floppy144_site.h"
#include "floppy144_site_2d_camera.h"
#include <stddef.h>
#include <string.h>

#define RGB(r,g,b) FLOPPY144_RGB((r),(g),(b))
#define FILL(s,x,y,w,h,c) Floppy144DrawFillRect((s),(x),(y),(w),(h),(c))
#define LINE(s,x,y,w,h,c) Floppy144DrawFillRect((s),(x),(y),(w),(h),(c))
#define TXT(s,x,y,t,c) Floppy144DrawText((s),(x),(y),(t),1U,(c))
#define DEVELOPER_X 455
#define DEVELOPER_Y 264
/* In this 2D vignette the Developer is approached by the player foot point.
   The old 112px radius allowed inspection from across the office. */
#define DEVELOPER_INSPECT_RADIUS 24

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
        case FLOPPY144_GREY_BLACKOUT: return 2000U;
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
    scene->restoration_start=Floppy144RunStateRecoveredPercent(run);
    Floppy144PlayerVisualReset(&scene->player_visual);
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
    (void)Floppy144PlayerVisualSetMovement(&scene->player_visual,dx,dy,F144_ACTION_NONE);
    return true;
}
bool Floppy144GreyEncounterInspect(Floppy144GreyEncounter *scene)
{
    if(!Floppy144GreyEncounterInspectNearby(scene)) return false;
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
    if(scene==NULL || scene->phase==(uint8_t)FLOPPY144_GREY_DONE)
        return false;
    /* Large suspend-frame deltas cannot skip directly to DONE without
       visiting the intended ordered presentation states. */
    if(elapsed_ms>100U) elapsed_ms=100U;
    if(scene->phase==(uint8_t)FLOPPY144_GREY_EXPLORE) {
        /* The exact shared player gait continues to animate in free roam. */
        return Floppy144PlayerVisualAdvance(&scene->player_visual,elapsed_ms);
    }
    scene->elapsed_ms+=elapsed_ms;
    if(scene->phase==(uint8_t)FLOPPY144_GREY_ENTERING ||
       scene->phase==(uint8_t)FLOPPY144_GREY_BLACKOUT)
        (void)Floppy144PlayerVisualSetMovement(&scene->player_visual,0,0,F144_ACTION_NONE);
    (void)Floppy144PlayerVisualAdvance(&scene->player_visual,elapsed_ms);
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
uint32_t Floppy144GreyEncounterDisplayPercent(const Floppy144GreyEncounter *scene)
{
    uint32_t t,elapsed,span=3200U+1800U,delta;
    uint64_t cube;
    if(scene==NULL) return 0U;
    if(scene->phase<(uint8_t)FLOPPY144_GREY_DIALOGUE)
        return scene->restoration_start;
    if(scene->phase>=(uint8_t)FLOPPY144_GREY_GLITCH)
        return 144U;
    t=scene->phase==(uint8_t)FLOPPY144_GREY_DIALOGUE
        ?scene->elapsed_ms:3200U+scene->elapsed_ms;
    if(t>span) t=span;
    delta=144U-scene->restoration_start;
    /* A monotonic cubic curve: slow recognition, increasingly fast escalation.
       64-bit intermediate prevents overflow; no RunState capacity is touched. */
    cube=(uint64_t)t*t*t;
    elapsed=(uint32_t)((cube*delta)/((uint64_t)span*span*span));
    return scene->restoration_start+elapsed;
}

bool Floppy144GreyEncounterPercentFlash(const Floppy144GreyEncounter *scene)
{
    if(scene==NULL || Floppy144GreyEncounterDisplayPercent(scene)<=100U)
        return false;
    /* Two hertz with a readable 250ms dwell: no full-frame strobe. */
    return (scene->elapsed_ms/250U)%2U==0U;
}

bool Floppy144GreyEncounterInspectNearby(const Floppy144GreyEncounter *scene)
{
    int32_t dx,dy;
    if(scene==NULL || scene->phase!=(uint8_t)FLOPPY144_GREY_EXPLORE)
        return false;
    dx=scene->local_x-DEVELOPER_X;
    dy=scene->local_y-DEVELOPER_Y;
    return dx*dx+dy*dy<=DEVELOPER_INSPECT_RADIUS*DEVELOPER_INSPECT_RADIUS;
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
static uint8_t DeveloperFacing(const Floppy144GreyEncounter *scene)
{
    uint32_t turn=scene->phase==(uint8_t)FLOPPY144_GREY_TURN
        ?scene->elapsed_ms:scene->phase>(uint8_t)FLOPPY144_GREY_TURN?1900U:0U;
    if(turn<635U) return FLOPPY144_PLAYER_FACING_UP;
    if(turn<1265U) return FLOPPY144_PLAYER_FACING_RIGHT;
    return FLOPPY144_PLAYER_FACING_DOWN;
}
static void Developer(Floppy144Surface *s,const Floppy144GreyEncounter *scene)
{
    Floppy144PlayerVisualState figure;
    uint8_t facing=DeveloperFacing(scene);
    int32_t x=DEVELOPER_X;
    int32_t bob=0;
    uint32_t chair=RGB(46,67,84),edge=RGB(28,39,51);
    if(scene->phase>=(uint8_t)FLOPPY144_GREY_DIALOGUE &&
       scene->phase<(uint8_t)FLOPPY144_GREY_GLITCH)
        bob=(scene->elapsed_ms/620U)%2U==0U?0:-1;
    /* Turn the entire upholstered chair assembly, not just the head:
       backrest, seat, armrests and both swivel supports change direction. */
    FILL(s,x-2,262,4,15,edge);
    FILL(s,x-21,277,42,3,edge);
    FILL(s,x-18,270,7,5,edge);
    FILL(s,x+11,270,7,5,edge);
    if(facing==FLOPPY144_PLAYER_FACING_UP) {
        FILL(s,x-20,204,40,51,chair);
        FILL(s,x-17,208,34,43,RGB(62,85,108));
        FILL(s,x-22,239,8,21,edge);
        FILL(s,x+14,239,8,21,edge);
    } else if(facing==FLOPPY144_PLAYER_FACING_RIGHT) {
        FILL(s,x-12,205,27,51,chair);
        FILL(s,x+10,207,8,46,edge);
        FILL(s,x-20,245,32,8,RGB(62,85,108));
        FILL(s,x-13,234,6,19,edge);
    } else {
        FILL(s,x-20,204,40,49,chair);
        FILL(s,x-17,210,34,40,RGB(62,85,108));
        FILL(s,x-23,235,8,25,edge);
        FILL(s,x+15,235,8,25,edge);
    }
    Floppy144PlayerVisualReset(&figure);
    figure.facing=facing;
    /* The same vector skeleton drives the player and Developer. Only the
       outfit differs; seated legs are naturally occluded by the desk edge. */
    Floppy144PlayerVisualDrawDeveloper(s,x,255+bob,35,54,&figure,
        32,85,576,215);
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
    FILL(s,25,26,590,56,RGB(242,246,243));
    Floppy144DrawRect(s,25,26,590,56,RGB(179,193,196));
    Floppy144DrawText(s,94,41,"GREY DOOR REPUBLIK",2U,RGB(35,61,73));
    FILL(s,74,76,491,2,RGB(73,151,166));
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
    /* Shared game character renderer, with the vignette's transient movement
       state; this does not mutate the actual Site player. */
    Floppy144PlayerVisualDraw(s,scene->local_x,scene->local_y,
        FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16*
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT/FLOPPY144_SITE_FIXED_ONE,
        6*FLOPPY144_SITE_2D_PIXELS_PER_UNIT,
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16*
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT/FLOPPY144_SITE_FIXED_ONE,
        scene->body_style,&scene->player_visual,32,85,576,215);
    /* Exactly the Site HUD footer position and grammar. */
    FILL(s,20,312,600,28,RGB(12,17,21));
    Floppy144DrawRect(s,20,312,600,28,RGB(113,124,120));
    if(scene->phase==(uint8_t)FLOPPY144_GREY_EXPLORE)
        TXT(s,32,322,Floppy144GreyEncounterInspectNearby(scene)
            ?"I INSPECT    ARROWS MOVE":"ARROWS MOVE",RGB(201,210,203));
    else if(scene->phase>=(uint8_t)FLOPPY144_GREY_IDENTIFY &&
            scene->phase<=(uint8_t)FLOPPY144_GREY_TURN)
        TXT(s,32,322,"DEVELOPER",RGB(201,210,203));

    if(scene->phase==(uint8_t)FLOPPY144_GREY_DIALOGUE) {
        /* Spatial speech bubble centred above the Developer's head,
           never masquerading as footer/system or restoration text. */
        FILL(s,306,153,291,40,RGB(32,51,62));
        Floppy144DrawRect(s,306,153,291,40,RGB(97,174,181));
        TXT(s,318,160,"You're not supposed to",RGB(255,255,255));
        TXT(s,318,174,"be able to get in here.",RGB(255,255,255));
        FILL(s,453,193,4,8,RGB(32,51,62));
    }
}
static void Glitch(Floppy144Surface *s,uint32_t t)
{
    uint32_t stripe,y,count;
    uint32_t buffer[640];
    if(s==NULL || s->pixels==NULL || s->width!=640U || s->height!=360U)
        return;
    /* Deterministic build-up: light scanline instability, then displaced
       image slices, sync loss, then isolated channel corruption. No fast
       full-screen flashes; the final catastrophic failure is a hard blackout. */
    count=2U+t/78U;
    if(count>14U) count=14U;
    for(stripe=0U;stripe<count;++stripe)
    {
        uint32_t start=(stripe*47U+t/17U)%315U+20U;
        uint32_t offset=((stripe*11U+t/41U)%19U)+2U;
        uint32_t height=1U+((t/240U+stripe)%5U);
        for(y=start;y<start+height && y<312U;++y)
        {
            uint32_t x,*row=&s->pixels[(uint64_t)y*640U];
            memcpy(buffer,row,sizeof(buffer));
            for(x=24U;x<615U;++x)
            {
                uint32_t v=buffer[(x+offset)%640U];
                if(t>500U && stripe%5U==0U)
                    v=((v&0x00f0f0f0U)>>1U)|0x000a1824U;
                row[x]=v;
            }
        }
    }
    if(t>360U)
    {
        for(y=55U+(t/31U)%27U;y<300U;y+=31U)
            FILL(s,32,y,576,1,RGB(47,70,81));
    }
    if(t>720U)
    {
        uint32_t jump=(t/67U)%12U;
        FILL(s,32,122+jump,576,4,RGB(6,11,15));
        TXT(s,76,141+jump,"SIGNAL LOST / RECOVERY INVALID",RGB(235,186,180));
    }
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
    if(scene->phase==(uint8_t)FLOPPY144_GREY_BLACKOUT)
    {
        Floppy144DrawClear(surface,RGB(0,0,0));
        return;
    }
    Office(surface,scene);
    if(scene->phase==(uint8_t)FLOPPY144_GREY_GLITCH)
        Glitch(surface,scene->elapsed_ms);
}
