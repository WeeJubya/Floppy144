/* Standalone Beta.2 display contract tests: no game state or inherited runtime. */
#include "floppy144_win32_display.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned failures;
static unsigned assertions;

#define CHECK(condition, description) do {                       \
    ++assertions;                                                \
    if(!(condition)) {                                            \
        ++failures;                                               \
        printf("FAIL: %s\n", (description));                     \
    }                                                            \
} while(0)

typedef struct AllocationCase
{
    unsigned allocate_calls;
    unsigned release_calls;
    unsigned fail_on;
} AllocationCase;

static void *TestAllocate(size_t bytes, void *context)
{
    AllocationCase *test = (AllocationCase *)context;
    ++test->allocate_calls;
    if(test->allocate_calls == test->fail_on)
    {
        return NULL;
    }
    return malloc(bytes);
}

static void TestRelease(void *memory, void *context)
{
    AllocationCase *test = (AllocationCase *)context;
    ++test->release_calls;
    free(memory);
}

static void TestMemory(void)
{
    Floppy144Win32Display *display = Floppy144Win32DisplayCreate();
    Floppy144Surface *frame = Floppy144Win32DisplayFramebuffer(display);
    size_t i;
    CHECK(display != NULL && frame != NULL, "normal frame allocation");
    if(frame != NULL)
    {
        CHECK(frame->width == 640U && frame->height == 360U,
              "logical frame dimensions");
        for(i = 0; i < 640U * 360U; ++i)
        {
            if(frame->pixels[i] != 0U)
            {
                break;
            }
        }
        CHECK(i == 640U * 360U, "entire frame zero initialised");
        frame->pixels[0] = 0x00FF0000U;
        frame->pixels[639U] = 0x0000FF00U;
        frame->pixels[359U * 640U] = 0x000000FFU;
        frame->pixels[360U * 640U - 1U] = 0x00FFFFFFU;
        CHECK(frame->pixels[0] == 0x00FF0000U &&
              frame->pixels[639U] == 0x0000FF00U &&
              frame->pixels[359U * 640U] == 0x000000FFU &&
              frame->pixels[360U * 640U - 1U] == 0x00FFFFFFU,
              "corner pixel address and colour values");
    }
    Floppy144Win32DisplayDestroy(&display);
    CHECK(display == NULL, "destroy invalidates owned pointer");
    CHECK(Floppy144Win32DisplayFramebuffer(NULL) == NULL,
          "NULL frame access is unavailable");
    Floppy144Win32DisplayDestroy(&display);
    Floppy144Win32DisplayDestroy(NULL);
    CHECK(display == NULL, "repeated and null destroy are safe");
}

static void TestAllocationFailure(void)
{
    AllocationCase test = {0};
    Floppy144Win32Display *display;

    test.fail_on = 1U;
    display = Floppy144Win32DisplayCreateWithAllocator(
        TestAllocate, TestRelease, &test
    );
    CHECK(display == NULL && test.allocate_calls == 1U &&
          test.release_calls == 0U, "failed state allocation leaves no owner");

    memset(&test, 0, sizeof(test));
    test.fail_on = 2U;
    display = Floppy144Win32DisplayCreateWithAllocator(
        TestAllocate, TestRelease, &test
    );
    CHECK(display == NULL && test.allocate_calls == 2U &&
          test.release_calls == 1U, "failed pixel allocation releases state");

    memset(&test, 0, sizeof(test));
    display = Floppy144Win32DisplayCreateWithAllocator(
        TestAllocate, TestRelease, &test
    );
    CHECK(display != NULL && test.allocate_calls == 2U,
          "injected allocation succeeds with both objects");
    Floppy144Win32DisplayDestroy(&display);
    Floppy144Win32DisplayDestroy(&display);
    CHECK(test.release_calls == 2U, "each owned allocation released once");
    CHECK(Floppy144Win32DisplayCreateWithAllocator(NULL, TestRelease, &test) == NULL &&
          Floppy144Win32DisplayCreateWithAllocator(TestAllocate, NULL, &test) == NULL,
          "incomplete allocator contract rejected");
}

typedef struct GeometryCase
{
    int64_t client_width;
    int64_t client_height;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} GeometryCase;

