#pragma once

#include "string_view.h"

#include <stdint.h>
#include <stdio.h>

#define bool  _Bool
#define true  1
#define false 0

#define v_persistent  static
#define s_global      static
#define f_internal    static

#ifdef BUILD_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include "Windows.h"
    #define  F144_CONFIG_PATH "./river2D.ini"

    #define F144_MOUSE1 0x01
    #define F144_MOUSE2 0x02
    #define F144_MOUSE3 0x10
#endif

#define F144_ERROR_LOADIMAGE_PTR    1
#define F144_ERROR_LOADIMAGE_FILE   2
#define F144_SUCCESS                128

#define F144_VERTICAL   1
#define F144_HORIZONTAL 2

#define F144_UP    1
#define F144_DOWN  2
#define F144_LEFT  3
#define F144_RIGHT 4

#define F144_BPP                      4
#define F144_PIXDEPTH                 32
#define F144_MAX_PLANES               64
#define F144_MAX_THREADS              8

#define F144_RENDERER_SOFTWARE        0
#define F144_RENDERER_OPENGL          1
#define F144_RENDERER_VULKAN          2
#define F144_RENDERER_DIRECTX         3

#define F144_ASCII_CURSOR             0x01
#define F144_ASCII_UP                 0x02
#define F144_ASCII_DOWN               0x03
#define F144_ASCII_LEFT               0x04
#define F144_ASCII_RIGHT              0x05
#define F144_ASCII_BACKSPACE          0x08
#define F144_ASCII_TAB                0x09
#define F144_ASCII_ENTER              0x0A
#define F144_ASCII_LSHIFT             0x0E
#define F144_ASCII_RSHIFT             0x0F
#define F144_ASCII_LCTRL              0x11
#define F144_ASCII_RCTRL              0x12
#define F144_ASCII_LALT               0x13
#define F144_ASCII_ALTGR              0x14
#define F144_ASCII_ESCAPE             0x1B
#define F144_ASCII_DELETE             0x7F

#define F144_CHANNELS_RGBA            0
#define F144_CHANNELS_BGRA            1
#define F144_CHANNELS_RGB             2
#define F144_CHANNELS_BGR             3
#define F144_CHANNELS_MAX             3

#define F144_FONT_DEFAULT             0
#define F144_FONT_MAX                 0

#define F144_PICTOP_MINIMUM           0
#define F144_PICTOP_CLEAR             0
#define F144_PICTOP_SRC               1
#define F144_PICTOP_DST               2
#define F144_PICTOP_OVER              3
#define F144_PICTOP_OVERREVERSE       4
#define F144_PICTOP_IN                5
#define F144_PICTOP_INREVERSE         6
#define F144_PICTOP_OUT               7
#define F144_PICTOP_OUTREVERSE        8
#define F144_PICTOP_ATOP              9
#define F144_PICTOP_ATOPREVERSE       10
#define F144_PICTOP_XOR               11
#define F144_PICTOP_ADD               12
#define F144_PICTOP_SATURATE          13
#define F144_PICTOP_MAXIMUM           13

#define F144_CHOICE_SHOW_FPS_BIT      1
#define F144_CHOICE_STATIC_CANVAS_BIT 2
#define F144_CHOICE_BACKGROUNDS_BYTE  0xFF000000

#define F144_BIT_HOVER                0x01

#define F144_ALIGN_TOPLEFT            1
#define F144_ALIGN_TOPCENTER          2
#define F144_ALIGN_TOPRIGHT           3
#define F144_ALIGN_CENTERLEFT         4
#define F144_ALIGN_CENTERRIGHT        5
#define F144_ALIGN_BOTTOMLEFT         6
#define F144_ALIGN_BOTTOMCENTER       7
#define F144_ALIGN_BOTTOMRIGHT        8

typedef struct PerformanceCounter
{
    uint64_t time;
    uint64_t freq;
}
PerformanceCounter;

typedef struct F144Config
{
    uint32_t choices;
    uint8_t  renderer;
    uint8_t  backgrounds;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t canvas_width;
    uint32_t canvas_height;
}
F144Config;

typedef struct F144Image
{
    StringView path;
    uint8_t    *data;
    uint8_t    channels;
    uint32_t   width;
    uint32_t   height;

    #ifdef BUILD_LINUX
    Pixmap   pixmap;
    Picture  picture;
    #endif

    #ifdef BUILD_WINDOWS
    BITMAPINFO info;
    #endif
}
F144Image;

typedef struct F144Time
{
    int64_t s;
    int64_t ns;
}
F144Time;

typedef struct AsciiKey
{
    uint8_t key;
    uint8_t raw;
}
AsciiKey;

typedef struct Coordinates
{
    float x;
    float y;
}
Coordinates;

typedef struct TileCoords
{
    uint16_t x;
    uint16_t y;
}
TileCoords;

typedef struct Area
{
    Coordinates upLeft;
    Coordinates upRight;
    Coordinates lowLeft;
    Coordinates lowRight;
}
Area;

typedef struct Rect
{
    Coordinates upLeft;
    Coordinates lowRight;
}
Rect;

typedef struct Button
{
    StringView name;
    Rect       area;
    uint8_t    status;
}
Button;

typedef struct Dimensions
{
    uint32_t width;
    uint32_t height;
}
Dimensions;

typedef struct F144Controls
{
    uint64_t     keymap;
    uint64_t     buttonmap;
    Coordinates  pointer;
    F144Time     lastScrollTime;
    uint64_t     rumble;
    uint8_t      keycodes[128];
    uint8_t      buttoncodes[64];
    char         ascii;
}
F144Controls;

