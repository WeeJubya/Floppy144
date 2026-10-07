/*
 * FLOPPY//144 Stage 4D presentation-consistency primitive regression.
 *
 * Shared scrollbars are tested as a small rendering contract rather than
 * snapshotting whole screens or hard-coding every UI coordinate.
 */

#include "floppy144_draw.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_WIDTH 64U
#define TEST_HEIGHT 64U
#define TEST_PIXELS (TEST_WIDTH*TEST_HEIGHT)

static uint32_t g_pixels[TEST_PIXELS];
static int g_failures;

#define CHECK(condition,label)                                      \
    do                                                              \
    {                                                               \
        if(!(condition))                                            \
        {                                                           \
            ++g_failures;                                           \
            printf("FAIL: %s\n",(label));                         \
        }                                                           \
    }                                                               \
    while(0)

static uint32_t HashPixels(void)
{
    uint32_t hash=2166136261U;
    uint32_t i;

    for(i=0U;i<TEST_PIXELS;++i)
    {
        hash^=g_pixels[i];
        hash*=16777619U;
    }

    return hash;
}

static uint32_t CountPixelsOutsideTrack(void)
{
    uint32_t x;
    uint32_t y;
    uint32_t count=0U;

    for(y=0U;y<TEST_HEIGHT;++y)
    {
        for(x=0U;x<TEST_WIDTH;++x)
        {
            if(
                g_pixels[y*TEST_WIDTH+x]!=0U &&
                !(x>=10U&&x<16U&&y>=8U&&y<48U)
            )
            {
                ++count;
            }
        }
    }

    return count;
}

static void TestScrollbarContract(void)
{
    Floppy144Surface surface={g_pixels,TEST_WIDTH,TEST_HEIGHT};
    uint32_t top_hash;
    uint32_t bottom_hash;
    uint32_t clamped_hash;

    memset(g_pixels,0,sizeof(g_pixels));

    Floppy144DrawScrollbar(
        &surface,
        10U,
        8U,
        40U,
        10U,
        10U,
        0U,
        0x00111111U,
        0x00222222U,
        0x00333333U
    );

    CHECK(
        HashPixels()==2166136261U,
        "scrollbar is absent when all content fits"
    );

    memset(g_pixels,0,sizeof(g_pixels));

    Floppy144DrawScrollbar(
        &surface,
        10U,
        8U,
        40U,
        20U,
        10U,
        0U,
        0x00111111U,
        0x00222222U,
        0x00333333U
    );

    top_hash=HashPixels();

    CHECK(
        top_hash!=2166136261U,
        "overflow content draws a visible scrollbar"
    );

    CHECK(
        CountPixelsOutsideTrack()==0U,
        "shared scrollbar remains inside its six-pixel track"
    );

    memset(g_pixels,0,sizeof(g_pixels));

    Floppy144DrawScrollbar(
        &surface,
        10U,
        8U,
        40U,
        20U,
        10U,
        10U,
        0x00111111U,
        0x00222222U,
        0x00333333U
    );

    bottom_hash=HashPixels();

    CHECK(
        bottom_hash!=top_hash,
        "scrollbar thumb position reflects scroll state"
    );

    memset(g_pixels,0,sizeof(g_pixels));

    Floppy144DrawScrollbar(
        &surface,
        10U,
        8U,
        40U,
        20U,
        10U,
        999U,
        0x00111111U,
        0x00222222U,
        0x00333333U
    );

    clamped_hash=HashPixels();

    CHECK(
        clamped_hash==bottom_hash,
        "out-of-range presentation top index clamps safely"
    );
}

int main(void)
{
    TestScrollbarContract();

    if(g_failures!=0)
    {
        printf(
            "STAGE 4D PRESENTATION CONSISTENCY TESTS: FAIL (%d)\n",
            g_failures
        );
        return 1;
    }

    printf(
        "STAGE 4D PRESENTATION CONSISTENCY TESTS: PASS\n"
    );
    return 0;
}
