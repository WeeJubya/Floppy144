/*
 * Floppy//144: standalone Windows software-display boundary.
 *
 * Independent native module. This header is NOT a replacement for the
 * portable F144PlatformApi; an integration stage will bind them later.
 * All dimensions refer to client pixels, not the decorated window rectangle.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "f144_platform.h"

#define FLOPPY144_DISPLAY_WIDTH 640U
#define FLOPPY144_DISPLAY_HEIGHT 360U

typedef struct Floppy144Win32Display Floppy144Win32Display;

typedef struct Floppy144DisplayViewport
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} Floppy144DisplayViewport;

typedef void *(*Floppy144DisplayAllocate)(size_t bytes, void *context);
typedef void (*Floppy144DisplayRelease)(void *memory, void *context);

/* NULL indicates a failed allocation; no partially initialised state escapes. */
Floppy144Win32Display *Floppy144Win32DisplayCreate(void);

/* Testable allocator boundary; both callbacks are mandatory. */
Floppy144Win32Display *Floppy144Win32DisplayCreateWithAllocator(
    Floppy144DisplayAllocate allocate,
    Floppy144DisplayRelease release,
    void *context
);

/* Safe for NULL, repeated calls and a previous failed create. */
void Floppy144Win32DisplayDestroy(Floppy144Win32Display **display);

/* Borrowed by game drawing until Destroy, never caller-freed. */
Floppy144Surface *Floppy144Win32DisplayFramebuffer(
    Floppy144Win32Display *display
);

/* Pure, platform-independent client-space layout for deterministic tests. */
bool Floppy144Win32DisplayCalculateViewport(
    int64_t client_width,
    int64_t client_height,
    Floppy144DisplayViewport *viewport
);

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 * Call from an OS paint path with an acquired HDC, or with a memory DC in
 * tests. A positive client area receives the complete last committed frame;
 * zero-sized clients are harmless no-ops. No HDC is retained by the display.
 * The host owns BeginPaint/EndPaint and WM_ERASEBKGND handling.
 */
bool Floppy144Win32DisplayPresent(
    Floppy144Win32Display *display,
    HDC target,
    int64_t client_width,
    int64_t client_height
);
#endif
