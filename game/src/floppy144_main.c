/*
 * Floppy//144 - Win32 entry point and game coordinator
 *
 * Connects Floppy144 to Win32, owns the active screen and top-level session state,
 * routes keyboard input and asks the appropriate module to redraw.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_startup_config.h"
#include "f144_platform.h"
#include "f144_runtime.h"
#include "f144_win32_startup_config.h"
#include "f144_win32_input.h"
#include "f144_win32_lifecycle.h"
#include "f144_win32_platform.h"
#include "f144_win32_single_instance.h"
#include "f144_win32_timing.h"
#include "floppy144_resource.h"

#include "floppy144_catalogue.h"
#include "floppy144_cabinet.h"
#include "floppy144_document.h"
#include "floppy144_draw.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_input.h"
#include "floppy144_lifecycle.h"
#include "floppy144_notebook_view.h"
#include "floppy144_recovery.h"
#include "floppy144_intro.h"
#include "floppy144_audio.h"
#include "floppy144_restore_presentation.h"
#include "floppy144_player_visual.h"
#include "floppy144_profile_edit.h"
#include "floppy144_profile_view.h"
#include "floppy144_terminal.h"
#include "floppy144_timing.h"
#include "floppy144_world.h"
#include "floppy144_run_state.h"
#include "floppy144_persistence.h"
#include "floppy144_storage.h"
#include "floppy144_site_2d.h"
#include "floppy144_site_2d_camera.h"
#include "floppy144_site_directory.h"
#include "floppy144_site_isometric.h"
#include "floppy144_site_object.h"
#include "floppy144_grey_door.h"
#include "floppy144_grey_encounter.h"
#include "floppy144_site_rooms.h"
#include "floppy144_site_view.h"
#include "floppy144_settings_runtime.h"
#include "floppy144_settings_view.h"
#include "floppy144_credits_view.h"
#include "floppy144_completion_view.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*
 * Top-level screen state
 *
 * Only one top-level presentation screen is active at a time. Profile is a
 * child of Session Control rather than a resumable recovery-session screen.
 * This enum is the game's small screen-state machine.
 */

typedef enum Floppy144Screen
{
    FLOPPY144_SCREEN_SPLASH,
    FLOPPY144_SCREEN_MAIN_MENU,
    FLOPPY144_SCREEN_PROFILE,
    FLOPPY144_SCREEN_SETTINGS,
    FLOPPY144_SCREEN_CREDITS,
    FLOPPY144_SCREEN_OFFICE,
    FLOPPY144_SCREEN_GREY_ENCOUNTER,
    FLOPPY144_SCREEN_SITE_DIRECTORY,
    FLOPPY144_SCREEN_CABINET,
    FLOPPY144_SCREEN_NOTEBOOK,
    FLOPPY144_SCREEN_COMPLETION,
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
static F144Platform global_platform;
static Floppy144MovementInput global_movement_input;
static Floppy144PlayerVisualState global_player_visual;
static Floppy144LifecycleState global_lifecycle;
static Floppy144TimingState global_timing;

static Floppy144Screen global_screen;
static Floppy144TerminalState global_terminal;
static Floppy144CatalogueState global_catalogue;
static Floppy144CabinetState global_cabinet;
static Floppy144NotebookViewState global_notebook;
static Floppy144DiscoveryProfile global_profile;
static Floppy144ProfileNameEditState global_profile_name_edit;
static Floppy144Settings global_settings;
static Floppy144AudioMusic global_audio_cue;
static uint8_t global_intro_sound_stage;
static char global_intro_login[FLOPPY144_PROFILE_NAME_CAPACITY];
static uint32_t global_intro_login_length;
static bool global_intro_login_save_failed;
static uint64_t global_last_footstep;
static uint64_t global_last_movement_ms;
static bool global_footstep_pending;
static uint32_t global_known_evidence[FLOPPY144_RUN_WORD_COUNT(FLOPPY144_EVIDENCE_COUNT)];
static bool global_audio_evidence_snapshot_valid;
static uint32_t global_audio_evidence_seed;
static Floppy144WorldState global_world;
static Floppy144RunState global_run_state;
static Floppy144GreyEncounter global_grey_encounter;
static Floppy144RunState global_recorded_run_state;

static Floppy144StoragePaths global_storage_paths;

static bool global_recorded_session_available;
static bool global_recorded_session_is_autosave;
static F144StartupConfig global_config;

/*
 * One writing boundary for every ordinary profile/settings/run save in the
 * Win32 coordinator. Hathaway is entirely disposable and must not modify
 * a real recovery, profile or Settings file. No game-core module parses
 * command-line text, and the canonical save codecs remain unchanged.
 */
static bool Floppy144HathawayMode(void)
{
    return f144StartupConfigVisualInspectionEnabled(&global_config);
}
static bool Floppy144InspectionSaveRunState(
    const char *path,Floppy144RunState *state
)
{
    return !Floppy144HathawayMode() &&
        Floppy144PersistenceSaveRunState(path,state);
}
static bool Floppy144InspectionSaveProfile(
    const char *path,Floppy144DiscoveryProfile *profile
)
{
    return Floppy144HathawayMode() ? true :
        Floppy144PersistenceSaveProfile(path,profile);
}
static bool Floppy144InspectionSaveSettings(
    const char *path,Floppy144Settings *settings
)
{
    return Floppy144HathawayMode() ? true :
        Floppy144PersistenceSaveSettings(path,settings);
}
#define Floppy144PersistenceSaveRunState Floppy144InspectionSaveRunState
#define Floppy144PersistenceSaveProfile Floppy144InspectionSaveProfile
#define Floppy144PersistenceSaveSettings Floppy144InspectionSaveSettings

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
static bool global_terminal_authentication_complete;
static bool global_catalogue_direct_document;
static const char *global_main_menu_notice;
static bool global_main_menu_notice_is_warning;
static bool global_reinstate_confirmation_pending;
static bool global_reinstate_continue_on_keyup;
static uint32_t global_reinstate_continue_key;
static Floppy144CompletionViewState global_completion_view;
static bool global_completion_evidence_resolved;
static bool global_completion_capacity_exhausted;
static Floppy144Screen global_credits_return_screen;

static Floppy144MainMenuOption
    global_main_menu_option;

static Floppy144SettingsOption
    global_settings_option;

static const char *
    global_settings_notice;

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

static const char *Floppy144PersistenceWarningText(
    void
);

static void Floppy144HandleLifecycleEvent(
    HWND window,
    const F144LifecycleEvent *event
);

static void Floppy144OpenMainMenu(
    HWND window
);

static void Floppy144UpdateTiming(
    HWND window
);

/* Gameplay communicates sound only through the semantic, platform-neutral
 * façade. Music is not restarted on screen transitions within an Act.
 */
static void Floppy144UpdateAudio(uint64_t now)
{
    Floppy144AudioMusic desired=FLOPPY144_AUDIO_MUSIC_NONE;
    uint32_t recovery=global_session_active
        ? Floppy144RunStateRecoveredPercent(&global_run_state):0U;
    uint32_t i;
    bool evidence_new=false;
    if(global_screen==FLOPPY144_SCREEN_COMPLETION ||
       (global_screen==FLOPPY144_SCREEN_CREDITS &&
        global_credits_return_screen==FLOPPY144_SCREEN_COMPLETION))
        desired=FLOPPY144_AUDIO_MUSIC_COMPLETION;
    else if(global_screen==FLOPPY144_SCREEN_MAIN_MENU ||
            global_screen==FLOPPY144_SCREEN_SETTINGS ||
            global_screen==FLOPPY144_SCREEN_PROFILE ||
            global_screen==FLOPPY144_SCREEN_CREDITS)
        desired=FLOPPY144_AUDIO_MUSIC_MENU;
    else if(global_screen!=FLOPPY144_SCREEN_SPLASH &&
            global_screen!=FLOPPY144_SCREEN_GREY_ENCOUNTER)
    {
        uint32_t act=global_session_active?
            (uint32_t)Floppy144RunStateAct(&global_run_state):0U;
        if(act>3U)act=3U;
        desired=(Floppy144AudioMusic)((uint32_t)FLOPPY144_AUDIO_MUSIC_PROLOGUE+act);
    }
    if(global_run_state.recovery_seed!=global_audio_evidence_seed) {
        global_audio_evidence_seed=global_run_state.recovery_seed;
        global_audio_evidence_snapshot_valid=false;
    }
    if(desired!=global_audio_cue) {
        Floppy144AudioMusicSet(&global_platform,desired);
        global_audio_cue=desired;
    }
    global_platform.music_tempo_percent=(uint8_t)(
        global_screen==FLOPPY144_SCREEN_TERMINAL &&
        Floppy144TerminalRestoreInProgress(&global_terminal)?
        Floppy144RestoreTempo(Floppy144TerminalRestoreProgressPercent(&global_terminal)):100U);
    Floppy144AudioTick(&global_platform,now,recovery);
    for(i=0U;i<FLOPPY144_RUN_WORD_COUNT(FLOPPY144_EVIDENCE_COUNT);i++) {
        uint32_t current=global_run_state.evidence[i];
        if(global_audio_evidence_snapshot_valid &&
           global_session_active && ((current & ~global_known_evidence[i])!=0U))
            evidence_new=true;
        global_known_evidence[i]=current;
    }
    global_audio_evidence_snapshot_valid=true;
    if(evidence_new)
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_EVIDENCE_FOUND);
}

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
    Floppy144Surface *pSurface =
        f144PlatformFramebuffer(&global_platform);

    if(pSurface == NULL)
    {
        return;
    }

    switch(global_screen)
    {
        case FLOPPY144_SCREEN_SPLASH:
        {
            uint32_t elapsed_milliseconds =
                Floppy144TimingSplashElapsedMs(
                    &global_timing,
                    f144PlatformMonotonicMs(
                        &global_platform
                    )
                );

            Floppy144IntroDrawWithLogin(
                pSurface,
                elapsed_milliseconds,
                Floppy144SettingsTextElapsedMs(
                    &global_settings,
                    1U
                ),
                global_intro_login,
                global_intro_login_save_failed
            );

            break;
        }
        case FLOPPY144_SCREEN_MAIN_MENU:
        {
            Floppy144MainMenuDraw(
                pSurface,
                global_main_menu_option,
                global_session_active,
                global_recorded_session_available,
                &global_run_state,
                &global_recorded_run_state,
                Floppy144PersistenceWarningText(),
                global_main_menu_notice,
                global_main_menu_notice_is_warning,
                global_reinstate_confirmation_pending
            );

            break;
        }

        case FLOPPY144_SCREEN_PROFILE:
        {
            Floppy144ProfileViewDraw(
                pSurface,
                &global_profile,
                &global_profile_name_edit
            );

            break;
        }

        case FLOPPY144_SCREEN_SETTINGS:
        {
            Floppy144SettingsViewDraw(
                pSurface,
                &global_settings,
                global_settings_option,
                global_settings_notice
            );

            break;
        }

        case FLOPPY144_SCREEN_CREDITS:
        {
            Floppy144CreditsViewDraw(
                pSurface,
                global_credits_return_screen !=
                    FLOPPY144_SCREEN_COMPLETION
            );

            break;
        }

        case FLOPPY144_SCREEN_OFFICE:
        {
            /*
             * Stage 3 ships the proven 2D exploration renderer. FM-23 now
             * enhances the Site Directory only; playable ISO room projection
             * is retained in source for a later release.
             */
            Floppy144Site2DDrawForPlayerStateWithIndicators(
                pSurface,
                &global_run_state,
                global_office_notice,
                Floppy144DiscoveryProfileBodyStyle(
                    &global_profile
                ),
                &global_player_visual,
                global_settings.show_inspection_indicators!=0U
            );

            break;
        }

        case FLOPPY144_SCREEN_GREY_ENCOUNTER:
        {
            Floppy144GreyEncounterDraw(pSurface,&global_grey_encounter);
            if(global_grey_encounter.phase!=(uint8_t)FLOPPY144_GREY_BLACKOUT &&
               global_grey_encounter.phase!=(uint8_t)FLOPPY144_GREY_DONE)
            {
                /* Exactly the same Site HUD implementation, with a strictly
                   ephemeral visual percentage. Nothing edits the RunState. */
                Floppy144Site2DDrawHeader(pSurface,&global_run_state,
                    Floppy144GreyEncounterDisplayPercent(&global_grey_encounter),
                    Floppy144GreyEncounterPercentFlash(&global_grey_encounter));
            }
            break;
        }

        case FLOPPY144_SCREEN_SITE_DIRECTORY:
        {
            Floppy144SiteDirectoryDraw(
                pSurface,
                &global_run_state
            );

            break;
        }

        /* STAGE 3B.5 CABINET DRAW */
        case FLOPPY144_SCREEN_CABINET:
        {
            Floppy144CabinetDraw(
                pSurface,
                &global_cabinet,
                &global_run_state
            );

            break;
        }

        case FLOPPY144_SCREEN_NOTEBOOK:
        {
            Floppy144NotebookViewDraw(
                pSurface,
                &global_notebook,
                &global_run_state
            );

            break;
        }

        case FLOPPY144_SCREEN_COMPLETION:
        {
            Floppy144CompletionViewDraw(
                pSurface,
                &global_completion_view,
                &global_run_state,
                &global_profile,
                global_completion_evidence_resolved,
                global_completion_capacity_exhausted
            );
            break;
        }

        case FLOPPY144_SCREEN_TERMINAL:
        {
            Floppy144TerminalDraw(
                pSurface,
                &global_terminal,
                &global_run_state
            );

            break;
        }

        case FLOPPY144_SCREEN_CATALOGUE:
        {
            Floppy144CatalogueDraw(
                pSurface,
                &global_catalogue
            );

            break;
        }
    }

    /* The impossible office still uses the same CRT presentation path.
       Never filter the complete black two-second return interval. */
    if(global_screen != FLOPPY144_SCREEN_GREY_ENCOUNTER ||
       (global_grey_encounter.phase!=(uint8_t)FLOPPY144_GREY_BLACKOUT &&
        global_grey_encounter.phase!=(uint8_t)FLOPPY144_GREY_ENTERING))
        Floppy144SettingsApplyCrtFilter(pSurface,&global_settings);

    InvalidateRect(
        window,
        0,
        FALSE
    );
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
    /* Never import normal saved progress into a synthetic Hathaway session. */
    if(Floppy144HathawayMode()) return false;
    bool manual_exists;
    bool autosave_exists;

    manual_exists =
    Floppy144PersistenceFileExists(
        Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_MANUAL_SAVE)
    );

    autosave_exists =
    Floppy144PersistenceFileExists(
        Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_AUTOSAVE)
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
            Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_MANUAL_SAVE),
            &global_recorded_run_state
        )
    )
    {
        /*
         * A successfully finished Grey Door event is deliberately irreversible
         * in this saved recovery. It is silently checkpointed to autosave on
         * exit, even if an older manually recorded session still exists.
         *
         * Preserve the standard manual-first rule for every other case and
         * do not transfer the hidden flag between different run seeds.
         */
        Floppy144RunState completed_autosave;
        if(
            Floppy144PersistenceLoadRunState(
                Floppy144StoragePath(
                    &global_storage_paths, F144_PERSISTENCE_AUTOSAVE
                ),
                &completed_autosave
            ) &&
            Floppy144RunStateGreyDoorCompletedAutosavePreferred(
                &global_recorded_run_state, &completed_autosave
            )
        )
        {
            global_recorded_run_state = completed_autosave;
            global_recorded_session_is_autosave = true;
        }

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
            Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_AUTOSAVE),
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
    if(global_session_active && !Floppy144HathawayMode())
    {
        Floppy144DiscoveryProfileMergeRunState(
            &global_profile,
            &global_run_state
        );
    }

    if(global_profile.dirty != 0U)
    {
        Floppy144PersistenceSaveProfile(
            Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_PROFILE),
            &global_profile
        );
    }
}

