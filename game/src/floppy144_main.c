/*
 * Floppy//144 - Win32 entry point and game coordinator
 *
 * Connects river2D to Win32, owns the active screen and top-level session state,
 * routes keyboard input and asks the appropriate module to redraw.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "river2D_main.h"
#include "win32_river2Dsoftware_platform.h"

#include "floppy144_catalogue.h"
#include "floppy144_document.h"
#include "floppy144_office.h"
#include "floppy144_object_registry.h"
#include "floppy144_recovery.h"
#include "floppy144_terminal.h"
#include "floppy144_world.h"

#include <stdbool.h>

/*
 * Top-level screen state
 *
 * Only one of the recovery screen, office, terminal or catalogue is active.
 * This enum is the game's small screen-state machine.
 */

typedef enum Floppy144Screen
{
    FLOPPY144_SCREEN_RECOVERY,
    FLOPPY144_SCREEN_OFFICE,
    FLOPPY144_SCREEN_TERMINAL,
    FLOPPY144_SCREEN_CATALOGUE
} Floppy144Screen;

/*
 * Shared application state
 *
 * The Win32 callback cannot receive custom game arguments directly, so this
 * small prototype stores the engine and screen states at file scope.
 */

static EngineData *global_engine;

static Floppy144Screen global_screen;
static Floppy144Player global_player;
static Floppy144TerminalState global_terminal;
static Floppy144CatalogueState global_catalogue;
static Floppy144WorldState global_world;

/*
 * Short-lived interface state
 *
 * The office notice points to static text shown after an inspection. Movement
 * clears it. recovery_started controls the two-step boot sequence.
 */

static const char *global_office_notice;
static bool global_recovery_started;
static bool global_catalogue_direct_document;

/*
 * Bind river2D's statically linked software renderer
 *
 * The original engine can resolve a renderer DLL. This build assigns the function
 * pointers directly so Floppy144 ships as one game executable.
 */

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

