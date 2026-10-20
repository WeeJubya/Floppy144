# B2-02: Public F144 platform API compatibility contract

**Normative reference:** `include/f144_platform.h` from `WeeJubya/Floppy144` branch `development/Beta-2-runtime` at the B2-02 documentation commit. Also consult `include/f144_startup_config.h`. **These project-owned public headers are permitted implementation inputs**. This document is a semantic, independent contract, not a substitute pasted from any inherited private runtime.

## 1. No API or ABI drift

Keep the public declarations, field types, order, enum ordinals and façade function signatures unchanged. Do not replace the contract with an old render-engine data model. Use `F144Platform` as an opaque native binding from the game perspective and `F144PlatformApi` as the installed virtual-function table. The backend may allocate its own entirely new **private** context type behind the existing `void *state`; game code may not cast that pointer or include Windows headers.

`Floppy144Surface` exposes exactly three fields in order: `uint32_t *pixels`, `uint32_t width`, `uint32_t height`. When available it describes a contiguous top-left-origin 640 × 360 0x00RRGGBB pixel surface. The runtime owns the bytes; caller owns only drawing operations. The `F144Platform` struct retains its `api` pointer, `state` pointer, `surface` value, and the existing audio flags/settings `music_tempo_percent`, `audio_initialized`, `music_volume` and `sfx_volume`. On x64 preserve natural Windows/MSVC C layout; don't add padding assumptions or publicly expose native types.

## 2. Callback slots, order and failure semantics

The **existing** `F144PlatformApi` callback table has 15 slots in this exact order:

| # | Field and result | Semantic contract |
|---:|---|---|
| 1 | `framebuffer(platform) -> Floppy144Surface *` | Valid borrowed writable 640×360 surface once initialised; NULL before ready/after teardown |
| 2 | `present(platform) -> void` | Present stable latest completed pixels; no game simulation and no double-free; harmless when unavailable |
| 3 | `persistence_path(platform,file,path,capacity) -> bool` | NUL-terminated location for manual/autosave/profile/settings; false if unavailable or capacity insufficient |
| 4 | `legacy_persistence_path(platform,file,candidate,path,capacity) -> bool` | Enumerate same legacy candidates without changing compatibility |
| 5 | `audio_init(platform) -> bool` | Initialise existing generated-audio player; return true only for functional init per existing service semantics |
| 6 | `audio_shutdown(platform) -> void` | Release playback device(s); repeat calls safe through existing façade |
| 7 | `music_play(platform,cue) -> void` | Play authorised procedural music cue |
| 8 | `music_stop(platform) -> void` | Stop current music without ending display session |
| 9 | `sfx_play(platform,cue) -> void` | Trigger existing project SFX ID |
| 10 | `music_volume(platform,volume) -> void` | Apply volume 0..10 |
| 11 | `sfx_volume(platform,volume) -> void` | Apply volume 0..10 |
| 12 | `monotonic_ms(platform) -> uint64_t` | Monotonic time in milliseconds (no system-time jumps) |
| 13 | `quit(platform) -> void` | Request orderly termination once; repeated requests harmless |
| 14 | `local_calendar_date(platform,date) -> bool` | Supply valid local civil date; no persisted debug override side effects |
| 15 | `audio_update(platform,now_ms,recovered_percent) -> void` | Scheduler tick for procedural sound with current progress |

**Careful:** public implementation code and some existing test mocks may use partial positional initialisers. Do not re-order slots or alter number/types even if a modern alternative appears tidier. Backend must supply all required non-optional services used by the game. The generic façade functions (e.g. `f144PlatformFramebuffer`, `f144PlatformPresent`, `f144PlatformQuit`, `f144PlatformAudioInit`, `f144PlatformMonotonicMs` and `f144PlatformCalendarDate`) remain owned by project-neutral `src/f144_platform.c` and are not a request to reimplement or duplicate dispatch glue.

## 3. Supporting types and constants

- `F144PersistenceFile`: `MANUAL_SAVE`, `AUTOSAVE`, `PROFILE`, `SETTINGS`; also `FILE_COUNT` sentinel. Preserve ordinal values and meaning; path capacity symbol `F144_PLATFORM_PATH_CAPACITY = 512U`.
- `F144Action` values: `NONE`, 4 directional MOVE, NAV_UP/NAV_DOWN, ACCESS, INSPECT, CONFIRM, BACK, PAGE_UP/PAGE_DOWN, NOTEBOOK, MENU, COUNT. The ordering in the header is part of existing code compatibility.
- `F144ActionEvent`: `action`, `type` (DOWN/UP), `physical_token`. Token correlates press/release and is opaque outside native boundary. Existing regression expects the tested virtual-key value as the token for the public translator; if a new key identity is desired, seek approval and update tests with a documented compatibility reason.
- `F144TextInputEvent`: `uint32_t codepoint`; text input uses the character stream, not logical action events.
- `F144LifecycleEvent`: event type and `uint8_t request_autosave`. Preserve START, ACTIVE, INACTIVE, SUSPEND, RESUME, SHUTDOWN_REQUESTED, SHUTDOWN and NONE/COUNT sentinel ordinals. Do not invent an autosave request on Win32 focus changes.
- `F144MusicCueId` and `F144SfxCueId`: `uint16_t`. `F144_AUDIO_VOLUME_MAX = 10U`. Semantic identifiers are owned by procedural content code.
- `F144StartupConfig` and `F144CalendarDate` are portable and keep their established debug/seed/fixed-date semantics; there is no new CLI/storage format specified here.