static void Floppy144AutosaveIfNeeded(
    HWND window
)
{
    if(
        !global_session_active ||
        global_run_state.dirty == 0U ||
        global_screen == FLOPPY144_SCREEN_GREY_ENCOUNTER ||
        Floppy144HathawayMode()
    )
    {
        return;
    }

    Floppy144UpdateDiscoveryProfile();

    if(
        Floppy144PersistenceSaveRunState(
            Floppy144StoragePath(
                &global_storage_paths,
                F144_PERSISTENCE_AUTOSAVE
            ),
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

static void Floppy144HandleLifecycleEvent(
    HWND window,
    const F144LifecycleEvent *event
)
{
    if(event == NULL)
    {
        return;
    }

    Floppy144LifecycleApply(
        &global_lifecycle,
        event
    );

    /*
     * Win32 never requests an autosave through lifecycle events today. The
     * neutral flag exists so a future mobile suspend/background adapter can
     * request the already-proven autosave path without learning persistence
     * internals.
     */
    if(event->request_autosave != 0U)
    {
        Floppy144AutosaveIfNeeded(
            window
        );
    }

    if(
        event->type ==
        F144_LIFECYCLE_SHUTDOWN_REQUESTED
    )
    {
        Floppy144UpdateDiscoveryProfile();

        f144PlatformQuit(
            &global_platform
        );
    }
}

static void Floppy144UpdateTiming(
    HWND window
)
{
    Floppy144TimingEvents events =
        Floppy144TimingAdvance(
            &global_timing,
            f144PlatformMonotonicMs(
                &global_platform
            )
        );

    bool redraw =
        false;

    Floppy144UpdateAudio(f144PlatformMonotonicMs(&global_platform));

    if(
        events.splash_frame_due != 0U &&
        global_screen == FLOPPY144_SCREEN_SPLASH
    )
    {
        uint32_t intro_elapsed_ms =
            Floppy144TimingSplashElapsedMs(
                &global_timing,
                f144PlatformMonotonicMs(
                    &global_platform
                )
            );

        if(intro_elapsed_ms>=FLOPPY144_INTRO_DISK_CLICK_MS &&
           global_intro_sound_stage<1U)
        {
            Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_FLOPPY_INSERT);
            global_intro_sound_stage=1U;
        }
        if(intro_elapsed_ms>=FLOPPY144_INTRO_CRT_START_MS &&
           global_intro_sound_stage<2U)
        {
            Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_CRT_WAKE);
            global_intro_sound_stage=2U;
        }
        if(intro_elapsed_ms>=FLOPPY144_INTRO_DURATION_MS &&
           global_intro_sound_stage<3U)
        {
            Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_ACCEPT);
            global_intro_sound_stage=3U;
        }

        redraw =
            true;
    }

    if(
        events.presentation_elapsed_ms != 0U &&
        global_screen == FLOPPY144_SCREEN_OFFICE &&
        Floppy144PlayerVisualAdvance(
            &global_player_visual,
            events.presentation_elapsed_ms
        )
    )
    {
        /* Match the shared gait contact; never sound a blocked/idle step. */
        if(global_footstep_pending &&
           f144PlatformMonotonicMs(&global_platform)<=global_last_movement_ms+FLOPPY144_PLAYER_WALK_FRAME_MS)
            Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_FOOTSTEP);
        global_footstep_pending=false;
        redraw =
            true;
    }

    if(global_screen == FLOPPY144_SCREEN_GREY_ENCOUNTER)
    {
        if(Floppy144GreyEncounterAdvance(
               &global_grey_encounter,events.presentation_elapsed_ms))
            redraw=true;

        if(Floppy144GreyEncounterFinished(&global_grey_encounter))
        {
            /*
             * All world state remained untouched inside the vignette.
             * Mark ONLY the one-shot bit, then return to the exact
             * original foot point and normal site renderer.
             */
            if(global_run_state.player_site_x ==
                   global_grey_encounter.return_x16 &&
               global_run_state.player_site_y ==
                   global_grey_encounter.return_y16)
            {
                if(!Floppy144HathawayMode() &&
                   Floppy144RunStateGreyDoorComplete(&global_run_state))
                {
                    /*
                     * Complete the one-shot transaction immediately, before
                     * returning to an ordinary save/reload entry point.
                     * Do not update the Profile, add a notebook note or show
                     * a player-facing unlock/completion message.
                     */
                    if(!Floppy144PersistenceSaveRunState(
                        Floppy144StoragePath(
                            &global_storage_paths, F144_PERSISTENCE_AUTOSAVE
                        ),
                        &global_run_state
                    ))
                    {
                        global_persistence_warnings |=
                            FLOPPY144_PERSISTENCE_WARNING_AUTOSAVE;
                    }
                    else
                    {
                        global_persistence_warnings &=
                            (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_AUTOSAVE;
                    }
                }
            }
            Floppy144MovementInputReset(&global_movement_input);
            (void)Floppy144PlayerVisualSetMovement(
                &global_player_visual,0,0,F144_ACTION_NONE);
            global_office_notice=NULL;
            global_screen=FLOPPY144_SCREEN_OFFICE;
            redraw=true;
        }
    }

    if(
        events.terminal_restore_elapsed_ms != 0U &&
        global_screen == FLOPPY144_SCREEN_TERMINAL &&
        Floppy144TerminalRestoreInProgress(
            &global_terminal
        )
    )
    {
        {
            uint32_t progress_before=Floppy144TerminalRestoreProgressPercent(&global_terminal);
            uint32_t recovered_before=Floppy144RunStateRecoveredKb(&global_run_state);
            Floppy144TerminalAdvanceRestore(
                &global_terminal,&global_world,&global_run_state,
                events.terminal_restore_elapsed_ms
            );
            if(Floppy144RunStateRecoveredKb(&global_run_state)>recovered_before)
                Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_FILE_RESTORE);
            else if(Floppy144TerminalRestoreInProgress(&global_terminal) &&
                    Floppy144TerminalRestoreProgressPercent(&global_terminal)>progress_before)
                Floppy144AudioPlay(&global_platform,
                    Floppy144TerminalRestoreProgressPercent(&global_terminal)>=97U?
                    FLOPPY144_AUDIO_RESTORE_TICK_HIGH:FLOPPY144_AUDIO_RESTORE_TICK);
        }

        redraw =
            true;
    }

    if(
        events.terminal_cursor_toggle != 0U &&
        global_screen == FLOPPY144_SCREEN_TERMINAL
    )
    {
        global_terminal.cursor_visible =
            !global_terminal.cursor_visible;

        redraw =
            true;
    }

    if(events.autosave_due != 0U)
    {
        Floppy144AutosaveIfNeeded(
            window
        );
    }

    if(redraw)
    {
        Floppy144Redraw(
            window
        );
    }
}

