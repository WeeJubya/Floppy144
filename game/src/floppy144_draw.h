/*
 * Floppy//144 - tiny software drawing API
 *
 * Provides the platform-neutral pixel primitives used by every game screen.
 * All art and text are generated directly into river2D's backbuffer.
 */

#pragma once

#include <stdint.h>

/*
 * Pixel surface
 *
 * A lightweight view of a 32-bit pixel buffer. The renderer owns the
 * memory; these routines only write colours into it.
 */

typedef struct Floppy144Surface
{
    uint32_t *pixels;
    uint32_t width;
    uint32_t height;
} Floppy144Surface;

/*
 * Colour packing
 *
 * Combines red, green and blue bytes into the 0x00RRGGBB format used by
 * the software renderer.
 */

#define FLOPPY144_RGB(red, green, blue) \
    (((uint32_t)(red) << 16) | \
     ((uint32_t)(green) << 8) | \
     ((uint32_t)(blue)))

/*
 * Drawing primitives
 *
 * Clear fills the whole surface. FillRect draws a clipped solid block.
 * Rect draws a one-pixel outline. Text uses the embedded 5x7 font.
 */

void Floppy144DrawClear(
    Floppy144Surface *surface,
    uint32_t colour
);

void Floppy144DrawFillRect(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t colour
);

void Floppy144DrawRect(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t colour
);

uint32_t Floppy144DrawTextWidth(
    const char *text,
    uint32_t scale
);

void Floppy144DrawText(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    const char *text,
    uint32_t scale,
    uint32_t colour
);
