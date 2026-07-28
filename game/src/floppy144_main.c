#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "river2D_main.h"

static EngineData *global_engine;

static void Floppy144FillBackbuffer(
    EngineData *engine,
    uint32_t colour
)
{
    uint32_t *pixels;
    size_t pixel_count;
    size_t pixel_index;

    if(!engine || !engine->backbuffer.data)
    {
        return;
    }

    pixels = (uint32_t *)engine->backbuffer.data;

    pixel_count =
        (size_t)engine->backbuffer.width *
        (size_t)engine->backbuffer.height;

    for(pixel_index = 0; pixel_index < pixel_count; ++pixel_index)
    {
        pixels[pixel_index] = colour;
    }
}

static LRESULT CALLBACK Floppy144WindowProc(
    HWND window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param
)
{
    (void)w_param;
    (void)l_param;

    switch(message)
    {
        case WM_CLOSE:
        {
            if(global_engine)
            {
                global_engine->running = false;
            }

            PostQuitMessage(0);
            return 0;
        }

        case WM_DESTROY:
        {
            if(global_engine)
            {
                global_engine->running = false;
            }

            PostQuitMessage(0);
            return 0;
        }

        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_SIZE:
        {
            InvalidateRect(window, 0, FALSE);
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT paint = {0};
            HDC device_context = BeginPaint(window, &paint);

            if(
                global_engine &&
                global_engine->bltBuffer &&
                global_engine->backbuffer.data
            )
            {
                global_engine->context = device_context;
                global_engine->bltBuffer(global_engine);
            }

            EndPaint(window, &paint);
            return 0;
        }
    }

    return DefWindowProcA(window, message, w_param, l_param);
}

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE previous_instance,
    LPSTR command_line,
    int show_command
)
{
    const char *class_name = "Floppy144WindowClass";

    EngineData engine = {0};
    RiverImage planes[RV_MAX_PLANES] = {0};
    StringView renderer_path = {".", 1};

    WNDCLASSA window_class = {0};
    RECT window_rect = {0, 0, 640, 360};
    MSG message = {0};

    (void)previous_instance;
    (void)command_line;

    engine.instance = instance;
    engine.windowName = "Floppy//144";

    engine.config.renderer = RV_RENDERER_SOFTWARE;
    engine.config.choices = RV_CHOICE_STATIC_CANVAS_BIT;

    engine.config.window_width = 640;
    engine.config.window_height = 360;
    engine.config.canvas_width = 640;
    engine.config.canvas_height = 360;

    rvResolveRenderer(
        &engine,
        renderer_path,
        RV_RENDERER_SOFTWARE
    );

    if(
        !engine.init ||
        !engine.shutdown ||
        !engine.bltBuffer
    )
    {
        MessageBoxA(
            0,
            "river2Dsoftware.dll could not be loaded.",
            "Floppy//144",
            MB_OK | MB_ICONERROR
        );

        return 1;
    }

    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Floppy144WindowProc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(0, IDC_ARROW);
    window_class.lpszClassName = class_name;

    if(!RegisterClassA(&window_class))
    {
        return 2;
    }

    AdjustWindowRect(
        &window_rect,
        WS_OVERLAPPEDWINDOW,
        FALSE
    );

    engine.window = CreateWindowExA(
        0,
        class_name,
        engine.windowName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        0,
        0,
        instance,
        0
    );

    if(!engine.window)
    {
        return 3;
    }

    global_engine = &engine;

    engine.init(&engine, planes);

    if(!engine.backbuffer.data)
    {
        MessageBoxA(
            engine.window,
            "river2D could not create the software framebuffer.",
            "Floppy//144",
            MB_OK | MB_ICONERROR
        );

        DestroyWindow(engine.window);
        return 4;
    }

    Floppy144FillBackbuffer(
        &engine,
        0x00261F18
    );

    ShowWindow(engine.window, show_command);
    UpdateWindow(engine.window);

    while(GetMessageA(&message, 0, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    global_engine = 0;

    return engine.shutdown(&engine);
}
