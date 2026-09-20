/*
 * Floppy//144 - Win32 entry point and game coordinator
 *
 * Connects Floppy144 to Win32, owns the active screen and top-level session state,
 * routes keyboard input and asks the appropriate module to redraw.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_runtime.h"
#include "f144_win32_platform.h"

#include "floppy144_catalogue.h"
#include "floppy144_cabinet.h"
#include "floppy144_document.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_notebook_view.h"
#include "floppy144_recovery.h"
#include "floppy144_terminal.h"
#include "floppy144_world.h"
#include "floppy144_run_state.h"
#include "floppy144_persistence.h"
#include "floppy144_site_2d.h"
#include "floppy144_site_isometric.h"
#include "floppy144_site_object.h"
#include "floppy144_site_rooms.h"
#include "floppy144_site_view.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*
 * Top-level screen state
 *
 * Only one of the splash, main menu, office, terminal or catalogue is active.
 * This enum is the game's small screen-state machine.
 */

typedef enum Floppy144Screen
{
    FLOPPY144_SCREEN_SPLASH,
    FLOPPY144_SCREEN_MAIN_MENU,
    FLOPPY144_SCREEN_OFFICE,
    FLOPPY144_SCREEN_CABINET,
    FLOPPY144_SCREEN_NOTEBOOK,
    FLOPPY144_SCREEN_TERMINAL,
    FLOPPY144_SCREEN_CATALOGUE
} Floppy144Screen;

/*
 * Shared application state
 *
 * The Win32 callback cannot receive custom game arguments directly, so this
 * small prototype stores the runtime and screen states at file scope.
 */

static F144Runtime *global_runtime;

static Floppy144Screen global_screen;
static Floppy144TerminalState global_terminal;
static Floppy144CatalogueState global_catalogue;
static Floppy144CabinetState global_cabinet;
static Floppy144NotebookViewState global_notebook;
static Floppy144DiscoveryProfile global_profile;
static Floppy144Settings global_settings;
static Floppy144WorldState global_world;
static Floppy144RunState global_run_state;
static Floppy144RunState global_recorded_run_state;

static const char floppy144_manual_save_path[] = "floppy144_manual.sav";
static const char floppy144_autosave_path[] = "floppy144_auto.sav";
static const char floppy144_profile_path[] = "floppy144_profile.dat";
static const char floppy144_settings_path[] = "floppy144_settings.dat";

static bool global_recorded_session_available;
static bool global_recorded_session_is_autosave;

/*
 * Short-lived interface state
 *
 * The office notice points to static text shown after an inspection. Movement
 * clears it. session state controls menu availability and suspended-screen recovery.
 */

#define FLOPPY144_OFFICE_NOTICE_CAPACITY 160U

static char global_office_notice_buffer[FLOPPY144_OFFICE_NOTICE_CAPACITY];
static const char *global_office_notice;
static bool global_session_active;
static bool global_catalogue_direct_document;

static Floppy144MainMenuOption
    global_main_menu_option;

static Floppy144Screen
    global_resume_screen;

typedef enum Floppy144PersistenceWarning
{
    FLOPPY144_PERSISTENCE_WARNING_NONE =
    0U,

    FLOPPY144_PERSISTENCE_WARNING_SAVE =
    1U << 0,

    FLOPPY144_PERSISTENCE_WARNING_PROFILE =
    1U << 1,

    FLOPPY144_PERSISTENCE_WARNING_SETTINGS =
    1U << 2,

    FLOPPY144_PERSISTENCE_WARNING_AUTOSAVE =
    1U << 3
}
Floppy144PersistenceWarning;

static uint8_t global_persistence_warnings;

static DWORD global_splash_started_ticks;

#define FLOPPY144_SPLASH_TIMER_ID          144U
#define FLOPPY144_SPLASH_FRAME_MS           16U
#define FLOPPY144_SPLASH_ANIMATION_MS     3700U

#define FLOPPY144_AUTOSAVE_TIMER_ID        145U

static const char *Floppy144PersistenceWarningText(
    void
);

/*
 * Bind Floppy144's statically linked software renderer
 *
 * F144 uses a single Win32 software renderer. These assignments connect the
 * runtime directly to its platform implementation.
 */

