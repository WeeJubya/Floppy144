/*
 * Floppy//144 - isometric presentation helpers
 *
 * The playable room projection is retained for future work, but Stage 3 uses
 * the isometric renderer only for FM-23's recovered Site Directory overview.
 * Canonical Site coordinates remain authoritative.
 */
#pragma once

#include "floppy144_draw.h"
#include "floppy144_run_state.h"

void Floppy144SiteIsometricDraw(
    Floppy144Surface *pRuntime,
    const Floppy144RunState *pRunState,
    const char *pszNotice
);


void Floppy144SiteIsometricDirectoryDraw(
    Floppy144Surface *pRuntime,
    const Floppy144RunState *pRunState
);

/* BF35: scene-local ISO projection, using the normal Site x16 unit. */
typedef struct Floppy144SiteIsoScene {
    int32_t origin_x, origin_y;
    int32_t centre_x16, centre_y16;
    int32_t half_tile_x, half_tile_y, height_scale;
} Floppy144SiteIsoScene;
void Floppy144SiteIsoSceneProject(
    const Floppy144SiteIsoScene *scene, int32_t x16, int32_t y16,
    int32_t height16, int32_t *screen_x, int32_t *screen_y
);
void Floppy144SiteIsoSceneLine(
    Floppy144Surface *surface, const Floppy144SiteIsoScene *scene,
    int32_t x0, int32_t y0, int32_t z0,
    int32_t x1, int32_t y1, int32_t z1, uint32_t colour
);
void Floppy144SiteIsoScenePrism(
    Floppy144Surface *surface, const Floppy144SiteIsoScene *scene,
    int32_t x16, int32_t y16, int32_t width16, int32_t depth16,
    int32_t bottom16, int32_t top16, uint32_t colour
);