static void TestGeometry(void)
{
    static const GeometryCase cases[] = {
        {640,360,0,0,640,360},
        {1280,720,0,0,1280,720},
        {800,600,0,75,800,450},
        {1000,700,0,69,1000,562},
        {1200,500,156,0,888,500},
        {1024,768,0,96,1024,576},
        {700,1000,0,303,700,393},
        {320,180,0,0,320,180},
        {1,1,0,0,1,1},
        {1000,300,233,0,533,300},
        {2147483647LL,1,1073741823,0,1,1}
    };
    Floppy144DisplayViewport rect;
    size_t i;
    for(i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        const GeometryCase *wanted = &cases[i];
        bool ok = Floppy144Win32DisplayCalculateViewport(
            wanted->client_width, wanted->client_height, &rect
        );
        CHECK(ok && rect.x == wanted->x && rect.y == wanted->y &&
              rect.width == wanted->width && rect.height == wanted->height,
              "viewport fits and centres in client");
    }

    CHECK(!Floppy144Win32DisplayCalculateViewport(10, 10, NULL),
          "NULL viewport rejected");
    CHECK(!Floppy144Win32DisplayCalculateViewport(0, 360, &rect) &&
          rect.width == 0 && rect.height == 0,
          "zero client rejected and output reset");
    CHECK(!Floppy144Win32DisplayCalculateViewport(-1, 360, &rect),
          "negative client rejected");
    CHECK(!Floppy144Win32DisplayCalculateViewport(INT64_MAX, 1, &rect) &&
          !Floppy144Win32DisplayCalculateViewport(1, INT64_MAX, &rect),
          "int32 and multiplication overflow prevented");
}

#ifdef _WIN32
/* Offscreen GDI tests require no full game and no visible desktop. */
static uint32_t *CreateTarget(HDC *dc_out, HBITMAP *bitmap_out,
                              HGDIOBJ *previous_out, int width, int height)
{
    BITMAPINFO bitmap_info;
    void *bits = NULL;
    *dc_out = CreateCompatibleDC(NULL);
    if(*dc_out == NULL)
    {
        return NULL;
    }
    memset(&bitmap_info, 0, sizeof(bitmap_info));
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = width;
    bitmap_info.bmiHeader.biHeight = -height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    *bitmap_out = CreateDIBSection(*dc_out, &bitmap_info, DIB_RGB_COLORS,
                                   &bits, NULL, 0);
    if(*bitmap_out == NULL || bits == NULL)
    {
        DeleteDC(*dc_out);
        *dc_out = NULL;
        return NULL;
    }
    *previous_out = SelectObject(*dc_out, *bitmap_out);
    if(*previous_out == NULL || *previous_out == HGDI_ERROR)
    {
        DeleteObject(*bitmap_out);
        DeleteDC(*dc_out);
        *dc_out = NULL;
        return NULL;
    }
    return (uint32_t *)bits;
}

static void TestGdiTarget(Floppy144Win32Display *display,
                          int width, int height)
{
    HDC dc = NULL;
    HBITMAP bitmap = NULL;
    HGDIOBJ previous = NULL;
    Floppy144DisplayViewport rect;
    uint32_t *target;
    size_t i;
    size_t count = (size_t)width * (size_t)height;
    target = CreateTarget(&dc, &bitmap, &previous, width, height);
    CHECK(target != NULL, "offscreen Win32 GDI target allocation");
    if(target == NULL)
    {
        return;
    }
    for(i = 0U; i < count; ++i)
    {
        target[i] = 0x00777777U;
    }
    CHECK(Floppy144Win32DisplayPresent(display, dc, width, height),
          "GDI pixel transfer and presentation bands");
    CHECK(Floppy144Win32DisplayCalculateViewport(width, height, &rect),
          "offscreen rectangle computed");
    CHECK((target[(size_t)rect.y * (size_t)width + (size_t)rect.x] & 0x00FFFFFFU)
          == 0x00FF0000U, "top-left red stays red and upright");
    CHECK((target[(size_t)rect.y * (size_t)width +
                  (size_t)(rect.x + rect.width - 1)] & 0x00FFFFFFU)
          == 0x0000FF00U, "top-right green stays green");
    CHECK((target[(size_t)(rect.y + rect.height - 1) * (size_t)width +
                  (size_t)rect.x] & 0x00FFFFFFU)
          == 0x000000FFU, "bottom-left blue stays blue");
    CHECK((target[(size_t)(rect.y + rect.height - 1) * (size_t)width +
                  (size_t)(rect.x + rect.width - 1)] & 0x00FFFFFFU)
          == 0x00FFFFFFU, "bottom-right white stays white");
    if(rect.y > 0)
    {
        CHECK((target[0] & 0x00FFFFFFU) == 0U,
              "top presentation band is black");
    }
    if(rect.x > 0)
    {
        CHECK((target[(size_t)rect.y * (size_t)width] & 0x00FFFFFFU) == 0U,
              "side presentation band is black");
    }
    if(width == 640 && height == 360)
    {
        CHECK((target[10U * 640U + 10U] & 0x00FFFFFFU) == 0x00123456U,
              "GDI ignores nonzero reserved high colour byte");
    }
    (void)SelectObject(dc, previous);
    (void)DeleteObject(bitmap);
    (void)DeleteDC(dc);
}

