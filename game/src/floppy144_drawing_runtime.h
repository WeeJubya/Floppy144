#pragma once
/* Generic interpreter for JSON normalised drawing recipes. */
#include "floppy144_draw.h"
#include <stdbool.h>
#include <stdint.h>
bool Floppy144DrawingRuntimeDraw(Floppy144Surface *pSurface,const char *pszDrawingId,int32_t nX,int32_t nY,int32_t nWidth,int32_t nHeight);