/*
 * Redraw the active screen
 *
 * Each screen module writes a complete 640x360 frame into the backbuffer.
 * InvalidateRect then asks Windows to present that frame through WM_PAINT.
 */

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
                &global_world,
                global_office_notice
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

        case FLOPPY144_SCREEN_CATALOGUE:
        {
            Floppy144CatalogueDraw(
                global_engine,
                &global_catalogue
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

/*
 * Move the player and clear inspection text
 *
 * All movement keys pass through this helper so the behaviour is consistent.
 */

static void Floppy144MovePlayer(
    HWND window,
    int32_t movement_x,
    int32_t movement_y
)
{
    global_office_notice = 0;

    Floppy144OfficeMove(
        &global_player,
        &global_world,
        movement_x,
        movement_y
    );

    Floppy144Redraw(
        window
    );
}

/*
 * Execute the action declared by the best eligible nearby object.
 */

static void Floppy144InteractOffice(
    HWND window
)
{
    Floppy144ObjectId object =
        Floppy144OfficeInteractionTarget(
            &global_world,
            &global_player
        );

    const Floppy144ObjectDefinition *definition =
        Floppy144ObjectGet(
            object
        );

    const Floppy144ObjectInteractionDefinition *interaction =
        definition != NULL
            ? definition->interaction
            : NULL;

    if(interaction == NULL)
    {
        return;
    }

    switch(interaction->action)
    {
        case FLOPPY144_OBJECT_ACTION_OPEN_TERMINAL:
        {
            global_office_notice =
                0;

            global_screen =
                FLOPPY144_SCREEN_TERMINAL;

            Floppy144TerminalReset(
                &global_terminal,
                &global_world
            );

            Floppy144Redraw(
                window
            );

            return;
        }

        case FLOPPY144_OBJECT_ACTION_SHOW_NOTICE:
        {
            global_office_notice =
                interaction->notice;

            Floppy144Redraw(
                window
            );

            return;
        }

        case FLOPPY144_OBJECT_ACTION_NONE:
        default:
        {
            return;
        }
    }
}

/*
 * Win32 message handler
 *
 * Windows sends close, keyboard, resize and paint messages here. Keyboard handling
 * is routed first by active game screen, then by the key pressed.
 */

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
        /* Window lifetime: stop the engine loop and post the process quit message. */
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

        /*
         * Keyboard input
         *
         * The same key can mean different things on different screens, so each screen
         * owns a nested key switch.
         */

        case WM_CHAR:
        {
            if(
                global_screen !=
                FLOPPY144_SCREEN_TERMINAL
            )
            {
                break;
            }

            switch(w_param)
            {
                case '\b':
                {
                    Floppy144TerminalBackspace(
                        &global_terminal
                    );

                    break;
                }

                case '\r':
                {
                    Floppy144TerminalSubmitInput(
                        &global_terminal,
                        &global_world
                    );

                    if(global_terminal.exit_requested)
                    {
                        global_screen =
                            FLOPPY144_SCREEN_OFFICE;
                    }
                    else if(global_terminal.open_record_requested)
                    {
                        if(
                            Floppy144CatalogueOpenRecord(
                                &global_catalogue,
                                global_terminal.requested_collection,
                                global_terminal.requested_record_index
                            )
                        )
                        {
                            Floppy144DocumentApplyEffects(
                                &global_world,
                                global_terminal.requested_collection,
                                global_terminal.requested_record_index
                            );

                            global_catalogue_direct_document =
                                true;

                            global_screen =
                                FLOPPY144_SCREEN_CATALOGUE;
                        }

                        global_terminal.open_record_requested =
                            false;
                    }

                    break;
                }

                default:
                {
                    if(
                        w_param >= 32U &&
                        w_param <= 126U
                    )
                    {
                        Floppy144TerminalInputCharacter(
                            &global_terminal,
                            (char)w_param
                        );
                    }

                    break;
                }
            }

            Floppy144Redraw(
                window
            );

            return 0;
        }
        case WM_KEYDOWN:
        {
            switch(global_screen)
            {
                /* Recovery: Enter starts reconstruction, then enters the office. Escape quits. */
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

                            Floppy144Redraw(window);
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

                /* Office: movement uses eight-pixel steps; E interacts; Escape returns to recovery. */
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

                        /* Resolve and execute the registry-declared nearby interaction. */
                        case 'E':
                        {
                            Floppy144InteractOffice(
                                window
                            );

                            return 0;
                        }
                        case VK_ESCAPE:
                        {
                            global_screen =
                                FLOPPY144_SCREEN_RECOVERY;

                            Floppy144Redraw(window);
                            return 0;
                        }
                    }

                    break;
                }

                /* Terminal: printable input arrives through WM_CHAR. */
                case FLOPPY144_SCREEN_TERMINAL:
                {
                    if(w_param == VK_ESCAPE)
                    {
                        global_screen =
                            FLOPPY144_SCREEN_OFFICE;

                        Floppy144Redraw(
                            window
                        );

                        return 0;
                    }

                    break;
                }
                /* Catalogue: move or page through records, open a document, or back out. */
                case FLOPPY144_SCREEN_CATALOGUE:
                {
                    switch(w_param)
                    {
                        case VK_UP:
                        case 'W':
                        {
                            Floppy144CatalogueMove(
                                &global_catalogue,
                                -1
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_DOWN:
                        case 'S':
                        {
                            Floppy144CatalogueMove(
                                &global_catalogue,
                                1
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_PRIOR:
                        {
                            Floppy144CataloguePage(
                                &global_catalogue,
                                -1
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_NEXT:
                        {
                            Floppy144CataloguePage(
                                &global_catalogue,
                                1
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_RETURN:
                        {
                            Floppy144CatalogueOpenDocument(
                                &global_catalogue
                            );

                            /*
                             * Authored records declare their own effects.
                             * Index-only records have no registered effects.
                             */
                            Floppy144DocumentApplyEffects(
                                &global_world,
                                global_catalogue.collection,
                                global_catalogue.selected_index
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_ESCAPE:
                        {
                            if(global_catalogue_direct_document)
                            {
                                if(
                                    Floppy144CatalogueDocumentOpen(
                                        &global_catalogue
                                    )
                                )
                                {
                                    Floppy144CatalogueCloseDocument(
                                        &global_catalogue
                                    );
                                }

                                global_catalogue_direct_document =
                                    false;

                                global_screen =
                                    FLOPPY144_SCREEN_TERMINAL;
                            }
                            else
                            {
                                switch(
                                    Floppy144CatalogueDocumentOpen(
                                        &global_catalogue
                                    )
                                )
                                {
                                    case true:
                                    {
                                        Floppy144CatalogueCloseDocument(
                                            &global_catalogue
                                        );

                                        break;
                                    }

                                    case false:
                                    {
                                        global_screen =
                                            FLOPPY144_SCREEN_TERMINAL;

                                        break;
                                    }
                                }
                            }

                            Floppy144Redraw(window);
                            return 0;
                        }
                    }

                    break;
                }
            }

            return 0;
        }

        /*
         * Window painting
         *
         * Suppress Windows background erasing to avoid flicker. WM_PAINT asks river2D
         * to scale and copy the logical backbuffer into the window.
         */

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

/*
 * Application entry point
 *
 * Creates the Win32 window, configures the 640x360 logical canvas inside a
 * 1280x720 window, initialises game state and runs the Windows message loop.
 */

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE previous_instance,
    LPSTR command_line,
    int show_command
)
{
    /*
     * Local Win32 and river2D objects
     *
     * All engine storage lives for the duration of WinMain. global_engine points to
     * this engine only while the application is running.
     */

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
        1280,
        720
    };

    MSG message = {0};

    (void)previous_instance;
    (void)command_line;

    /*
     * Configure river2D
     *
     * Static-canvas mode preserves a crisp 640x360 internal image while the window
     * is twice that size.
     */

    engine.instance = instance;
    engine.windowName = "Floppy//144";

    engine.config.renderer =
        RV_RENDERER_SOFTWARE;

    engine.config.choices =
        RV_CHOICE_STATIC_CANVAS_BIT;

    engine.config.window_width = 1280;
    engine.config.window_height = 720;
    engine.config.canvas_width = 640;
    engine.config.canvas_height = 360;

    Floppy144BindStaticRenderer(
        &engine
    );

    /*
     * Register and create the native Win32 window
     *
     * AdjustWindowRect expands the requested client area to include borders and the
     * title bar before CreateWindowExA is called.
     */

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

    /*
     * Initialise game state
     *
     * Every subsystem is reset before the renderer is asked to allocate its backbuffer.
     */

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
        &global_terminal,
        &global_world
    );

    Floppy144CatalogueReset(
        &global_catalogue,
        FLOPPY144_COLLECTION_XX01
    );

    /*
     * Initialise rendering and show the first frame
     *
     * A missing backbuffer is fatal because every screen draws directly into it.
     */

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

    /*
     * Standard Windows message loop
     *
     * GetMessage waits for input, TranslateMessage handles key translation and
     * DispatchMessage sends each event to Floppy144WindowProc.
     */

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

    /*
     * Shutdown
     *
     * Clear the callback-visible pointer, then let the renderer release its resources.
     */

    global_engine = 0;

    return engine.shutdown(
        &engine
    );
}
