/*
 * Floppy//144 - lightweight Stage 2 isometric Site projection
 *
 * The canonical Site remains a 100 x 100 fixed-point model. This module is
 * presentation only. FM-23 can switch the persisted projection enum to this
 * renderer without changing collision, room classification or interactions.
 */
#pragma once

#include "f144_runtime.h"
#include "floppy144_run_state.h"

void Floppy144SiteIsometricDraw(
    F144Runtime *pRuntime,
    const Floppy144RunState *pRunState,
    const char *pszNotice
);
