/*
 * F144 platform contract
 *
 * This is the platform-neutral boundary seen by FLOPPY//144 game code.
 * Stage 4B now covers display/presentation, logical input, persistent-storage
 * paths, semantic audio, monotonic timing and application lifecycle concepts.
 * Text input remains a separate native-to-core stream.
 *
 * Single-instance acquisition and raw command-line parsing are launcher-only
 * concerns and intentionally do not belong in this game-facing contract.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Logical input
 *
 * Native backends translate physical keys into these player-facing actions.
 * Text entry is intentionally not represented as gameplay actions.
 */
typedef enum F144Action
{
    F144_ACTION_NONE = 0,

    F144_ACTION_MOVE_UP,
    F144_ACTION_MOVE_DOWN,
    F144_ACTION_MOVE_LEFT,
    F144_ACTION_MOVE_RIGHT,

    F144_ACTION_NAV_UP,
    F144_ACTION_NAV_DOWN,
    F144_ACTION_ACCESS,
    F144_ACTION_INSPECT,
    F144_ACTION_CONFIRM,
    F144_ACTION_BACK,
    F144_ACTION_PAGE_UP,
    F144_ACTION_PAGE_DOWN,
    F144_ACTION_NOTEBOOK,
    F144_ACTION_MENU,

    F144_ACTION_COUNT
} F144Action;

typedef enum F144ActionEventType
{
    F144_ACTION_EVENT_DOWN = 0,
    F144_ACTION_EVENT_UP
} F144ActionEventType;

typedef struct F144ActionEvent
{
    F144Action action;
    F144ActionEventType type;

    /*
     * Opaque native identity used only to match one down/up pair. Core code
     * must never interpret this as a platform-specific key code.
     */
    uint32_t physical_token;
} F144ActionEvent;

/*
 * Text input remains distinct from logical gameplay actions. The current game
 * consumes ASCII/control characters, while the codepoint shape leaves room
 * for a later UTF-32-capable platform implementation without changing input
 * semantics again.
 */
typedef struct F144TextInputEvent
{
    uint32_t codepoint;
} F144TextInputEvent;

typedef enum F144LifecycleEventType
{
    F144_LIFECYCLE_NONE = 0,
    F144_LIFECYCLE_START,
    F144_LIFECYCLE_ACTIVE,
    F144_LIFECYCLE_INACTIVE,
    F144_LIFECYCLE_SUSPEND,
    F144_LIFECYCLE_RESUME,
    F144_LIFECYCLE_SHUTDOWN_REQUESTED,
    F144_LIFECYCLE_SHUTDOWN,
    F144_LIFECYCLE_COUNT
} F144LifecycleEventType;

typedef struct F144LifecycleEvent
{
    F144LifecycleEventType type;

    /*
     * Future mobile/platform policy may ask the game to make a safety autosave
     * while delivering a lifecycle event. Win32 does not request one today.
     */
    uint8_t request_autosave;
} F144LifecycleEvent;

#define F144_PLATFORM_PATH_CAPACITY 512U

typedef enum F144PersistenceFile
{
    F144_PERSISTENCE_MANUAL_SAVE = 0,
    F144_PERSISTENCE_AUTOSAVE,
    F144_PERSISTENCE_PROFILE,
    F144_PERSISTENCE_SETTINGS,
    F144_PERSISTENCE_FILE_COUNT
} F144PersistenceFile;

/*
 * Semantic audio
 *
 * Cue IDs describe game-owned/generated audio content. The platform owns only
 * playback. S4B-04 deliberately defines no shipping cues because the verified
 * Stage 4 baseline contains no music/SFX generator or playback backend yet.
 */
#define F144_AUDIO_VOLUME_MAX 10U

typedef uint16_t F144MusicCueId;
typedef uint16_t F144SfxCueId;

typedef struct Floppy144Surface
{
    uint32_t *pixels;
    uint32_t width;
    uint32_t height;
} Floppy144Surface;

typedef struct F144Platform F144Platform;

typedef struct F144PlatformApi
{
    Floppy144Surface *(*framebuffer)(F144Platform *platform);
    void (*present)(F144Platform *platform);

    bool (*persistence_path)(
        F144Platform *platform,
        F144PersistenceFile file,
        char *path,
        uint32_t path_capacity
    );

    bool (*legacy_persistence_path)(
        F144Platform *platform,
        F144PersistenceFile file,
        uint32_t candidate,
        char *path,
        uint32_t path_capacity
    );

    bool (*audio_init)(F144Platform *platform);
    void (*audio_shutdown)(F144Platform *platform);
    void (*music_play)(
        F144Platform *platform,
        F144MusicCueId cue
    );
    void (*music_stop)(F144Platform *platform);
    void (*sfx_play)(
        F144Platform *platform,
        F144SfxCueId cue
    );
    void (*music_volume)(
        F144Platform *platform,
        uint8_t volume
    );
    void (*sfx_volume)(
        F144Platform *platform,
        uint8_t volume
    );

    uint64_t (*monotonic_ms)(
        F144Platform *platform
    );

    void (*quit)(
        F144Platform *platform
    );
} F144PlatformApi;

struct F144Platform
{
    const F144PlatformApi *api;
    void *state;
    Floppy144Surface surface;

    uint8_t audio_initialized;
    uint8_t music_volume;
    uint8_t sfx_volume;
};

Floppy144Surface *f144PlatformFramebuffer(
    F144Platform *platform
);

void f144PlatformPresent(
    F144Platform *platform
);

bool f144PlatformPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
);

bool f144PlatformLegacyPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    uint32_t candidate,
    char *path,
    uint32_t path_capacity
);

bool f144PlatformAudioInit(
    F144Platform *platform
);

void f144PlatformAudioShutdown(
    F144Platform *platform
);

bool f144PlatformPlayMusic(
    F144Platform *platform,
    F144MusicCueId cue
);

bool f144PlatformStopMusic(
    F144Platform *platform
);

bool f144PlatformPlaySfx(
    F144Platform *platform,
    F144SfxCueId cue
);

bool f144PlatformSetMusicVolume(
    F144Platform *platform,
    uint8_t volume
);

bool f144PlatformSetSfxVolume(
    F144Platform *platform,
    uint8_t volume
);

uint64_t f144PlatformMonotonicMs(
    F144Platform *platform
);

void f144PlatformQuit(
    F144Platform *platform
);
