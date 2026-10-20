/*
 * GREY DOOR REPUBLIK. A tiny procedural office, outside world coordinates.
 * Every pixel is drawn through the existing 640x360 software framebuffer.
 */
#include "floppy144_grey_encounter.h"
#include "floppy144_grey_door.h"
#include "floppy144_player_visual.h"
#include "floppy144_brand_logo.h"
#include "floppy144_site.h"
#include "floppy144_site_2d_camera.h"
#include "floppy144_site_isometric.h"
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
        case FLOPPY144_GREY_CAPACITY: return 2200U;
        case FLOPPY144_GREY_GLITCH: return 1400U;
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
    if(y>280) y=280;
    /* Keep the screen-space feet inside the projected ISO floor diamond.
       At an edge the ordinary movement is blocked, not teleported. */
    {
        int32_t fx=x-320,fy=y-234;
        if(fx<0) fx=-fx;
        if(fy<0) fy=-fy;
        if(fx*110+fy*308>33880) return false;
    }
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
        /* Five seconds without a player-initiated Inspect makes the
           occupied Developer notice the player and turn. */
        bool gait=Floppy144PlayerVisualAdvance(&scene->player_visual,elapsed_ms);
        if(scene->elapsed_ms<5000U) {
            scene->elapsed_ms+=elapsed_ms;
            if(scene->elapsed_ms>=5000U && !scene->developer_inspected) {
                scene->phase=(uint8_t)FLOPPY144_GREY_TURN;
                scene->elapsed_ms=0U;
                return true;
            }
        }
        return gait;
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
    /* Dialogue is completed before the false restoration can climb. The
       final 300 ms visibly holds exactly 144% before the hard blackout. */
    const uint32_t capacity=2200U, span=3300U;
    uint32_t t,delta,elapsed;
    uint64_t cube;
    if(scene==NULL) return 0U;
    if(scene->phase<(uint8_t)FLOPPY144_GREY_CAPACITY)
        return scene->restoration_start;
    if(scene->phase>=(uint8_t)FLOPPY144_GREY_BLACKOUT)
        return 144U;
    t=scene->phase==(uint8_t)FLOPPY144_GREY_CAPACITY
        ?scene->elapsed_ms:capacity+scene->elapsed_ms;
    if(t>span) t=span;
    delta=144U-scene->restoration_start;
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

/* The same ISO camera scale and fixed-point convention as normal Site. */
static const Floppy144SiteIsoScene room_iso={
    320,234,11*FLOPPY144_SITE_FIXED_ONE,11*FLOPPY144_SITE_FIXED_ONE,14,5,8
};
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
    /* Retain the BF35 code artwork, but draw it locally as a wall texture. */
    FILL(s,0,0,167,105,RGB(30,43,53));
    FILL(s,5,4,157,94,RGB(12,23,34));
    TXT(s,12,13,"floppy144_run_state.c",RGB(130,207,229));
    TXT(s,12,30,"if (site->restored)",RGB(189,203,216));
    TXT(s,18,43,"draw_world(site);",RGB(172,213,167));
    TXT(s,12,59,"if (door->unseen)",RGB(189,203,216));
    TXT(s,18,72,"return false;",RGB(222,174,121));
    FILL(s,10,88,105,1,RGB(80,116,137));
}

/* Rasterise an authored 2D panel on a projected rear-wall face instead
   of painting a flat overlay above the scene. The same projection as the
   floor, wall trim and furniture establishes its ISO slope and height. */
