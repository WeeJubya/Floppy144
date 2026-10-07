/*
 * FLOPPY//144 credits and runtime-attribution screen.
 *
 * Legal/provenance wording is intentionally conservative and mirrors the
 * repository's Stage 4 runtime provenance audit.
 */
#include "floppy144_credits_view.h"
#include <stddef.h>

static const char *const floppy144_credit_lines[]=
{
    "GAME DESIGN COMPANY: GREY DOOR REPUBLIC",
    "MAIN DESIGNER / DEVELOPER: WEEJUBYA",
    "",
    "RUNTIME ACKNOWLEDGEMENT",
    "F144 CONTAINS RIVER2D-DERIVED RUNTIME MATERIAL",
    "RIVER2D COPYRIGHT (C) 2026 BADACRONYM",
    "DERIVED RUNTIME: GNU GENERAL PUBLIC LICENSE VERSION 3",
    "FULL LICENCE TERMS ARE DISTRIBUTED IN: LICENSE",
    "NO WARRANTY TO THE EXTENT STATED IN GNU GPL V3"
};

uint32_t Floppy144CreditsViewLineCount(void)
{
    return (uint32_t)(sizeof(floppy144_credit_lines)/sizeof(floppy144_credit_lines[0]));
}

const char *Floppy144CreditsViewLineAt(uint32_t index)
{
    if(index>=Floppy144CreditsViewLineCount()) return NULL;
    return floppy144_credit_lines[index];
}

uint32_t Floppy144CreditsViewMaxLineWidth(void)
{
    uint32_t index;
    uint32_t maximum=0U;

    for(index=0U;index<Floppy144CreditsViewLineCount();++index)
    {
        uint32_t width=Floppy144DrawTextWidth(floppy144_credit_lines[index],1U);
        if(width>maximum) maximum=width;
    }

    return maximum;
}

void Floppy144CreditsViewDraw(Floppy144Surface *surface)
{
    const uint32_t background=FLOPPY144_RGB(17,23,28);
    const uint32_t panel=FLOPPY144_RGB(24,33,39);
    const uint32_t panel_dark=FLOPPY144_RGB(12,17,21);
    const uint32_t border=FLOPPY144_RGB(86,103,107);
    const uint32_t text=FLOPPY144_RGB(202,211,205);
    const uint32_t muted=FLOPPY144_RGB(118,133,132);
    const uint32_t amber=FLOPPY144_RGB(194,153,76);
    const uint32_t green=FLOPPY144_RGB(100,156,111);
    uint32_t index;

    if(surface==NULL||surface->pixels==NULL) return;

    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,0U,0U,640U,16U,panel_dark);
    Floppy144DrawText(surface,10U,5U,"GDR PUBLIC RECORD / ATTRIBUTION",1U,muted);
    Floppy144DrawText(surface,556U,5U,"APS-12",1U,amber);

    Floppy144DrawFillRect(surface,40U,28U,560U,306U,panel);
    Floppy144DrawRect(surface,40U,28U,560U,306U,border);

    Floppy144DrawText(
        surface,
        320U-Floppy144DrawTextWidth("FLOPPY//144",3U)/2U,
        42U,
        "FLOPPY//144",
        3U,
        text
    );

    Floppy144DrawText(
        surface,
        320U-Floppy144DrawTextWidth("CREDITS + RUNTIME ATTRIBUTION",1U)/2U,
        72U,
        "CREDITS + RUNTIME ATTRIBUTION",
        1U,
        amber
    );

    Floppy144DrawFillRect(surface,64U,88U,512U,1U,border);

    for(index=0U;index<Floppy144CreditsViewLineCount();++index)
    {
        const char *line=floppy144_credit_lines[index];
        uint32_t colour=text;

        if(index==3U) colour=amber;
        else if(index>=4U) colour=(index==5U||index==6U)?green:muted;

        if(line[0]!='\0')
        {
            Floppy144DrawText(surface,68U,104U+index*21U,line,1U,colour);
        }
    }

    Floppy144DrawFillRect(surface,64U,300U,512U,1U,border);
    Floppy144DrawText(
        surface,
        68U,
        310U,
        "LICENSING SCOPE QUESTIONS REMAIN DOCUMENTED IN PROJECT PROVENANCE",
        1U,
        muted
    );
    Floppy144DrawText(surface,10U,346U,"BACKSPACE  BACK TO SETTINGS",1U,muted);
}