typedef struct F144Runtime
{
    const char    *windowName;
    F144Controls  controls;
    F144Config    config;
    F144Image     backbuffer;
    F144Image     *planes;
    F144Image     *currentCursor;
    bool          running;

#ifdef BUILD_LINUX
    Display           *display;
    Screen            *screen;
    XRenderPictFormat *format;
    Visual            *visual;
    Window            window;
    GC                context;
    Picture           blitDstPict;
#endif

#ifdef BUILD_WINDOWS
    HINSTANCE          instance;
    HWND               window;
    HDC                context;
    HBITMAP            cursorBitmap;
    HBITMAP            cursorMask;
    HCURSOR            hCursor;
#endif

    void    (*init)           (struct F144Runtime *runtime,    F144Image *planes);
    int32_t (*shutdown)       (struct F144Runtime *runtime);
    void    (*bltBuffer)      (struct F144Runtime *runtime);

    void    (*loadText)       (struct F144Runtime *runtime,    F144Image *image,
                               StringView        *sv,        uint8_t    font,
                               uint16_t          charsize,   uint32_t   spacing,
                               uint32_t          offsetX,    uint32_t   offsetY);

    void    (*compositeImage) (struct F144Runtime *runtime,    F144Image *src,
                               F144Image        *dst,       uint8_t    pictop,
                               uint32_t          offsetSrcX, uint32_t   offsetSrcY,
                               uint32_t          offsetDstX, uint32_t   offsetDstY,
                               uint32_t          cropWidth,  uint32_t   cropHeight);
}
F144Runtime;

typedef struct f144ButtonSettings
{
    F144Image  *img;
    Coordinates point;
    StringView  *name;
    Button      *button;
    uint8_t     alignment;
    uint8_t     font;
    uint16_t    charsize;
    uint32_t    spacing;
}
f144ButtonSettings;

extern void f144CreateImage
(
    F144Runtime *runtime,
    F144Image *image,
    uint32_t   width,
    uint32_t   height
);

extern void f144AppendImage
(
    F144Runtime *runtime,
    F144Image *src,
    F144Image *dst,
    uint8_t    direction
);

extern void f144SyncImage
(
    F144Runtime *runtime,
    F144Image *image,
    bool       CPU_to_GPU
);

extern void f144ClearImage
(
    F144Runtime *runtime,
    F144Image *image
);

extern void f144DestroyImage
(
    F144Image *image
);

extern F144Time f144QueryTime
(
    void
);

// delta is taken from time2 - time1.
extern F144Time f144DeltaTime
(
    const F144Time *time1,
    const F144Time *time2
);

// delta is taken from time2 - time1.
extern float f144DeltaTime_ms
(
    const F144Time *time1,
    const F144Time *time2
);

// delta is taken from time2 - time1.
extern int64_t f144DeltaTime_ns
(
    const F144Time *time1,
    const F144Time *time2
);

extern F144Time f144DeltaTime_now
(
    const F144Time *time
);

extern float f144DeltaTime_now_ms
(
    const F144Time *time
);

extern uint64_t f144DeltaTime_now_ns
(
    const F144Time *time
);

#ifdef BUILD_LINUX
extern AsciiKey f144ProcessXKey
(
    F144Runtime *runtime,
    XEvent     *event
);
#endif

extern Dimensions f144GetWindowSize
(
    F144Runtime *runtime
);

extern void f144ChangeCursor
(
    F144Runtime *runtime,
    F144Image *image
);

extern bool f144InsideArea
(
    const Coordinates *point,
    const Area        *area
);

extern bool f144InsideRect
(
    const Coordinates *point,
    const Rect        *rect
);

extern void f144CreateButton
(
    F144Runtime       *runtime,
    f144ButtonSettings *settings
);

// initializes the runtime and all needed resources.
extern void f144Init
(
    F144Runtime *runtime,
    F144Image *planes
);

// shuts down the runtime and safely frees all used resources.
extern int32_t f144Shutdown
(
    F144Runtime *runtime
);

// takes whatever is in `runtime->backbuffer` and blts it to the window, performing
// scaling, if necessary.
extern void f144BltBuffer
(
    F144Runtime *runtime
);

typedef struct f144LoadTextSettings
{
    F144Image *image;
    StringView *sv;
    uint8_t    font;
    uint16_t   charsize;
    uint32_t   spacing;
    uint32_t   offsetX;
    uint32_t   offsetY;
}
f144LoadTextSettings;

// reads the text from `sv`, creates an image with the wanted text,
// taking the font image from `runtime->planes[font]`.
// Needs you to specify `charsize` and the `offsetX`, `offsetY`.
extern void f144LoadText
(
    F144Runtime         *runtime,
    f144LoadTextSettings *settings
);

typedef struct f144CompositeSettings
{
    F144Image *src;
    F144Image *dst;
    uint8_t    pictop;
    uint32_t   offsetSrcX;
    uint32_t   offsetSrcY;
    uint32_t   offsetDstX;
    uint32_t   offsetDstY;
    uint32_t   cropWidth;
    uint32_t   cropHeight;
}
f144CompositeSettings;

// Takes `src` at offsets `offsetSrcX` and `offsetSrcY`.
// Composites `src` onto `dst`, at `offsetDstX`, `offsetDstY`, given `pictop`.
extern void f144CompositeImage
(
    F144Runtime          *runtime,
    f144CompositeSettings *settings
);
