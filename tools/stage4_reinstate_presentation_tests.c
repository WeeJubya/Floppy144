/*
 * FLOPPY//144 S4C-07 restored-session presentation regression.
 *
 * This is deliberately a renderer-only test. Persistence/load semantics stay
 * in the established Stage 3A/3B and Stage 4 persistence suites.
 */

#include "floppy144_recovery.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_WIDTH 640U
#define TEST_HEIGHT 360U
#define TEST_PIXEL_COUNT (TEST_WIDTH * TEST_HEIGHT)

static uint32_t guarded_pixels[TEST_PIXEL_COUNT + 2U];
static int failures;

static void Expect(int condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

/*
 * Recovery rendering needs only these two RunState display queries.
 * Stubs keep this test focused on presentation instead of duplicating the
 * complete gameplay dependency graph.
 */
uint32_t Floppy144RunStateRecoveredPercent(
    const Floppy144RunState *state
)
{
    return state != NULL ? 42U : 0U;
}

void Floppy144RunStateFormatCapacity(
    const Floppy144RunState *state,
    char *buffer,
    uint32_t capacity
)
{
    if(buffer == NULL || capacity == 0U)
    {
        return;
    }

    (void)snprintf(
        buffer,
        capacity,
        state != NULL
            ? "RESTORATION CAPACITY: 42%%"
            : "RESTORATION CAPACITY: --"
    );
}

static uint32_t PixelHash(const uint32_t *pixels,uint32_t count)
{
    uint32_t hash=2166136261U;
    uint32_t index;

    for(index=0U;index<count;++index)
    {
        hash^=pixels[index];
        hash*=16777619U;
    }

    return hash;
}

static uint32_t DrawMenu(
    bool active,
    bool recorded,
    bool confirmation,
    Floppy144RunState *run_state,
    Floppy144RunState *recorded_state
)
{
    Floppy144Surface surface;

    memset(guarded_pixels,0,sizeof(guarded_pixels));
    guarded_pixels[0]=0x13579BDFU;
    guarded_pixels[TEST_PIXEL_COUNT+1U]=0x2468ACE0U;

    surface.pixels=&guarded_pixels[1];
    surface.width=TEST_WIDTH;
    surface.height=TEST_HEIGHT;

    Floppy144MainMenuDraw(
        &surface,
        FLOPPY144_MAIN_MENU_RETURN_TO_SITE,
        active,
        recorded,
        run_state,
        recorded_state,
        NULL,
        NULL,
        false,
        confirmation
    );

    Expect(
        guarded_pixels[0]==0x13579BDFU &&
        guarded_pixels[TEST_PIXEL_COUNT+1U]==0x2468ACE0U,
        "menu renderer stays inside the 640x360 framebuffer"
    );

    return PixelHash(
        &guarded_pixels[1],
        TEST_PIXEL_COUNT
    );
}

int main(void)
{
    Floppy144RunState restored;
    Floppy144RunState before_draw;
    Floppy144RunState recorded;
    uint32_t no_save_hash;
    uint32_t recorded_hash;
    uint32_t restored_confirmation_hash;
    uint32_t restored_active_hash;

    memset(&restored,0,sizeof(restored));
    memset(&recorded,0,sizeof(recorded));

    restored.recovery_seed=144U;
    recorded.recovery_seed=144U;
    before_draw=restored;

    Expect(
        !Floppy144MainMenuOptionEnabled(
            FLOPPY144_MAIN_MENU_REINSTATE_SESSION,
            false,
            false
        ),
        "reinstate remains unavailable when no recorded session exists"
    );

    Expect(
        Floppy144MainMenuOptionEnabled(
            FLOPPY144_MAIN_MENU_REINSTATE_SESSION,
            false,
            true
        ),
        "reinstate remains available when a recorded session exists"
    );

    no_save_hash=DrawMenu(
        false,
        false,
        false,
        &restored,
        &recorded
    );

    recorded_hash=DrawMenu(
        false,
        true,
        false,
        &restored,
        &recorded
    );

    restored_confirmation_hash=DrawMenu(
        true,
        true,
        true,
        &restored,
        &recorded
    );

    Expect(
        memcmp(&restored,&before_draw,sizeof(restored))==0,
        "restored-session presentation does not mutate RunState"
    );

    restored_active_hash=DrawMenu(
        true,
        true,
        false,
        &restored,
        &recorded
    );

    Expect(no_save_hash!=0U,"no-save Session Control renders");
    Expect(recorded_hash!=no_save_hash,"recorded-session availability changes menu presentation");
    Expect(
        restored_confirmation_hash!=restored_active_hash,
        "SESSION RESTORED acknowledgement is visually distinct from the revealed active-session frame"
    );
    Expect(
        restored_confirmation_hash!=recorded_hash,
        "successful restore confirmation is visually distinct from pre-restore recorded-session menu"
    );

    if(failures!=0)
    {
        printf(
            "STAGE 4C RESTORED-SESSION PRESENTATION TESTS: FAIL (%d)\n",
            failures
        );
        return 1;
    }

    printf("STAGE 4C RESTORED-SESSION PRESENTATION TESTS: PASS\n");
    return 0;
}
