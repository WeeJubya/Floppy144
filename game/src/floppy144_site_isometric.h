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
