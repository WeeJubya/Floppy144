/*
 * BF20 revised Intro.pptx: four scenes, a single boot text clock, no stored media.
 * The recovered-media journey is intentionally not dismissible before LOGIN.
 */
#include "floppy144_intro.h"
#include <string.h>

#define C_BG    FLOPPY144_RGB(9,12,14)
#define C_DARK  FLOPPY144_RGB(20,27,29)
#define C_CASE  FLOPPY144_RGB(62,66,63)
#define C_EDGE  FLOPPY144_RGB(127,133,126)
#define C_DISK  FLOPPY144_RGB(43,47,49)
#define C_LABEL FLOPPY144_RGB(207,208,191)
#define C_GREEN FLOPPY144_RGB(132,211,153)
#define C_DIM   FLOPPY144_RGB(87,139,108)
#define C_AMBER FLOPPY144_RGB(193,156,89)

static const char *const g_boot[]={
    "BOOTSTRAP ......................... FOUND",
    "MEDIA HEADER ...................... VALID",
    "RECOVERY EXECUTABLE .............. FOUND",
    "INITIALISING ...........................",
    "SYSTEM READY",
    "NETWORK INTERFACE ................ ONLINE",
    "SEARCHING ..............................",
    "REMOTE SESSION ACCEPTED"
};

/* The source PowerPoint uses flat, layered vector shapes and a fixed camera.
 * Reusing the Site ISO camera would change that framing and inherit x<306
 * inspection clipping. Use the same software primitives with authored vectors. */
typedef struct IntroVector {
    int32_t x,y,w,h;
    uint32_t colour;
    uint8_t kind;
} IntroVector;
#include "floppy144_intro_vectors.generated.inc"

static void vector_rect(Floppy144Surface *s,int32_t x,int32_t y,int32_t w,int32_t h,uint32_t c)
{
    int32_t right=x+w,bottom=y+h;
    if(x<0)x=0;
    if(y<0)y=0;
    if(right>(int32_t)s->width)right=(int32_t)s->width;
    if(bottom>(int32_t)s->height)bottom=(int32_t)s->height;
    if(right>x&&bottom>y)Floppy144DrawFillRect(s,(uint32_t)x,(uint32_t)y,
        (uint32_t)(right-x),(uint32_t)(bottom-y),c);
}
static void vector_draw(Floppy144Surface *s,const IntroVector *v)
{
    if(v->kind==4U) {
        if(v->x>=0&&v->y>=0&&v->w>=75) {
            Floppy144DrawText(s,(uint32_t)v->x+4U,(uint32_t)v->y+4U,"GDR",1U,v->colour);
            if(v->w>=120) {
                Floppy144DrawText(s,(uint32_t)v->x+4U,(uint32_t)v->y+19U,"SITE RECOVERY DISK",1U,v->colour);
                Floppy144DrawText(s,(uint32_t)v->x+4U,(uint32_t)v->y+48U,"DISK 144/233",1U,v->colour);
            }
        }
        return;
    }
    vector_rect(s,v->x,v->y,v->w,v->h,v->colour);
}
static void discovery(Floppy144Surface *s,uint32_t phase)
{
    uint32_t i;
    (void)phase;
    Floppy144DrawClear(s,C_BG);
    for(i=0U;i<sizeof(g_intro_scene1)/sizeof(g_intro_scene1[0]);i++)
        vector_draw(s,&g_intro_scene1[i]);
}
static void reveal(Floppy144Surface *s,uint32_t phase)
{
    uint32_t i,t=phase>3150U?3150U:phase;
    Floppy144DrawClear(s,C_BG);
    for(i=0U;i<sizeof(g_intro_scene2)/sizeof(g_intro_scene2[0]);i++) {
        IntroVector v=g_intro_scene2[i];
        const IntroVector *a=i<sizeof(g_intro_scene1)/sizeof(g_intro_scene1[0])?
            &g_intro_scene1[i]:&g_intro_scene2[i];
        v.x=a->x+(v.x-a->x)*(int32_t)t/3150;
        v.y=a->y+(v.y-a->y)*(int32_t)t/3150;
        v.w=a->w+(v.w-a->w)*(int32_t)t/3150;
        v.h=a->h+(v.h-a->h)*(int32_t)t/3150;
        if(i==13U&&phase>3150U)v.x+=(int32_t)((phase-3150U)*70U/650U);
        vector_draw(s,&v);
    }
}
static void computer(Floppy144Surface *s,uint32_t phase)
{
    uint32_t i,t=phase>2450U?2450U:phase;
    Floppy144DrawClear(s,C_BG);
    for(i=0U;i<sizeof(g_intro_scene3)/sizeof(g_intro_scene3[0]);i++) {
        IntroVector v=g_intro_scene3[i];
        if(i==3U) {v.kind=1U;v.colour=phase>=3400U?FLOPPY144_RGB(13,30,23):C_DARK;}
        if(i>=14U) {
            if(phase>=2650U)continue;
            /* Label upward, shutter leading into the tower's authored slot. */
            v.x+=(int32_t)(t*30U/2450U);
            v.y=155+(v.y-118)*(int32_t)(2450U-t)/2450;
            v.h=v.h*(int32_t)(2450U-t)/2450;
            if(v.h<1)continue;
        }
        vector_draw(s,&v);
    }
    if(phase>=2500U)vector_rect(s,456,151,5,5,C_AMBER);
}

