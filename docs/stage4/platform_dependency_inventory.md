# Stage 4 Platform Dependency / Architecture Inventory

**Audit date:** 2026-10-06  
**Branch:** `Stage4`  
**Audited HEAD:** `772d83f536c73192b6026144cb650aa3276bb698`  
**Purpose:** Map the current Windows/platform coupling before Stage 4B. No refactor is performed by this document.

## Executive summary

FLOPPY//144 is closer to a portable core than the current source layout suggests.

Most gameplay, generated-data interpretation, world/site state, terminal logic, catalogue logic, notebook logic, triggers, interactions, profile/settings structures, save encoding, and software drawing primitives are already platform-neutral C.

The principal coupling is concentrated in four areas:

1. **`game/src/floppy144_main.c`** mixes the platform launcher/message loop with the game coordinator.
2. **`game/src/floppy144_persistence.c`** mixes platform-neutral binary encoding with Win32 file replacement and MSVC-specific file opening.
3. **the current F144 runtime** mixes a generic framebuffer structure with Win32 handles, timing, cursor and presentation code.
4. **screen draw entry points** accept `F144Runtime *` even though they normally use only its backbuffer pointer, width and height.

The existing `Floppy144Surface` in `floppy144_draw.h` is already the correct core rendering boundary. The safest Stage 4B architecture is therefore to make the core render into a caller-provided 640x360 `Floppy144Surface`, while a narrow F144 platform interface owns window/display presentation, native input translation, storage, monotonic time, lifecycle, audio and platform-only startup concerns.

This inventory is also constrained by the Stage 4 provenance audit. The new interface should be source-lineage-neutral and should **not** spread the current River2D-derived F144 implementation into additional platform backends. Whether the Win32 implementation temporarily wraps the legacy runtime or replaces it should remain a separately agreed implementation decision.

## Current build architecture

`premake5.lua` currently defines:

### `F144 Runtime` static library

Compiled from:

- `src/f144_runtime.c`
- `src/f144_win32_runtime.c`
- `src/string_view.c`
- `include/f144_runtime.h`
- `include/string_view.h`

### `Floppy144` Windows executable

Compiled from:

- every `game/src/**.c`
- every `game/src/**.h`
- `src/f144_win32_platform.c`
- `include/f144_win32_platform.h`

Linked against:

- `F144 Runtime`
- `user32`
- `gdi32`

The Premake workspace currently exposes only the `windows` platform and x86-64 architecture.

The current Stage 3C CI workflow is also Windows-only and is configured to run automatically on pushes to `development/stage-3c-complete`, not `Stage4`. It can be manually dispatched, but Stage 4 does not currently have an automatic regression gate.

## Desired ownership model

### Floppy144 Core

The core should own:

- application/game screen state;
- recovery/session state;
- world and Site state;
- terminal state and commands;
- catalogue/document behaviour;
- notebook state;
- cabinet/contents behaviour;
- triggers/interactions/evidence;
- profile and settings data structures;
- persistence encoding/decoding and validation;
- logical action handling;
- terminal text handling;
- timer semantics and deterministic state updates;
- rendering all screens into a 640x360 `Floppy144Surface`;
- decisions such as "quit requested", "redraw required" or "audio cue requested".

The core should not contain Win32 handles, message IDs, virtual-key constants, AppData discovery, GDI calls, native audio APIs or OS-specific file replacement.

### F144 Platform Layer

The platform contract should own only services the core cannot provide portably:

- creation/ownership of the native display/window;
- ownership of the physical framebuffer memory supplied to the core;
- presentation/scaling/letterboxing;
- translation of native keyboard events to logical actions;
- text-input events;
- monotonic clock;
- persistent-storage file access under the platform-selected user-data directory;
- audio execution;
- lifecycle events;
- quit request execution;
- startup/debug configuration supplied by the launcher.

### Platform implementation

Initial implementation:

- `Floppy144PlatformWin32`

Future implementations can target SDL3, Linux, macOS, iOS or Android without duplicating core gameplay.

No future platform is required to build as part of S4A-04.

## Direct dependency inventory

