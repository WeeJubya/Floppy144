#include "floppy144_credits_view.h"
#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
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
    Floppy144CreditsViewDraw(&surface,true);
    {
        const char *dir=getenv("F144_BF33_CAPTURE_DIR");
        if(dir!=NULL&&dir[0]!='\0')
        {
            char path[512];
            uint32_t x,y;
            FILE *out;
            (void)snprintf(path,sizeof(path),"%s/bf33_credits.ppm",dir);
            out=fopen(path,"wb");
            Expect(out!=NULL,"Credits screenshot can be opened");
            if(out!=NULL)
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
    }
    for(i=0U;i<640U*360U;++i){if(pixels[i]!=0U){nonzero=1;break;}}

    Expect(nonzero,"credits screen renders");
    Expect(Floppy144CreditsViewLineCount()==10U,"credits remain one compact page");
    Expect(Floppy144CreditsViewMaxLineWidth()<=504U,"credit lines fit panel");
    Expect(Contains("GREY DOOR REPUBLIK"),"company credit");
    Expect(!Contains("GREY DOOR REPUBLIC"),"obsolete Grey Door spelling absent");
    Expect(Contains("WEEJUBYA"),"designer/developer credit");
    Expect(Contains("RIVER2D-DERIVED"),"River2D lineage");
    Expect(Contains("BADACRONYM"),"River2D copyright holder");
    Expect(Contains("GNU GENERAL PUBLIC LICENSE VERSION 3"),"GPLv3 notice");
    Expect(Contains("REDISTRIBUTED UNDER GNU GPL V3 TERMS"),"redistribution notice");
    Expect(Contains("LICENSE"),"full licence reference");
    Expect(Contains("NO WARRANTY"),"no-warranty notice");
    Expect(
        strcmp(
            Floppy144CreditsViewFooter(true),
            "ENTER REPLAY INTRO   BACKSPACE BACK"
        )==0,
        "Settings-owned Credits offers intro replay"
    );
    Expect(
        strcmp(
            Floppy144CreditsViewFooter(false),
            "BACKSPACE BACK"
        )==0,
        "Completion-owned Credits preserves completion journey"
    );

    if(failures!=0){printf("STAGE 4C CREDITS TESTS: FAIL (%d)\n",failures);return 1;}
    printf("STAGE 4C CREDITS TESTS: PASS\n");
    return 0;
}
