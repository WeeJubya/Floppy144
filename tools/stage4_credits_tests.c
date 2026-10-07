#include "floppy144_credits_view.h"
#include <stdio.h>
#include <string.h>

static int failures;
static uint32_t pixels[640U*360U];

static void Expect(int condition,const char *label)
{
    if(!condition){++failures;printf("FAIL: %s\n",label);}
}

static int Contains(const char *needle)
{
    uint32_t i;
    for(i=0U;i<Floppy144CreditsViewLineCount();++i)
    {
        const char *line=Floppy144CreditsViewLineAt(i);
        if(line!=NULL&&strstr(line,needle)!=NULL) return 1;
    }
    return 0;
}

int main(void)
{
    Floppy144Surface surface={pixels,640U,360U};
    uint32_t i;
    int nonzero=0;

    memset(pixels,0,sizeof(pixels));
    Floppy144CreditsViewDraw(&surface);
    for(i=0U;i<640U*360U;++i){if(pixels[i]!=0U){nonzero=1;break;}}

    Expect(nonzero,"credits screen renders");
    Expect(Floppy144CreditsViewLineCount()==10U,"credits remain one compact page");
    Expect(Floppy144CreditsViewMaxLineWidth()<=504U,"credit lines fit panel");
    Expect(Contains("GREY DOOR REPUBLIC"),"company credit");
    Expect(Contains("WEEJUBYA"),"designer/developer credit");
    Expect(Contains("RIVER2D-DERIVED"),"River2D lineage");
    Expect(Contains("BADACRONYM"),"River2D copyright holder");
    Expect(Contains("GNU GENERAL PUBLIC LICENSE VERSION 3"),"GPLv3 notice");
    Expect(Contains("REDISTRIBUTED UNDER GNU GPL V3 TERMS"),"redistribution notice");
    Expect(Contains("LICENSE"),"full licence reference");
    Expect(Contains("NO WARRANTY"),"no-warranty notice");

    if(failures!=0){printf("STAGE 4C CREDITS TESTS: FAIL (%d)\n",failures);return 1;}
    printf("STAGE 4C CREDITS TESTS: PASS\n");
    return 0;
}