| Source / component | Current responsibility | Current platform dependency | Correct owner | Recommended Stage 4B destination | Risks / dependencies | Existing regression protection |
| --- | --- | --- | --- | --- | --- | --- |
| `game/src/floppy144_main.c` top-level includes | Game coordinator and Win32 launcher in one file | Direct `<windows.h>`, F144 Win32 headers | Split Core / Win32 | Core coordinator in new core module; native entry point in Win32 launcher | Largest extraction seam | Full Stage 2/3A/3B suite plus release build |
| `global_runtime` | Gives every screen access to native runtime/backbuffer | `F144Runtime *` contains Win32 state | Platform/Core boundary | Replace screen dependency with caller-provided `Floppy144Surface` | Must keep pixel format and 640x360 dimensions exact | Stage 3B rendering audits, terminal tests, cabinet tests |
| `global_reinstate_continue_key` | Matches the acknowledgement key-down with its key-up | Type is `WPARAM` | Core state using platform-neutral token | `uint32_t input_token` or equivalent opaque token | Reinstate flow depends on matching the same press/release | Main Menu Reinstate Flow Audit |
| `global_splash_started_ticks` | Splash elapsed-time origin | Type `DWORD`; uses `GetTickCount` | Core timing state | Platform-neutral millisecond timestamp | Preserve animation duration | Release/manual visual check; Stage 3 baseline |
| `Floppy144BindStaticRenderer` | Assigns Win32 renderer callbacks into `F144Runtime` | Direct F144 Win32 implementation symbols | Win32 implementation only, then removable | Platform creates framebuffer/presenter directly | Provenance-sensitive legacy runtime seam | Release build and size gate |
| `Floppy144Redraw` | Selects active game renderer and requests presentation | Takes `HWND`, calls `GetTickCount`, `InvalidateRect` | Split Core / Platform | `Floppy144CoreRender(surface, now_ms)`; platform presents afterwards | Do not make individual screens know native window state | Full Stage 3B visual/source audits |
| `Floppy144PersistenceFileExists` in `main.c` | Detects manual/autosave files | `GetFileAttributesA` | Platform storage | Platform storage `exists` service | Must preserve corrupt-file warning semantics | Main Menu record/reinstate audits |
| Session initiation in `Floppy144MainMenuActivate` | Generates recovery seed | `GetTickCount` | Core using injected monotonic time | Derive non-zero seed from platform time supplied to core | Seed affects deterministic content/order | Stage 3A/3B gameplay regressions |
| Main-menu terminate action | Requests application shutdown | `PostMessageA(... WM_CLOSE ...)` | Core request + platform execution | Core returns/sets `quit_requested`; launcher closes | Preserve profile update on orderly quit | Main menu/source audit plus new lifecycle test |
| `Floppy144MovePlayer(HWND,...)` | Core movement and exterior-exit transition | `HWND` exists only so helper can redraw/open menu | Core | Remove native window parameter | Must preserve exterior exit handoff | Stage 3B.4 Coordinator Wiring + Door/Access |
| `Floppy144InteractOffice(HWND,...)` | Access/Inspect coordinator | `HWND` exists only for redraw | Core | Remove native window parameter | High gameplay importance, low true platform dependence | Stage 3B.2, Stage 3B.4, Stage 3B.5 |
| `Floppy144OpenMainMenu(HWND)` | Core session/screen transition | `HWND` exists only for redraw | Core | Remove native window parameter | Resume-screen behaviour must remain exact | Stage 3B coordinator/main-menu audits |
| `Floppy144WindowProc` | Native events plus almost all input routing | `HWND`, `UINT`, `WPARAM`, `LPARAM`, `WM_*`, `VK_*` | Split Core / Win32 | Win32 translates messages; core handles logical events | Highest behavioural refactor risk | Stage 3A, Stage 3B.1/2/4/5, main-menu audits |
| `WM_CHAR` handler | Terminal text, pager controls, cabinet keypad digits | Win32 text-message semantics | Core text event fed by Platform | Logical text event plus action controls | Avoid double-processing Enter/Backspace during transition | Stage 3B.1 Terminal, Stage 3B.5 Cabinet |
| `WM_KEYDOWN` handler | Movement, menu/nav, Access, Inspect, Notebook, Confirm, Back, paging | `VK_*` and literal Win32 key values | Core logical actions | Platform maps native keys to `F144Action`; core routes by screen | Windows auto-repeat is currently not filtered | All interaction/navigation regressions; add input-map test |
| `WM_KEYUP` handler | Reinstate acknowledgement completion | Native key release and `WPARAM` matching | Core logical release event | Action/key token release event | This is the only current gameplay use of key-up | Main Menu Reinstate Flow Audit |
| `WM_TIMER` handler | Splash redraw, terminal restore, cursor blink, autosave | Windows timers and IDs | Core timing semantics, Platform clock/wake | Deterministic core update driven by monotonic time | Timing changes can alter restore UX and autosave cadence | Stage 3B.1 restoration tests; persistence tests; new tick tests |
| `WM_PAINT` | Presents software framebuffer | `BeginPaint`, HDC, runtime context, blit callback | Win32 implementation | `F144PlatformPresent` | Must preserve aspect ratio/crisp scaling | Release/manual visual smoke |
| `WM_SIZE` / `WM_ERASEBKGND` | Native resize/repaint behaviour | Win32 paint lifecycle | Win32 implementation | Remain native | Avoid flicker regressions | Manual resize/presentation smoke |
| `WM_CLOSE` / `WM_DESTROY` | Shutdown, timer teardown, profile merge/save | Win32 lifecycle | Split Core / Win32 | Core orderly-shutdown callback; platform tears native resources down | Current profile update occurs on `WM_CLOSE`, not generic focus/suspend | Add lifecycle regression |
| `Floppy144CommandLineHasSwitch` | Parses only `-debug` | WinMain raw command-line string | Win32 launcher/startup config | Launcher parses; core receives `debug_guidance` boolean | Preserve debug-only terminal breadcrumbs | Terminal behaviour plus new startup-config test |
| `WinMain` | Window registration, creation, runtime config, load profile/settings, event loop | Entire Win32 application lifecycle | Split Core / Win32 | Minimal Win32 launcher plus `Floppy144CoreInit` | Large but mostly mechanical split | Full suite, launch smoke, release size |
| `game/src/floppy144_persistence.c` | Binary codec plus file IO | Includes `windows.h`; `MoveFileExA`, `DeleteFileA`, `MAX_PATH`, `fopen_s` | Codec Core, IO Platform | Keep encode/decode in Core; platform owns read/exists/atomic-write | Atomic replacement semantics must be preserved | Stage 3A persistence, Stage 3B.3, Stage 3B.5 |
| Four filename constants in `main.c` | Manual save, autosave, profile, settings locations | Bare relative filenames resolve against process CWD | Platform storage | Retain leaf names, root them under platform user-data dir | Migration must not strand existing players | Main-menu save/reinstate plus new migration tests |
| `include/f144_runtime.h` | Generic runtime/backbuffer plus native fields | Contains Win32 handles and dormant Linux/X11 fields | Legacy platform/runtime | Do not expose to Core after extraction | Provenance issue from S4A-03; dormant Linux fields are not Linux support | Release build |
| `src/f144_win32_runtime.c` | Image allocation, QPC time, cursor utilities, wrapper functions | Win32 performance counter and cursor/GDI types | Win32 implementation or replacement | Keep only facilities actually needed by new Win32 platform | Much of API is unused by game layer | Release build |
| `src/f144_win32_platform.c` | Software framebuffer/presentation/text/composite helpers | GDI, VirtualAlloc, HDC, cursor/window calls | Win32 implementation or replacement | Presenter/backbuffer implementation behind F144 Platform API | Provenance-sensitive; preserve hard-edged scaling | Release/manual presentation smoke |
| Screen draw functions in Cabinet, Catalogue, Notebook, Recovery, Site 2D, Site Directory, Site ISO, Terminal and completion screen | Draw UI/game image | Accept `F144Runtime *` but generally only read backbuffer pointer/size | Core | Accept `Floppy144Surface *` directly | Broad signature change, mechanically simple | Stage 3B rendering tests and full suite |
| `floppy144_draw.c/.h` | Pixel/text primitives | None | Core | Keep unchanged as fundamental render API | Pixel format must remain documented | Terminal glyph test and many rendering regressions |
| `floppy144_settings.c/.h` | CRT/text/audio/autosave settings data | None | Core | Keep structures in Core | Only autosave currently has active runtime effect | Persistence codec tests should be expanded |
| Current audio | No playback backend exists | None | Platform API not yet exercised | Future Platform | Add semantic cue boundary when audio work begins | No current audio regression |
| Current focus/pause/resume | No explicit handling exists | No `WM_ACTIVATE`, `WM_SETFOCUS`, `WM_KILLFOCUS` handling | Future lifecycle contract | Introduce explicit events without inventing gameplay side effects | Mobile suspend policy requires later decision | New lifecycle tests required |
| Current single-instance behaviour | No implementation exists | No mutex/window discovery code | Platform only | Add late as Win32 launcher concern | Entirely additive; should never enter Core | New launcher smoke/test |
| `premake5.lua` | Windows-only two-target build | `platforms({"windows"})`, Win32 source linked in executable | Build architecture | Three targets: Core, PlatformWin32, launcher | Do after code seams are real to avoid fake library boundaries | Full suite + release build + size gate |

