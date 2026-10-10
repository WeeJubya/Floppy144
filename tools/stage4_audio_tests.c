/*
 * FLOPPY//144 Stage 4B audio-platform contract regression.
 *
 * Verifies the semantic boundary, original procedural MIDI and F144SFX,
 * and the WinMM adapters with an optional real device. Device absence is safe.
 */

#include "f144_platform.h"
#include "f144_win32_audio.h"
#include "floppy144_settings.h"
#include "floppy144_audio.h"
#include "f144audio.h"
#include "f144sfx.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

_Static_assert(
    F144_AUDIO_VOLUME_MAX == FLOPPY144_SETTINGS_VOLUME_MAX,
    "Platform and persisted settings volume scales must remain aligned"
);

typedef struct TestAudioState
{
    bool init_result;
    bool music_active;

    uint32_t init_calls;
    uint32_t shutdown_calls;
    uint32_t music_play_calls;
    uint32_t music_stop_calls;
    uint32_t sfx_play_calls;
    uint32_t music_volume_calls;
    uint32_t sfx_volume_calls;

    F144MusicCueId last_music_cue;
    F144SfxCueId last_sfx_cue;
    uint8_t last_music_volume;
    uint8_t last_sfx_volume;
} TestAudioState;

static int failures;

static void Expect(bool condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

static bool TestAudioInit(F144Platform *platform)
{
    TestAudioState *state=(TestAudioState *)platform->state;
    ++state->init_calls;
    return state->init_result;
}

static void TestAudioShutdown(F144Platform *platform)
{
    TestAudioState *state=(TestAudioState *)platform->state;
    ++state->shutdown_calls;
    state->music_active=false;
}

static void TestMusicPlay(F144Platform *platform,F144MusicCueId cue)
{
    TestAudioState *state=(TestAudioState *)platform->state;
    ++state->music_play_calls;
    state->last_music_cue=cue;
    state->music_active=true;
}

static void TestMusicStop(F144Platform *platform)
{
    TestAudioState *state=(TestAudioState *)platform->state;
    ++state->music_stop_calls;
    state->music_active=false;
}

static void TestSfxPlay(F144Platform *platform,F144SfxCueId cue)
{
    TestAudioState *state=(TestAudioState *)platform->state;
    ++state->sfx_play_calls;
    state->last_sfx_cue=cue;
}

static void TestMusicVolume(F144Platform *platform,uint8_t volume)
{
    TestAudioState *state=(TestAudioState *)platform->state;
    ++state->music_volume_calls;
    state->last_music_volume=volume;
}

static void TestSfxVolume(F144Platform *platform,uint8_t volume)
{
    TestAudioState *state=(TestAudioState *)platform->state;
    ++state->sfx_volume_calls;
    state->last_sfx_volume=volume;
}

static const F144PlatformApi test_audio_api =
{
    NULL,
    NULL,
    NULL,
    NULL,
    TestAudioInit,
    TestAudioShutdown,
    TestMusicPlay,
    TestMusicStop,
    TestSfxPlay,
    TestMusicVolume,
    TestSfxVolume
};

static void TestSuccessfulLifecycle(void)
{
    TestAudioState state;
    F144Platform platform;

    memset(&state,0,sizeof(state));
    memset(&platform,0,sizeof(platform));

    state.init_result=true;
    platform.api=&test_audio_api;
    platform.state=&state;

    Expect(!f144PlatformPlayMusic(&platform,10U),"music cannot play before init");
    Expect(!f144PlatformPlaySfx(&platform,20U),"SFX cannot play before init");
    Expect(!f144PlatformStopMusic(&platform),"music cannot stop before init");
    Expect(!f144PlatformSetMusicVolume(&platform,0U),"volume cannot apply before init");

    Expect(f144PlatformAudioInit(&platform),"audio initialises");
    Expect(
        state.init_calls==1U&&platform.audio_initialized!=0U,
        "audio init state is retained"
    );
    Expect(
        platform.music_volume==F144_AUDIO_VOLUME_MAX&&
        platform.sfx_volume==F144_AUDIO_VOLUME_MAX,
        "audio initialises at current default volume"
    );
    Expect(
        f144PlatformAudioInit(&platform)&&state.init_calls==1U,
        "repeated init is idempotent"
    );

    Expect(
        f144PlatformSetMusicVolume(&platform,0U)&&
        state.music_volume_calls==1U&&
        state.last_music_volume==0U&&
        platform.music_volume==0U,
        "music mute volume is supported"
    );
    Expect(
        f144PlatformSetSfxVolume(&platform,0U)&&
        state.sfx_volume_calls==1U&&
        state.last_sfx_volume==0U&&
        platform.sfx_volume==0U,
        "SFX mute volume is supported"
    );

    Expect(
        f144PlatformPlayMusic(&platform,7U)&&
        state.music_play_calls==1U&&
        state.last_music_cue==7U&&
        state.music_active,
        "music starts"
    );

    Expect(
        f144PlatformPlaySfx(&platform,3U)&&
        f144PlatformPlaySfx(&platform,3U)&&
        state.sfx_play_calls==2U&&
        state.last_sfx_cue==3U,
        "repeated SFX triggering works"
    );
    Expect(state.music_active,"SFX coexists with music");

    Expect(
        f144PlatformStopMusic(&platform)&&
        state.music_stop_calls==1U&&
        !state.music_active,
        "music stops"
    );

    Expect(
        f144PlatformSetMusicVolume(&platform,F144_AUDIO_VOLUME_MAX)&&
        f144PlatformSetSfxVolume(&platform,F144_AUDIO_VOLUME_MAX),
        "maximum/current default volumes are accepted"
    );

    {
        uint32_t music_calls=state.music_volume_calls;
        uint32_t sfx_calls=state.sfx_volume_calls;

        Expect(
            !f144PlatformSetMusicVolume(
                &platform,
                (uint8_t)(F144_AUDIO_VOLUME_MAX+1U)
            )&&
            !f144PlatformSetSfxVolume(
                &platform,
                (uint8_t)(F144_AUDIO_VOLUME_MAX+1U)
            )&&
            state.music_volume_calls==music_calls&&
            state.sfx_volume_calls==sfx_calls,
            "out-of-range volume is rejected"
        );
    }

    f144PlatformAudioShutdown(&platform);
    Expect(
        state.shutdown_calls==1U&&platform.audio_initialized==0U,
        "audio shuts down cleanly"
    );

    f144PlatformAudioShutdown(&platform);
    Expect(state.shutdown_calls==1U,"repeated shutdown is harmless");
}

static void TestInitialisationFailure(void)
{
    TestAudioState state;
    F144Platform platform;

    memset(&state,0,sizeof(state));
    memset(&platform,0,sizeof(platform));

    state.init_result=false;
    platform.api=&test_audio_api;
    platform.state=&state;

    Expect(
        !f144PlatformAudioInit(&platform)&&
        state.init_calls==1U&&
        platform.audio_initialized==0U,
        "backend init failure remains non-initialised"
    );
    Expect(
        !f144PlatformPlayMusic(&platform,1U)&&
        !f144PlatformPlaySfx(&platform,1U),
        "failed backend stays safely silent"
    );

    f144PlatformAudioShutdown(&platform);
    Expect(
        state.shutdown_calls==0U,
        "failed init does not run backend shutdown"
    );
}

static void TestPersistedDefaultVolumes(void)
{
    Floppy144Settings settings;

    Floppy144SettingsReset(&settings);

    Expect(
        settings.music_volume==F144_AUDIO_VOLUME_MAX,
        "persisted default music volume matches platform maximum"
    );
    Expect(
        settings.sfx_volume==F144_AUDIO_VOLUME_MAX,
        "persisted default SFX volume matches platform maximum"
    );
}

static void TestOriginalGeneratedAudio(void)
{
    Expect(F144_SFX_COUNT==22, "original fourteen + eight procedural SFX");
    F144_MusicSetAct(F144_ACT_I);
    Expect(F144_MusicGetAct()==F144_ACT_I,"Act I selection retains original API");
    F144_MusicSetAct(F144_ACT_III);
    Expect(F144_MusicGetAct()==F144_ACT_III,"Act III selection");
    F144_MusicSetSeed(0x31415926U);
    Expect(F144_MusicGetSeed()==0x31415926U,"deterministic music seed");
    F144_MusicSetRestoration(0U);
    Expect(F144_MusicGetDistortion()==0U,"0% restoration is clean");
    F144_MusicSetRestoration(29U);
    Expect(F144_MusicGetDistortion()==9U,"floor(29/3)");
    F144_MusicSetRestoration(100U);
    Expect(F144_MusicGetDistortion()==33U,"floor(100/3)");
    F144_MusicSetRestoration(144U);
    Expect(F144_MusicGetDistortion()==48U,"Grey Door 144% supports distortion");
}

static void TestWin32Backend(void)
{
    F144Platform platform;
    F144PlatformApi api =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        f144Win32AudioInit,
        f144Win32AudioShutdown,
        f144Win32AudioPlayMusic,
        f144Win32AudioStopMusic,
        f144Win32AudioPlaySfx,
        f144Win32AudioSetMusicVolume,
        f144Win32AudioSetSfxVolume
    };

    memset(&platform,0,sizeof(platform));
    platform.api=&api;

    /* Headless Windows CI may lack audio devices; that is never fatal. */
    if(f144PlatformAudioInit(&platform))
    {
        Expect(f144PlatformSetMusicVolume(&platform,5U) &&
               f144PlatformSetSfxVolume(&platform,3U),
               "original backend accepts separate persisted volumes");
        Expect(f144PlatformPlayMusic(&platform,1U) &&
               f144PlatformPlaySfx(&platform,F144_SFX_TELEPHONE),
               "original v0.9 phone and WinMIDI requests are accepted");
        f144PlatformAudioUpdate(&platform,1000U,35U);
        f144PlatformAudioUpdate(&platform,1230U,62U);
        Expect(f144PlatformSetMusicVolume(&platform,0U) &&
               f144PlatformSetSfxVolume(&platform,0U),
               "independent mute controls are supported");
        Expect(f144PlatformStopMusic(&platform),"MIDI stop request works");
        f144PlatformAudioShutdown(&platform);
    }
    else
    {
        Expect(!f144PlatformPlayMusic(&platform,1U) &&
               !f144PlatformPlaySfx(&platform,F144_SFX_TERMINAL_KEY),
               "missing MIDI and waveOut devices remain safely silent");
    }
    Expect(platform.audio_initialized==0U,"native audio stops cleanly");
}

int main(void)
{
    TestSuccessfulLifecycle();
    TestInitialisationFailure();
    TestPersistedDefaultVolumes();
    TestOriginalGeneratedAudio();
    TestWin32Backend();

    if(failures!=0)
    {
        printf("STAGE 4 AUDIO CONTRACT TESTS: FAIL (%d)\n",failures);
        return 1;
    }

    printf("STAGE 4 AUDIO CONTRACT TESTS: PASS\n");
    return 0;
}