/*
 * Apply portable launch configuration and profile policy to the active
 * terminal session.
 */
static void Floppy144ConfigureTerminalSession(
    bool allow_authentication
)
{
    bool authenticate;

    Floppy144TerminalConfigureSession(
        &global_terminal,
        f144StartupConfigDebugEnabled(
            &global_config
        ),
        global_profile.recovery_sessions_begun <= 1U,
        true
    );

    Floppy144TerminalRefreshEnvironmentLine(
        &global_terminal,
        &global_run_state
    );

    if(
        Floppy144RunStateArchiveServicesInitialised(&global_run_state) &&
        !Floppy144RunStateRoomReconstructed(
            &global_run_state, FLOPPY144_ROOM_RECEPTION)
    )
    {
        Floppy144TerminalPrintNextAction(
            &global_terminal, &global_run_state);
    }

    if(!allow_authentication || Floppy144HathawayMode())
    {
        return;
    }

    authenticate =
        !global_terminal_authentication_complete;

    Floppy144TerminalApplyOperatorIdentity(
        &global_terminal,
        Floppy144DiscoveryProfileOperatorName(
            &global_profile
        ),
        authenticate
    );

    if(authenticate)
    {
        global_terminal_authentication_complete =
            true;
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
        global_screen != FLOPPY144_SCREEN_MAIN_MENU &&
        global_screen != FLOPPY144_SCREEN_PROFILE &&
        global_screen != FLOPPY144_SCREEN_SETTINGS &&
        global_screen != FLOPPY144_SCREEN_CREDITS
    )
    {
        global_resume_screen =
            global_screen;
    }

    global_main_menu_notice =
        NULL;

    global_main_menu_notice_is_warning =
        false;

    global_reinstate_confirmation_pending =
        false;

    global_reinstate_continue_on_keyup =
        false;

    global_reinstate_continue_key =
        0U;

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

    global_main_menu_notice =
        NULL;

    global_main_menu_notice_is_warning =
        false;

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
 * Open the persistent operator record.
 *
 * A fresh profile begins in the optional name-setup editor immediately. This
 * does not make identity mandatory for gameplay: Escape may cancel setup and
 * the player can return to Session Control with the profile still UNASSIGNED.
 */
static void Floppy144OpenOperatorProfile(
    HWND window
)
{
    Floppy144ProfileNameEditReset(
        &global_profile_name_edit
    );

    global_screen =
        FLOPPY144_SCREEN_PROFILE;

    if(
        !Floppy144DiscoveryProfileHasOperatorName(
            &global_profile
        )
    )
    {
        Floppy144ProfileNameEditBegin(
            &global_profile_name_edit,
            &global_profile,
            true
        );
    }

    Floppy144Redraw(
        window
    );
}

/*
 * Commit the transient operator-name edit atomically at profile level.
 *
 * The persistent profile is copied before mutation. If the profile file
 * cannot be replaced successfully, the in-memory profile is restored and the
 * player's edit buffer remains available for retry or cancellation.
 */
static void Floppy144CommitProfileNameEdit(
    HWND window
)
{
    Floppy144DiscoveryProfile original_profile;
    const char *edited_name;

    if(
        !Floppy144ProfileNameEditActive(
            &global_profile_name_edit
        )
    )
    {
        return;
    }

    edited_name =
        Floppy144ProfileNameEditText(
            &global_profile_name_edit
        );

    if(
        strcmp(
            edited_name,
            Floppy144DiscoveryProfileOperatorName(
                &global_profile
            )
        ) == 0
    )
    {
        Floppy144ProfileNameEditCancel(
            &global_profile_name_edit
        );

        Floppy144Redraw(
            window
        );

        return;
    }

    if(
        !Floppy144ProfileNameEditReadyToSave(
            &global_profile_name_edit
        )
    )
    {
        Floppy144Redraw(
            window
        );

        return;
    }

    original_profile =
        global_profile;

    if(
        !Floppy144DiscoveryProfileSetOperatorName(
            &global_profile,
            edited_name
        ) ||
        !Floppy144PersistenceSaveProfile(
            Floppy144StoragePath(
                &global_storage_paths,
                F144_PERSISTENCE_PROFILE
            ),
            &global_profile
        )
    )
    {
        global_profile =
            original_profile;

        global_persistence_warnings |=
            FLOPPY144_PERSISTENCE_WARNING_PROFILE;

        Floppy144ProfileNameEditMarkSaveFailed(
            &global_profile_name_edit
        );

        Floppy144Redraw(
            window
        );

        return;
    }

    global_persistence_warnings &=
        (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_PROFILE;

    Floppy144ProfileNameEditCancel(
        &global_profile_name_edit
    );

    Floppy144Redraw(
        window
    );
}

/*
 * Change the cosmetic body style and persist it immediately.
 *
 * This is profile-only identity state. No recovery/session data is touched,
 * and a failed file replacement restores the exact previous profile.
 */
static void Floppy144CommitProfileBodyStyle(
    HWND window,
    bool forward
)
{
    Floppy144DiscoveryProfile original_profile;
    Floppy144OperatorBodyStyle current_style;
    Floppy144OperatorBodyStyle next_style;
    uint32_t style_count;
    uint32_t next_index;

    style_count =
        (uint32_t)FLOPPY144_OPERATOR_BODY_STYLE_COUNT;

    if(style_count == 0U)
    {
        return;
    }

    current_style =
        Floppy144DiscoveryProfileBodyStyle(
            &global_profile
        );

    if(forward)
    {
        next_index =
            ((uint32_t)current_style + 1U) %
            style_count;
    }
    else
    {
        next_index =
            ((uint32_t)current_style +
             style_count - 1U) %
            style_count;
    }

    next_style =
        (Floppy144OperatorBodyStyle)next_index;

    original_profile =
        global_profile;

    if(
        !Floppy144DiscoveryProfileSetBodyStyle(
            &global_profile,
            next_style
        )
    )
    {
        Floppy144Redraw(
            window
        );

        return;
    }

    if(
        !Floppy144PersistenceSaveProfile(
            Floppy144StoragePath(
                &global_storage_paths,
                F144_PERSISTENCE_PROFILE
            ),
            &global_profile
        )
    )
    {
        global_profile =
            original_profile;

        global_persistence_warnings |=
            FLOPPY144_PERSISTENCE_WARNING_PROFILE;
    }
    else
    {
        global_persistence_warnings &=
            (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_PROFILE;
    }

    Floppy144Redraw(
        window
    );
}

/*
 * Open persistent environment settings.
 */
static void Floppy144OpenSettings(
    HWND window
)
{
    global_settings_option =
        FLOPPY144_SETTINGS_OPTION_CRT;

    global_settings_notice =
        NULL;

    global_screen =
        FLOPPY144_SCREEN_SETTINGS;

    Floppy144Redraw(
        window
    );
}

/*
 * Apply one stepped settings adjustment and persist it atomically.
 *
 * Runtime consumers are updated only after the settings file has been
 * replaced successfully. A failed save restores the complete previous
 * settings structure, so the screen never advertises an unpersisted value.
 */
/* BF20: commit operator login without making empty or partial identities permanent. */
static void Floppy144CommitIntroLogin(HWND window)
{
    Floppy144DiscoveryProfile before=global_profile;
    char name[FLOPPY144_PROFILE_NAME_CAPACITY];
    uint32_t first=0U,last=global_intro_login_length,len;
    while(first<last&&global_intro_login[first]==' ')++first;
    while(last>first&&global_intro_login[last-1U]==' ')--last;
    len=last-first;
    if(len>FLOPPY144_PROFILE_NAME_MAX_LENGTH)
        len=FLOPPY144_PROFILE_NAME_MAX_LENGTH;
    if(len>0U)memcpy(name,global_intro_login+first,len);
    name[len]='\0';
    if(!Floppy144DiscoveryProfileOperatorNameValid(name))
        (void)snprintf(name,sizeof(name),"GUEST");
    if(strcmp(Floppy144DiscoveryProfileOperatorName(&global_profile),name)!=0)
    {
        if(!Floppy144DiscoveryProfileSetOperatorName(&global_profile,name))
        {
            global_intro_login_save_failed=true;
            Floppy144Redraw(window);
            return;
        }
    }
    if(global_profile.dirty!=0U &&
       !Floppy144PersistenceSaveProfile(
           Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_PROFILE),
           &global_profile))
    {
        global_profile=before;
        global_intro_login_save_failed=true;
        global_persistence_warnings|=FLOPPY144_PERSISTENCE_WARNING_PROFILE;
        Floppy144Redraw(window);
        return;
    }
    global_persistence_warnings&=
        (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_PROFILE;
    global_intro_login_save_failed=false;
    Floppy144TimingStopSplash(&global_timing);
    Floppy144OpenMainMenu(window);
}

static void Floppy144CommitSettingsAdjustment(
    HWND window,
    int32_t direction
)
{
    Floppy144Settings original_settings;
    bool changed =
        false;
    int32_t step =
        direction < 0
            ? -1
            : 1;

    if(direction == 0)
    {
        return;
    }

    original_settings =
        global_settings;

    switch(global_settings_option)
    {
        case FLOPPY144_SETTINGS_OPTION_CRT:
        {
            int32_t next =
                (int32_t)global_settings.crt_mode +
                step;

            if(next < 0)
            {
                next =
                    (int32_t)FLOPPY144_CRT_COUNT - 1;
            }
            else if(
                next >=
                    (int32_t)FLOPPY144_CRT_COUNT
            )
            {
                next =
                    0;
            }

            changed =
                Floppy144SettingsSetCrtMode(
                    &global_settings,
                    (Floppy144CrtMode)next
                );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_TEXT_SPEED:
        {
            int32_t next =
                (int32_t)global_settings.text_speed +
                step;

            if(next < 0)
            {
                next =
                    (int32_t)FLOPPY144_TEXT_SPEED_COUNT - 1;
            }
            else if(
                next >=
                    (int32_t)FLOPPY144_TEXT_SPEED_COUNT
            )
            {
                next =
                    0;
            }

            changed =
                Floppy144SettingsSetTextSpeed(
                    &global_settings,
                    (Floppy144TextSpeed)next
                );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_MUSIC_VOLUME:
        {
            int32_t next =
                (int32_t)global_settings.music_volume +
                step;

            if(next < 0)
            {
                next = 0;
            }

            if(
                next >
                    (int32_t)FLOPPY144_SETTINGS_VOLUME_MAX
            )
            {
                next =
                    (int32_t)FLOPPY144_SETTINGS_VOLUME_MAX;
            }

            changed =
                Floppy144SettingsSetMusicVolume(
                    &global_settings,
                    (uint8_t)next
                );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_SFX_VOLUME:
        {
            int32_t next =
                (int32_t)global_settings.sfx_volume +
                step;

            if(next < 0)
            {
                next = 0;
            }

            if(
                next >
                    (int32_t)FLOPPY144_SETTINGS_VOLUME_MAX
            )
            {
                next =
                    (int32_t)FLOPPY144_SETTINGS_VOLUME_MAX;
            }

            changed =
                Floppy144SettingsSetSfxVolume(
                    &global_settings,
                    (uint8_t)next
                );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_AUTOSAVE:
        {
            int32_t next =
                (int32_t)global_settings.autosave_mode +
                step;

            if(next < 0)
            {
                next =
                    (int32_t)FLOPPY144_AUTOSAVE_MODE_COUNT - 1;
            }
            else if(
                next >=
                    (int32_t)FLOPPY144_AUTOSAVE_MODE_COUNT
            )
            {
                next =
                    0;
            }

            changed =
                Floppy144SettingsSetAutosaveMode(
                    &global_settings,
                    (Floppy144AutosaveMode)next
                );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_INSPECTION_INDICATORS:
        {
            changed=Floppy144SettingsSetInspectionIndicators(
                &global_settings,
                global_settings.show_inspection_indicators==0U
            );
            break;
        }
        case FLOPPY144_SETTINGS_OPTION_CREDITS:
        case FLOPPY144_SETTINGS_OPTION_COUNT:
        {
            break;
        }
    }

    if(!changed)
    {
        Floppy144Redraw(
            window
        );

        return;
    }

    if(
        !Floppy144PersistenceSaveSettings(
            Floppy144StoragePath(
                &global_storage_paths,
                F144_PERSISTENCE_SETTINGS
            ),
            &global_settings
        )
    )
    {
        global_settings =
            original_settings;

        global_settings_notice =
            "SETTINGS COULD NOT BE SAVED";

        global_persistence_warnings |=
            FLOPPY144_PERSISTENCE_WARNING_SETTINGS;

        Floppy144Redraw(
            window
        );

        return;
    }

    global_persistence_warnings &=
        (uint8_t)~FLOPPY144_PERSISTENCE_WARNING_SETTINGS;

    global_settings_notice =
        NULL;

    switch(global_settings_option)
    {
        case FLOPPY144_SETTINGS_OPTION_MUSIC_VOLUME:
        {
            (void)f144PlatformSetMusicVolume(
                &global_platform,
                global_settings.music_volume
            );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_SFX_VOLUME:
        {
            (void)f144PlatformSetSfxVolume(
                &global_platform,
                global_settings.sfx_volume
            );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_AUTOSAVE:
        {
            Floppy144TimingSetAutosaveInterval(
                &global_timing,
                f144PlatformMonotonicMs(
                    &global_platform
                ),
                Floppy144SettingsAutosaveIntervalMs(
                    &global_settings
                )
            );

            break;
        }

        case FLOPPY144_SETTINGS_OPTION_CRT:
        case FLOPPY144_SETTINGS_OPTION_TEXT_SPEED:
        case FLOPPY144_SETTINGS_OPTION_CREDITS:
        case FLOPPY144_SETTINGS_OPTION_COUNT:
        {
            break;
        }
    }

    Floppy144Redraw(
        window
    );
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
                uint32_t recovery_seed;

                /*
                 * Stage 4E regression runs may inject a deterministic seed
                 * through portable configuration. Ordinary launches retain
                 * the established monotonic-clock-derived seed.
                 */
                if(
                    !f144StartupConfigRecoverySeedOverride(
                        &global_config,
                        &recovery_seed
                    )
                )
                {
                    recovery_seed =
                        (uint32_t)f144PlatformMonotonicMs(
                            &global_platform
                        );

                    if(recovery_seed == 0U)
                    {
                        recovery_seed =
                            1U;
                    }
                }

                Floppy144RunStateBegin(
                    &global_run_state,
                    recovery_seed
                );
                if(Floppy144HathawayMode())
                {
                    Floppy144RunStateEnableHathawayInspection(
                        &global_run_state
                    );
                    Floppy144WorldHydrateFromRunState(
                        &global_world,&global_run_state
                    );
                }

                Floppy144PlayerVisualReset(
                    &global_player_visual
                );

                if(!Floppy144HathawayMode())
                {
                    Floppy144DiscoveryProfileBeginRecovery(
                        &global_profile
                    );

                    Floppy144PersistenceSaveProfile(
                        Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_PROFILE),
                        &global_profile
                    );
                }
            }

            global_terminal_authentication_complete =
                false;

            Floppy144TerminalReset(
                &global_terminal,
                &global_world
            );

            Floppy144ConfigureTerminalSession(
                true
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

            global_resume_screen = Floppy144HathawayMode()
                ? FLOPPY144_SCREEN_OFFICE : FLOPPY144_SCREEN_TERMINAL;

            global_screen = global_resume_screen;

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
            if(Floppy144HathawayMode())
            {
                global_main_menu_notice="INSPECTION MODE - SAVING DISABLED";
                global_main_menu_notice_is_warning=false;
                Floppy144Redraw(window);
                return;
            }
            Floppy144UpdateDiscoveryProfile();

            global_main_menu_notice =
                NULL;

            global_main_menu_notice_is_warning =
                false;

            if(
                global_session_active &&
                Floppy144PersistenceSaveRunState(
                    Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_MANUAL_SAVE),
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

                global_main_menu_notice =
                    "CURRENT SESSION RECORDED";
            }
            else
            {
                global_main_menu_notice =
                    "CURRENT SESSION COULD NOT BE RECORDED";

                global_main_menu_notice_is_warning =
                    true;
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
                    ? Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_AUTOSAVE)
                    : Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_MANUAL_SAVE),
                    &global_run_state
                )
            )
            {
                Floppy144WorldHydrateFromRunState(
                    &global_world,
                    &global_run_state
                );

                global_terminal_authentication_complete =
                    false;

                {
                    Floppy144RoomId eTerminalRoom =
                        Floppy144SiteRoomAtPosition(
                            global_run_state.player_site_x,
                            global_run_state.player_site_y
                        );

                    if(
                        (uint32_t)eTerminalRoom <
                        (uint32_t)FLOPPY144_ROOM_COUNT
                    )
                    {
                        Floppy144TerminalResetAtRoom(
                            &global_terminal,
                            &global_world,
                            eTerminalRoom
                        );
                    }
                    else
                    {
                        Floppy144TerminalReset(
                            &global_terminal,
                            &global_world
                        );
                    }
                }

                Floppy144ConfigureTerminalSession(
                    true
                );

                /*
                 * TerminalReset normally suppresses the WM_CHAR generated by
                 * the key which opened a physical terminal. Reinstate is
                 * different: its acknowledgement key is consumed entirely by
                 * Session Control before the terminal becomes active, so
                 * leaving this armed would discard the player's first real
                 * command character.
                 */
                global_terminal.suppress_next_character =
                    false;

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

                /*
                 * Do not jump directly into gameplay. Session Control first
                 * confirms the reinstatement. The loaded reconstruction
                 * percentage is hidden under that confirmation until the
                 * player's next key-down, then the matching key-up enters the
                 * restored terminal without leaking a character into it.
                 */
                global_main_menu_option =
                FLOPPY144_MAIN_MENU_RETURN_TO_SITE;

                global_main_menu_notice =
                NULL;

                global_main_menu_notice_is_warning =
                false;

                global_reinstate_confirmation_pending =
                true;

                global_reinstate_continue_on_keyup =
                false;

                global_reinstate_continue_key =
                0U;

                global_screen =
                FLOPPY144_SCREEN_MAIN_MENU;

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

            global_reinstate_confirmation_pending =
                false;

            global_reinstate_continue_on_keyup =
                false;

            global_reinstate_continue_key =
                0U;

            global_persistence_warnings |=
                FLOPPY144_PERSISTENCE_WARNING_SAVE;

            Floppy144Redraw(
                window
            );

            return;
        }

        case FLOPPY144_MAIN_MENU_OPERATOR_PROFILE:
        {
            /*
             * Profile reads only persistent operator history. Opening it does
             * not merge current run state or modify recovery progression.
             */
            Floppy144OpenOperatorProfile(
                window
            );

            return;
        }

        case FLOPPY144_MAIN_MENU_SETTINGS:
        {
            Floppy144OpenSettings(
                window
            );

            return;
        }

        case FLOPPY144_MAIN_MENU_TERMINATE:
        {
            F144LifecycleEvent event =
            {
                F144_LIFECYCLE_SHUTDOWN_REQUESTED,
                0U
            };

            Floppy144HandleLifecycleEvent(
                window,
                &event
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

    {
        int32_t before_x=global_run_state.player_site_x;
        int32_t before_y=global_run_state.player_site_y;
        uint64_t now=f144PlatformMonotonicMs(&global_platform);
        Floppy144RunStateMovePlayerSite(
            &global_run_state,world_movement_x,world_movement_y
        );
        if(before_x!=global_run_state.player_site_x ||
           before_y!=global_run_state.player_site_y)
        {
            global_footstep_pending=true;
            global_last_movement_ms=now;
        }
        else
        {
            bool locked=false;
            global_footstep_pending=false;
            const char *door=Floppy144SiteCorridorDoorLabelNearby(
                &global_run_state,&locked);
            if(door!=NULL && locked && now>=global_last_footstep+340U) {
                Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_DOOR_BLOCKED);
                global_last_footstep=now;
            }
        }
    }

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

    /* Physical-object names are data-driven. No action is synthesized unless
       inspection actually resolves to an authored nearby object. */
    if(strstr(pszItemName,"Fridge") || strstr(pszItemName,"FRIDGE"))
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_FRIDGE_OPEN);
    else if(strstr(pszItemName,"Coffee") || strstr(pszItemName,"COFFEE"))
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_COFFEE_MACHINE);
    else if(strstr(pszItemName,"Phone") || strstr(pszItemName,"PHONE"))
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_TELEPHONE);
    else if(strstr(pszItemName,"Printer") || strstr(pszItemName,"PRINTER"))
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_PRINTER);
    else if(strstr(pszItemName,"Stapler") || strstr(pszItemName,"STAPLER"))
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_STAPLER);
    else if(strstr(pszItemName,"Paper") || strstr(pszItemName,"PAPER"))
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_PAPER);
    else if(strstr(pszItemName,"Stamp") || strstr(pszItemName,"STAMP"))
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_STAMP);
}

