/*
 * FLOPPY//144 procedural startup intro.
 *
 * Four compact scenes establish the recovered-media framing without external
 * images or video:
 *   1. Disk 144 is found.
 *   2. The disk is inserted into a drive.
 *   3. FLOPPY//144 bootstraps from the media.
 *   4. The program unexpectedly locates and connects to the GDR network.
 */
#include "floppy144_intro.h"

#include "floppy144_settings_runtime.h"

#include <stddef.h>

static void Floppy144IntroTextCentred(
    Floppy144Surface *surface,
    uint32_t y,
    const char *text,
    uint32_t scale,
    uint32_t colour
)
{
    uint32_t width;

    if(surface == NULL || text == NULL)
    {
        return;
    }

    width = Floppy144DrawTextWidth(text,scale);

    Floppy144DrawText(
        surface,
        surface->width > width ? (surface->width - width) / 2U : 0U,
        y,
        text,
        scale,
        colour
    );
}

static uint32_t Floppy144IntroTextTime(
    const Floppy144Settings *settings,
    uint32_t phase_elapsed_ms
)
{
    return Floppy144SettingsTextElapsedMs(
        settings,
        phase_elapsed_ms
    );
}

static void Floppy144IntroFooter(
    Floppy144Surface *surface,
    uint32_t muted
)
{
    Floppy144DrawText(
        surface,
        10U,
        FLOPPY144_UI_FORMAL_FOOTER_Y,
        "ENTER / BACKSPACE / ESC SKIP",
        1U,
        muted
    );
}

static void Floppy144IntroDisk(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y
)
{
    const uint32_t body = FLOPPY144_RGB(38,43,46);
    const uint32_t inner = FLOPPY144_RGB(25,29,31);
    const uint32_t edge = FLOPPY144_RGB(104,113,115);
    const uint32_t label = FLOPPY144_RGB(191,193,183);
    const uint32_t label_text = FLOPPY144_RGB(49,54,53);
    const uint32_t metal = FLOPPY144_RGB(151,158,157);
    const uint32_t metal_light = FLOPPY144_RGB(193,198,195);
    const uint32_t amber = FLOPPY144_RGB(194,153,76);
    const uint32_t clear = FLOPPY144_RGB(8,11,13);

    Floppy144DrawFillRect(surface,x,y,104U,92U,body);
    Floppy144DrawFillRect(surface,x,y+82U,4U,10U,clear);
    Floppy144DrawFillRect(surface,x+4U,y+86U,4U,6U,clear);
    Floppy144DrawFillRect(surface,x+8U,y+90U,4U,2U,clear);

    Floppy144DrawRect(surface,x,y,104U,82U,edge);
    Floppy144DrawFillRect(surface,x+12U,y+90U,92U,2U,edge);
    Floppy144DrawRect(surface,x+8U,y+8U,88U,76U,inner);

    Floppy144DrawFillRect(surface,x+14U,y+16U,76U,30U,label);
    Floppy144DrawRect(surface,x+14U,y+16U,76U,30U,edge);
    Floppy144DrawText(surface,x+24U,y+21U,"DISK 144",1U,label_text);
    Floppy144DrawText(surface,x+27U,y+34U,"// GDR ?",1U,amber);

    Floppy144DrawFillRect(surface,x+18U,y+57U,70U,25U,metal);
    Floppy144DrawRect(surface,x+18U,y+57U,70U,25U,metal_light);
    Floppy144DrawFillRect(surface,x+61U,y+61U,14U,17U,inner);
    Floppy144DrawRect(surface,x+61U,y+61U,14U,17U,edge);
}

static void Floppy144IntroDiscovery(
    Floppy144Surface *surface,
    uint32_t phase_ms,
    const Floppy144Settings *settings
)
{
    const uint32_t background = FLOPPY144_RGB(8,11,13);
    const uint32_t desk = FLOPPY144_RGB(31,30,28);
    const uint32_t desk_edge = FLOPPY144_RGB(54,50,44);
    const uint32_t paper = FLOPPY144_RGB(76,73,66);
    const uint32_t muted = FLOPPY144_RGB(118,133,132);
    const uint32_t amber = FLOPPY144_RGB(194,153,76);
    uint32_t text_ms = Floppy144IntroTextTime(settings,phase_ms);

    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,0U,82U,640U,258U,desk);

    Floppy144DrawFillRect(surface,30U,104U,210U,124U,desk_edge);
    Floppy144DrawFillRect(surface,38U,112U,194U,108U,FLOPPY144_RGB(22,23,23));

    Floppy144DrawFillRect(surface,412U,105U,152U,92U,paper);
    Floppy144DrawRect(surface,412U,105U,152U,92U,desk_edge);
    Floppy144DrawFillRect(surface,430U,128U,104U,2U,desk_edge);
    Floppy144DrawFillRect(surface,430U,145U,82U,2U,desk_edge);
    Floppy144DrawFillRect(surface,430U,162U,96U,2U,desk_edge);

    if(phase_ms >= 350U)
    {
        uint32_t lift =
            phase_ms < 1050U
                ? (1050U - phase_ms) / 12U
                : 0U;

        Floppy144IntroDisk(surface,268U,102U + lift);
    }

    if(text_ms >= 1050U)
    {
        Floppy144IntroTextCentred(
            surface,
            245U,
            "UNIDENTIFIED REMOVABLE MEDIA",
            1U,
            muted
        );
    }

    if(text_ms >= 1650U)
    {
        Floppy144IntroTextCentred(
            surface,
            270U,
            "HANDWRITTEN MARKING: 144",
            1U,
            amber
        );
    }

    if(text_ms >= 2150U)
    {
        Floppy144IntroTextCentred(
            surface,
            303U,
            "NO CATALOGUE ENTRY",
            1U,
            muted
        );
    }

    Floppy144IntroFooter(surface,muted);
}

