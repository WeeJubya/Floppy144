/*
 * Floppy//144 - scrolling flat 2D Site projection
 *
 * Renders the projection-neutral Site into the runtime backbuffer using the
 * Stage 4D fixed presentation scale and player-following camera. World-space
 * geometry, collision and interaction rules remain projection-neutral.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_profile.h"
#include "floppy144_run_state.h"

void Floppy144Site2DDraw(
    Floppy144Surface *runtime,
    const Floppy144RunState *run_state,
    const char *notice
);

/*
 * Profile-aware draw used by the Stage 4 coordinator. The legacy entry point
 * above remains for Stage 3 regressions and renders the historical Type A.
 */
void Floppy144Site2DDrawForBodyStyle(
    Floppy144Surface *runtime,
    const Floppy144RunState *run_state,
    const char *notice,
    Floppy144OperatorBodyStyle body_style
);
