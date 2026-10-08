/*
 * FLOPPY//144 Stage 4B deterministic timing and lifecycle regression.
 *
 * No sleeping or wall-clock timing is used. All progression is driven by
 * explicit fake monotonic timestamps.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_platform.h"
#include "f144_win32_lifecycle.h"
#include "floppy144_lifecycle.h"
#include "floppy144_settings.h"
#include "floppy144_timing.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct TestPlatformState
{
    uint64_t now_ms;
    uint32_t quit_calls;
} TestPlatformState;

static int failures;

static void Expect(bool condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

static uint64_t TestMonotonicMs(F144Platform *platform)
{
    TestPlatformState *state=(TestPlatformState *)platform->state;
    return state->now_ms;
}

static void TestQuit(F144Platform *platform)
{
    TestPlatformState *state=(TestPlatformState *)platform->state;
    ++state->quit_calls;
}

static const F144PlatformApi test_platform_api =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    TestMonotonicMs,
    TestQuit,
    NULL
};

static void TestPlatformClockAndQuit(void)
{
    F144Platform platform;
    TestPlatformState state;

    memset(&platform,0,sizeof(platform));
    memset(&state,0,sizeof(state));

    state.now_ms=123456789ULL;
    platform.api=&test_platform_api;
    platform.state=&state;

    Expect(
        f144PlatformMonotonicMs(&platform)==123456789ULL,
        "platform monotonic clock dispatches"
    );

    f144PlatformQuit(&platform);

    Expect(
        state.quit_calls==1U,
        "platform quit dispatches"
    );
}

static void TestTimingProgression(void)
{
    Floppy144TimingState timing;
    Floppy144TimingEvents events;

    Floppy144TimingReset(
        &timing,
        1000ULL,
        300000U
    );

    Expect(
        Floppy144TimingSplashElapsedMs(&timing,1000ULL)==0U,
        "splash starts at zero elapsed"
    );
    Expect(
        Floppy144TimingSplashElapsedMs(&timing,1375ULL)==375U,
        "splash elapsed time uses monotonic milliseconds"
    );

    events=Floppy144TimingAdvance(&timing,1015ULL);
    Expect(
        events.presentation_elapsed_ms==15U,
        "presentation timing reports monotonic frame elapsed time"
    );
    Expect(
        events.splash_frame_due==0U &&
        events.terminal_restore_elapsed_ms==0U &&
        events.terminal_cursor_toggle==0U &&
        events.autosave_due==0U,
        "nothing fires before first deadlines"
    );

    events=Floppy144TimingAdvance(&timing,1016ULL);
    Expect(
        events.splash_frame_due!=0U,
        "splash frame becomes due at 16 ms"
    );

    events=Floppy144TimingAdvance(&timing,1050ULL);
    Expect(
        events.terminal_restore_elapsed_ms==50U,
        "terminal restore keeps 50 ms quantisation"
    );

    events=Floppy144TimingAdvance(&timing,1500ULL);
    Expect(
        events.terminal_restore_elapsed_ms==50U,
        "missed restore wakes still emit one 50 ms quantum"
    );
    Expect(
        events.terminal_cursor_toggle!=0U,
        "terminal cursor toggles at 500 ms"
    );

    events=Floppy144TimingAdvance(&timing,2500ULL);
    Expect(
        events.terminal_cursor_toggle!=0U,
        "missed cursor periods collapse into one visible toggle"
    );

    Floppy144TimingStopSplash(&timing);
    events=Floppy144TimingAdvance(&timing,2600ULL);
    Expect(
        events.splash_frame_due==0U,
        "stopped splash no longer requests animation frames"
    );

    /*
     * A faulty clock moving backwards is clamped. Deadlines therefore do not
     * replay or underflow.
     */
    events=Floppy144TimingAdvance(&timing,2000ULL);
    Expect(
        events.presentation_elapsed_ms==0U,
        "backwards clock produces no negative presentation elapsed time"
    );
    Expect(
        events.splash_frame_due==0U,
        "backwards clock does not resurrect stopped splash timing"
    );
}

static void TestAutosaveTiming(void)
{
    Floppy144TimingState timing;
    Floppy144TimingEvents events;
    Floppy144Settings settings;

    Floppy144SettingsReset(&settings);

    Floppy144TimingReset(
        &timing,
        0ULL,
        Floppy144SettingsAutosaveIntervalMs(&settings)
    );

    events=Floppy144TimingAdvance(&timing,299999ULL);
    Expect(
        events.autosave_due==0U,
        "default autosave is not early"
    );

    events=Floppy144TimingAdvance(&timing,300000ULL);
    Expect(
        events.autosave_due!=0U,
        "default five-minute autosave becomes due"
    );

    events=Floppy144TimingAdvance(&timing,900000ULL);
    Expect(
        events.autosave_due!=0U,
        "missed autosave periods collapse into one safe request"
    );

    settings.autosave_mode=
        (uint8_t)FLOPPY144_AUTOSAVE_OFF;

    Floppy144TimingReset(
        &timing,
        0ULL,
        Floppy144SettingsAutosaveIntervalMs(&settings)
    );

    events=Floppy144TimingAdvance(&timing,3600000ULL);
    Expect(
        events.autosave_due==0U,
        "autosave off creates no deadline"
    );
}