static void WallPanel(
    Floppy144Surface *dest,const Floppy144Surface *art,
    bool north,int32_t along,int32_t length,int32_t bottom,int32_t top
)
{
    int32_t x0,y0,x1,y1,x2,y2,minx,maxx,miny,maxy,x,y;
    int32_t wall_x=north?along:1,wall_y=north?1:along;
    int32_t end_x=north?along+length:1;
    int32_t end_y=north?1:along+length;
    int64_t dx,dy,vertical;
    if(dest==NULL || dest->pixels==NULL || art==NULL ||
       art->pixels==NULL || dest->width==0U || dest->height==0U ||
       art->width==0U || art->height==0U) return;
    Floppy144SiteIsoSceneProject(&room_iso,wall_x*16,wall_y*16,top*16,&x0,&y0);
    Floppy144SiteIsoSceneProject(&room_iso,end_x*16,end_y*16,top*16,&x1,&y1);
    Floppy144SiteIsoSceneProject(&room_iso,wall_x*16,wall_y*16,bottom*16,&x2,&y2);
    dx=(int64_t)x1-x0;
    dy=(int64_t)y1-y0;
    vertical=(int64_t)y2-y0;
    if(dx==0 || vertical<=0) return;
    minx=x0<x1?x0:x1; maxx=x0>x1?x0:x1;
    miny=y0<y1?y0:y1; maxy=y0>y1?y0:y1;
    if(y2<miny) miny=y2;
    if(y2+(int32_t)dy>maxy) maxy=y2+(int32_t)dy;
    if(minx<0) minx=0;
    if(miny<0) miny=0;
    if(maxx>(int32_t)dest->width) maxx=(int32_t)dest->width;
    if(maxy>(int32_t)dest->height) maxy=(int32_t)dest->height;
    for(y=miny;y<maxy;++y) {
        for(x=minx;x<maxx;++x) {
            int64_t u=(int64_t)x-x0,ud=dx;
            int64_t v=((int64_t)y-y0)*dx-u*dy,vd=dx*vertical;
            uint32_t sx,sy;
            if(ud<0) { u=-u; ud=-ud; }
            if(vd<0) { v=-v; vd=-vd; }
            if(u<0 || u>=ud || v<0 || v>=vd) continue;
            sx=(uint32_t)(u*art->width/ud);
            sy=(uint32_t)(v*art->height/vd);
            dest->pixels[(uint32_t)y*dest->width+(uint32_t)x]=
                art->pixels[sy*art->width+sx];
        }
    }
}
static void WallArtwork(
    Floppy144Surface *dest,bool north,int32_t along,int32_t length,
    int32_t bottom,int32_t top,uint32_t kind
)
{
    uint32_t pixels[167U*105U];
    Floppy144Surface art;
    memset(&art,0,sizeof(art));
    art.pixels=pixels;
    art.width=kind==3U?167U:76U;
    art.height=kind==3U?105U:64U;
    if(kind==3U) SourceMonitor(&art);
    else Poster(&art,0U,0U,kind);
    WallPanel(dest,&art,north,along,length,bottom,top);
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
    Floppy144PlayerVisualDrawDeveloper(s,x,264+bob,62,91,&figure,
        32,85,576,215);
}
static void Office(Floppy144Surface *s,const Floppy144GreyEncounter *scene)
{
    uint32_t i;
#define BOX(x,y,w,d,base,top,c) Floppy144SiteIsoScenePrism(s,&room_iso,\
        (x)*FLOPPY144_SITE_FIXED_ONE,(y)*FLOPPY144_SITE_FIXED_ONE,\
        (w)*FLOPPY144_SITE_FIXED_ONE,(d)*FLOPPY144_SITE_FIXED_ONE,\
        (base)*FLOPPY144_SITE_FIXED_ONE,(top)*FLOPPY144_SITE_FIXED_ONE,(c))
    Floppy144DrawClear(s,RGB(24,32,45));
    /* North and west walls only. Never insert real room geometry. */
    BOX(0,0,22,22,0,0,RGB(129,224,206));
    for(i=2U;i<22U;i+=2U) {
        Floppy144SiteIsoSceneLine(s,&room_iso,(int32_t)i*16,0,0,
                                  (int32_t)i*16,22*16,0,RGB(66,147,155));
        Floppy144SiteIsoSceneLine(s,&room_iso,0,(int32_t)i*16,0,
                                  22*16,(int32_t)i*16,0,RGB(66,147,155));
    }
    BOX(0,0,22,1,0,11,RGB(252,128,173));
    BOX(0,0,1,22,0,11,RGB(125,164,253));
    BOX(3,0,5,1,3,8,RGB(252,233,134));
    BOX(0,4,1,6,3,9,RGB(74,221,240));
    BOX(0,15,1,4,3,9,RGB(246,203,108));
    /* Shallow ISO frames and front-facing wall artwork. Draw before the
       furniture so nearer objects naturally obscure wall attachments. */
    BOX(9,0,8,1,2,10,RGB(30,43,53));
    BOX(18,0,3,1,3,9,RGB(245,245,236));
    WallArtwork(s,false,4,6,3,9,0U);
    WallArtwork(s,false,15,4,3,9,1U);
    WallArtwork(s,true,18,3,3,9,2U);
    WallArtwork(s,true,9,8,2,10,3U);
    /* Painter-ordered rear-to-front ISO furniture. */
    BOX(5,4,3,2,0,7,RGB(165,104,202));
    BOX(14,3,5,3,0,3,RGB(64,136,202));
    BOX(16,6,4,3,0,2,RGB(242,197,81));
    BOX(16,6,4,3,2,3,RGB(250,227,132));
    BOX(18,8,2,2,0,1,RGB(65,81,108));
    BOX(18,8,1,2,1,5,RGB(50,93,156));
    BOX(18,9,2,2,2,3,RGB(89,143,211));
    BOX(7,10,5,3,0,2,RGB(129,72,140));
    BOX(7,10,5,3,2,3,RGB(229,133,189));
    BOX(6,15,5,3,0,2,RGB(87,112,170));
    BOX(6,15,5,3,2,3,RGB(235,226,164));