static void Floppy144BindStaticRenderer(
    F144Runtime *runtime
)
{
    runtime->init = f144Win32Init;
    runtime->shutdown = f144Win32Shutdown;
    runtime->bltBuffer = f144Win32BltBuffer;
    runtime->loadText = f144Win32LoadText;
    runtime->compositeImage = f144Win32CompositeImage;
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
        case FLOPPY144_SCREEN_SPLASH:
        {
            DWORD elapsed_milliseconds =
                GetTickCount() -
                global_splash_started_ticks;

            Floppy144SplashDraw(
                global_runtime,
                (uint32_t)elapsed_milliseconds
            );

            break;
        }
        case FLOPPY144_SCREEN_MAIN_MENU:
        {
            Floppy144MainMenuDraw(
                global_runtime,
                global_main_menu_option,
                global_session_active,
                global_recorded_session_available,
                &global_run_state,
                &global_recorded_run_state,
                Floppy144PersistenceWarningText()
            );

            break;
        }

        case FLOPPY144_SCREEN_OFFICE:
        {
            /*
             * Projection is persistent run state. FM-23 changes it through the
             * same JSON SET_PROJECTION effect interpreter used by every trigger.
             */
            if(Floppy144RunStateIsIsometric(&global_run_state))
            {
                Floppy144SiteIsometricDraw(
                    global_runtime,
                    &global_run_state,
                    global_office_notice
                );
            }
            else
            {
                Floppy144Site2DDraw(
                    global_runtime,
                    &global_run_state,
                    global_office_notice
                );
            }

            break;
        }

        /* STAGE 3B.5 CABINET DRAW */
        case FLOPPY144_SCREEN_CABINET:
        {
            Floppy144CabinetDraw(
                global_runtime,
                &global_cabinet,
                &global_run_state
            );

            break;
        }

        case FLOPPY144_SCREEN_NOTEBOOK:
        {
            Floppy144NotebookViewDraw(
                global_runtime,
                &global_notebook,
                &global_run_state
            );

            break;
        }

        case FLOPPY144_SCREEN_TERMINAL:
        {
            Floppy144TerminalDraw(
                global_runtime,
                &global_terminal,
                &global_run_state
            );

            break;
        }

        case FLOPPY144_SCREEN_CATALOGUE:
        {
            Floppy144CatalogueDraw(
                global_runtime,
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

static bool Floppy144PersistenceFileExists(
    const char *path
)
{
    DWORD attributes;

    if(path == NULL)
    {
        return false;
    }

    attributes =
    GetFileAttributesA(path);

    return
    attributes != INVALID_FILE_ATTRIBUTES &&
    !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static const char *Floppy144PersistenceWarningText(
    void
)
{
    switch(global_persistence_warnings)
    {
        case FLOPPY144_PERSISTENCE_WARNING_SAVE:
        {
            return
            "RECORDED SESSION COULD NOT BE LOADED";
        }

        case FLOPPY144_PERSISTENCE_WARNING_PROFILE:
        {
            return
            "DISCOVERY PROFILE INVALID - NEW PROFILE IN USE";
        }

        case FLOPPY144_PERSISTENCE_WARNING_SETTINGS:
        {
            return
            "SETTINGS INVALID - DEFAULTS APPLIED";
        }

        case FLOPPY144_PERSISTENCE_WARNING_AUTOSAVE:
        {
            return
            "AUTOSAVE COULD NOT BE RECORDED";
        }

        case FLOPPY144_PERSISTENCE_WARNING_NONE:
        {
            return NULL;
        }

        default:
        {
            return
            "PERSISTENCE WARNING - MULTIPLE OPERATIONS FAILED";
        }
    }
}

static bool Floppy144RecordedSessionAvailable(
    void
)
{
    bool manual_exists;
    bool autosave_exists;

    manual_exists =
    Floppy144PersistenceFileExists(
        floppy144_manual_save_path
    );

    autosave_exists =
    Floppy144PersistenceFileExists(
        floppy144_autosave_path
    );

    /*
     * Refresh only the recorded-session warning. Other persistence warnings
     * describe independent profile/settings/autosave operations and must not
     * be disturbed by this availability check.
     */
    global_persistence_warnings &=
        (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_SAVE;

    Floppy144RunStateReset(
        &global_recorded_run_state
    );

    global_recorded_session_is_autosave =
    false;

    /*
     * An explicit player-recorded session always takes precedence over
     * the automatic safety copy. A corrupt manual save is not itself a
     * player-facing failure if the automatic safety copy can be loaded.
     */
    if(
        Floppy144PersistenceLoadRunState(
            floppy144_manual_save_path,
            &global_recorded_run_state
        )
    )
    {
        return true;
    }

    /*
     * If no usable manual checkpoint exists, fall back to the autosave.
     */
    Floppy144RunStateReset(
        &global_recorded_run_state
    );

    if(
        Floppy144PersistenceLoadRunState(
            floppy144_autosave_path,
            &global_recorded_run_state
        )
    )
    {
        global_recorded_session_is_autosave =
        true;

        return true;
    }

    /*
     * Only report a load warning when a recorded-session file really exists
     * and neither the manual checkpoint nor autosave can be recovered.
     */
    if(manual_exists || autosave_exists)
    {
        global_persistence_warnings |=
        FLOPPY144_PERSISTENCE_WARNING_SAVE;
    }

    return false;
}

static void Floppy144UpdateDiscoveryProfile(
    void
)
{
    if(global_session_active)
    {
        Floppy144DiscoveryProfileMergeRunState(
            &global_profile,
            &global_run_state
        );
    }

    if(global_profile.dirty != 0U)
    {
        Floppy144PersistenceSaveProfile(
            floppy144_profile_path,
            &global_profile
        );
    }
}

/*
 * Return to the GDR main menu.
 *
 * An active session remembers the exact screen that was suspended.
 */

static void Floppy144OpenMainMenu(
    HWND window
)
{
    if(
        global_session_active &&
        global_screen != FLOPPY144_SCREEN_SPLASH &&
        global_screen != FLOPPY144_SCREEN_MAIN_MENU
    )
    {
        global_resume_screen =
            global_screen;
    }

    global_screen =
        FLOPPY144_SCREEN_MAIN_MENU;

    global_main_menu_option =
        global_session_active
            ? FLOPPY144_MAIN_MENU_RETURN_TO_SITE
            : FLOPPY144_MAIN_MENU_INITIATE_SESSION;

    Floppy144Redraw(
        window
    );
}

/*
 * Move to the next available session-control option.
 */

static void Floppy144MainMenuMoveSelection(
    int32_t direction
)
{
    int32_t next_option;
    uint32_t attempts;

    if(direction == 0)
    {
        return;
    }

    next_option =
        (int32_t)global_main_menu_option;

    for(
        attempts = 0U;
        attempts <
            (uint32_t)FLOPPY144_MAIN_MENU_OPTION_COUNT;
        ++attempts
    )
    {
        next_option +=
            direction;

        if(next_option < 0)
        {
            next_option =
                (int32_t)
                    FLOPPY144_MAIN_MENU_OPTION_COUNT -
                1;
        }

        if(
            next_option >=
                (int32_t)
                    FLOPPY144_MAIN_MENU_OPTION_COUNT
        )
        {
            next_option =
                0;
        }

        if(
            Floppy144MainMenuOptionEnabled(
                (Floppy144MainMenuOption)next_option,
                global_session_active,
                global_recorded_session_available
            )
        )
        {
            global_main_menu_option =
                (Floppy144MainMenuOption)next_option;

            return;
        }
    }
}

/*
 * Execute the selected session-control option.
 */

static void Floppy144MainMenuActivate(
    HWND window
)
{
    switch(global_main_menu_option)
    {
        case FLOPPY144_MAIN_MENU_INITIATE_SESSION:
        {
            Floppy144UpdateDiscoveryProfile();

            Floppy144WorldReset(
                &global_world
            );

            {
                uint32_t recovery_seed =
                (uint32_t)GetTickCount();

                if(recovery_seed == 0U)
                {
                    recovery_seed =
                    1U;
                }

                Floppy144RunStateBegin(
                    &global_run_state,
                    recovery_seed
                );

                Floppy144DiscoveryProfileBeginRecovery(
                    &global_profile
                );

                Floppy144PersistenceSaveProfile(
                    floppy144_profile_path,
                    &global_profile
                );
            }

            Floppy144TerminalReset(
                &global_terminal,
                &global_world
            );

            Floppy144CatalogueReset(
                &global_catalogue,
                FLOPPY144_COLLECTION_DR01
            );

            Floppy144NotebookViewReset(
                &global_notebook
            );

            global_office_notice =
                NULL;

            global_catalogue_direct_document =
                false;

            global_session_active =
                true;

            global_resume_screen =
                FLOPPY144_SCREEN_TERMINAL;

            global_screen =
                FLOPPY144_SCREEN_TERMINAL;

            Floppy144Redraw(
                window
            );

            return;
        }

        case FLOPPY144_MAIN_MENU_RETURN_TO_SITE:
        {
            if(global_session_active)
            {
                global_screen =
                    global_resume_screen;

                Floppy144Redraw(
                    window
                );
            }

            return;
        }

        case FLOPPY144_MAIN_MENU_RECORD_SESSION:
        {
            Floppy144UpdateDiscoveryProfile();

            if(
                global_session_active &&
                Floppy144PersistenceSaveRunState(
                    floppy144_manual_save_path,
                    &global_run_state
                )
            )
            {
                global_recorded_session_available =
                true;

                global_recorded_session_is_autosave =
                false;

                global_recorded_run_state =
                global_run_state;

                global_persistence_warnings &=
                    (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_SAVE;
            }

            Floppy144Redraw(
                window
            );

            return;
        }

        case FLOPPY144_MAIN_MENU_REINSTATE_SESSION:
        {
            if(
                global_recorded_session_available &&
                Floppy144PersistenceLoadRunState(
                    global_recorded_session_is_autosave
                    ? floppy144_autosave_path
                    : floppy144_manual_save_path,
                    &global_run_state
                )
            )
            {
                Floppy144WorldHydrateFromRunState(
                    &global_world,
                    &global_run_state
                );

                Floppy144TerminalReset(
                    &global_terminal,
                    &global_world
                );

                Floppy144CatalogueReset(
                    &global_catalogue,
                    FLOPPY144_COLLECTION_DR01
                );

                Floppy144NotebookViewReset(
                    &global_notebook
                );

                global_office_notice =
                NULL;

                global_catalogue_direct_document =
                false;

                global_session_active =
                true;

                global_persistence_warnings &=
                    (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_SAVE;

                Floppy144UpdateDiscoveryProfile();

                global_resume_screen =
                FLOPPY144_SCREEN_TERMINAL;

                global_screen =
                FLOPPY144_SCREEN_TERMINAL;

                Floppy144Redraw(
                    window
                );

                return;
            }

            /*
             * A save which existed when the menu was opened may since have become
             * invalid or unavailable.
             */

            global_recorded_session_available =
                false;

            global_recorded_session_is_autosave =
                false;

            global_persistence_warnings |=
                FLOPPY144_PERSISTENCE_WARNING_SAVE;

            Floppy144Redraw(
                window
            );

            return;
        }

        case FLOPPY144_MAIN_MENU_TERMINATE:
        {
            PostMessageA(
                window,
                WM_CLOSE,
                0,
                0
            );

            return;
        }

        case FLOPPY144_MAIN_MENU_OPTION_COUNT:
        {
            return;
        }
    }
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
    int32_t world_movement_x;
    int32_t world_movement_y;

    global_office_notice =
    NULL;

    /*
     * Input is expressed in the same axes as canonical Site space.
     * The view facade is intentionally identity-mapped after the orientation
     * migration, so collision receives the same direction the player presses.
     */

    Floppy144SiteViewMovementToWorld(
        movement_x,
        movement_y,
        &world_movement_x,
        &world_movement_y
    );

    /*
     * OUTSIDE is a real connection endpoint, not another reconstructable room.
     * Crossing an unlocked exterior door therefore suspends Site exploration
     * and opens the existing recovery/session-control menu. The player is not
     * moved beyond the threshold, so RETURN TO SITE places them just inside
     * the same exit. Locked exterior doors continue through ordinary collision
     * and remain impassable.
     */
    if(
        Floppy144RunStateWouldExitSite(
            &global_run_state,
            world_movement_x,
            world_movement_y
        )
    )
    {
        Floppy144OpenMainMenu(
            window
        );

        return;
    }

    Floppy144RunStateMovePlayerSite(
        &global_run_state,
        world_movement_x,
        world_movement_y
    );

    Floppy144Redraw(
        window
    );
}

/*
 * Execute the action declared by the best eligible nearby object.
 */

typedef enum Floppy144OfficeInteractionMode
{
    FLOPPY144_OFFICE_INTERACTION_ACCESS,
    FLOPPY144_OFFICE_INTERACTION_INSPECT
}
Floppy144OfficeInteractionMode;

static void Floppy144OfficeSetItemNotice(
    const char *pszItemName,
    const char *pszSuffix
)
{
    if(pszItemName == NULL)
    {
        global_office_notice = NULL;
        return;
    }

    snprintf(
        global_office_notice_buffer,
        sizeof(global_office_notice_buffer),
        "%s%s",
        pszItemName,
        pszSuffix != NULL ? pszSuffix : ""
    );

    global_office_notice =
        global_office_notice_buffer;
}

/*
 * Execute a projection-neutral Site action.
 *
 * Access is currently the GDR-terminal action. Doors/openable furniture remain
 * for the later connection/access pass. Inspect resolves the nearest canonical
 * physical item through generated furniture/fixture geometry and then routes
 * any progression through the generic interaction engine.
 */
static void Floppy144InteractOffice(
    HWND window,
    Floppy144OfficeInteractionMode eMode
)
{
    if(eMode == FLOPPY144_OFFICE_INTERACTION_ACCESS)
    {
        Floppy144RoomId eTerminalRoom;

        if(
            !Floppy144SiteAccessTerminalRoom(
                &global_run_state,
                &eTerminalRoom
            )
        )
        {
            return;
        }

        global_office_notice = NULL;

        global_screen =
            FLOPPY144_SCREEN_TERMINAL;

        Floppy144TerminalResetAtRoom(
            &global_terminal,
            &global_world,
            eTerminalRoom
        );

        Floppy144Redraw(window);
        return;
    }

    if(eMode == FLOPPY144_OFFICE_INTERACTION_INSPECT)
    {
        Floppy144SiteInspectionTarget sTarget;
        Floppy144CabinetState sNearbyCabinet;

        /*
         * Inspect is not the route into a locked secure cabinet. If the player
         * presses I anyway, give an in-world access denial instead of implying
         * that the cabinet is merely empty. A=Access remains the keypad route.
         */
        Floppy144CabinetReset(&sNearbyCabinet);
        if(
            Floppy144CabinetOpenNearby(
                &sNearbyCabinet,
                &global_run_state
            ) &&
            !Floppy144CabinetUnlocked(
                &sNearbyCabinet,
                &global_run_state
            )
        )
        {
            (void)snprintf(
                global_office_notice_buffer,
                sizeof(global_office_notice_buffer),
                "UNAUTHORISED ACCESS - SECURE CABINET LOCKED."
            );
            global_office_notice = global_office_notice_buffer;
            Floppy144Redraw(window);
            return;
        }

        if(
            !Floppy144SiteResolveInspectionTarget(
                &global_run_state,
                &sTarget
            )
        )
        {
            return;
        }

        /* Ordinary physical context needs no persistent interaction bit. */
        if(
            sTarget.eInteraction ==
            FLOPPY144_INTERACTION_COUNT
        )
        {
            Floppy144OfficeSetItemNotice(
                sTarget.pszPhysicalItemName,
                "."
            );

            Floppy144Redraw(window);
            return;
        }

        if(sTarget.bInteractionCompleted)
        {
            Floppy144OfficeSetItemNotice(
                sTarget.pszPhysicalItemName,
                ": INSPECTION ALREADY RECORDED."
            );

            Floppy144Redraw(window);
            return;
        }

        if(!sTarget.bInteractionAvailable)
        {
            Floppy144OfficeSetItemNotice(
                sTarget.pszPhysicalItemName,
                ": NO ACTIONABLE FINDING YET."
            );

            Floppy144Redraw(window);
            return;
        }

        if(
            Floppy144InteractionTryRun(
                &global_world,
                &global_run_state,
                sTarget.eInteraction
            )
        )
        {
            const Floppy144DataRecord *pInteraction =
                Floppy144InteractionRecord(
                    sTarget.eInteraction
                );

            bool bEvidenceRecorded = false;

            if(
                pInteraction != NULL &&
                pInteraction->pszE != NULL
            )
            {
                Floppy144EvidenceId eEvidence =
                    Floppy144GameDataEvidenceId(
                        pInteraction->pszE
                    );

                bEvidenceRecorded =
                    eEvidence != FLOPPY144_EVIDENCE_COUNT &&
                    Floppy144RunStateEvidenceEstablished(
                        &global_run_state,
                        eEvidence
                    );
            }

            Floppy144OfficeSetItemNotice(
                sTarget.pszPhysicalItemName,
                bEvidenceRecorded
                    ? ": EVIDENCE RECORDED."
                    : ": INSPECTION RECORDED."
            );
        }
        else
        {
            Floppy144OfficeSetItemNotice(
                sTarget.pszPhysicalItemName,
                ": INSPECTION COULD NOT BE RECORDED."
            );
        }

        Floppy144Redraw(window);
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
        /* Window lifetime: stop the runtime loop and post the process quit message. */
        case WM_CLOSE:
        {
            if(global_runtime)
            {
                global_runtime->running = false;
            }

            Floppy144UpdateDiscoveryProfile();

            KillTimer(
                window,
                FLOPPY144_AUTOSAVE_TIMER_ID
            );

            PostQuitMessage(0);
            return 0;
        }

        case WM_DESTROY:
        {
            if(global_runtime)
            {
                global_runtime->running = false;
            }

            KillTimer(
                window,
                FLOPPY144_AUTOSAVE_TIMER_ID
            );

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
            /* STAGE 3B.5 CABINET CHARACTER INPUT */
            if(global_screen == FLOPPY144_SCREEN_CABINET)
            {
                if(w_param >= '0' && w_param <= '9')
                {
                    (void)Floppy144CabinetInputDigit(
                        &global_cabinet,
                        (char)w_param
                    );
                }

                Floppy144Redraw(window);
                return 0;
            }

            if(
                global_screen !=
                FLOPPY144_SCREEN_TERMINAL
            )
            {
                break;
            }

            /*
             * Consume the character generated by the key that opened the
             * terminal. This must happen before command input is interpreted,
             * rather than discarding the player's next printable character.
             */

            if(global_terminal.suppress_next_character)
            {
                global_terminal.suppress_next_character =
                    false;

                return 0;
            }

            /*
             * Built-in operating guidance owns terminal character input while open.
             */
            if(
                Floppy144TerminalHelpPagerActive(
                    &global_terminal
                )
            )
            {
                switch(w_param)
                {
                    case ' ':
                    case '\r':
                    {
                        Floppy144TerminalMoveHelpPager(
                            &global_terminal,
                            1
                        );

                        break;
                    }

                    case '\b':
                    {
                        Floppy144TerminalMoveHelpPager(
                            &global_terminal,
                            -1
                        );

                        break;
                    }

                    case 'q':
                    case 'Q':
                    {
                        Floppy144TerminalCloseHelpPager(
                            &global_terminal
                        );

                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                Floppy144Redraw(
                    window
                );

                return 0;
            }

            /*
             * The record pager temporarily owns terminal character input.
             * Escape remains handled by WM_KEYDOWN and opens session control.
             */

            if(
                Floppy144TerminalRecordPagerActive(
                    &global_terminal
                )
            )
            {
                switch(w_param)
                {
                    case ' ':
                    case '\r':
                    {
                        Floppy144TerminalMoveRecordPager(
                            &global_terminal,
                            1
                        );

                        break;
                    }

                    case '\b':
                    {
                        Floppy144TerminalMoveRecordPager(
                            &global_terminal,
                            -1
                        );

                        break;
                    }

                    case 'q':
                    case 'Q':
                    {
                        Floppy144TerminalCloseRecordPager(
                            &global_terminal
                        );

                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                Floppy144Redraw(
                    window
                );

                return 0;
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
                        &global_world,
                        &global_run_state
                    );

                    if(global_terminal.exit_requested)
                    {
                        global_terminal.exit_requested =
                        false;

                        if(
                            Floppy144RunStateRoomReconstructed(
                                &global_run_state,
                                FLOPPY144_ROOM_RECEPTION
                            )
                        )
                        {
                            global_resume_screen =
                            FLOPPY144_SCREEN_OFFICE;

                            global_screen =
                            FLOPPY144_SCREEN_OFFICE;
                        }
                        else
                        {
                            Floppy144OpenMainMenu(
                                window
                            );

                            return 0;
                        }
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
                                &global_run_state,
                                global_terminal.requested_collection,
                                global_terminal.requested_record_index
                            );

                            /*
                             * Refresh the objective from resulting persistent
                             * state. It becomes visible when the player returns
                             * from the document to the terminal.
                             */
                            Floppy144TerminalPrintNextAction(
                                &global_terminal,
                                &global_run_state
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
        /*
         * Splash animation timer
         *
         * Elapsed time, rather than frame count, controls movement. The timer
         * stops once the disk has settled.
         */

        case WM_TIMER:
        {
            if(
                w_param ==
                    FLOPPY144_SPLASH_TIMER_ID &&
                global_screen ==
                    FLOPPY144_SCREEN_SPLASH
            )
            {
                DWORD elapsed_milliseconds =
                    GetTickCount() -
                    global_splash_started_ticks;

                Floppy144Redraw(
                    window
                );

                if(
                    elapsed_milliseconds >=
                        FLOPPY144_SPLASH_ANIMATION_MS
                )
                {
                    KillTimer(
                        window,
                        FLOPPY144_SPLASH_TIMER_ID
                    );
                }

                return 0;
            }

            if(w_param == FLOPPY144_AUTOSAVE_TIMER_ID)
            {
                if(
                    global_session_active &&
                    global_run_state.dirty != 0U
                )
                {
                    Floppy144UpdateDiscoveryProfile();

                    if(
                        Floppy144PersistenceSaveRunState(
                            floppy144_autosave_path,
                            &global_run_state
                        )
                    )
                    {
                        global_persistence_warnings &=
                        (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_AUTOSAVE;
                    }
                    else
                    {
                        global_persistence_warnings |=
                        FLOPPY144_PERSISTENCE_WARNING_AUTOSAVE;

                        Floppy144Redraw(
                            window
                        );
                    }
                }

                return 0;
            }

            break;
        }
        case WM_KEYDOWN:
        {
            /*
             * Escape never terminates the application.
             *
             * From every non-menu screen it suspends the current view and
             * returns to GDR session control. On the menu it has no effect.
             */

            if(w_param == VK_ESCAPE)
            {
                if(global_screen == FLOPPY144_SCREEN_SPLASH)
                {
                    KillTimer(
                        window,
                        FLOPPY144_SPLASH_TIMER_ID
                    );
                }

                if(global_screen != FLOPPY144_SCREEN_MAIN_MENU)
                {
                    Floppy144OpenMainMenu(
                        window
                    );
                }

                return 0;
            }

            switch(global_screen)
            {
                /*
                 * Splash: Enter advances to session control.
                 * Escape is handled by the universal menu route.
                 */

                case FLOPPY144_SCREEN_SPLASH:
                {
                    if(w_param == VK_RETURN)
                    {
                        KillTimer(
                            window,
                            FLOPPY144_SPLASH_TIMER_ID
                        );

                        Floppy144OpenMainMenu(
                            window
                        );

                        return 0;
                    }

                    break;
                }
                /*
                 * GDR main menu: move through available options and execute
                 * the highlighted administrative action.
                 */

                case FLOPPY144_SCREEN_MAIN_MENU:
                {
                    switch(w_param)
                    {
                        case VK_UP:
                        case 'W':
                        {
                            Floppy144MainMenuMoveSelection(
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
                            Floppy144MainMenuMoveSelection(
                                1
                            );

                            Floppy144Redraw(
                                window
                            );

                            return 0;
                        }

                        case VK_RETURN:
                        {
                            Floppy144MainMenuActivate(
                                window
                            );

                            return 0;
                        }
                    }

                    break;
                }

                /*
                 * Site: arrow keys move in 0.5-unit fixed-point steps.
                 *
                 * A is reserved for access actions such as terminals. I is
                 * reserved for inspecting physical objects and evidence. The
                 * previous WASD aliases are intentionally removed so A has one
                 * unambiguous meaning while the player is in the Site.
                 */
                case FLOPPY144_SCREEN_OFFICE:
                {
                    switch(w_param)
                    {
                        case VK_LEFT:
                        {
                            Floppy144MovePlayer(
                                window,
                                -8,
                                0
                            );

                            return 0;
                        }

                        case VK_RIGHT:
                        {
                            Floppy144MovePlayer(
                                window,
                                8,
                                0
                            );

                            return 0;
                        }

                        case VK_UP:
                        {
                            Floppy144MovePlayer(
                                window,
                                0,
                                -8
                            );

                            return 0;
                        }

                        case VK_DOWN:
                        {
                            Floppy144MovePlayer(
                                window,
                                0,
                                8
                            );

                            return 0;
                        }

                        case 'A':
                        {
                            /* STAGE 3B.5 CABINET ACCESS KEY */
                            {
                                if(
                                    (
                                        Floppy144SiteAvailableActions(&global_run_state) &
                                        FLOPPY144_SITE_ACTION_ACCESS
                                    ) == 0U &&
                                    Floppy144CabinetOpenNearby(
                                        &global_cabinet,
                                        &global_run_state
                                    )
                                )
                                {
                                    global_office_notice = NULL;
                                    global_resume_screen = FLOPPY144_SCREEN_CABINET;
                                    global_screen = FLOPPY144_SCREEN_CABINET;
                                    Floppy144Redraw(window);
                                    return 0;
                                }
                            }

                            Floppy144InteractOffice(
                                window,
                                FLOPPY144_OFFICE_INTERACTION_ACCESS
                            );

                            return 0;
                        }

                        case 'I':
                        {
                            Floppy144InteractOffice(
                                window,
                                FLOPPY144_OFFICE_INTERACTION_INSPECT
                            );

                            return 0;
                        }

                        case 'N':
                        {
                            global_screen =
                                FLOPPY144_SCREEN_NOTEBOOK;

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_ESCAPE:
                        {
                            global_screen =
                                FLOPPY144_SCREEN_MAIN_MENU;

                            Floppy144Redraw(window);
                            return 0;
                        }
                    }

                    break;
                }

                /*
                 * Notebook: browse recovered persistent knowledge. Up/Down
                 * moves one entry, Page Up/Page Down jumps five entries, and
                 * N or Backspace returns to Site exploration.
                 */
                /* STAGE 3B.5 CABINET KEY ROUTING
                 *
                 * Keypad and Interior share one reusable screen state.
                 * Escape remains the universal Session Control route.
                 */
                case FLOPPY144_SCREEN_CABINET:
                {
                    switch(w_param)
                    {
                        case VK_UP:
                        {
                            Floppy144CabinetMoveSelection(
                                &global_cabinet,
                                &global_run_state,
                                -1
                            );
                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_DOWN:
                        {
                            Floppy144CabinetMoveSelection(
                                &global_cabinet,
                                &global_run_state,
                                1
                            );
                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_RETURN:
                        {
                            if(Floppy144CabinetInteriorOpen(&global_cabinet))
                            {
                                (void)Floppy144CabinetInspectSelected(
                                    &global_cabinet,
                                    &global_world,
                                    &global_run_state
                                );
                            }
                            else
                            {
                                (void)Floppy144CabinetSubmitCode(
                                    &global_cabinet,
                                    &global_world,
                                    &global_run_state
                                );
                            }

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case 'I':
                        {
                            if(Floppy144CabinetInteriorOpen(&global_cabinet))
                            {
                                (void)Floppy144CabinetInspectSelected(
                                    &global_cabinet,
                                    &global_world,
                                    &global_run_state
                                );
                                Floppy144Redraw(window);
                            }
                            return 0;
                        }

                        case VK_BACK:
                        {
                            if(!Floppy144CabinetBackspace(&global_cabinet))
                            {
                                global_resume_screen = FLOPPY144_SCREEN_OFFICE;
                                global_screen = FLOPPY144_SCREEN_OFFICE;
                            }

                            Floppy144Redraw(window);
                            return 0;
                        }
                    }

                    break;
                }

                case FLOPPY144_SCREEN_NOTEBOOK:
                {
                    switch(w_param)
                    {
                        case VK_UP:
                        {
                            Floppy144NotebookViewMove(
                                &global_notebook,
                                &global_run_state,
                                -1
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_DOWN:
                        {
                            Floppy144NotebookViewMove(
                                &global_notebook,
                                &global_run_state,
                                1
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_PRIOR:
                        {
                            Floppy144NotebookViewMove(
                                &global_notebook,
                                &global_run_state,
                                -12
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case VK_NEXT:
                        {
                            Floppy144NotebookViewMove(
                                &global_notebook,
                                &global_run_state,
                                12
                            );

                            Floppy144Redraw(window);
                            return 0;
                        }

                        case 'N':
                        case VK_BACK:
                        {
                            global_screen =
                                FLOPPY144_SCREEN_OFFICE;

                            Floppy144Redraw(window);
                            return 0;
                        }
                    }

                    break;
                }

                /*
                 * Terminal: printable input arrives through WM_CHAR. Arrow
                 * keys are reserved for session-local command history while
                 * the help/record pagers are not active.
                 */
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

                    if(
                        !Floppy144TerminalHelpPagerActive(
                            &global_terminal
                        ) &&
                        !Floppy144TerminalRecordPagerActive(
                            &global_terminal
                        )
                    )
                    {
                        switch(w_param)
                        {
                            case VK_UP:
                            {
                                Floppy144TerminalMoveHistory(
                                    &global_terminal,
                                    -1
                                );

                                Floppy144Redraw(window);
                                return 0;
                            }

                            case VK_DOWN:
                            {
                                Floppy144TerminalMoveHistory(
                                    &global_terminal,
                                    1
                                );

                                Floppy144Redraw(window);
                                return 0;
                            }
                        }
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
                            /*
                             * Enter opens a record only from catalogue-list view.
                             *
                             * Once a document is already open, further Enter
                             * presses must not re-apply its effects or append the
                             * same data-derived next-action guidance again.
                             */
                            if(
                                !Floppy144CatalogueDocumentOpen(
                                    &global_catalogue
                                )
                            )
                            {
                                /*
                                 * Do not let the graphical catalogue bypass a
                                 * trigger gate which would defer the same record
                                 * through terminal OPEN.
                                 */
                                if(
                                    !Floppy144DocumentAccessible(
                                        &global_run_state,
                                        global_catalogue.collection,
                                        global_catalogue.selected_index
                                    )
                                )
                                {
                                    Floppy144Redraw(window);
                                    return 0;
                                }

                                Floppy144CatalogueOpenDocument(
                                    &global_catalogue
                                );

                                /*
                                 * Authored records declare their own effects.
                                 * Index-only records have no registered effects.
                                 */
                                Floppy144DocumentApplyEffects(
                                    &global_world,
                                    &global_run_state,
                                    global_catalogue.collection,
                                    global_catalogue.selected_index
                                );

                                Floppy144TerminalPrintNextAction(
                                    &global_terminal,
                                    &global_run_state
                                );
                            }

                            Floppy144Redraw(window);
                            return 0;
                        }

                        /*
                         * Backspace returns through the archive-view hierarchy.
                         *
                         * Direct OPEN commands return straight to the terminal.
                         * Catalogue documents return to their record list first.
                         */

                        case VK_BACK:
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
         * Suppress Windows background erasing to avoid flicker. WM_PAINT asks Floppy144
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
                global_runtime &&
                global_runtime->bltBuffer &&
                global_runtime->backbuffer.data
            )
            {
                global_runtime->context =
                    device_context;

                global_runtime->bltBuffer(
                    global_runtime
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
     * Local Win32 and Floppy144 objects
     *
     * All runtime storage lives for the duration of WinMain. global_runtime points to
     * this runtime only while the application is running.
     */

    const char *class_name =
        "Floppy144WindowClass";

    F144Runtime runtime = {0};

    F144Image planes[
        F144_MAX_PLANES
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
     * Configure Floppy144
     *
     * Static-canvas mode preserves a crisp 640x360 internal image while the window
     * is twice that size.
     */

    runtime.instance = instance;
    runtime.windowName = "Floppy//144";

    runtime.config.static_canvas = 1;

    runtime.config.window_width = 1280;
    runtime.config.window_height = 720;
    runtime.config.canvas_width = 640;
    runtime.config.canvas_height = 360;

    Floppy144BindStaticRenderer(
        &runtime
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

    runtime.window = CreateWindowExA(
        0,
        class_name,
        runtime.windowName,
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

    if(!runtime.window)
    {
        return 2;
    }

    /*
     * Initialise game state
     *
     * Every subsystem is reset before the renderer is asked to allocate its backbuffer.
     */

    global_runtime = &runtime;

    global_screen =
        FLOPPY144_SCREEN_SPLASH;

    global_splash_started_ticks =
        0U;

    global_persistence_warnings =
        FLOPPY144_PERSISTENCE_WARNING_NONE;

    global_recorded_session_available =
        Floppy144RecordedSessionAvailable();

    global_main_menu_option =
        FLOPPY144_MAIN_MENU_INITIATE_SESSION;

    global_resume_screen =
        FLOPPY144_SCREEN_TERMINAL;

    Floppy144WorldReset(
        &global_world
    );

    Floppy144RunStateReset(
        &global_run_state
    );

    {
        bool profile_file_exists =
        Floppy144PersistenceFileExists(
            floppy144_profile_path
        );

        if(
            !Floppy144PersistenceLoadProfile(
                floppy144_profile_path,
                &global_profile
            )
        )
        {
            Floppy144DiscoveryProfileReset(
                &global_profile
            );

            if(profile_file_exists)
            {
                global_persistence_warnings |=
                FLOPPY144_PERSISTENCE_WARNING_PROFILE;
            }
        }
    }

    {
        bool settings_file_exists =
        Floppy144PersistenceFileExists(
            floppy144_settings_path
        );

        if(
            !Floppy144PersistenceLoadSettings(
                floppy144_settings_path,
                &global_settings
            )
        )
        {
            Floppy144SettingsReset(
                &global_settings
            );

            if(settings_file_exists)
            {
                global_persistence_warnings |=
                FLOPPY144_PERSISTENCE_WARNING_SETTINGS;
            }

            Floppy144PersistenceSaveSettings(
                floppy144_settings_path,
                &global_settings
            );
        }
    }

    Floppy144TerminalReset(
        &global_terminal,
        &global_world
    );

    Floppy144CatalogueReset(
        &global_catalogue,
        FLOPPY144_COLLECTION_DR01
    );

    Floppy144NotebookViewReset(
        &global_notebook
    );

    /*
     * Initialise rendering and show the first frame
     *
     * A missing backbuffer is fatal because every screen draws directly into it.
     */

    runtime.init(
        &runtime,
        planes
    );

    if(!runtime.backbuffer.data)
    {
        MessageBoxA(
            runtime.window,
            "Floppy144 could not create the software framebuffer.",
            "Floppy//144",
            MB_OK | MB_ICONERROR
        );

        DestroyWindow(
            runtime.window
        );

        return 3;
    }

    Floppy144SplashDraw(
        &runtime,
        0U
    );

    ShowWindow(
        runtime.window,
        show_command
    );

    global_splash_started_ticks =
        GetTickCount();

    Floppy144SplashDraw(
        &runtime,
        0U
    );

    SetTimer(
        runtime.window,
        FLOPPY144_SPLASH_TIMER_ID,
        FLOPPY144_SPLASH_FRAME_MS,
        NULL
    );

    {
        uint32_t autosave_interval =
        Floppy144SettingsAutosaveIntervalMs(
            &global_settings
        );

        if(autosave_interval != 0U)
        {
            SetTimer(
                runtime.window,
                FLOPPY144_AUTOSAVE_TIMER_ID,
                (UINT)autosave_interval,
                     NULL
            );
        }
    }

    UpdateWindow(
        runtime.window
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

    global_runtime = 0;

    return runtime.shutdown(
        &runtime
    );
}
