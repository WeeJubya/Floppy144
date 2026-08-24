#pragma once

#include "f144_runtime.h"

#include <stdint.h>

#define EXPORT __declspec(dllexport)

EXPORT void f144Win32Init
(
    F144Runtime *runtime,
    F144Image *planes
);

EXPORT int32_t f144Win32Shutdown
(
    F144Runtime *runtime
);

EXPORT void f144Win32BltBuffer
(
    F144Runtime *runtime
);

EXPORT void f144Win32CompositeImage
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
);

EXPORT void f144Win32LoadText
(
    F144Runtime *runtime,
    F144Image *image,
    StringView *sv,
    uint8_t    font,
    uint16_t   charsize,
    uint32_t   spacing,
    uint32_t   offsetX,
    uint32_t   offsetY
);