## Direct Win32 surface area

The direct Win32/API coupling found by this audit is concentrated in:

- `game/src/floppy144_main.c`
- `game/src/floppy144_persistence.c`
- `include/f144_runtime.h`
- `include/f144_win32_platform.h`
- `src/f144_win32_runtime.c`
- `src/f144_win32_platform.c`
- `premake5.lua`

The rest of the gameplay tree is largely platform-neutral.

Several game presentation headers include `f144_runtime.h` solely because their draw function accepts `F144Runtime *`. They do not use native Win32 APIs themselves.

## Rendering boundary

The game already defines the appropriate platform-neutral surface:

```c
typedef struct Floppy144Surface
{
    uint32_t *pixels;
    uint32_t width;
    uint32_t height;
} Floppy144Surface;
```

Current game drawing writes packed `0x00RRGGBB` pixels into a logical 640x360 surface.

The Win32 presenter then:

- reads the software framebuffer;
- fits it to the client area while preserving aspect ratio;
- uses GDI `StretchDIBits`;
- uses `COLORONCOLOR` for hard-edged scaling;
- clears only unused letterbox/pillarbox bands;
- does not change core geometry when the window resizes.

### Recommended Stage 4B rendering split

Core:

```text
Floppy144CoreRender(core, Floppy144Surface*, now_ms)
```