static Floppy144Win32Display *window_display;
static unsigned window_paints;
static unsigned window_paint_failures;

static LRESULT CALLBACK TestWindowProc(HWND window, UINT message,
                                       WPARAM w_param, LPARAM l_param)
{
    (void)w_param;
    (void)l_param;
    if(message == WM_ERASEBKGND)
    {
        return 1;
    }
    if(message == WM_PAINT)
    {
        PAINTSTRUCT paint;
        RECT client;
        HDC dc = BeginPaint(window, &paint);
        if(dc != NULL)
        {
            ++window_paints;
            if(!GetClientRect(window, &client) ||
               !Floppy144Win32DisplayPresent(window_display, dc,
                         client.right - client.left,
                         client.bottom - client.top))
            {
                ++window_paint_failures;
            }
            (void)EndPaint(window, &paint);
        }
        else
        {
            ++window_paint_failures;
        }
        return 0;
    }
    return DefWindowProcA(window, message, w_param, l_param);
}

static void TestWindowSmoke(Floppy144Win32Display *display)
{
    WNDCLASSA klass;
    ATOM registered;
    HWND window;
    memset(&klass, 0, sizeof(klass));
    klass.lpfnWndProc = TestWindowProc;
    klass.hInstance = GetModuleHandleA(NULL);
    klass.lpszClassName = "Floppy144Beta2DisplayIsolationTest";
    registered = RegisterClassA(&klass);
    if(registered == 0)
    {
        printf("SKIP: window smoke (native window class unavailable)\n");
        return;
    }
    window_display = display;
    window = CreateWindowExA(0, klass.lpszClassName, "Beta2 isolated test",
                WS_OVERLAPPEDWINDOW, 0, 0, 720, 540,
                NULL, NULL, klass.hInstance, NULL);
    if(window == NULL)
    {
        printf("SKIP: window smoke (window creation unavailable)\n");
        (void)UnregisterClassA(klass.lpszClassName, klass.hInstance);
        return;
    }
    (void)InvalidateRect(window, NULL, FALSE);
    (void)UpdateWindow(window);
    (void)SetWindowPos(window, NULL, 0, 0, 1000, 700,
                       SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    (void)RedrawWindow(window, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    if(window_paints == 0U)
    {
        printf("SKIP: window smoke (no OS paint dispatched)\n");
    }
    else
    {
        CHECK(window_paint_failures == 0U,
              "real Win32 paint and resize path");
    }
    (void)DestroyWindow(window);
    (void)UnregisterClassA(klass.lpszClassName, klass.hInstance);
    window_display = NULL;
}

static void TestWindowsGdi(void)
{
    Floppy144Win32Display *display = Floppy144Win32DisplayCreate();
    Floppy144Surface *frame = Floppy144Win32DisplayFramebuffer(display);
    size_t i;
    CHECK(frame != NULL, "GDI fixture frame allocation");
    if(frame == NULL)
    {
        return;
    }
    for(i = 0U; i < 640U * 360U; ++i)
    {
        frame->pixels[i] = 0x00111111U;
    }
    /* Four-pixel corner tiles remain observable under 2:1 downscaling. */
    {
        size_t row;
        size_t col;
        for(row = 0U; row < 4U; ++row)
        {
            for(col = 0U; col < 4U; ++col)
            {
                frame->pixels[row * 640U + col] = 0x00FF0000U;
                frame->pixels[row * 640U + (639U - col)] = 0x0000FF00U;
                frame->pixels[(359U - row) * 640U + col] = 0x000000FFU;
                frame->pixels[(359U - row) * 640U + (639U - col)] = 0x00FFFFFFU;
            }
        }
    }
    frame->pixels[10U * 640U + 10U] = 0xFF123456U;
    CHECK(!Floppy144Win32DisplayPresent(display, NULL, 640, 360),
          "invalid HDC explicitly rejected");
    TestGdiTarget(display, 640, 360);
    TestGdiTarget(display, 1280, 720);
    TestGdiTarget(display, 800, 600);
    TestGdiTarget(display, 1000, 700);
    TestGdiTarget(display, 1200, 500);
    TestGdiTarget(display, 320, 180);
    TestWindowSmoke(display);
    Floppy144Win32DisplayDestroy(&display);
}
#endif

int main(void)
{
    TestMemory();
    TestAllocationFailure();
    TestGeometry();
#ifdef _WIN32
    TestWindowsGdi();
#else
    printf("SKIP: Win32 GDI and window smoke (non-Windows host)\n");
#endif
    if(failures != 0U)
    {
        printf("B2-03 DISPLAY TESTS: FAIL (%u/%u assertions)\n",
               failures, assertions);
        return 1;
    }
    printf("B2-03 DISPLAY TESTS: PASS (%u assertions)\n", assertions);
    return 0;
}