/* One character clock is shared by CRT and full-screen terminal. */
static uint32_t boot_budget(uint32_t elapsed_ms,uint32_t rate)
{
    uint32_t diff,characters;
    if(elapsed_ms<=FLOPPY144_INTRO_CRT_START_MS)return 0U;
    diff=elapsed_ms-FLOPPY144_INTRO_CRT_START_MS;
    if(rate==UINT32_MAX)return UINT32_MAX;
    if(rate==0U)rate=1U;
    if(diff>UINT32_MAX/rate)return UINT32_MAX;
    characters=(diff*rate)/FLOPPY144_INTRO_TEXT_MS_PER_CHAR;
    return characters;
}

static void terminal_lines(
    Floppy144Surface *s,uint32_t elapsed_ms,uint32_t rate,
    uint32_t left,uint32_t top,uint32_t max_lines
)
{
    uint32_t i,available=boot_budget(elapsed_ms,rate);
    uint32_t lengths[8],first=0U,lines_shown=0U;
    uint32_t total_lines=sizeof(g_boot)/sizeof(g_boot[0]);
    char buffer[90];
    for(i=0U;i<total_lines;++i)
    {
        uint32_t length=(uint32_t)strlen(g_boot[i]);
        lengths[i]=available>length?length:available;
        if(available>=length)available-=length;
        else available=0U;
        if(available>0U)available-=1U; /* natural inter-line pause */
        if(lengths[i]>0U)lines_shown=i+1U;
    }
    if(lines_shown>max_lines)first=lines_shown-max_lines;
    for(i=first;i<lines_shown;++i)
    {
        uint32_t n=lengths[i];
        if(n>=sizeof(buffer))n=(uint32_t)sizeof(buffer)-1U;
        memcpy(buffer,g_boot[i],n);buffer[n]='\0';
        Floppy144DrawText(s,left,top+(i-first)*20U,buffer,1U,
            i==lines_shown-1U?C_GREEN:C_DIM);
    }
}

static void crt_and_zoom(Floppy144Surface *s,uint32_t elapsed,uint32_t phase,uint32_t rate)
{
    uint32_t t=phase>5700U?phase-5700U:0U;
    uint32_t dx,dy,w,h;
    if(t>1700U)t=1700U;
    computer(s,3400U);
    dx=0U;
    dy=47U-27U*t/1700U;
    w=342U+272U*t/1700U;
    h=236U+79U*t/1700U;
    Floppy144DrawFillRect(s,dx,dy,w,h,FLOPPY144_RGB(13,30,23));
    Floppy144DrawText(s,dx+16U,dy+11U,"FLOPPY//144",1U,C_GREEN);
    terminal_lines(s,elapsed,rate,dx+16U,dy+34U,t>=1000U?10U:5U);
}

