/*
 * Floppy//144 - tiny software drawing API
 *
 * Provides the platform-neutral pixel primitives used by every game screen.
 * All art and text are generated directly into the renderer's backbuffer.
 */

#pragma once

#include "f144_platform.h"

#include <stdint.h>

/*
 * Pixel surface
 *
 * A lightweight view of a 32-bit pixel buffer. The renderer owns the
 * memory; these routines only write colours into it.
 */

/* Floppy144Surface is defined by the platform-neutral F144 contract. */

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
 * Shared presentation grammar.
 *
 * Formal GDR record/configuration screens leave their functional panel at
 * y=334 and place the one-line input footer on this common baseline.
 */
#define FLOPPY144_UI_FORMAL_FOOTER_Y 346U

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

/*
 * Shared six-pixel scrollbar used by scrollable lists and text views.
 * Nothing is drawn when the complete content already fits in the viewport.
 */
void Floppy144DrawScrollbar(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t height,
    uint32_t total_items,
    uint32_t visible_items,
    uint32_t top_item,
    uint32_t track_colour,
    uint32_t border_colour,
    uint32_t thumb_colour
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