Platform:

```text
acquire fixed 640x360 surface
        -> Core renders pixels
        -> platform presents/scales native window
```

All current screen draw entry points should eventually take `Floppy144Surface *` rather than `F144Runtime *`.

That change removes the F144 runtime header from most game presentation modules without altering the renderer itself.

## Input inventory

### Current Win32 input path

`GetMessageA -> TranslateMessage -> DispatchMessageA -> Floppy144WindowProc`

Two different native streams matter:

- `WM_KEYDOWN/WM_KEYUP` for navigation/actions;
- `WM_CHAR` for terminal text, terminal pager character controls and cabinet digits.

`l_param` is currently ignored, so Win32 key auto-repeat is not explicitly filtered.

### Existing screen behaviour

| Screen/context | Existing native keys | Proposed logical meaning |
| --- | --- | --- |
| Splash | Enter | `CONFIRM` |
| Splash / ordinary screens | Escape | `MENU` |
| Main menu | Up or W | `UP` / `NAV_UP_ALIAS` |
| Main menu | Down or S | `DOWN` / `NAV_DOWN_ALIAS` |
| Main menu | Enter | `CONFIRM` |
| Site exploration | Left/Right/Up/Down arrows | `MOVE_LEFT/RIGHT/UP/DOWN` |
| Site exploration | A | `ACCESS` |
| Site exploration | I | `INSPECT` |
| Site exploration | N | `NOTEBOOK` |
| Site Directory | any subsequent key-down | `ANY_KEY` close overlay |
| Cabinet/contents | Up/Down | `NAV_UP/NAV_DOWN` |
| Cabinet/contents | Enter | `CONFIRM` |
| Cabinet/contents | I | `INSPECT` |
| Cabinet/contents | Backspace | `BACK` |
| Cabinet keypad | digit characters | text/digit input |
| Notebook | Up/Down | `NAV_UP/NAV_DOWN` |
| Notebook | Page Up/Page Down | `PAGE_UP/PAGE_DOWN` |
| Notebook | N or Backspace | `BACK` |
| Completion screen | Up/Down | `NAV_UP/NAV_DOWN` |
| Completion screen | Page Up/Page Down | `PAGE_UP/PAGE_DOWN` |
| Completion screen | Enter | `CONFIRM` |
| Terminal normal mode | printable ASCII via `WM_CHAR` | text input |
| Terminal normal mode | Backspace/Enter via `WM_CHAR` | editing/submit control |
| Terminal normal mode | Up/Down | history previous/next |
| Terminal HELP/LIST pager | Space/Enter | next page |
| Terminal HELP/LIST pager | Backspace | previous page |
| Terminal HELP/LIST pager | Q/q | close pager |
| Catalogue list | Up/W, Down/S | navigation |
| Catalogue open document | Up/Down | document line scroll |
| Catalogue | Page Up/Page Down | page navigation |
| Catalogue | Enter | open selected document |
| Catalogue | Backspace | close document/back |
| Reinstate confirmation | any key-down then matching key-up | dismiss/reveal/continue handshake |

### Important W/S rule

W/S are currently aliases only in menu/catalogue-style navigation.

They are intentionally **not** movement controls in Site exploration because A is reserved for Access. The logical-action bridge must preserve that distinction.

The cleanest mapping is therefore not a single global "Up" action for every physical key.

Recommended logical actions:

```c
typedef enum F144Action
{
    F144_ACTION_NONE = 0,

    F144_ACTION_MOVE_LEFT,
    F144_ACTION_MOVE_RIGHT,
    F144_ACTION_MOVE_UP,
    F144_ACTION_MOVE_DOWN,

    F144_ACTION_NAV_UP,
    F144_ACTION_NAV_DOWN,
    F144_ACTION_PAGE_UP,
    F144_ACTION_PAGE_DOWN,

    F144_ACTION_ACCESS,
    F144_ACTION_INSPECT,
    F144_ACTION_CONFIRM,
    F144_ACTION_BACK,
    F144_ACTION_NOTEBOOK,
    F144_ACTION_MENU
} F144Action;
```

Win32 mapping:

- arrow keys can map to movement actions when Site owns input and navigation actions elsewhere, or the platform can emit a neutral arrow token and the core can map by current screen;
- W/S map only to navigation aliases;
- A, I, N, Enter, Backspace, Escape and paging keys map directly.