static void full_terminal(
    Floppy144Surface *s,uint32_t elapsed,uint32_t rate,
    const char *login_name,bool save_failed,bool has_profile
)
{
    char prompt[64];
    uint32_t length;
    Floppy144DrawClear(s,FLOPPY144_RGB(13,30,23));
    Floppy144DrawText(s,24U,24U,"FLOPPY//144",2U,C_GREEN);
    Floppy144DrawFillRect(s,24U,52U,592U,1U,C_DIM);
    terminal_lines(s,elapsed,rate,30U,75U,10U);
    if(elapsed>=FLOPPY144_INTRO_DURATION_MS)
    {
        length=login_name!=NULL?(uint32_t)strlen(login_name):0U;
        if(length>30U)length=30U;
        memcpy(prompt,"LOGIN: ",7U);
        if(length)memcpy(prompt+7U,login_name,length);
        prompt[7U+length]=has_profile?'\0':'_';prompt[8U+length]='\0';
        Floppy144DrawText(s,30U,270U,prompt,1U,C_GREEN);
        if(!has_profile)Floppy144DrawText(s,30U,302U,
            save_failed?"PROFILE WRITE FAILED - PRESS ENTER TO RETRY":
                "ENTER CONFIRM   BACKSPACE DELETE   EMPTY NAME = GUEST",
            1U,save_failed?C_AMBER:C_DIM);
    }
}

Floppy144IntroBeat Floppy144IntroBeatAt(uint32_t elapsed_ms)
{
    if(elapsed_ms<FLOPPY144_INTRO_DISCOVERY_END_MS)return FLOPPY144_INTRO_DISCOVERY;
    if(elapsed_ms<FLOPPY144_INTRO_INSERTION_END_MS)return FLOPPY144_INTRO_INSERTION;
    if(elapsed_ms<FLOPPY144_INTRO_PROGRAM_END_MS)return FLOPPY144_INTRO_PROGRAM_START;
    if(elapsed_ms<FLOPPY144_INTRO_DURATION_MS)return FLOPPY144_INTRO_GDR_CONNECTION;
    return FLOPPY144_INTRO_COMPLETE;
}

bool Floppy144IntroActionSkips(F144Action action)
{
    (void)action;
    return false; /* No key may skip the introduction or LOGIN. */
}

bool Floppy144IntroLoginReady(uint32_t elapsed_ms)
{
    return elapsed_ms>=FLOPPY144_INTRO_DURATION_MS;
}

void Floppy144IntroDrawForProfile(
    Floppy144Surface *surface,uint32_t elapsed_ms,uint32_t text_rate,
    const char *login_name,bool save_failed,bool has_profile
)
{
    Floppy144IntroBeat beat;
    if(surface==NULL||surface->pixels==NULL||surface->width==0U||surface->height==0U)return;
    beat=Floppy144IntroBeatAt(elapsed_ms);
    switch(beat)
    {
        case FLOPPY144_INTRO_DISCOVERY:
            discovery(surface,elapsed_ms);break;
        case FLOPPY144_INTRO_INSERTION:
            reveal(surface,elapsed_ms-FLOPPY144_INTRO_DISCOVERY_END_MS);break;
        case FLOPPY144_INTRO_PROGRAM_START:
            if(elapsed_ms<FLOPPY144_INTRO_CRT_START_MS)
                computer(surface,elapsed_ms-FLOPPY144_INTRO_INSERTION_END_MS);
            else
                crt_and_zoom(surface,elapsed_ms,
                    elapsed_ms-FLOPPY144_INTRO_INSERTION_END_MS,text_rate);
            break;
        case FLOPPY144_INTRO_GDR_CONNECTION:
        case FLOPPY144_INTRO_COMPLETE:
        default:
            full_terminal(surface,elapsed_ms,text_rate,login_name,save_failed,has_profile);
            break;
    }
}
void Floppy144IntroDraw(Floppy144Surface *surface,uint32_t elapsed_ms,uint32_t text_rate)
{
    Floppy144IntroDrawWithLogin(surface,elapsed_ms,text_rate,"",false);
}

bool Floppy144IntroAutoLoginReady(uint32_t elapsed_ms,bool has_profile)
{
    return has_profile && elapsed_ms>=FLOPPY144_INTRO_DURATION_MS+FLOPPY144_INTRO_AUTO_LOGIN_PAUSE_MS;
}
void Floppy144IntroDrawWithLogin(
    Floppy144Surface *surface,uint32_t elapsed_ms,uint32_t text_rate,
    const char *login_name,bool save_failed)
{
    Floppy144IntroDrawForProfile(surface,elapsed_ms,text_rate,login_name,save_failed,false);
}