## 4. Native-to-game event bridge (not API-table additions)

Input and lifecycle events are delivered by the application coordination layer using existing project-owned headers, **not by extending F144PlatformApi**. Approved interface signatures/roles:

| Project-owned interface | Requirement |
|---|---|
| `f144Win32ActionFromVirtualKey(uint32_t) -> F144Action` | Physical key-to-action mapping exactly as existing tests |
| `f144Win32TranslateKeyEvent(uint32_t,F144ActionEventType,F144ActionEvent *)` | Preserve press/up and token; no pointer retention |
| `f144Win32TranslateTextEvent(uint32_t,F144TextInputEvent *)` | Preserve codepoint and separation from actions |
| `f144Win32TranslateLifecycleEvent(uint32_t,uintptr_t,F144LifecycleEvent *) -> bool` | Translate known native lifecycle messages, false/NONE for unrelated messages |
| `Floppy144MovementInputSetAction` / `Floppy144MovementInputVector` | Portable held-cardinal movement, no diagonal native action |
| `Floppy144LifecycleApply` | Game-neutral lifecycle reducer, no Win32 details |
| `Floppy144TimingAdvance` | Game cadence driven only by monotonic timestamps |
| `f144Win32TimingStartWake` / `StopWake` / `IsWakeMessage` | Existing wake service; native-context hookup may need a fresh independent adapter |
| `f144Win32SingleInstanceAcquire` / `Release` | Existing profile scoped launcher lifetime |
| `f144Win32PlatformPersistencePath` / `LegacyPersistencePath` | Existing path semantics, compatible formats |
| `f144Win32AudioInit` / `Shutdown` / `PlayMusic` / `PlaySfx` / `Update` | Existing WinMM procedural implementation, no stored audio |

The native window loop must call these services through independently written coordinator glue. The inherited runtime's private header and callbacks are **not** required or authorised for the clean implementer. The current game coordinator can be adapted by a separate integration maintainer; the clean implementer receives only this table, approved project-owned declarations/tests and no derived implementation.

## 5. Lifecycle and input compatibility fixtures

| Native event | Required semantic event | Notes |
|---|---|---|
| `WM_ACTIVATEAPP` nonzero | `F144_LIFECYCLE_ACTIVE` | No automatic autosave |
| `WM_ACTIVATEAPP` zero | `F144_LIFECYCLE_INACTIVE` | Clear held physical input safely |
| `WM_CLOSE` | `F144_LIFECYCLE_SHUTDOWN_REQUESTED` | Once, before destroy |
| `WM_DESTROY` | `F144_LIFECYCLE_SHUTDOWN` | Once, no use-after-free |
| `WM_SIZE` | No lifecycle event | Separate presentation geometry |
| Startup successful | `F144_LIFECYCLE_START` | Before gameplay begins; follow existing game reducer |
| Unmapped key down/up | `F144_ACTION_NONE` with event type retained | Text interpretation remains distinct |

Key map for Windows tests (not source expression): VK_UP/DOWN/LEFT/RIGHT -> movement; 'W'/'S' -> NAV_UP/NAV_DOWN; 'A' -> ACCESS; 'I' -> INSPECT; VK_RETURN -> CONFIRM; VK_BACK -> BACK; VK_PRIOR/VK_NEXT -> PAGE_UP/PAGE_DOWN; 'N' -> NOTEBOOK; VK_ESCAPE -> MENU. 'Q', '0', VK_SPACE and VK_F1 are not mapped to a gameplay action in current regression. WM_CHAR input enters the separate character stream.

## 6. Persistence / audio / ABI test gates

- Include the unchanged public header from at least one independent C translation unit; compile using MSVC x64, check sizeof/member offsets in tests only when compared to the compiler's same header, not hand-authored hard-coded offsets.
- Provide mock-callback tests proving all 15 slots dispatch without swapping, including optional/no-op-safe absent callbacks where the **existing** façade permits them.
- For every file category and legacy candidate, verify canonical locations and byte-identical read/write/round-trip against Beta.1 fixtures; no schema changes.
- Audio tests cover no-device startup, init/shutdown, music cue mapping, procedural SFX, volume 0 and 10, mute, tempo/progress updates and missing device recovery.
- Keep input and lifecycle tests `tools/test_stage4_input.ps1` and `tools/test_stage4_timing_lifecycle.ps1`, along with platform boundary and presentation tests; these are test expectations, not inherited algorithm sources.
- No new Windows-specific types, object lifetime rules, render APIs or stored assets become part of the public surface.

## 7. Relationship to missing evidence

The shared API header and current source tests establish **interface compatibility**, not exact end-user screenshot equivalence. Beta.1 independently verified screenshots, framebuffer hashes, listening tests and final original distribution/licence validation are still pending. When captured, append externally observable additions to the acceptance matrix and do not change this ABI unless an explicit project decision authorises it.