/*
 * Execute a projection-neutral Site action.
 *
 * Access is data-driven for conventional locked doors and GDR terminals.
 * Inspect resolves canonical physical material through generated
 * furniture/fixture geometry, while locked doors remain inspectable until
 * their authored Access interaction succeeds.
 */
static void Floppy144InteractOffice(
    HWND window,
    Floppy144OfficeInteractionMode eMode
)
{
    /* The transient scene takes over the existing logical A/Access key. */
    if(Floppy144GreyDoorNearby(&global_run_state))
    {
        if(eMode == FLOPPY144_OFFICE_INTERACTION_ACCESS)
        {
            if(Floppy144GreyEncounterBegin(
                &global_grey_encounter,&global_run_state))
            {
                Floppy144MovementInputReset(&global_movement_input);
                global_grey_encounter.body_style=
                    Floppy144DiscoveryProfileBodyStyle(&global_profile);
                global_office_notice=NULL;
                global_screen=FLOPPY144_SCREEN_GREY_ENCOUNTER;
                Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_POWER_FAIL);
            }
        }
        else
        {
            Floppy144OfficeSetItemNotice("GREY DOOR",".");
        }
        Floppy144Redraw(window);
        return;
    }

    if(eMode == FLOPPY144_OFFICE_INTERACTION_ACCESS)
    {
        Floppy144InteractionId eAccessInteraction;
        Floppy144RoomId eTerminalRoom;

        if(
            Floppy144SiteAccessInteractionNearby(
                &global_run_state,
                &eAccessInteraction
            )
        )
        {
            if(
                Floppy144InteractionTryRun(
                    &global_world,
                    &global_run_state,
                    eAccessInteraction
                )
            )
            {
                Floppy144OfficeSetItemNotice(
                    "DOOR",
                    ": ACCESS GRANTED."
                );
                Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_DOOR_OPEN);
            }

            Floppy144Redraw(window);
            return;
        }

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
        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_CRT_WAKE);

        Floppy144ConfigureTerminalSession(
            true
        );

        Floppy144Redraw(window);
        return;
    }

    if(eMode == FLOPPY144_OFFICE_INTERACTION_INSPECT)
    {
        Floppy144SiteInspectionTarget sTarget;

        /*
         * The footer and the keyboard share one contract. If I: INSPECT is not
         * advertised, a blind I keypress is a true no-op. Secure cabinets are
         * accessed only through A: ACCESS; being beside a locked cabinet does
         * not create a hidden Inspect response.
         */
        if(
            (
                Floppy144SiteAvailableActions(
                    &global_run_state
                ) &
                FLOPPY144_SITE_ACTION_INSPECT
            ) == 0U
        )
        {
            return;
        }

        /*
         * Site Directory fixtures are UI gateways rather than evidence items.
         * They deliberately take precedence over contextual material mounted
         * on the same fixture.
         */
        if(Floppy144SiteDirectoryNearby(&global_run_state))
        {
            global_office_notice = NULL;
            global_resume_screen = FLOPPY144_SCREEN_OFFICE;
            global_screen = FLOPPY144_SCREEN_SITE_DIRECTORY;
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
            bool bDoorLocked = false;

            const char *pszDoorLabel =
                Floppy144SiteCorridorDoorLabelNearby(
                    &global_run_state,
                    &bDoorLocked
                );

            /*
             * Authored room plates and notices are resolved above as physical
             * contents. This is only the fallback for a Corridor-facing door
             * which has no physical child of its own, such as the Secretary
             * suite entrance and the emergency exits.
             */
            if(pszDoorLabel != NULL)
            {
                Floppy144OfficeSetItemNotice(
                    pszDoorLabel,
                    bDoorLocked
                    ? " - LOCKED."
                    : "."
                );

                Floppy144Redraw(window);
                return;
            }

            /*
             * Non-Corridor doors without authored contents retain the generic
             * locked-door response.
             */
            if(
                Floppy144SiteLockedDoorNearby(
                    &global_run_state
                )
            )
            {
                Floppy144OfficeSetItemNotice(
                    "LOCKED DOOR",
                    "."
                );

                Floppy144Redraw(window);
            }

            return;
        }

        /*
         * Every inspectable parent now opens the reusable contents screen.
         * The Site resolver still chooses the most useful nearby parent, but
         * item selection and interaction happen inside that parent view. This
         * prevents one arbitrary child from representing an entire desk,
         * shelf, worktop or cupboard.
         */
        if(
            Floppy144CabinetOpenParent(
                &global_cabinet,
                &global_run_state,
                sTarget.pszParentId
            )
        )
        {
            F144CalendarDate today;

            /*
             * Only the Noticeboard setter can attach contextual material.
             * No content code touches Win32 or the animation clock.
             */
            if(f144PlatformCalendarDate(&global_platform, &global_config, &today))
            {
                Floppy144CabinetSetNoticeboardDate(
                    &global_cabinet, &global_run_state, &today
                );
            }

            global_office_notice = NULL;
            global_resume_screen = FLOPPY144_SCREEN_CABINET;
            global_screen = FLOPPY144_SCREEN_CABINET;
            Floppy144Redraw(window);
            return;
        }

        Floppy144OfficeSetItemNotice(
            sTarget.pszPhysicalItemName,
            ": CONTENTS COULD NOT BE OPENED."
        );

        Floppy144Redraw(window);
    }
}

