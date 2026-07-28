#pragma once

#include <stdint.h>

typedef struct Floppy144Surface
{
    uint32_t *pixels;
    uint32_t width;
    uint32_t height;
} Floppy144Surface;

#define FLOPPY144_RGB(red, green, blue) \
    (((uint32_t)(red) << 16) | \
     ((uint32_t)(green) << 8) | \
     ((uint32_t)(blue)))

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
