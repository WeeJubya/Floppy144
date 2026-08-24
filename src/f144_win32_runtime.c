#include "f144_runtime.h"
#include <sys/stat.h>
#include <stdlib.h>

void f144CreateImage
(
    F144Image *image,
    uint32_t   width,
    uint32_t   height
){
    image->path   = cstr_sv("f144CreateImage");
    image->data   = calloc(width * height * F144_BPP, 1);
    image->width  = width;
    image->height = height;

    image->info.bmiHeader.biSize        = sizeof(image->info.bmiHeader);
    image->info.bmiHeader.biWidth       =  (long)width;
    image->info.bmiHeader.biHeight      = -(long)height;
    image->info.bmiHeader.biPlanes      = 1;
    image->info.bmiHeader.biBitCount    = 32;
    image->info.bmiHeader.biCompression = BI_RGB;
}

F144Time f144QueryTime
(
    void
){
    static LARGE_INTEGER freq;
    static int initialized = 0;

    if (!initialized)
    {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    uint64_t seconds = counter.QuadPart / freq.QuadPart;
    uint64_t remainder = counter.QuadPart % freq.QuadPart;

    uint64_t nanoseconds = (remainder * 1000000000ULL) / freq.QuadPart;

    F144Time time;
    time.s  = seconds;
    time.ns = nanoseconds;

    return time;
}

uint8_t f144CharToKey
(
    char inp
){
    if(inp >= 'a' && inp <= 'z')
    {
        return inp - 0x20;
    }

    if(inp == F144_ASCII_LSHIFT)
    {
        return 0x10;
    }
    else if(inp == '-')
    {
        return 0xC0;
    }
    else if(inp == '=')
    {
        return 0xBB;
    }

    return inp;
}

Dimensions f144GetWindowSize
(
    F144Runtime *runtime
){
    Dimensions dim  = {0};
    RECT       rect = {0};

    GetWindowRect(runtime->window, &rect);

    dim.width  = rect.right  - rect.left;
    dim.height = rect.bottom - rect.top;

    return dim;
}

void f144ChangeCursor
(
    F144Runtime  *runtime,
    F144Image *image
){
    if(runtime->currentCursor == image)
    {
        return;
    }

    DestroyCursor(runtime->hCursor);
    DeleteObject(runtime->cursorBitmap);
    DeleteObject(runtime->cursorMask);

    BITMAPINFO bmi              = {0};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = image->width;
    bmi.bmiHeader.biHeight      = -((LONG)image->height);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biSizeImage   = 0;
    bmi.bmiHeader.biCompression = BI_RGB;

    uint32_t *data = 0;
    runtime->cursorBitmap = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, (void**)&data, 0, 0);

    bool null_cursor = true;

    for(uint32_t i = 0; i < image->height * image->width; ++i)
    {
        data[i] = ((uint32_t*)image->data)[i];
        if(data[i] & 0x000000FF)
        {
            null_cursor = false;
        }
    }

    runtime->cursorMask = CreateBitmap(image->width, image->height, 1, 1, 0);

    ICONINFO iconInfo = {0};
    iconInfo.hbmColor = runtime->cursorBitmap;
    iconInfo.hbmMask  = runtime->cursorMask;

    runtime->hCursor       = CreateIconIndirect(&iconInfo);
    runtime->currentCursor = image;

    if(null_cursor)
    {
        SetCursor(0);
        return;
    }

    SetCursor(runtime->hCursor);
}

// Renderer-agnostic wrapper functions.
void f144Init
(
    F144Runtime *runtime,
    F144Image *planes
){
    runtime->init(runtime, planes);
}

int32_t f144Shutdown
(
    F144Runtime *runtime
){
    return runtime->shutdown(runtime);
}

void f144BltBuffer
(
    F144Runtime *runtime
){
    runtime->bltBuffer(runtime);
}

void f144LoadText
(
    F144Runtime       *runtime,
    f144LoadTextSettings *settings
){
    runtime->loadText(
        runtime,
        settings->image,
        settings->sv,
        settings->font,
        settings->charsize,
        settings->spacing,
        settings->offsetX,
        settings->offsetY
    );
}

void f144CompositeImage
(
    F144Runtime        *runtime,
    f144CompositeSettings *settings
){
    runtime->compositeImage(
        runtime,
        settings->src,
        settings->dst,
        settings->pictop,
        settings->offsetSrcX,
        settings->offsetSrcY,
        settings->offsetDstX,
        settings->offsetDstY,
        settings->cropWidth,
        settings->cropHeight
    );
}


