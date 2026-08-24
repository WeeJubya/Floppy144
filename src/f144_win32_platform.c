#include "f144_runtime.h"

#include "f144_win32_platform.h"

#include <stdio.h>

void f144ResizeBackbuffer
(
    F144Runtime *runtime,
    uint32_t   width,
    uint32_t   height
){
    if(runtime->backbuffer.data)
    {
        VirtualFree(runtime->backbuffer.data, 0, MEM_RELEASE);
    }

    runtime->backbuffer.width  = width;
    runtime->backbuffer.height = height;

    runtime->backbuffer.info.bmiHeader.biSize        = sizeof(runtime->backbuffer.info.bmiHeader);
    runtime->backbuffer.info.bmiHeader.biWidth       =  (long)runtime->backbuffer.width;
    runtime->backbuffer.info.bmiHeader.biHeight      = -(long)runtime->backbuffer.height;
    runtime->backbuffer.info.bmiHeader.biPlanes      = 1;
    runtime->backbuffer.info.bmiHeader.biBitCount    = 32;
    runtime->backbuffer.info.bmiHeader.biCompression = BI_RGB;

    runtime->backbuffer.data = VirtualAlloc(0, width * height * F144_BPP,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(!runtime->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: failed to resize backbuffer.\033[0m");
    }
}

void f144Win32Init
(
    F144Runtime *runtime,
    F144Image *planes
){
    runtime->running = true;
    runtime->planes  = planes;

    if(!runtime->windowName)
    {
        runtime->windowName = "Floppy//144";
    }

    if(runtime->config.static_canvas)
    {
        f144CreateImage(&runtime->backbuffer,
                            runtime->config.canvas_width, runtime->config.canvas_height);
    }
    else
    {
        f144CreateImage(&runtime->backbuffer,
                            runtime->config.window_width, runtime->config.window_height);
    }
}

int32_t f144Win32Shutdown
(
    F144Runtime *runtime
){
    DeleteObject(runtime->cursorMask);
    DestroyWindow(runtime->window);

    return 0;
}

void f144Win32CompositeImage
(
    F144Runtime *runtime,
    F144Image *src,
    F144Image *dst,
    uint8_t    pictop,
    uint32_t   offsetSrcX,
    uint32_t   offsetSrcY,
    uint32_t   offsetDstX,
    uint32_t   offsetDstY,
    uint32_t   cropWidth,
    uint32_t   cropHeight
){
    if(pictop != F144_PICTOP_OVER)
    {
        fprintf(stderr, "\033[31;1;7mERROR: pictop %u not impletmented on windows.\033[0m\n", pictop);
        return;
    }

    if(!src)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite with.\033[0m\n");
        return;
    }
    if(!src->data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: src->data is nullptr.\033[0m\n");
        return;
    }

    if(!dst)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite onto.\033[0m\n");
        return;
    }
    if(!dst->data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: dst->data is nullptr.\033[0m\n");
        return;
    }

    uint64_t copyWidth = src->width * F144_BPP;
    uint64_t bufWidth  = dst->width * F144_BPP;

    if(offsetDstX + cropWidth > runtime->backbuffer.width)
    {
        cropWidth = runtime->backbuffer.width - offsetDstX;
    }

    if(offsetDstY + cropHeight > runtime->backbuffer.height)
    {
        cropHeight = runtime->backbuffer.height - offsetDstY;
    }

    uint8_t *dst_data = (uint8_t*)dst->data +
                        offsetDstY * bufWidth + offsetDstX * F144_BPP;
    uint8_t *src_data = src->data + offsetSrcY * copyWidth + offsetSrcX * F144_BPP;

    cropWidth *= F144_BPP;
    for(uint32_t y = 0; y < cropHeight; ++y)
    {
        for(uint32_t x = 0; x < cropWidth; x += F144_BPP)
        {
            uint64_t srcIndex = y * copyWidth + x;
            uint64_t dstIndex = y * bufWidth + x;
            if(src_data[srcIndex + 3])
            {
                dst_data[dstIndex]     = src_data[srcIndex];
                dst_data[dstIndex + 1] = src_data[srcIndex + 1];
                dst_data[dstIndex + 2] = src_data[srcIndex + 2];
                dst_data[dstIndex + 3] = src_data[srcIndex + 3];
            }
        }
    }
}

void f144Win32BltBuffer
(
    F144Runtime *runtime
){
    Dimensions dim = {0};
    dim = f144GetWindowSize(runtime);

    StretchDIBits(runtime->context, 0, 0, dim.width, dim.height, 0, 0,
                  (int)runtime->backbuffer.width, (int)runtime->backbuffer.height,
                  runtime->backbuffer.data, &runtime->backbuffer.info, DIB_RGB_COLORS,
                  SRCCOPY);
}

void f144Win32LoadText
(
    F144Runtime    *runtime,
    F144Image *image,
    StringView    *sv,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetX,
    uint32_t      offsetY
){
    if(!runtime->planes[font].data)
    {
        fprintf(stderr, "\033[31;3;1mERROR: Font not found. Check loaded planes."
                "\033[0m\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "\033[31;3;1mERROR: Destination image is null.\033[0m\n");
        return;
    }

    uint32_t minTextWidth = (charsize + spacing) * (uint32_t)sv->size;

    if(image->width < minTextWidth || image->height < charsize)
    {
        f144DestroyImage(image);
    }

    if(!image->data)
    {
        f144CreateImage(image, minTextWidth, charsize);
    }

    if(offsetX > image->width)
    {
        fprintf(stderr, "offsetX too large.\n");
        return;
    }
    if(offsetY > image->height)
    {
        fprintf(stderr, "offsetY too large.\n");
        return;
    }

    uint32_t fontImgWidth = runtime->planes[font].width;
    uint32_t imageChars = (image->width) / ((charsize + spacing));

    for(uint32_t i = 0; i < imageChars; ++i)
    {
        char character = 0x7F;
        if(i < sv->size)
        {
            character = sv->data[i];
        }

        if(character < 0x21 || character > 0x7F)
        {
            continue;
        }

        uint32_t charBigX = (uint32_t)(character - 0x21) * charsize % fontImgWidth;
        uint32_t charBigY = (uint32_t)(character - 0x21) * charsize / fontImgWidth;

        uint64_t trueSrcOffset = (charBigY * charsize * fontImgWidth + charBigX) *
                                 F144_BPP;
        uint64_t trueDestOffset = (offsetY * image->width + offsetX + i *
                                  (charsize + spacing)) * F144_BPP;

        uint8_t* charloc = runtime->planes[font].data + trueSrcOffset;
        uint8_t* destloc = image->data + trueDestOffset;

        for(uint32_t j = 0; j < charsize; ++j)
        {
            uint8_t* charlineLoc = charloc + j * fontImgWidth * F144_BPP;
            uint8_t* destlineLoc = destloc + j * image->width * F144_BPP;

            memcpy(destlineLoc, charlineLoc, charsize * F144_BPP);
        }
    }
}