static void Floppy144IntroInsertion(
    Floppy144Surface *surface,
    uint32_t phase_ms,
    const Floppy144Settings *settings
)
{
    const uint32_t background = FLOPPY144_RGB(7,10,12);
    const uint32_t casing = FLOPPY144_RGB(45,49,50);
    const uint32_t edge = FLOPPY144_RGB(104,113,115);
    const uint32_t dark = FLOPPY144_RGB(17,20,21);
    const uint32_t text = FLOPPY144_RGB(202,211,205);
    const uint32_t muted = FLOPPY144_RGB(118,133,132);
    const uint32_t amber = FLOPPY144_RGB(194,153,76);
    uint32_t travel = 0U;
    uint32_t disk_x = 82U;
    uint32_t text_ms = Floppy144IntroTextTime(settings,phase_ms);

    Floppy144DrawClear(surface,background);

    if(phase_ms > 250U)
    {
        uint32_t motion_ms =
            phase_ms - 250U;

        if(motion_ms > 1550U)
        {
            motion_ms = 1550U;
        }

        travel =
            (uint32_t)(
                ((uint64_t)motion_ms * 296U) /
                1550U
            );
    }

    disk_x += travel;

    Floppy144IntroDisk(surface,disk_x,126U);

    Floppy144DrawFillRect(surface,360U,74U,246U,202U,casing);
    Floppy144DrawRect(surface,360U,74U,246U,202U,edge);
    Floppy144DrawFillRect(surface,380U,120U,188U,18U,dark);
    Floppy144DrawRect(surface,380U,120U,188U,18U,edge);
    Floppy144DrawFillRect(surface,392U,126U,164U,6U,FLOPPY144_RGB(4,6,7));
    Floppy144DrawText(surface,388U,92U,"REMOVABLE MEDIA",1U,muted);
    Floppy144DrawFillRect(surface,582U,91U,8U,8U,phase_ms > 1850U ? amber : dark);

    if(text_ms >= 1750U)
    {
        Floppy144IntroTextCentred(
            surface,
            300U,
            "MEDIA ENGAGED",
            1U,
            text
        );
    }

    if(text_ms >= 2050U)
    {
        Floppy144IntroTextCentred(
            surface,
            320U,
            "DRIVE ACTIVITY DETECTED",
            1U,
            amber
        );
    }

    Floppy144IntroFooter(surface,muted);
}

static void Floppy144IntroProgram(
    Floppy144Surface *surface,
    uint32_t phase_ms,
    const Floppy144Settings *settings
)
{
    const uint32_t background = FLOPPY144_RGB(7,10,12);
    const uint32_t panel = FLOPPY144_RGB(13,24,19);
    const uint32_t border = FLOPPY144_RGB(55,92,72);
    const uint32_t text = FLOPPY144_RGB(202,211,205);
    const uint32_t green = FLOPPY144_RGB(127,196,146);
    const uint32_t muted = FLOPPY144_RGB(118,133,132);
    const uint32_t amber = FLOPPY144_RGB(194,153,76);
    uint32_t text_ms = Floppy144IntroTextTime(settings,phase_ms);

    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,34U,30U,572U,292U,panel);
    Floppy144DrawRect(surface,34U,30U,572U,292U,border);

    if(text_ms >= 180U)
    {
        Floppy144IntroTextCentred(surface,64U,"FLOPPY//144",3U,text);
    }

    if(text_ms >= 560U)
    {
        Floppy144DrawText(surface,72U,132U,"BOOTSTRAP FOUND",1U,green);
    }

    if(text_ms >= 980U)
    {
        Floppy144DrawText(surface,72U,158U,"MEDIA HEADER .............. VALID",1U,muted);
    }

    if(text_ms >= 1360U)
    {
        Floppy144DrawText(surface,72U,184U,"RECOVERY EXECUTABLE ....... FOUND",1U,muted);
    }

    if(text_ms >= 1740U)
    {
        Floppy144DrawText(surface,72U,210U,"INITIALISING...",1U,amber);
    }

    if(text_ms >= 2050U)
    {
        Floppy144DrawText(surface,72U,250U,"SYSTEM READY",1U,green);
    }

    Floppy144IntroFooter(surface,muted);
}

