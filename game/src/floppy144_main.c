#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "river2D_main.h"
#include "win32_river2Dsoftware_platform.h"

#include "floppy144_office.h"
#include "floppy144_recovery.h"
#include "floppy144_terminal.h"
#include "floppy144_world.h"

#include <stdbool.h>

typedef enum Floppy144Screen
{
    FLOPPY144_SCREEN_RECOVERY,
    FLOPPY144_SCREEN_OFFICE,
    FLOPPY144_SCREEN_TERMINAL
} Floppy144Screen;

static EngineData *global_engine;

static Floppy144Screen global_screen;
static Floppy144Player global_player;
static Floppy144TerminalState global_terminal;
static Floppy144WorldState global_world;

static bool global_recovery_started;

static void Floppy144BindStaticRenderer(
    EngineData *engine
)
{
    engine->init = init;
    engine->shutdown = shutdown;
    engine->bltBuffer = bltBuffer;
    engine->loadText = loadText;
    engine->compositeImage = compositeImage;
}

static void Floppy144Redraw(
    HWND window
)
{
    switch(global_screen)
    {
        case FLOPPY144_SCREEN_RECOVERY:
        {
            Floppy144RecoveryDraw(
                global_engine,
                global_recovery_started,
                &global_world
            );

            break;
        }

        case FLOPPY144_SCREEN_OFFICE:
        {
            Floppy144OfficeDraw(
                global_engine,
                &global_player,
                &global_world
            );

            break;
        }

        case FLOPPY144_SCREEN_TERMINAL:
        {
            Floppy144TerminalDraw(
                global_engine,
                &global_terminal,
                &global_world
            );

            break;
        }
    }

    InvalidateRect(
        window,
        0,
        FALSE
    );
}

static void Floppy144MovePlayer(
    HWND window,
    int32_t movement_x,
    int32_t movement_y
)
{
    Floppy144OfficeMove(
        &global_player,
        movement_x,
        movement_y
    );

    Floppy144Redraw(
        window
    );
}

