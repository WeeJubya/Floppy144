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

static void disk(Floppy144Surface *s,uint32_t x,uint32_t y,uint32_t w,uint32_t h,bool detailed)
{
    uint32_t edge=FLOPPY144_RGB(139,148,150);
    uint32_t label_x=x+w/10U,label_w=w-w/5U;
    if(w<24U||h<32U)return;
    Floppy144DrawFillRect(s,x,y,w,h,C_DISK);
    Floppy144DrawRect(s,x,y,w,h,edge);
    Floppy144DrawFillRect(s,x+w/6U,y+2U,w*2U/3U,h/8U,FLOPPY144_RGB(163,170,169));
    Floppy144DrawFillRect(s,x+w*3U/5U,y+3U,w/7U,h/9U,C_DARK);
    Floppy144DrawFillRect(s,label_x,y+h/4U,label_w,h*2U/5U,C_LABEL);
    Floppy144DrawRect(s,label_x,y+h/4U,label_w,h*2U/5U,C_EDGE);
    Floppy144DrawFillRect(s,x+w*3U/4U,y+h*3U/4U,w/8U,h/8U,C_DARK);
    if(detailed&&w>=128U)
    {
        Floppy144DrawText(s,label_x+7U,y+h/4U+5U,"GDR",1U,C_DISK);
        Floppy144DrawText(s,label_x+7U,y+h/4U+18U,"SITE RECOVERY DISK",1U,C_DISK);
        Floppy144DrawText(s,label_x+7U,y+h/4U+31U,"DISK 144/233",1U,C_DISK);
    }
}

static void cabinet(Floppy144Surface *s,int32_t x,int32_t y,uint32_t w,uint32_t h)
{
    uint32_t i;
    if(x<0||y<0)return;
    Floppy144DrawFillRect(s,(uint32_t)x,(uint32_t)y,w,h,C_CASE);
    Floppy144DrawRect(s,(uint32_t)x,(uint32_t)y,w,h,C_EDGE);
    Floppy144DrawFillRect(s,(uint32_t)x+4U,(uint32_t)y+5U,w-8U,h-10U,FLOPPY144_RGB(75,77,70));
    for(i=0U;i<3U;++i)
    {
        uint32_t ly=(uint32_t)y+h*(i+1U)/4U;
        Floppy144DrawFillRect(s,(uint32_t)x+8U,ly,w-16U,4U,C_DARK);
        Floppy144DrawFillRect(s,(uint32_t)x+w/2U-16U,ly-12U,32U,12U,C_LABEL);
        Floppy144DrawRect(s,(uint32_t)x+w/2U-16U,ly-12U,32U,12U,C_EDGE);
        Floppy144DrawFillRect(s,(uint32_t)x+w/2U-5U,ly-7U,10U,2U,C_CASE);
    }
    Floppy144DrawFillRect(s,(uint32_t)x+8U,(uint32_t)y+h-4U,20U,4U,C_DARK);
    Floppy144DrawFillRect(s,(uint32_t)x+w-28U,(uint32_t)y+h-4U,20U,4U,C_DARK);
}

static void discovery(Floppy144Surface *s,uint32_t phase)
{
    Floppy144DrawClear(s,C_BG);
    Floppy144DrawFillRect(s,0U,289U,640U,71U,FLOPPY144_RGB(42,38,32));
    Floppy144DrawFillRect(s,30U,64U,160U,186U,FLOPPY144_RGB(32,34,32));
    Floppy144DrawFillRect(s,443U,214U,156U,65U,FLOPPY144_RGB(69,65,54));
    Floppy144DrawRect(s,443U,214U,156U,65U,C_EDGE);
    Floppy144DrawFillRect(s,466U,235U,102U,3U,C_DARK);
    Floppy144DrawFillRect(s,466U,247U,77U,3U,C_DARK);
    /* The disk is physically BEHIND the cabinet; only an eye-catching sliver shows. */
    disk(s,392U,135U,140U,128U,false);
    cabinet(s,105,58,400U,249U);
    if(phase>=1900U)
        Floppy144DrawText(s,54U,322U,"SOMETHING WAS LEFT BEHIND.",1U,C_DIM);
}

