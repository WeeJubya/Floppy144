/*
 * FLOPPY//144 public Credits. The original drawing of the studio mark is
 * shared with the Developer office; detailed River2D / GPL notices remain in the
 * preserved Beta.1 licence (licenses/history/Beta1-GPL-3.0.txt) and Git
 * provenance history. The operative Beta.2 licence is in the root LICENSE.
 */
#include "floppy144_credits_view.h"
#include "floppy144_brand_logo.h"
#include <stddef.h>

static const char *const floppy144_credit_lines[]=
{
    "(C) MMXXVI"
};

uint32_t Floppy144CreditsViewLineCount(void)
{
    return (uint32_t)(sizeof(floppy144_credit_lines)/sizeof(floppy144_credit_lines[0]));
}
const char *Floppy144CreditsViewLineAt(uint32_t index)
{
    if(index>=Floppy144CreditsViewLineCount())return NULL;
    return floppy144_credit_lines[index];
}
uint32_t Floppy144CreditsViewMaxLineWidth(void)
{
    return Floppy144DrawTextWidth(floppy144_credit_lines[0],1U);
}
const char *Floppy144CreditsViewFooter(bool intro_replay_available)
{
    return intro_replay_available
        ? "ENTER REPLAY INTRO   BACKSPACE BACK"
        : "BACKSPACE BACK";
}
void Floppy144CreditsViewDraw(
    Floppy144Surface *surface,bool intro_replay_available
)
{
    const uint32_t background=FLOPPY144_RGB(17,23,28);
    const uint32_t panel=FLOPPY144_RGB(24,33,39);
    const uint32_t panel_dark=FLOPPY144_RGB(12,17,21);
    const uint32_t border=FLOPPY144_RGB(86,103,107);
    const uint32_t text=FLOPPY144_RGB(202,211,205);
    const uint32_t muted=FLOPPY144_RGB(118,133,132);
    const uint32_t amber=FLOPPY144_RGB(194,153,76);
    uint32_t title_width,line_width;

    if(surface==NULL||surface->pixels==NULL)return;
    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,0U,0U,640U,16U,panel_dark);
    Floppy144DrawText(surface,10U,5U,"GDR PUBLIC RECORD / ATTRIBUTION",1U,muted);
    Floppy144DrawText(surface,556U,5U,"APS-12",1U,amber);
    Floppy144DrawFillRect(surface,40U,28U,560U,306U,panel);
    Floppy144DrawRect(surface,40U,28U,560U,306U,border);
    title_width=Floppy144DrawTextWidth("FLOPPY//144",3U);
    Floppy144DrawText(surface,(640U-title_width)/2U,42U,
        "FLOPPY//144",3U,text);
    Floppy144DrawText(surface,320U-Floppy144DrawTextWidth("CREDITS",1U)/2U,
        72U,"CREDITS",1U,amber);
    Floppy144DrawFillRect(surface,64U,88U,512U,1U,border);

    /* The reference mark has dark letterforms on pale paper. Give both the
     * architectural symbol and its wordmark an intentional light substrate,
     * rather than allowing black text to disappear into the dark Credits UI. */
    Floppy144DrawFillRect(surface,205U,92U,230U,188U,
        FLOPPY144_RGB(244,241,233));
    Floppy144BrandLogoDraw(surface,215U,94U,210U,186U);
    line_width=Floppy144CreditsViewMaxLineWidth();
    Floppy144DrawText(surface,320U-line_width/2U,286U,
        floppy144_credit_lines[0],1U,text);
    Floppy144DrawFillRect(surface,64U,300U,512U,1U,border);
    Floppy144DrawText(surface,10U,FLOPPY144_UI_FORMAL_FOOTER_Y,
        Floppy144CreditsViewFooter(intro_replay_available),1U,muted);
}
