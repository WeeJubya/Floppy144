#pragma once

/*
 * Generic interpreter for JSON-normalised drawing recipes.
 *
 * DrawClipped preserves the authored object rectangle and clips individual
 * drawing primitives to a camera rectangle. This is important for scrolling
 * Site views: camera clipping must never become a new object width/height.
 */

#include "floppy144_draw.h"

#include <stdbool.h>
#include <stdint.h>

bool Floppy144DrawingRuntimeDraw(
    Floppy144Surface *pSurface,
    const char *pszDrawingId,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight
);

bool Floppy144DrawingRuntimeDrawClipped(
    Floppy144Surface *pSurface,
    const char *pszDrawingId,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight,
    int32_t nClipX,
    int32_t nClipY,
    int32_t nClipWidth,
    int32_t nClipHeight
);
