/*
 * F144 platform contract
 *
 * This is the platform-neutral boundary seen by FLOPPY//144 game code.
 * Stage 4B starts with display/presentation because that is the first service
 * being migrated. Timing, storage, input/text events, lifecycle, quit
 * handling and audio are added only when their owning migration reaches them.
 *
 * Single-instance acquisition and raw command-line parsing are launcher-only
 * concerns and intentionally do not belong in this game-facing contract.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct Floppy144Surface
{
    uint32_t *pixels;
    uint32_t width;
    uint32_t height;
} Floppy144Surface;

typedef struct F144Platform F144Platform;

typedef struct F144PlatformApi
{
    Floppy144Surface *(*framebuffer)(F144Platform *platform);
    void (*present)(F144Platform *platform);
} F144PlatformApi;

struct F144Platform
{
    const F144PlatformApi *api;
    void *state;
    Floppy144Surface surface;
};

Floppy144Surface *f144PlatformFramebuffer(
    F144Platform *platform
);

void f144PlatformPresent(
    F144Platform *platform
);
