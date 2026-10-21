/*
 * Floppy//144 independent 640x360 software frame store and native presenter.
 * Designed against B2-02 behavioural contracts and Microsoft Win32 GDI docs.
 * No production window or game integration is performed in this module.
 */
#include "floppy144_win32_display.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct Floppy144Win32Display
{
    Floppy144Surface surface;
    Floppy144DisplayRelease release;
    void *allocator_context;
};

static void *DisplayDefaultAllocate(size_t bytes, void *context)
{
    (void)context;
    return malloc(bytes);
}

static void DisplayDefaultRelease(void *memory, void *context)
{
    (void)context;
    free(memory);
}

Floppy144Win32Display *Floppy144Win32DisplayCreateWithAllocator(
    Floppy144DisplayAllocate allocate,
    Floppy144DisplayRelease release,
    void *context
)
{
    Floppy144Win32Display *display;
    const size_t count = (size_t)FLOPPY144_DISPLAY_WIDTH *
                         (size_t)FLOPPY144_DISPLAY_HEIGHT;

    if(allocate == NULL || release == NULL ||
       count > SIZE_MAX / sizeof(uint32_t))
    {
        return NULL;
    }

    display = (Floppy144Win32Display *)allocate(sizeof(*display), context);
    if(display == NULL)
    {
        return NULL;
    }
    memset(display, 0, sizeof(*display));

    display->surface.pixels = (uint32_t *)allocate(
        count * sizeof(uint32_t), context
    );
    if(display->surface.pixels == NULL)
    {
        release(display, context);
        return NULL;
    }

    memset(display->surface.pixels, 0, count * sizeof(uint32_t));
    display->surface.width = FLOPPY144_DISPLAY_WIDTH;
    display->surface.height = FLOPPY144_DISPLAY_HEIGHT;
    display->release = release;
    display->allocator_context = context;
    return display;
}

Floppy144Win32Display *Floppy144Win32DisplayCreate(void)
{
    return Floppy144Win32DisplayCreateWithAllocator(
        DisplayDefaultAllocate, DisplayDefaultRelease, NULL
    );
}

void Floppy144Win32DisplayDestroy(Floppy144Win32Display **display)
{
    Floppy144Win32Display *owned;
    Floppy144DisplayRelease release;
    void *context;

    if(display == NULL || *display == NULL)
    {
        return;
    }

    owned = *display;
    *display = NULL;
    release = owned->release;
    context = owned->allocator_context;
    release(owned->surface.pixels, context);
    release(owned, context);
}

Floppy144Surface *Floppy144Win32DisplayFramebuffer(
    Floppy144Win32Display *display
)
{
    return display == NULL ? NULL : &display->surface;
}

bool Floppy144Win32DisplayCalculateViewport(
    int64_t client_width,
    int64_t client_height,
    Floppy144DisplayViewport *viewport
)
{
    uint64_t width;
    uint64_t height;
    uint64_t fitted_width;
    uint64_t fitted_height;

    if(viewport == NULL)
    {
        return false;
    }
    memset(viewport, 0, sizeof(*viewport));
    if(client_width <= 0 || client_height <= 0 ||
       client_width > INT32_MAX || client_height > INT32_MAX)
    {
        return false;
    }

    width = (uint64_t)client_width;
    height = (uint64_t)client_height;
    if(width * FLOPPY144_DISPLAY_HEIGHT >
       height * FLOPPY144_DISPLAY_WIDTH)
    {
        fitted_height = height;
        fitted_width = height * FLOPPY144_DISPLAY_WIDTH /
                       FLOPPY144_DISPLAY_HEIGHT;
        if(fitted_width == 0U)
        {
            fitted_width = 1U;
        }
    }
    else
    {
        fitted_width = width;
        fitted_height = width * FLOPPY144_DISPLAY_HEIGHT /
                        FLOPPY144_DISPLAY_WIDTH;
        if(fitted_height == 0U)
        {
            fitted_height = 1U;
        }
    }

    viewport->x = (int32_t)((width - fitted_width) / 2U);
    viewport->y = (int32_t)((height - fitted_height) / 2U);
    viewport->width = (int32_t)fitted_width;
    viewport->height = (int32_t)fitted_height;
    return true;
}

#ifdef _WIN32
/* Fill only the areas not occupied by the bitmap, after a successful copy. */
static bool DisplayPaintBands(
    HDC target,
    int32_t client_width,
    int32_t client_height,
    const Floppy144DisplayViewport *v
)
{
    const RECT bands[4] = {
        {0, 0, client_width, v->y},
        {0, v->y + v->height, client_width, client_height},
        {0, v->y, v->x, v->y + v->height},
        {v->x + v->width, v->y, client_width, v->y + v->height}
    };
    HBRUSH black = (HBRUSH)GetStockObject(BLACK_BRUSH);
    size_t i;

    if(black == NULL)
    {
        return false;
    }
    for(i = 0U; i < 4U; ++i)
    {
        if(bands[i].left < bands[i].right &&
           bands[i].top < bands[i].bottom &&
           FillRect(target, &bands[i], black) == 0)
        {
            return false;
        }
    }
    return true;
}

bool Floppy144Win32DisplayPresent(
    Floppy144Win32Display *display,
    HDC target,
    int64_t client_width,
    int64_t client_height
)
{
    Floppy144DisplayViewport v;
    BITMAPINFO bitmap;
    int previous_mode;
    int copied;
    bool bands_ok;

    if(display == NULL || display->surface.pixels == NULL || target == NULL ||
       client_width < 0 || client_height < 0 ||
       client_width > INT32_MAX || client_height > INT32_MAX)
    {
        return false;
    }
    if(client_width == 0 || client_height == 0)
    {
        return true;
    }
    if(!Floppy144Win32DisplayCalculateViewport(
        client_width, client_height, &v))
    {
        return false;
    }

    memset(&bitmap, 0, sizeof(bitmap));
    bitmap.bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);
    bitmap.bmiHeader.biWidth = (LONG)FLOPPY144_DISPLAY_WIDTH;
    bitmap.bmiHeader.biHeight = -(LONG)FLOPPY144_DISPLAY_HEIGHT;
    bitmap.bmiHeader.biPlanes = 1;
    bitmap.bmiHeader.biBitCount = 32;
    bitmap.bmiHeader.biCompression = BI_RGB;

    previous_mode = SetStretchBltMode(target, COLORONCOLOR);
    if(previous_mode == 0)
    {
        return false;
    }
    copied = StretchDIBits(
        target, v.x, v.y, v.width, v.height,
        0, 0, (int)FLOPPY144_DISPLAY_WIDTH,
        (int)FLOPPY144_DISPLAY_HEIGHT,
        display->surface.pixels, &bitmap, DIB_RGB_COLORS, SRCCOPY
    );
    if(copied == 0 || copied == GDI_ERROR)
    {
        (void)SetStretchBltMode(target, previous_mode);
        return false;
    }

    bands_ok = DisplayPaintBands(
        target, (int32_t)client_width, (int32_t)client_height, &v
    );
    (void)SetStretchBltMode(target, previous_mode);
    return bands_ok;
}
#endif