static void Floppy144IntroNetwork(
    Floppy144Surface *surface,
    uint32_t phase_ms,
    const Floppy144Settings *settings
)
{
    const uint32_t background = FLOPPY144_RGB(8,13,11);
    const uint32_t panel = FLOPPY144_RGB(13,24,19);
    const uint32_t border = FLOPPY144_RGB(55,92,72);
    const uint32_t text = FLOPPY144_RGB(127,196,146);
    const uint32_t bright = FLOPPY144_RGB(172,231,183);
    const uint32_t muted = FLOPPY144_RGB(76,119,91);
    const uint32_t amber = FLOPPY144_RGB(194,153,76);
    uint32_t text_ms = Floppy144IntroTextTime(settings,phase_ms);

    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,24U,22U,592U,306U,panel);
    Floppy144DrawRect(surface,24U,22U,592U,306U,border);
    Floppy144DrawText(surface,42U,40U,"FLOPPY//144 NETWORK BOOTSTRAP",1U,muted);
    Floppy144DrawFillRect(surface,42U,58U,556U,1U,border);

    if(text_ms >= 250U)
    {
        Floppy144DrawText(surface,54U,92U,"NETWORK INTERFACE ........ ONLINE",1U,text);
    }

    if(text_ms >= 760U)
    {
        Floppy144DrawText(surface,54U,124U,"SEARCHING...",1U,amber);
    }

    if(text_ms >= 1350U)
    {
        Floppy144DrawText(surface,54U,156U,"GDR NETWORK .............. FOUND",1U,bright);
    }

    if(text_ms >= 1960U)
    {
        Floppy144DrawText(surface,54U,188U,"CONNECTING...",1U,amber);
    }

    if(text_ms >= 2580U)
    {
        Floppy144DrawText(surface,54U,220U,"REMOTE SESSION ........... ACCEPTED",1U,bright);
    }

    if(text_ms >= 3000U)
    {
        Floppy144DrawFillRect(surface,42U,252U,556U,1U,border);
        Floppy144IntroTextCentred(
            surface,
            274U,
            "GDR SESSION CONTROL SYSTEM",
            2U,
            bright
        );
    }

    Floppy144IntroFooter(surface,muted);
}

Floppy144IntroBeat Floppy144IntroBeatAt(
    uint32_t elapsed_ms
)
{
    if(elapsed_ms < FLOPPY144_INTRO_DISCOVERY_END_MS)
    {
        return FLOPPY144_INTRO_DISCOVERY;
    }

    if(elapsed_ms < FLOPPY144_INTRO_INSERTION_END_MS)
    {
        return FLOPPY144_INTRO_INSERTION;
    }

    if(elapsed_ms < FLOPPY144_INTRO_PROGRAM_END_MS)
    {
        return FLOPPY144_INTRO_PROGRAM_START;
    }

    if(elapsed_ms < FLOPPY144_INTRO_DURATION_MS)
    {
        return FLOPPY144_INTRO_GDR_CONNECTION;
    }

    return FLOPPY144_INTRO_COMPLETE;
}

bool Floppy144IntroActionSkips(
    F144Action action
)
{
    return
        action == F144_ACTION_CONFIRM ||
        action == F144_ACTION_BACK ||
        action == F144_ACTION_MENU;
}

void Floppy144IntroDraw(
    Floppy144Surface *surface,
    uint32_t elapsed_ms,
    const Floppy144Settings *settings
)
{
    Floppy144IntroBeat beat;

    if(
        surface == NULL ||
        surface->pixels == NULL ||
        surface->width == 0U ||
        surface->height == 0U
    )
    {
        return;
    }

    beat = Floppy144IntroBeatAt(elapsed_ms);

    switch(beat)
    {
        case FLOPPY144_INTRO_DISCOVERY:
            Floppy144IntroDiscovery(surface,elapsed_ms,settings);
            break;

        case FLOPPY144_INTRO_INSERTION:
            Floppy144IntroInsertion(
                surface,
                elapsed_ms - FLOPPY144_INTRO_DISCOVERY_END_MS,
                settings
            );
            break;

        case FLOPPY144_INTRO_PROGRAM_START:
            Floppy144IntroProgram(
                surface,
                elapsed_ms - FLOPPY144_INTRO_INSERTION_END_MS,
                settings
            );
            break;

        case FLOPPY144_INTRO_GDR_CONNECTION:
            Floppy144IntroNetwork(
                surface,
                elapsed_ms - FLOPPY144_INTRO_PROGRAM_END_MS,
                settings
            );
            break;

        case FLOPPY144_INTRO_COMPLETE:
        default:
            Floppy144IntroNetwork(
                surface,
                FLOPPY144_INTRO_DURATION_MS -
                    FLOPPY144_INTRO_PROGRAM_END_MS,
                settings
            );
            break;
    }
}