/*
 * Win32 message handler
 *
 * Windows sends close, keyboard, resize and paint messages here. Keyboard handling
 * is routed first by active game screen, then by the key pressed.
 */

/*
 * Text-input coordinator
 *
 * Text entry is deliberately separate from F144Action. Win32 currently feeds
 * this from WM_CHAR; future platforms can deliver their native text event
 * without manufacturing gameplay actions for printable characters.
 */
static bool Floppy144HandleTextInput(
    HWND window,
    const F144TextInputEvent *pEvent
)
{
    uint32_t uCodepoint;

    if(pEvent == NULL)
    {
        return false;
    }

    uCodepoint =
        pEvent->codepoint;

    /* BF20: LOGIN is the only editable field during the staged introduction. */
    if(global_screen==FLOPPY144_SCREEN_SPLASH)
    {
        if(Floppy144IntroLoginReady(
            Floppy144TimingSplashElapsedMs(
                &global_timing,f144PlatformMonotonicMs(&global_platform))))
        {
            if(uCodepoint==(uint32_t)'\b')
            {
                if(global_intro_login_length>0U)
                {
                    global_intro_login[--global_intro_login_length]='\0';
                    Floppy144AudioPlay(&global_platform,
                        FLOPPY144_AUDIO_TERMINAL_BACKSPACE);
                }
            }
            else if(uCodepoint!=(uint32_t)'\r' &&
                global_intro_login_length<FLOPPY144_PROFILE_NAME_MAX_LENGTH &&
                Floppy144DiscoveryProfileOperatorNameCharacterSupported(uCodepoint))
            {
                global_intro_login[global_intro_login_length++]=(char)uCodepoint;
                global_intro_login[global_intro_login_length]='\0';
                Floppy144AudioPlay(&global_platform,
                    FLOPPY144_AUDIO_TERMINAL_TYPE);
            }
            global_intro_login_save_failed=false;
            Floppy144Redraw(window);
        }
        return true;
    }

    /*
     * Operator-name entry consumes only the platform-neutral text stream.
     *
     * Enter is handled by the logical Confirm action so its trailing carriage
     * return must not become name data. Backspace edits the transient buffer.
     */
    if(
        global_screen == FLOPPY144_SCREEN_PROFILE &&
        Floppy144ProfileNameEditActive(
            &global_profile_name_edit
        )
    )
    {
        if(uCodepoint == (uint32_t)'\b')
        {
            if(Floppy144ProfileNameEditBackspace(
                &global_profile_name_edit
            ))
                Floppy144AudioPlay(&global_platform,
                    FLOPPY144_AUDIO_TERMINAL_BACKSPACE);
        }
        else if(uCodepoint != (uint32_t)'\r')
        {
            if(Floppy144ProfileNameEditInputCodepoint(
                &global_profile_name_edit,
                uCodepoint
            ))
                Floppy144AudioPlay(&global_platform,
                    FLOPPY144_AUDIO_TERMINAL_TYPE);
        }

        Floppy144Redraw(
            window
        );

        return true;
    }

            /* STAGE 3B.5 CABINET CHARACTER INPUT */
            if(global_screen == FLOPPY144_SCREEN_CABINET)
            {
                if(uCodepoint >= '0' && uCodepoint <= '9')
                {
                    (void)Floppy144CabinetInputDigit(
                        &global_cabinet,
                        (char)uCodepoint
                    );
                }

                Floppy144Redraw(window);
                return true;
            }

            if(
                global_screen !=
                FLOPPY144_SCREEN_TERMINAL
            )
            {
                return false;
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

                return true;
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
                switch(uCodepoint)
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

                return true;
            }

            /*
             * The record pager temporarily owns terminal character input.
             * The separate Menu action remains responsible for session control.
             */

            if(
                Floppy144TerminalRecordPagerActive(
                    &global_terminal
                )
            )
            {
                switch(uCodepoint)
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

                return true;
            }

            if(uCodepoint>=32U && uCodepoint<=126U)
                Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_TERMINAL_TYPE);

            switch(uCodepoint)
            {
                case '\b':
                {
                    uint32_t before=global_terminal.input_length;
                    Floppy144TerminalBackspace(&global_terminal);
                    if(global_terminal.input_length<before)
                        Floppy144AudioPlay(&global_platform,
                            FLOPPY144_AUDIO_TERMINAL_BACKSPACE);
                    break;
                }

                case '\r':
                {
                    Floppy144TerminalSubmitInput(
                        &global_terminal,&global_world,&global_run_state
                    );
                    if(Floppy144TerminalRestoreInProgress(&global_terminal))
                        Floppy144AudioPlay(&global_platform,FLOPPY144_AUDIO_RELAY);
                    else
                    {
                        const char *line=global_terminal.output_count
                            ? global_terminal.output[global_terminal.output_count-1U]
                            : "";
                        bool rejected=strstr(line,"ERROR")!=NULL ||
                            strstr(line,"UNKNOWN")!=NULL ||
                            strstr(line,"DENIED")!=NULL ||
                            strstr(line,"INVALID")!=NULL ||
                            strstr(line,"FAILED")!=NULL;
                        Floppy144AudioPlay(&global_platform,rejected
                            ? FLOPPY144_AUDIO_ERROR : FLOPPY144_AUDIO_ACCEPT);
                    }

                    if(global_terminal.exit_requested)
                    {
                        global_terminal.exit_requested =
                        false;

                        global_completion_evidence_resolved=
                            Floppy144GameDataEvidenceResolved(
                                &global_run_state
                            );

                        global_completion_capacity_exhausted=
                            Floppy144RunStateAvailableRecoveryCapacityExhausted(
                                &global_run_state
                            );

                        if(
                            !Floppy144HathawayMode() &&
                            (global_completion_evidence_resolved ||
                             global_completion_capacity_exhausted)
                        )
                        {
                            /*
                             * Freeze this achieved version of the truth before
                             * presenting it. The profile keeps the final evidence
                             * mask for comparison with future recoveries, while
                             * the run itself becomes deliberately unsaveable.
                             */
                            (void)Floppy144DiscoveryProfileMergeRunState(
                                &global_profile,
                                &global_run_state
                            );
                            (void)Floppy144DiscoveryProfileRecordCompletion(
                                &global_profile,
                                &global_run_state,
                                global_completion_evidence_resolved,
                                global_completion_capacity_exhausted
                            );
                            (void)Floppy144PersistenceSaveProfile(
                                Floppy144StoragePath(&global_storage_paths,F144_PERSISTENCE_PROFILE),
                                &global_profile
                            );

                            global_run_state.dirty=0U;
                            global_session_active=false;
                            Floppy144CompletionViewReset(
                                &global_completion_view
                            );
                            global_main_menu_option=
                                FLOPPY144_MAIN_MENU_INITIATE_SESSION;
                            global_screen=FLOPPY144_SCREEN_COMPLETION;
                        }
                        else if(
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

                            return true;
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
                            global_catalogue.recovery_seed =
                                global_run_state.recovery_seed;
                            Floppy144DocumentApplyEffects(
                                &global_world,
                                &global_run_state,
                                global_terminal.requested_collection,
                                global_terminal.requested_record_index
                            );
                            (void)Floppy144GameDataRecordOpened(
                                &global_run_state,
                                global_terminal.requested_collection,
                                global_terminal.requested_record_index
                            );

                            /*
                             * Refresh the objective from resulting persistent
                             * state. It becomes visible when the player returns
                             * from the document to the terminal.
                             */
                            Floppy144TerminalPrintPostOpenAction(
                                &global_terminal,
                                &global_run_state,
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
                        uCodepoint >= 32U &&
                        uCodepoint <= 126U
                    )
                    {
                        Floppy144TerminalInputCharacter(
                            &global_terminal,
                            (char)uCodepoint
                        );
                    }

                    break;
                }
            }

            Floppy144Redraw(
                window
            );

            return true;
}

/*
 * Logical action coordinator
 *
 * Native backends supply F144ActionEvent values. The coordinator owns
 * screen-specific meaning; it never interprets the opaque physical token.
 * HWND remains a temporary redraw carrier until the later lifecycle/build
 * separation tasks remove the remaining Win32 launcher shell.
 */
static bool Floppy144HandleActionEvent(
    HWND window,
    const F144ActionEvent *pEvent
)
{
    F144Action eAction;

    if(pEvent == NULL)
    {
        return false;
    }

    if(pEvent->type == F144_ACTION_EVENT_UP)
    {
        bool movement_action =
            Floppy144MovementInputSetAction(
                &global_movement_input,
                pEvent->action,
                false
            );

        if(movement_action)
        {
            int32_t movement_x;
            int32_t movement_y;

            Floppy144MovementInputVector(
                &global_movement_input,
                FLOPPY144_SITE_MOVE_STEP_X16,
                &movement_x,
                &movement_y
            );

            if(global_screen==FLOPPY144_SCREEN_GREY_ENCOUNTER)
            {
                /* Same held-key release and gait semantics as Site movement. */
                (void)Floppy144PlayerVisualSetMovement(
                    &global_grey_encounter.player_visual,
                    movement_x,movement_y,F144_ACTION_NONE);
                Floppy144Redraw(window);
            }
            else if(
                Floppy144PlayerVisualSetMovement(
                    &global_player_visual,
                    movement_x,
                    movement_y,
                    F144_ACTION_NONE
                ) &&
                global_screen == FLOPPY144_SCREEN_OFFICE
            )
            {
                Floppy144Redraw(window);
            }
        }

            if(
                global_screen == FLOPPY144_SCREEN_MAIN_MENU &&
                global_reinstate_continue_on_keyup &&
                pEvent->physical_token == global_reinstate_continue_key
            )
            {
                global_reinstate_continue_on_keyup =
                    false;

                global_reinstate_continue_key =
                    0U;

                global_screen =
                    global_resume_screen;

                Floppy144Redraw(
                    window
                );

                return true;
            }


        return false;
    }

    if(pEvent->type != F144_ACTION_EVENT_DOWN)
    {
        return false;
    }

    eAction =
        pEvent->action;

    /* No Escape to Session Control or save/reinstate inside a transient scene.
       The same logical action adapter supplies movement and Inspect. */
    if(global_screen == FLOPPY144_SCREEN_GREY_ENCOUNTER)
    {
        bool changed=false;
        switch(eAction)
        {
            case F144_ACTION_MOVE_LEFT:
            case F144_ACTION_MOVE_RIGHT:
            case F144_ACTION_MOVE_UP:
            case F144_ACTION_MOVE_DOWN:
            {
                int32_t movement_x=0,movement_y=0;
                (void)Floppy144MovementInputSetAction(
                    &global_movement_input,eAction,true);
                Floppy144MovementInputVector(
                    &global_movement_input,FLOPPY144_SITE_MOVE_STEP_X16,
                    &movement_x,&movement_y);
                (void)Floppy144PlayerVisualSetMovement(
                    &global_grey_encounter.player_visual,
                    movement_x,movement_y,eAction);
                changed=Floppy144GreyEncounterMove(
                    &global_grey_encounter,
                    movement_x*FLOPPY144_SITE_2D_PIXELS_PER_UNIT/
                        FLOPPY144_SITE_FIXED_ONE,
                    movement_y*FLOPPY144_SITE_2D_PIXELS_PER_UNIT/
                        FLOPPY144_SITE_FIXED_ONE);
                break;
            }
            case F144_ACTION_INSPECT:
                changed=Floppy144GreyEncounterInspect(
                    &global_grey_encounter); break;
            default: break;
        }
        if(changed) Floppy144Redraw(window);
        return true;
    }

    if(global_screen != FLOPPY144_SCREEN_OFFICE)
    {
        Floppy144MovementInputReset(
            &global_movement_input
        );

        (void)Floppy144PlayerVisualSetMovement(
            &global_player_visual,
            0,
            0,
            F144_ACTION_NONE
        );
    }

            /*
             * A reinstated session owns the next complete key press.
             *
             * Key-down dismisses the confirmation box and exposes the loaded
             * reconstruction percentage. Key-up then enters gameplay. Keeping
             * the screen on Session Control between those messages also means
             * TranslateMessage cannot feed the continue key into the terminal.
             */
            if(
                global_screen == FLOPPY144_SCREEN_MAIN_MENU &&
                global_reinstate_continue_on_keyup
            )
            {
                return true;
            }

            if(
                global_screen == FLOPPY144_SCREEN_MAIN_MENU &&
                global_reinstate_confirmation_pending
            )
            {
                global_reinstate_confirmation_pending =
                    false;

                global_reinstate_continue_on_keyup =
                    true;

                global_reinstate_continue_key =
                    pEvent->physical_token;

                Floppy144Redraw(
                    window
                );

                /*
                 * Force this one transitional frame to the window before the
                 * matching key-up enters the restored session. Without this,
                 * Windows may coalesce the invalidated menu frame with the
                 * following terminal redraw and the restored percentage would
                 * never actually be visible.
                 */
                UpdateWindow(
                    window
                );

                return true;
            }

            /*
             * The Site Directory is a transient overlay over exploration.
             * Its opening key must not also close it, so only a subsequent
             * key-down is handled here. Escape is included and returns to the
             * player rather than opening Session Control.
             */
            if(global_screen == FLOPPY144_SCREEN_SITE_DIRECTORY)
            {
                global_resume_screen = FLOPPY144_SCREEN_OFFICE;
                global_screen = FLOPPY144_SCREEN_OFFICE;
                Floppy144Redraw(window);
                return true;
            }

            /* Boot transitions cannot be skipped. Enter confirms only LOGIN. */
            if(global_screen==FLOPPY144_SCREEN_SPLASH)
            {
                if(eAction==F144_ACTION_CONFIRM &&
                   Floppy144IntroLoginReady(
                       Floppy144TimingSplashElapsedMs(
                           &global_timing,
                           f144PlatformMonotonicMs(&global_platform))))
                    Floppy144CommitIntroLogin(window);
                return true;
            }

            /*
             * Escape never terminates the application.
             *
             * From every non-menu screen it suspends the current view and
             * returns to GDR session control. On the menu it has no effect.
             */

            if(eAction == F144_ACTION_MENU)
            {
                /*
                 * Escape cancels a name edit without leaving Profile. The
                 * original persistent name has not been changed at this point.
                 */
                if(
                    global_screen == FLOPPY144_SCREEN_PROFILE &&
                    Floppy144ProfileNameEditActive(
                        &global_profile_name_edit
                    )
                )
                {
                    Floppy144ProfileNameEditCancel(
                        &global_profile_name_edit
                    );

                    Floppy144Redraw(
                        window
                    );

                    return true;
                }

                if(
                    global_screen == FLOPPY144_SCREEN_TERMINAL &&
                    Floppy144TerminalRestoreInProgress(
                        &global_terminal
                    )
                )
                {
                    return true;
                }

                if(global_screen != FLOPPY144_SCREEN_MAIN_MENU)
                {
                    Floppy144OpenMainMenu(
                        window
                    );
                }

                return true;
            }

            switch(global_screen)
            {
                /*
                 * Intro skip actions are handled before screen-specific input.
                 * Other actions are ignored until the next animation frame.
                 */
                case FLOPPY144_SCREEN_SPLASH:
                {
                    return true;
                }
                /*
                 * GDR main menu: move through available options and execute
                 * the highlighted administrative action.
                 */

                case FLOPPY144_SCREEN_MAIN_MENU:
                {
                    switch(eAction)
                    {
                        case F144_ACTION_MOVE_UP:
                        case F144_ACTION_NAV_UP:
                        {
                            Floppy144MainMenuMoveSelection(
                                -1
                            );

                            Floppy144Redraw(
                                window
                            );

                            return true;
                        }

                        case F144_ACTION_MOVE_DOWN:
                        case F144_ACTION_NAV_DOWN:
                        {
                            Floppy144MainMenuMoveSelection(
                                1
                            );

                            Floppy144Redraw(
                                window
                            );

                            return true;
                        }

                        case F144_ACTION_CONFIRM:
                        {
                            Floppy144MainMenuActivate(
                                window
                            );

                            return true;
                        }
                    }

                    break;
                }

                /*
                 * Persistent environment settings use stepped controls.
                 */
                case FLOPPY144_SCREEN_SETTINGS:
                {
                    switch(eAction)
                    {
                        case F144_ACTION_MOVE_UP:
                        case F144_ACTION_NAV_UP:
                        {
                            int32_t next =
                                (int32_t)global_settings_option - 1;

                            if(next < 0)
                            {
                                next =
                                    (int32_t)FLOPPY144_SETTINGS_OPTION_COUNT - 1;
                            }

                            global_settings_option =
                                (Floppy144SettingsOption)next;

                            global_settings_notice =
                                NULL;

                            Floppy144Redraw(window);
                            return true;
                        }

                        case F144_ACTION_MOVE_DOWN:
                        case F144_ACTION_NAV_DOWN:
                        {
                            global_settings_option =
                                (Floppy144SettingsOption)(
                                    (
                                        (uint32_t)global_settings_option +
                                        1U
                                    ) %
                                    (uint32_t)FLOPPY144_SETTINGS_OPTION_COUNT
                                );

                            global_settings_notice =
                                NULL;

                            Floppy144Redraw(window);
                            return true;
                        }

                        case F144_ACTION_MOVE_LEFT:
                        {
                            Floppy144CommitSettingsAdjustment(
                                window,
                                -1
                            );

                            return true;
                        }

                        case F144_ACTION_MOVE_RIGHT:
                        {
                            Floppy144CommitSettingsAdjustment(
                                window,
                                1
                            );

                            return true;
                        }

                        case F144_ACTION_CONFIRM:
                        {
                            if(
                                global_settings_option ==
                                    FLOPPY144_SETTINGS_OPTION_CREDITS
                            )
                            {
                                global_credits_return_screen =
                                    FLOPPY144_SCREEN_SETTINGS;
                                global_screen =
                                    FLOPPY144_SCREEN_CREDITS;
                                global_settings_notice =
                                    NULL;
                                Floppy144Redraw(window);
                                return true;
                            }

                            Floppy144CommitSettingsAdjustment(
                                window,
                                1
                            );

                            return true;
                        }

                        case F144_ACTION_BACK:
                        {
                            global_screen =
                                FLOPPY144_SCREEN_MAIN_MENU;

                            global_main_menu_option =
                                FLOPPY144_MAIN_MENU_SETTINGS;

                            global_settings_notice =
                                NULL;

                            Floppy144Redraw(window);
                            return true;
                        }

                        default:
                        {
                            break;
                        }
                    }

                    break;
                }

                case FLOPPY144_SCREEN_CREDITS:
                {
                    if(
                        eAction == F144_ACTION_CONFIRM &&
                        global_credits_return_screen !=
                            FLOPPY144_SCREEN_COMPLETION
                    )
                    {
                        global_screen =
                            FLOPPY144_SCREEN_SPLASH;

                        global_intro_sound_stage=0U;
                        global_intro_login_length=0U;
                        global_intro_login[0]='\0';
                        global_intro_login_save_failed=false;
                    Floppy144TimingStartSplash(
                            &global_timing,
                            f144PlatformMonotonicMs(
                                &global_platform
                            )
                        );

                        Floppy144Redraw(window);
                        return true;
                    }

                    if(eAction == F144_ACTION_BACK)
                    {
                        if(
                            global_credits_return_screen ==
                                FLOPPY144_SCREEN_COMPLETION
                        )
                        {
                            global_screen =
                                FLOPPY144_SCREEN_COMPLETION;
                        }
                        else
                        {
                            global_screen =
                                FLOPPY144_SCREEN_SETTINGS;
                            global_settings_option =
                                FLOPPY144_SETTINGS_OPTION_CREDITS;
                        }

                        Floppy144Redraw(window);
                        return true;
                    }

                    return true;
                }

                /*
                 * Operator Profile owns its small edit state independently of
                 * recovery-session state.
                 */
                case FLOPPY144_SCREEN_PROFILE:
                {
                    if(
                        Floppy144ProfileNameEditActive(
                            &global_profile_name_edit
                        )
                    )
                    {
                        if(eAction == F144_ACTION_CONFIRM)
                        {
                            Floppy144CommitProfileNameEdit(
                                window
                            );

                            return true;
                        }

                        /*
                         * Backspace is performed by the following text event.
                         * Consume its logical action here so it cannot close
                         * the Profile before WM_CHAR delivers '\b'.
                         */
                        if(eAction == F144_ACTION_BACK)
                        {
                            return true;
                        }

                        /*
                         * Printable keys such as A/I/W/S also have gameplay
                         * meanings. While editing, their logical actions are
                         * swallowed and their characters arrive separately
                         * through F144TextInputEvent.
                         */
                        return true;
                    }

                    if(eAction == F144_ACTION_MOVE_LEFT)
                    {
                        Floppy144CommitProfileBodyStyle(
                            window,
                            false
                        );

                        return true;
                    }

                    if(eAction == F144_ACTION_MOVE_RIGHT)
                    {
                        Floppy144CommitProfileBodyStyle(
                            window,
                            true
                        );

                        return true;
                    }

                    if(eAction == F144_ACTION_CONFIRM)
                    {
                        Floppy144ProfileNameEditBegin(
                            &global_profile_name_edit,
                            &global_profile,
                            false
                        );

                        Floppy144Redraw(
                            window
                        );

                        return true;
                    }

                    if(eAction == F144_ACTION_BACK)
                    {
                        global_screen =
                            FLOPPY144_SCREEN_MAIN_MENU;

                        global_main_menu_option =
                            FLOPPY144_MAIN_MENU_OPERATOR_PROFILE;

                        Floppy144Redraw(
                            window
                        );

                        return true;
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
                    switch(eAction)
                    {
                        case F144_ACTION_MOVE_LEFT:
                        case F144_ACTION_MOVE_RIGHT:
                        case F144_ACTION_MOVE_UP:
                        case F144_ACTION_MOVE_DOWN:
                        {
                            int32_t movement_x;
                            int32_t movement_y;

                            (void)Floppy144MovementInputSetAction(
                                &global_movement_input,
                                eAction,
                                true
                            );

                            Floppy144MovementInputVector(
                                &global_movement_input,
                                FLOPPY144_SITE_MOVE_STEP_X16,
                                &movement_x,
                                &movement_y
                            );

                            (void)Floppy144PlayerVisualSetMovement(
                                &global_player_visual,
                                movement_x,
                                movement_y,
                                eAction
                            );

                            if(
                                movement_x != 0 ||
                                movement_y != 0
                            )
                            {
                                Floppy144MovePlayer(
                                    window,
                                    movement_x,
                                    movement_y
                                );
                            }

                            return true;
                        }

                        case F144_ACTION_ACCESS:
                        {
                            /* STAGE 3B.5 CABINET ACCESS KEY */
                            {
                                if(
                                    (
                                        Floppy144SiteAvailableActions(&global_run_state) &
 …60942 tokens truncated…;
    LARGE_INTEGER freq,last,now;
    int number_keys[4]={0},seek_keys[7]={0};
    int nextkey=0,prevkey=0,playkey=0,up=0,down=0,svup=0,svdown=0;
    int rkey=0,ekey=0,menu_key=0,ending_key=0,game_key=0;
    int original=0,authentic=0,retro=0,restore_key=0,cancel_key=0;
    unsigned cue=0U,i,variant=0U,restore_ms=0U,restore_progress=0U;
    int restoring=0;
    double restore_accum=0.0;
    int midi=F144_AudioInit(),sfx=F144_SFXInit();
    if(!midi && !sfx) { puts("No MIDI or waveOut device available."); return 1; }
    F144_MusicSetRestoration(0U);
    if(midi) F144_MusicRestart();
    puts("F144 original-sound review: ESC quit | 1-4 Acts | F1-F7: 0,5,10,15,20,25,29m");
    puts("N/B next/previous SFX | SPACE play cue | UP/DOWN music | LEFT/RIGHT SFX");
    puts("Restoration corruption: R increase by 10, E decrease by 10");
    puts("BF22 REVIEW: M = PLEASE HOLD (menu), C = YOUR CONTRIBUTION HAS BEEN NOTED (ending), G = gameplay");
    printf("Current cue %u/%u (see src/f144sfx.h for names)\n",cue,F144_SFX_COUNT-1U);
    puts("BF24: O ORIGINAL | A REVISED A: PHYSICALLY AUTHENTIC | S REVISED B: STYLISED RETRO");
    puts("T: BF23 3-second restoration demonstration | X: interrupt and restore tempo");
    printf("Cue: %s / ORIGINAL\n",F144_SFXReviewName((F144SFX)cue));
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    for(;;) {
        double dt;
        QueryPerformanceCounter(&now);
        dt=(double)(now.QuadPart-last.QuadPart)/(double)freq.QuadPart;
        last=now;
        if(pressed('T',&restore_key)) {
            restoring=1;restore_ms=0U;restore_progress=0U;restore_accum=0.0;
            F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
            puts("RESTORING...");
        }
        if(pressed('X',&cancel_key)) {restoring=0;F144_MusicSetTempoPercent(100U);puts("Interrupted / tempo returning to normal");}
        if(restoring) {
            unsigned next;
            restore_accum+=dt*1000.0;
            restore_ms=(unsigned)restore_accum;
            next=Floppy144RestoreProgress(restore_ms,3000U);
            F144_MusicSetTempoPercent(Floppy144RestoreTempo(next));
            if(next!=restore_progress) {
                printf("Restoration %u%% / tempo %u%%\n",next,Floppy144RestoreTempo(next));
                F144_SFXPlay(next==100U?F144_SFX_FILE_RESTORE:
                    (next>=97U?F144_SFX_RESTORE_TICK_HIGH:F144_SFX_RESTORE_TICK));
                restore_progress=next;
            }
            if(next==100U)restoring=0;
        }
        F144_AudioUpdate(dt);
        for(i=0;i<4;i++) {
            if(pressed('1'+(int)i,&number_keys[i])) {
                F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
                F144_MusicSetAct((F144Act)i);
                printf("Act %u\n",i);
            }
        }
        for(i=0;i<7;i++) {
            if(pressed(VK_F1+(int)i,&seek_keys[i])) {
                F144_MusicSeekBar(seek_bars[i]);
                printf("Seek %u:00\n",seek_bars[i]/20U);
            }
        }
        if(pressed('M',&menu_key)){
            F144_MusicSetScene(F144_MUSIC_MENU);
            puts("BF22 MENU: PLEASE HOLD / 70 BPM / looping");
        }
        if(pressed('C',&ending_key)){
            F144_MusicSetScene(F144_MUSIC_COMPLETION);
            puts("BF22 COMPLETION: YOUR CONTRIBUTION HAS BEEN NOTED / 65 BPM / looping");
        }
        if(pressed('G',&game_key)){
            F144_MusicSetScene(F144_MUSIC_GAMEPLAY);
            puts("BF22 GAMEPLAY: original approved 80 BPM composition");
        }
        if(pressed('O',&original)){variant=0U;puts("ORIGINAL");}
        if(pressed('A',&authentic)){variant=1U;puts("REVISED A: PHYSICALLY AUTHENTIC / awaiting review");}
        if(pressed('S',&retro)){variant=2U;puts("REVISED B: STYLISED RETRO / awaiting review");}
        F144_SFXReviewVariant(variant);
        if(pressed('N',&nextkey)) { cue=(cue+1U)%F144_SFX_COUNT; printf("SFX %u: %s\n",cue,F144_SFXReviewName((F144SFX)cue)); }
        if(pressed('B',&prevkey)) { cue=(cue+F144_SFX_COUNT-1U)%F144_SFX_COUNT; printf("SFX %u: %s\n",cue,F144_SFXReviewName((F144SFX)cue)); }
        if(pressed(VK_SPACE,&playkey)) F144_SFXPlay((F144SFX)cue);
        if(pressed(VK_UP,&up)) F144_MusicSetVolume(F144_MusicGetVolume()+0.1f);
        if(pressed(VK_DOWN,&down)) F144_MusicSetVolume(F144_MusicGetVolume()-0.1f);
        if(pressed(VK_RIGHT,&svup)) F144_SFXSetVolume(F144_SFXGetVolume()+0.1f);
        if(pressed(VK_LEFT,&svdown)) F144_SFXSetVolume(F144_SFXGetVolume()-0.1f);
        if(pressed('R',&rkey)) F144_MusicSetRestoration(F144_MusicGetRestoration()+10U);
        if(pressed('E',&ekey)) {
            unsigned restoration=F144_MusicGetRestoration();
            F144_MusicSetRestoration(restoration>10U?restoration-10U:0U);
        }
        if(GetAsyncKeyState(VK_ESCAPE)&0x8000) break;
        Sleep(4);
    }
    F144_SFXShutdown();
    F144_AudioShutdown();
    return 0;
}