The preferred design is for the **platform to translate native key identity but not game-screen policy**. The core knows whether an arrow means Site movement or menu navigation.

### Text input must remain separate

Terminal command text must not be represented as gameplay actions.

Recommended event shape:

```c
typedef enum F144InputEventType
{
    F144_INPUT_ACTION_DOWN,
    F144_INPUT_ACTION_UP,
    F144_INPUT_TEXT
} F144InputEventType;

typedef struct F144InputEvent
{
    F144InputEventType type;
    F144Action action;

    /* Stable only for one press/release pair; the Core never interprets it. */
    uint32_t physical_token;

    /* Valid for F144_INPUT_TEXT. ASCII first; future UTF-32 capable. */
    uint32_t codepoint;
} F144InputEvent;
```

The opaque `physical_token` is required because the current reinstate screen accepts **any** key and waits for the matching key-up. A logical action alone cannot represent printable keys that have no gameplay action.

During Stage 4B, Enter and Backspace should be normalised carefully so one native key press does not create both an action and a text-control event that executes twice.

## Timing inventory

### Active game timing

The active game currently uses:

- `GetTickCount` for splash elapsed time;
- `GetTickCount` as the source of a non-zero recovery seed;
- Win32 `SetTimer` at **16 ms** for splash animation redraw;
- Win32 `SetTimer` at **50 ms** for terminal restoration progression;
- Win32 `SetTimer` at **500 ms** for terminal cursor blink;
- Win32 `SetTimer` at settings-selected autosave interval:
  - 5 minutes default;
  - 10 minutes;
  - 30 minutes;
  - off.

### Legacy runtime timing

`src/f144_win32_runtime.c` implements `f144QueryTime` using `QueryPerformanceCounter`, with delta helpers in `src/f144_runtime.c`.

No current FLOPPY//144 game module calls those F144 time helpers. They are therefore not required in the minimal new Core API merely because they exist in the legacy runtime.

### Recommended timing boundary

Expose one platform service:

```c
uint64_t f144_platform_monotonic_ms(void *platform);
```

Move timer *semantics* into the Core.

A future:

```text
Floppy144CoreUpdate(core, now_ms)
```

should maintain splash, restore, cursor and autosave deadlines.

For initial parity, terminal restoration should retain the current 50 ms quantised progression rather than silently switching to unconstrained frame-delta behaviour.

## Persistence inventory

### Current location behaviour

The current filenames are:

- `floppy144_manual.sav`
- `floppy144_auto.sav`
- `floppy144_profile.dat`
- `floppy144_settings.dat`

They are bare relative paths.

The current code does **not** discover the executable directory.

Therefore these files currently resolve relative to the **process current working directory**.

### Current IO behaviour

The persistence module already has a useful separation internally:

Platform-neutral logic:

- fixed format constants;
- run-state encode/decode;
- profile encode/decode;
- settings encode/decode;
- checksums;
- header/version validation.

Platform-bound logic:

- `fopen_s`;
- `MAX_PATH`;
- `DeleteFileA`;
- `MoveFileExA(... MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`.

The atomic-write intent is important and must survive the refactor.

### Stage 4B Windows destination

Required Windows storage root:

```text
%APPDATA%\Floppy144
```

Leaf filenames should remain unchanged unless a later schema requirement explicitly changes them.

Recommended storage API:

```c
bool f144_platform_storage_exists(
    void *platform,
    const char *leaf_name
);

bool f144_platform_storage_read(
    void *platform,
    const char *leaf_name,
    void *buffer,
    uint32_t capacity,
    uint32_t *bytes_read
);

bool f144_platform_storage_write_atomic(
    void *platform,
    const char *leaf_name,
    const void *buffer,
    uint32_t size
);
```

The core persistence module should continue to own the exact bytes and validation rules.

The platform owns where those bytes live and how atomic replacement is implemented.

### Legacy migration requirement

A move to AppData without migration would make existing Stage 3 saves appear to vanish.

Recommended one-time Windows migration policy:

1. resolve/create `%APPDATA%\Floppy144`;
2. for each new storage file that does not already exist, probe the legacy bare filename in the launch working directory;
3. validate the legacy file with the existing Stage 3 codec before accepting it;
4. copy/write it atomically into AppData;
5. leave the legacy file in place during the first migration release rather than destructively moving it;
6. use AppData exclusively after successful migration.

Migration tests must cover valid, corrupt, missing and conflicting old/new files.

## Audio inventory

There is currently **no playback implementation** in the Stage 4 baseline.

The repository currently contains:

- settings fields for music volume;
- settings fields for SFX volume;
- persistence for those settings.

It does not currently contain:

- WinMIDI playback;
- WinMM linkage;
- `PlaySound`;
- `waveOut`;
- MCI calls;
- an external audio library;
- embedded shipping music/SFX data.