static void reveal(Floppy144Surface *s,uint32_t phase)
{
    uint32_t progress=phase>3150U?3150U:phase;
    int32_t cab_x=105-(int32_t)(progress/12U);
    uint32_t disk_x=392U-progress/15U;
    uint32_t disk_y=135U-progress/30U;
    uint32_t w=140U+progress/56U;
    uint32_t h=128U+progress/63U;
    Floppy144DrawClear(s,C_BG);
    Floppy144DrawFillRect(s,0U,300U,640U,60U,FLOPPY144_RGB(42,38,32));
    disk(s,disk_x,disk_y,w,h,progress>1250U);
    if(cab_x>=0)cabinet(s,cab_x,38,402U,285U);
    else cabinet(s,0,38,(uint32_t)(402+cab_x),285U);
    if(phase>=3000U)
        Floppy144DrawText(s,300U,314U,"RECOVERY MEDIA // 144",1U,C_AMBER);
}

/* A CRT with the slot BELOW the display, facing the viewer. */
static void computer(Floppy144Surface *s,uint32_t phase)
{
    uint32_t frame=FLOPPY144_RGB(112,106,91);
    uint32_t aperture=FLOPPY144_RGB(4,8,9);
    uint32_t ms=phase>2450U?2450U:phase;
    uint32_t disk_y=337U-ms/27U;
    uint32_t disk_h=84U-ms/57U;
    uint32_t disk_w=114U;
    Floppy144DrawClear(s,C_BG);
    Floppy144DrawFillRect(s,58U,30U,518U,256U,frame);
    Floppy144DrawRect(s,58U,30U,518U,256U,C_EDGE);
    Floppy144DrawFillRect(s,79U,46U,471U,170U,C_DARK);
    Floppy144DrawRect(s,79U,46U,471U,170U,C_EDGE);
    Floppy144DrawFillRect(s,88U,56U,453U,149U,FLOPPY144_RGB(13,30,23));
    Floppy144DrawFillRect(s,96U,243U,116U,12U,C_CASE);
    Floppy144DrawText(s,224U,233U,"3.5 INCH DISK DRIVE",1U,C_DISK);
    Floppy144DrawFillRect(s,278U,263U,127U,9U,aperture);
    /* Label side faces upward; shutter leads and the eject button clicks out. */
    if(phase<2650U)
    {
        disk(s,284U,disk_y,disk_w,disk_h,false);
        /* Restrained hand gripping the lower edge. */
        if(phase<2220U && disk_y+disk_h<355U)
        {
            uint32_t hand_y=disk_y+disk_h-8U;
            Floppy144DrawFillRect(s,313U,hand_y,56U,13U,FLOPPY144_RGB(133,103,84));
            Floppy144DrawFillRect(s,336U,hand_y+12U,13U,31U,FLOPPY144_RGB(126,96,78));
        }
    }
    if(phase>=2500U)
    {
        Floppy144DrawFillRect(s,281U,264U,122U,4U,C_DARK);
        Floppy144DrawFillRect(s,410U,259U,14U,12U,FLOPPY144_RGB(147,145,128));
        Floppy144DrawFillRect(s,448U,249U,7U,7U,C_AMBER);
    }
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
    computer(s,3000U);
    dx=88U-66U*t/1700U;
    dy=56U-36U*t/1700U;
    w=453U+161U*t/1700U;
    h=149U+166U*t/1700U;
    Floppy144DrawFillRect(s,dx,dy,w,h,FLOPPY144_RGB(13,30,23));
    Floppy144DrawText(s,dx+16U,dy+11U,"FLOPPY//144",1U,C_GREEN);
    terminal_lines(s,elapsed,rate,dx+16U,dy+34U,t>=1000U?10U:5U);
}

static void full_terminal(
    Floppy144Surface *s,uint32_t elapsed,uint32_t rate,
    const char *login_name,bool save_failed
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
        prompt[7U+length]='_';prompt[8U+length]='\0';
        Floppy144DrawText(s,30U,270U,prompt,1U,C_GREEN);
        Floppy144DrawText(s,30U,302U,
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

void Floppy144IntroDrawWithLogin(
    Floppy144Surface *surface,uint32_t elapsed_ms,uint32_t text_rate,
    const char *login_name,bool save_failed
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
            full_terminal(surface,elapsed_ms,text_rate,login_name,save_failed);
            break;
    }
}
void Floppy144IntroDraw(Floppy144Surface *surface,uint32_t elapsed_ms,uint32_t text_rate)
{
    Floppy144IntroDrawWithLogin(surface,elapsed_ms,text_rate,"",false);
}