#undef BOX
    Floppy144BrandLogoDraw(s,178U,29U,285U,49U);
    Sandwich(s);
    Developer(s,scene);
    Floppy144PlayerVisualDraw(s,scene->local_x,scene->local_y,
        FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16*
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT/FLOPPY144_SITE_FIXED_ONE,
        6*FLOPPY144_SITE_2D_PIXELS_PER_UNIT,
        FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16*
            FLOPPY144_SITE_2D_PIXELS_PER_UNIT/FLOPPY144_SITE_FIXED_ONE,
        scene->body_style,&scene->player_visual,32,85,576,215);
    FILL(s,20,312,600,28,RGB(12,17,21));
    Floppy144DrawRect(s,20,312,600,28,RGB(113,124,120));
    if(scene->phase==(uint8_t)FLOPPY144_GREY_EXPLORE)
        TXT(s,32,322,Floppy144GreyEncounterInspectNearby(scene)
            ?"I INSPECT    ARROWS MOVE":"ARROWS MOVE",RGB(201,210,203));
    else if(scene->phase>=(uint8_t)FLOPPY144_GREY_IDENTIFY &&
            scene->phase<=(uint8_t)FLOPPY144_GREY_TURN)
        TXT(s,32,322,"DEVELOPER",RGB(201,210,203));
    if(scene->phase==(uint8_t)FLOPPY144_GREY_DIALOGUE) {
        FILL(s,306,113,291,40,RGB(32,51,62));
        Floppy144DrawRect(s,306,113,291,40,RGB(97,174,181));
        TXT(s,318,120,"You're not supposed to",RGB(255,255,255));
        TXT(s,318,134,"be able to get in here.",RGB(255,255,255));
        FILL(s,453,153,4,8,RGB(32,51,62));
    }
}
static void Glitch(Floppy144Surface *s,uint32_t t)
{
    uint32_t frame,stripe,count,strength;
    uint32_t line[640];
    if(s==NULL || s->pixels==NULL || s->width!=640U || s->height!=360U ||
       t<700U) return;
    /* Escalating, deterministic 50 ms corruption frames; no global RNG. */
    frame=t/50U;
    strength=(t-700U)/250U+1U;
    if(strength>12U) strength=12U;
    count=2U+strength*2U;
    for(stripe=0U;stripe<count;++stripe) {
        uint32_t seed=frame*2654435761U+stripe*1013904223U;
        uint32_t y=53U+(seed%243U),height=1U+((seed>>13U)%5U);
        int32_t shift=(int32_t)((seed>>8U)%(strength*5U+2U))-
                      (int32_t)(strength*5U/2U);
        uint32_t row,x;
        for(row=y;row<y+height && row<309U;++row) {
            uint32_t *dst=&s->pixels[row*640U];
            memcpy(line,dst,sizeof(line));
            for(x=32U;x<608U;++x) {
                int32_t source=(int32_t)x+shift;
                if(source<32) source=32;
                if(source>607) source=607;
                dst[x]=line[(uint32_t)source];
            }
        }
        if(strength>3U && (stripe&3U)==0U) {
            uint32_t x0=40U+((seed>>16U)%480U),width=4U+strength*3U,dy;
            for(dy=y;dy<y+height+2U && dy<309U;++dy) {
                for(x=x0;x<608U && x<x0+width;++x) {
                    uint32_t *p=&s->pixels[dy*640U+x],v=*p;
                    *p=((v&0x0000ff00U)<<8U)|((v&0x00ff0000U)>>8U)|
                       (v&0x000000ffU);
                }
            }
        }
    }
    if(strength>=5U) {
        uint32_t band;
        for(band=0U;band<strength/3U;++band) {
            uint32_t seed=frame*2246822519U+band*3266489917U;
            uint32_t x=45U+seed%540U,y;
            int32_t offset=(int32_t)(seed%19U)-9;
            for(y=60U;y<307U;++y)
                line[y]=s->pixels[y*640U+x];
            for(y=60U;y<307U;++y) {
                int32_t source=(int32_t)y+offset;
                if(source<60) source=60;
                if(source>306) source=306;
                s->pixels[y*640U+x]=line[(uint32_t)source];
            }
        }
    }
    if(strength>=10U && ((frame/2U)&1U)!=0U)
        TXT(s,75,189,"SIGNAL LOST / RECOVERY INVALID",RGB(255,124,141));
}
void Floppy144GreyEncounterDrawInterference(
    Floppy144Surface *surface,const Floppy144GreyEncounter *scene
)
{
    uint32_t t;
    if(scene==NULL) return;
    if(scene->phase==(uint8_t)FLOPPY144_GREY_CAPACITY)
        t=scene->elapsed_ms;
    else if(scene->phase==(uint8_t)FLOPPY144_GREY_GLITCH)
        t=2200U+scene->elapsed_ms;
    else return;
    Glitch(surface,t);
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
}