static void TestLifecycleState(void)
{
    Floppy144LifecycleState state;
    F144LifecycleEvent event;

    Floppy144LifecycleReset(&state);

    Expect(
        state.started==0U &&
        state.active==0U &&
        state.suspended==0U,
        "lifecycle reset is neutral"
    );

    event.type=F144_LIFECYCLE_START;
    event.request_autosave=0U;
    Floppy144LifecycleApply(&state,&event);

    Expect(
        state.started!=0U &&
        state.active!=0U &&
        state.suspended==0U,
        "start marks application active"
    );

    event.type=F144_LIFECYCLE_INACTIVE;
    Floppy144LifecycleApply(&state,&event);
    Expect(
        state.active==0U,
        "inactive lifecycle state is represented without gameplay policy"
    );

    event.type=F144_LIFECYCLE_SUSPEND;
    event.request_autosave=1U;
    Floppy144LifecycleApply(&state,&event);
    Expect(
        state.suspended!=0U &&
        event.request_autosave!=0U,
        "suspend can carry a future autosave request without inventing one"
    );

    event.type=F144_LIFECYCLE_RESUME;
    event.request_autosave=0U;
    Floppy144LifecycleApply(&state,&event);
    Expect(
        state.suspended==0U &&
        state.active!=0U,
        "resume restores active lifecycle state"
    );

    event.type=F144_LIFECYCLE_SHUTDOWN_REQUESTED;
    Floppy144LifecycleApply(&state,&event);
    Expect(
        state.shutdown_requested!=0U,
        "orderly shutdown request is represented"
    );

    event.type=F144_LIFECYCLE_SHUTDOWN;
    Floppy144LifecycleApply(&state,&event);
    Expect(
        state.shutdown_complete!=0U &&
        state.active==0U,
        "shutdown completes lifecycle state"
    );
}

/*
 * Verify the Win32 message adapter maps focus and orderly shutdown messages to
 * the same lifecycle concepts consumed by portable game code.
 */
static void TestWin32LifecycleTranslation(void)
{
    F144LifecycleEvent event;

    memset(
        &event,
        0,
        sizeof(event)
    );

    Expect(
        f144Win32TranslateLifecycleEvent(
            (uint32_t)WM_ACTIVATEAPP,
            (uintptr_t)TRUE,
            &event
        ) &&
        event.type == F144_LIFECYCLE_ACTIVE &&
        event.request_autosave == 0U,
        "Win32 activation maps to active lifecycle state"
    );

    Expect(
        f144Win32TranslateLifecycleEvent(
            (uint32_t)WM_ACTIVATEAPP,
            (uintptr_t)FALSE,
            &event
        ) &&
        event.type == F144_LIFECYCLE_INACTIVE,
        "Win32 deactivation maps to inactive lifecycle state"
    );

    Expect(
        f144Win32TranslateLifecycleEvent(
            (uint32_t)WM_CLOSE,
            0U,
            &event
        ) &&
        event.type == F144_LIFECYCLE_SHUTDOWN_REQUESTED,
        "Win32 close maps to orderly shutdown request"
    );

    Expect(
        f144Win32TranslateLifecycleEvent(
            (uint32_t)WM_DESTROY,
            0U,
            &event
        ) &&
        event.type == F144_LIFECYCLE_SHUTDOWN,
        "Win32 destroy maps to completed shutdown"
    );

    event.type =
        F144_LIFECYCLE_ACTIVE;

    Expect(
        !f144Win32TranslateLifecycleEvent(
            (uint32_t)WM_SIZE,
            0U,
            &event
        ) &&
        event.type == F144_LIFECYCLE_NONE,
        "unrelated Win32 messages do not become lifecycle events"
    );
}

int main(void)
{
    TestPlatformClockAndQuit();
    TestTimingProgression();
    TestAutosaveTiming();
    TestLifecycleState();
    TestWin32LifecycleTranslation();

    if(failures!=0)
    {
        printf(
            "STAGE 4 TIMING/LIFECYCLE TESTS: FAIL (%d)\n",
            failures
        );
        return 1;
    }

    printf("STAGE 4 TIMING/LIFECYCLE TESTS: PASS\n");
    return 0;
}