Therefore game logic currently knows **nothing** about WinMIDI or Windows audio details.

This changes the Stage 4B order: audio abstraction is additive, not an extraction prerequisite.

### Recommended future audio boundary

Core should request semantic behaviour only:

```text
play/replace music cue
stop music
play one SFX cue
apply music/SFX volume
```

The platform implementation decides whether that means MIDI, generated audio, SDL audio, native mobile audio or another backend.

The core must never know about MIDI handles, WinMM messages, device IDs, sound buffers or platform callback types.

Do not introduce a broad mixer/streaming engine API until the game actually needs one.

## Lifecycle inventory

### What exists

Current lifecycle handling is limited to:

- `WM_CLOSE`;
- `WM_DESTROY`;
- the Win32 message loop;
- runtime shutdown after `GetMessage` exits.

On `WM_CLOSE`, the game:

- marks runtime not running;
- merges/saves discovery profile as needed;
- kills timers;
- posts quit.

### What does not exist

There is no explicit handling of:

- focus gained/lost;
- application activation;
- pause/resume;
- mobile suspend/background;
- OS session-end;
- low-memory notification.

That absence is part of the Stage 3 baseline. Stage 4B should introduce lifecycle events as a platform contract but should **not invent gameplay effects** for focus loss or suspend without a separate requirement.

Recommended lifecycle events:

```text
START
SUSPEND
RESUME
QUIT_REQUESTED
```

For the first Win32 implementation, only orderly quit needs to reproduce existing behaviour. Suspend/resume can initially be no-op core notifications until mobile policy is defined.

## Single-instance inventory

No single-instance implementation exists in the current repository.

There is no current mutex, named event, existing-window search or second-launch handoff.

Single-instance support is therefore an additive **platform-launcher feature**, not something to extract from game logic.

Recommended contract:

```text
platform launcher attempts to acquire the FLOPPY//144 instance lock
success -> create Core/window
already running -> platform-specific existing-instance behaviour
```

The Core should not know whether an instance lock exists.

## Debug/startup configuration

The current command-line parser exists only in `floppy144_main.c`.

It tokenises the WinMain command line on spaces/tabs and checks for exact `-debug`.

The resulting boolean is passed into:

`Floppy144TerminalConfigureSession(..., debug_guidance, ...)`

Debug guidance is therefore a **Core configuration value**, while command-line parsing is a **launcher responsibility**.

Recommended startup struct:

```c
typedef struct Floppy144StartupConfig
{
    bool debug_guidance;
} Floppy144StartupConfig;
```

Future platforms can source the same flag from a developer setting, launch argument or build configuration without the Core knowing how it arrived.

## Proposed minimal F144 Platform API

The API should be intentionally smaller than the current `F144Runtime`.

The game does not currently need a generic image engine, cursor-image API, renderer plug-in mechanism, arbitrary compositing service or the legacy F144 timing helper family.

A practical contract is:

```c
typedef struct F144Platform F144Platform;

typedef struct F144PlatformApi
{
    /* Display */
    Floppy144Surface *(*framebuffer)(F144Platform *);
    void (*present)(F144Platform *);

    /* Timing */
    uint64_t (*monotonic_ms)(F144Platform *);

    /* Persistence */
    bool (*storage_exists)(
        F144Platform *,
        const char *leaf_name
    );

    bool (*storage_read)(
        F144Platform *,
        const char *leaf_name,
        void *buffer,
        uint32_t capacity,
        uint32_t *bytes_read
    );

    bool (*storage_write_atomic)(
        F144Platform *,
        const char *leaf_name,
        const void *buffer,
        uint32_t size
    );

    /* Audio, initially no-op until Stage 4 audio work */
    void (*music_stop)(F144Platform *);
    void (*music_play)(F144Platform *, uint16_t cue, uint8_t volume);
    void (*sfx_play)(F144Platform *, uint16_t cue, uint8_t volume);

    /* Application request */
    void (*request_quit)(F144Platform *);
} F144PlatformApi;
```

Native input and lifecycle should flow **from the platform into the Core** as events, rather than being callback functions the Core polls:

```c
void Floppy144CoreHandleInput(
    Floppy144Core *,
    const F144InputEvent *
);

void Floppy144CoreHandleLifecycle(
    Floppy144Core *,
    F144LifecycleEvent
);

void Floppy144CoreUpdate(
    Floppy144Core *,
    uint64_t now_ms
);

void Floppy144CoreRender(
    Floppy144Core *,
    Floppy144Surface *
);
```

Single-instance acquisition and raw command-line parsing remain launcher-only and do not need to be present in the Core-facing service table.

## Core state extraction from `floppy144_main.c`

Most current `global_*` variables are game/application state, not Win32 state.

They should eventually become fields of a `Floppy144Core` struct, including:

- active screen;
- terminal state;
- catalogue state;
- cabinet state;
- notebook state;
- discovery profile;
- settings;
- world state;
- run state;
- recorded run state;
- persistence warning state;
- session/resume state;
- completion state;
- main-menu state;
- debug-guidance value;
- splash/timer deadlines.

Native handles must not enter that struct.

The following should remain outside it:

- HWND/HINSTANCE/HDC;
- window class;
- native message structures;
- GDI resources;
- native instance lock;
- platform storage root;
- native audio handles.

## Practical target build architecture

After Stage 4B extraction, Premake should converge toward:

### `Floppy144Core` static library

Contains:

- `game/src/floppy144_*.c/.h` except native launcher/platform files;
- core coordinator/app state;
- `floppy144_draw.c`;
- persistence codec;
- generated runtime tables.

It must compile without `BUILD_WINDOWS` and without including `windows.h`.

### `Floppy144PlatformWin32` static library

Contains:

- Win32 window/presentation implementation;
- native input translation;
- AppData storage and atomic file IO;
- monotonic clock;
- Win32 lifecycle adapter;
- future Windows audio backend;
- future single-instance implementation.

It links native Windows libraries as required.

### Windows `Floppy144` executable

Contains only a thin launcher:

```text
WinMain
 -> parse startup options
 -> acquire instance
 -> initialise Win32 platform
 -> initialise Core
 -> translate native events
 -> CoreUpdate / CoreRender
 -> present
 -> orderly shutdown
```

This structure permits another launcher/platform library to be added without cloning gameplay.

## Existing tests as architecture assets

A major advantage is that many Stage 3 tests already compile gameplay modules without `floppy144_main.c`.

That means much of the proposed Core is already proven to operate headlessly.

Existing useful gates include:

- Stage 2 headless gameplay/data regression;
- Stage 3A Prologue regression using terminal/document/run-state/persistence APIs;
- Stage 3B.1 Terminal regression;
- Stage 3B.2 Site Interaction regression;
- Stage 3B.3 Reconstruction regression;
- Stage 3B.4 Door / Access regression;
- Stage 3B.5 Cabinet regression;
- Main Menu Record Feedback audit;
- Main Menu Reinstate Flow audit;
- Stage 3B coordinator wiring audits;
- physical-item player-facing contract;
- release build;
- 1,474,560-byte size gate.

Some coordinator audits currently assert Win32 implementation details such as `WM_KEYUP` and `UpdateWindow`. During Stage 4B they must be converted carefully from "specific Win32 code exists" to "the player-visible behaviour still occurs".

## Stage 4B implementation order

Before the first source refactor, Stage 4 should receive an automatic CI gate equivalent to the immutable Stage 3 baseline workflow. The present workflow does not automatically run on `Stage4`.

### S4B-01 - Platform contract and render-surface seam

**Goal**

Introduce the narrow platform/core interfaces and change screen render entry points from `F144Runtime *` to `Floppy144Surface *`.

Do not change native input or persistence yet.

**Prerequisites**

- S4A-02 baseline.
- S4A-03 provenance decision acknowledged.
- Stage4 automatic CI guardrail enabled.

**Likely files**

- new platform/core interface headers;
- presentation headers and draw entry points:
  - cabinet;
  - catalogue;
  - notebook;
  - recovery;
  - Site 2D;
  - Site Directory;
  - Site ISO;
  - terminal;
- `floppy144_main.c` adapter;
- `premake5.lua` only if needed for interface compilation.

**Regression risk:** medium.

**Required gate**

Full Stage 2/3A/3B suite, Release build, size delta, visual smoke of every screen.

### S4B-02 - Core coordinator and logical input extraction

**Goal**

Move screen-state/input policy out of `Floppy144WindowProc`.

Win32 translates native events to `F144InputEvent`; Core owns action routing and terminal text behaviour.

**Prerequisites**

- S4B-01 render/core seam.

**Likely files**

- `floppy144_main.c`;
- new core coordinator source/header;
- new Win32 input adapter;
- terminal/cabinet only if control-event signatures need normalisation;
- Stage 3B coordinator tests.

**Regression risk:** high.

**Required gate**

Full suite, plus new logical-input tests covering every mapping table row, key repeat expectations, terminal text, cabinet digits, Site Directory any-key close and reinstate key-down/key-up handshake.

### S4B-03 - Platform storage and AppData migration

**Goal**

Move file existence/read/atomic-write out of Core and migrate Windows storage to `%APPDATA%\Floppy144`.

Keep existing binary formats byte-for-byte compatible.

**Prerequisites**

- Core/platform service plumbing from S4B-01/02.

**Likely files**

- `floppy144_persistence.c/.h`;
- core coordinator;
- new Win32 storage implementation;
- persistence tests.

**Regression risk:** high because player progress is at stake.

**Required gate**

