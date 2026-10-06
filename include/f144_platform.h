/*
 * F144 platform contract
 *
 * This is the platform-neutral boundary seen by FLOPPY//144 game code.
 * Stage 4B starts with display/presentation and logical actions. Timing,
 * storage, lifecycle, quit handling and audio are added only when their owning
 * migration reaches them. Text input remains a separate native-to-core stream.
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

#define F144_PLATFORM_PATH_CAPACITY 512U

typedef enum F144PersistenceFile
{
    F144_PERSISTENCE_MANUAL_SAVE = 0,
    F144_PERSISTENCE_AUTOSAVE,
    F144_PERSISTENCE_PROFILE,
    F144_PERSISTENCE_SETTINGS,
    F144_PERSISTENCE_FILE_COUNT
} F144PersistenceFile;

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
} F144PlatformApi;

struct F144Platform
{
    const F144PlatformApi *api;
    void *state;
    Floppy144Surface surface;
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