static LRESULT CALLBACK Floppy144WindowProc(
    HWND window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param
)
{
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

        case WM_KEYDOWN:
        {
            switch(global_screen)
            {
                case FLOPPY144_SCREEN_RECOVERY:
                {
                    switch(w_param)
                    {
                        case VK_RETURN:
                        {
                            switch(global_recovery_started)
                            {
                                case false:
                                {
                                    global_recovery_started =
                                        true;

                                    break;
                                }

                                case true:
                                {
                                    global_screen =
                                        FLOPPY144_SCREEN_OFFICE;

                                    Floppy144OfficeReset(
                                        &global_player
                                    );

                                    break;
                                }
                            }

                            Floppy144Redraw(
                                window
                            );

                            return 0;
                        }

                        case VK_ESCAPE:
                        {
                            PostMessageA(
                                window,
                                WM_CLOSE,
                                0,
                                0
                            );

                            return 0;
                        }
                    }

                    break;
                }

                case FLOPPY144_SCREEN_OFFICE:
                {
                    switch(w_param)
                    {
                        case VK_LEFT:
                        case 'A':
                        {
                            Floppy144MovePlayer(
                                window,
                                -8,
                                0
                            );

                            return 0;
                        }

                        case VK_RIGHT:
                        case 'D':
                        {
                            Floppy144MovePlayer(
                                window,
                                8,
                                0
                            );

                            return 0;
                        }

                        case VK_UP:
                        case 'W':
                        {
                            Floppy144MovePlayer(
                                window,
                                0,
                                -8
                            );

                            return 0;
                        }

                        case VK_DOWN:
                        case 'S':
                        {
                            Floppy144MovePlayer(
                                window,
                                0,
                                8
                            );

                            return 0;
                        }

                        case 'E':
                        {
                            if(
                                Floppy144OfficeNearTerminal(
                                    &global_player
                                )
                            )
                            {
                                global_screen =
                                    FLOPPY144_SCREEN_TERMINAL;

                                Floppy144TerminalReset(
                                    &global_terminal
                                );

                                Floppy144Redraw(
                                    window
                                );
                            }

                            return 0;
                        }

                        case VK_ESCAPE:
                        {
                            global_screen =
                                FLOPPY144_SCREEN_RECOVERY;

                            Floppy144Redraw(
                                window
                            );

                            return 0;
                        }
                    }

                    break;
                }

                case FLOPPY144_SCREEN_TERMINAL:
                {
                    switch(w_param)
                    {
                        case VK_UP:
                        case 'W':
                        {
                            Floppy144TerminalMoveSelection(
                                &global_terminal,
                                -1
                            );

                            Floppy144Redraw(
                                window
                            );

                            return 0;
                        }

                        case VK_DOWN:
                        case 'S':
                        {
                            Floppy144TerminalMoveSelection(
                                &global_terminal,
                                1
                            );

                            Floppy144Redraw(
                                window
                            );

                            return 0;
                        }

                        case VK_RETURN:
                        {
                            Floppy144TerminalOpenSelection(
                                &global_terminal,
                                &global_world
                            );

                            Floppy144Redraw(
                                window
                            );

                            return 0;
                        }

                        case VK_ESCAPE:
                        {
                            switch(
                                Floppy144TerminalDetailOpen(
                                    &global_terminal
                                )
                            )
                            {
                                case true:
                                {
                                    Floppy144TerminalCloseDetail(
                                        &global_terminal
                                    );

                                    break;
                                }

                                case false:
                                {
                                    global_screen =
                                        FLOPPY144_SCREEN_OFFICE;

                                    break;
                                }
                            }

                            Floppy144Redraw(
                                window
                            );

                            return 0;
                        }
                    }

                    break;
                }
            }

            return 0;
        }

        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_SIZE:
        {
            InvalidateRect(
                window,
                0,
                FALSE
            );

            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT paint = {0};

            HDC device_context =
                BeginPaint(window, &paint);

            if(
                global_engine &&
                global_engine->bltBuffer &&
                global_engine->backbuffer.data
            )
            {
                global_engine->context =
                    device_context;

                global_engine->bltBuffer(
                    global_engine
                );
            }

            EndPaint(
                window,
                &paint
            );

            return 0;
        }
    }

    return DefWindowProcA(
        window,
        message,
        w_param,
        l_param
    );
}

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE previous_instance,
    LPSTR command_line,
    int show_command
)
{
    const char *class_name =
        "Floppy144WindowClass";

    EngineData engine = {0};

    RiverImage planes[
        RV_MAX_PLANES
    ] = {0};

    WNDCLASSA window_class = {0};

    RECT window_rect =
    {
        0,
        0,
        640,
        360
    };

    MSG message = {0};

    (void)previous_instance;
    (void)command_line;

    engine.instance = instance;
    engine.windowName = "Floppy//144";

    engine.config.renderer =
        RV_RENDERER_SOFTWARE;

    engine.config.choices =
        RV_CHOICE_STATIC_CANVAS_BIT;

    engine.config.window_width = 640;
    engine.config.window_height = 360;
    engine.config.canvas_width = 640;
    engine.config.canvas_height = 360;

    Floppy144BindStaticRenderer(
        &engine
    );

    window_class.style =
        CS_HREDRAW | CS_VREDRAW;

    window_class.lpfnWndProc =
        Floppy144WindowProc;

    window_class.hInstance =
        instance;

    window_class.hCursor =
        LoadCursorW(0, IDC_ARROW);

    window_class.lpszClassName =
        class_name;

    if(!RegisterClassA(&window_class))
    {
        return 1;
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
        return 2;
    }

    global_engine = &engine;

    global_screen =
        FLOPPY144_SCREEN_RECOVERY;

    global_recovery_started =
        false;

    Floppy144OfficeReset(
        &global_player
    );

    Floppy144WorldReset(
        &global_world
    );

    Floppy144TerminalReset(
        &global_terminal
    );

    engine.init(
        &engine,
        planes
    );

    if(!engine.backbuffer.data)
    {
        MessageBoxA(
            engine.window,
            "river2D could not create the software framebuffer.",
            "Floppy//144",
            MB_OK | MB_ICONERROR
        );

        DestroyWindow(
            engine.window
        );

        return 3;
    }

    Floppy144RecoveryDraw(
        &engine,
        global_recovery_started,
        &global_world
    );

    ShowWindow(
        engine.window,
        show_command
    );

    UpdateWindow(
        engine.window
    );

    while(
        GetMessageA(
            &message,
            0,
            0,
            0
        ) > 0
    )
    {
        TranslateMessage(
            &message
        );

        DispatchMessageA(
            &message
        );
    }

    global_engine = 0;

    return engine.shutdown(
        &engine
    );
}