Stage 3A persistence, Stage 3B.3, Stage 3B.5, main-menu record/reinstate audits, plus dedicated valid/corrupt/missing/conflict migration tests.

### S4B-04 - Timing and lifecycle extraction

**Goal**

Replace Win32 timer IDs and direct `GetTickCount` use in Core with monotonic platform time and deterministic Core deadlines.

Introduce lifecycle events without inventing new pause behaviour.

**Prerequisites**

- Input/core coordinator extraction.

**Likely files**

- core coordinator;
- Win32 launcher/message adapter;
- terminal restoration timing calls;
- settings/autosave integration.

**Regression risk:** medium-high.

**Required gate**

Terminal restore timing tests, splash timing test, cursor blink test, autosave dirty/no-dirty tests, orderly shutdown/profile update test, then full Stage 3 suite.

### S4B-05 - Build separation

**Goal**

Create real build targets:

- `Floppy144Core`;
- `Floppy144PlatformWin32`;
- `Floppy144` launcher.

Add an explicit gate that Core compiles without Windows headers/defines.

**Prerequisites**

- rendering;
- input;
- persistence;
- timing/lifecycle seams already extracted.

**Likely files**

- `premake5.lua`;
- CI workflow;
- source file placement/listing.

**Regression risk:** medium.

**Required gate**

Core-only compile, all headless tests against Core, Windows Release build, 0 warnings/errors, executable-size comparison against 659,968-byte Stage 3 baseline and absolute floppy-size gate.

### S4B-06 - Debug/startup configuration

**Goal**

Move `-debug` parsing fully into the Windows launcher and pass a platform-neutral startup config into Core.

**Prerequisites**

- thin launcher exists.

**Likely files**

- Win32 launcher;
- core startup config;
- terminal configuration tests.

**Regression risk:** low.

**Required gate**

Normal run hides debug guidance; `-debug` exposes only intended diagnostic guidance; full terminal regression.

### S4B-07 - Single-instance support

**Goal**

Add platform-only single-instance behaviour without touching gameplay.

**Prerequisites**

- thin Win32 launcher.

**Likely files**

- `Floppy144PlatformWin32` / launcher only.

**Regression risk:** low to medium, primarily launch/lifecycle.

**Required gate**

First process starts normally; second process follows agreed behaviour; shutdown releases lock; game Core tests unchanged.

### S4B-08 - Audio platform boundary

**Goal**

Add the minimal semantic music/SFX interface required by the actual Stage 4 audio feature, with no WinMIDI details in Core.

This is deliberately later than originally assumed because there is no current audio backend to extract.

**Prerequisites**

- stable Platform API and build separation;
- separately agreed audio feature/cue set and provenance.

**Likely files**

- platform API;
- settings integration;
- Windows audio implementation;
- game sites that request semantic cues.

**Regression risk:** low to gameplay, medium to presentation/size.

**Required gate**

Audio on/off/volume/cue tests, no-audio fallback, full gameplay suite, release size gate.

## Blueprint changes caused by the actual source

The repository changes several assumptions from the preliminary Stage 4 plan:

1. **Persistence is CWD-relative, not executable-relative.**  
   AppData migration must explicitly preserve legacy CWD saves.

2. **There is no current WinMIDI/audio implementation.**  
   Audio is additive and can safely move later in Stage 4B.

3. **There is no single-instance implementation to extract.**  
   It belongs entirely in the future platform launcher.

4. **There is no current focus/pause/resume policy.**  
   Mobile lifecycle policy must be defined later rather than inferred from Windows behaviour.

5. **The game's own renderer is already platform-neutral.**  
   `Floppy144Surface` exists today and should be the Core display contract.

6. **The current F144 runtime is much broader than the game needs.**  
   FLOPPY//144 game code uses it mainly as a backbuffer/native-runtime carrier. Legacy cursor/composite/text/runtime timing facilities are not a reason to reproduce a large engine API.

7. **Stage4 currently lacks automatic baseline CI.**  
   That guardrail should be established before S4B source movement.

8. **S4A-03 provenance affects architecture.**  
   Do not port or duplicate River2D-derived F144 implementation code into Linux/macOS/iOS/Android backends before the licensing/replacement decision is resolved.

## Final architecture recommendation

The safest architecture is not "make F144Runtime cross-platform".

It is:

```text
                 Floppy144 Core
       gameplay + state + software rendering
                  640 x 360
                      |
              Floppy144Surface
                      |
             F144 Platform API
          /           |           \
      Win32         future       future
                    SDL3?        native?
```

The Core should know semantic actions, semantic audio cues, bytes to persist, monotonic milliseconds and a 640x360 pixel surface.

It should not know HWND, WM messages, VK constants, GDI, WinMIDI, AppData APIs, mutexes or mobile lifecycle APIs.

That boundary preserves the completed Stage 3 game while giving Stage 4 one narrow place to solve each operating system.
