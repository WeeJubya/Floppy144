# Stage 4B Platform Abstraction Completion Record

**Status:** **STAGE 4B: PASS**

The dedicated S4B-09 completion gate reproduced the Stage 3 behavioural
baseline, every Stage 4B contract/regression, independent Core and Win32
platform builds, a clean production-equivalent Release rebuild, and the
floppy-size gate.

**Immutable Stage 3 baseline:** `fe2239123f98346ed06b48bb500672a5e6e2e172`  
**Stage 4B implementation start:** `1f119d7aef760f27e2d2027aede35ee47545ff82`  
**Stage 4B implementation end before sign-off:** `09f8544506e53d739b6c200e22e040bb685d43c1`  
**Stage 4B final range:** the implementation range above plus the
`Complete Stage 4B platform abstraction` sign-off commit containing this
record.

## Decision

The dedicated completion gate passed in full:

```text
STAGE 4B: PASS
```

The validated gate includes the Stage 3 regression suite, every Stage 4B
regression, the cross-cutting integration audit, independent Core/Win32
platform builds, a clean Release rebuild, and the floppy-size gate.

No Stage 4C feature work is included in this sign-off.

## Stage 4B implementation sequence

| Task | Commit | Result |
| --- | --- | --- |
| Platform interface foundation | `1f119d7aef760f27e2d2027aede35ee47545ff82` | F144 platform contract and framebuffer/presentation seam |
| Logical input | `d623f1f98798ed72fad6b7ea9d932616401a8f70` | Native keys translated to semantic actions/text events |
| Persistence paths | `988679bd3b226b3654417c5df596008b31d9242b` | AppData paths and conservative Stage 3 migration |
| Audio boundary | `d443092f781a2a069625d7536fe01886e354831f` | Semantic music/SFX/volume API with silent Win32 backend |
| Timing/lifecycle | `11e66e0fac8155ef875461dcab7477d349a06e7d` | Monotonic time, deterministic timers and lifecycle events |
| Single instance | `e06fb77b9f1a9c1777c68425dcc64b0612e5555c` | Win32 named-mutex ownership |
| Debug/config | `97f15a9068110f60380bc96e6d28468db73c57f3` | Portable startup config and Stage 4E seed/date route |
| Build separation | `09f8544506e53d739b6c200e22e040bb685d43c1` | Core, PlatformWin32 and Windows application targets |
| Integration/sign-off | completion commit containing this document | Cross-cutting audit and final validation |

The older planning section in
`docs/stage4/platform_dependency_inventory.md` predates the executed task
ordering. The commit sequence above is the authoritative Stage 4B numbering and
implementation history.

## Final architecture

The production build is:

```text
Portable game systems
        |
        v
Floppy144Core
        |
        v
F144 Platform Interface
        ^
        |
Floppy144PlatformWin32
        ^
        |
Windows application / launcher (Floppy144.exe)
```

At build/link level the Windows application links the two static libraries:

```text
Floppy144
  +-- Floppy144Core
  +-- Floppy144PlatformWin32
  +-- user32
  +-- gdi32
  +-- shell32
```

No runtime DLL boundary was introduced.

### Floppy144Core

Core owns:

- game/run/world/Site state;
- generated-data interpretation;
- terminal, catalogue, document, notebook and cabinet behaviour;
- triggers, interactions, evidence and recovery logic;
- software rendering into `Floppy144Surface`;
- logical action/movement state;
- deterministic timing semantics;
- lifecycle state;
- profile/settings/run-state data;
- the semantic `F144Platform` contract;
- portable `F144StartupConfig`.

The sign-off architecture audit scans both portable C translation units and
portable headers for Win32/runtime leakage.

### Floppy144PlatformWin32

The Windows platform layer owns:

- native framebuffer/presentation support;
- Win32 logical-key/text translation;
- AppData and legacy path discovery;
- monotonic clock and native wake messages;
- lifecycle message translation;
- semantic audio callback implementation;
- single-instance mutex ownership;
- raw Windows startup argument acquisition;
- legacy F144 runtime support still required by the Windows presenter.

### Windows application / launcher

The current executable target owns:

- `WinMain` and Windows message-loop/application coordination;
- application-level persistence migration glue;
- the still-mixed persistence codec/file-I/O source described below.

## Validation coverage

### Stage 3 gameplay regression

The complete Stage 3 suite remains mandatory:

- canonical game-data generation and Site validation;
- Stage 2 headless regression;
- Stage 3A Prologue regression;
- complete Stage 3B terminal, Site interaction, reconstruction, coordinator,
  menu, door/access, physical-item and cabinet regressions.

These tests remain the behavioural contract proving the platform refactor did
not change Stage 3 gameplay.

### Platform interface

`tools/test_stage4_platform.ps1` verifies the portable interface compiles
without a Win32 dependency and that framebuffer, presentation, persistence,
monotonic-time and quit dispatch remain callable through `F144Platform`.

### Input and text

`tools/test_stage4_input.ps1` and `tools/stage4_input_tests.c` verify:

- arrow-key movement translation;
- simultaneous cardinal/diagonal movement state;
- Access and Inspect remain distinct;
- menu/back/notebook/paging actions;
- terminal/game text remains a separate text-input stream;
- ordinary terminal characters and cabinet digits do not become gameplay
  actions.

The full Stage 3 terminal/catalogue/notebook/Site regressions provide the
higher-level behaviour coverage for navigation, document paging, history,
editing and interaction routing.

### Persistence and migration

`tools/test_stage4_persistence.ps1` and
`tools/stage4_persistence_paths_tests.c` verify:

- fresh/no-existing-data startup;
- manual-save legacy migration;
- autosave legacy migration;
- Stage 3 discovery-profile migration;
- settings migration;
- new AppData location winning over legacy copies;
- corrupt legacy data rejection;
- source legacy files retained after migration;
- manual-save round trip/reinstate;
- autosave round trip/reinstate;
- profile round trip;
- settings round trip;
- path-buffer failure handling;
- creation of the Windows data directory;
- Stage 3 CWD compatibility;
- executable-directory compatibility when Stage 4 is launched from another
  working directory;
- current data-location independence from the launch working directory.

No test writes to the player's real AppData environment.

### Audio

`tools/test_stage4_audio.ps1` and `tools/stage4_audio_tests.c` verify:

- init/failure behaviour;
- semantic music start/stop requests;
- repeated SFX requests;
- simultaneous music/SFX request state;
- independent music/SFX volume;
- mute and maximum volume;
- invalid volume rejection;
- clean/repeated shutdown;
- persisted settings scale compatibility;
- the current Win32 silent backend accepts the semantic contract safely.

There is intentionally no audible Windows backend in Stage 4B. The repository
entered Stage 4 without shipping music/SFX playback; S4B-04 established the
boundary without inventing a new audio feature.

### Timing and lifecycle

`tools/test_stage4_timing_lifecycle.ps1` and its deterministic C harness
verify:

- monotonic time dispatch;
- splash elapsed time;
- 50 ms terminal restore quantisation;
- 500 ms cursor blinking;
- delayed-wake collapse semantics;
- 5/10/30-minute/off autosave timing;
- active/inactive state;
- suspend/resume state;
- future autosave-request carriage;
- orderly shutdown request/completion;
- Win32 `WM_ACTIVATEAPP`, `WM_CLOSE` and `WM_DESTROY` translation.

No lifecycle test depends on arbitrary real-time sleeping.

### Single instance

The single-instance regression launches a real child process against a
synthetic profile environment and verifies:

- first ownership succeeds;
- second ownership is refused;
- refused process does not alter a persistence sentinel;
- clean release removes ownership;
- a subsequent process succeeds;
- independent profile environments do not collide.

The OS-owned mutex leaves no persistent lock file.

### Debug/startup configuration

Configuration regression verifies:

- default launch has debug disabled;
- exact `-debug` enables the intended terminal guidance;
- seed/date overrides cannot activate without debug;
- deterministic seed/date values can be injected without the player's real
  command line or calendar;
- malformed overrides fail safely;
- game-facing code does not parse the Windows command line.

### Build architecture

The final build gate verifies:

- `Floppy144Core`, `Floppy144PlatformWin32` and `Floppy144` are distinct
  targets;
- Core has no `BUILD_WINDOWS` definition or Windows system-library link;
- Core C and header files contain no Win32/runtime implementation dependency;
- Win32 implementation source is owned by the platform project;
- native libraries are attached on the Windows side;
- canonical data generation still precedes project generation/build;
- Core and PlatformWin32 each build independently;
- final production build is a clean Release rebuild.

## Build and size metrics

Immutable Stage 3 baseline:

| Metric | Stage 3 baseline |
| --- | ---: |
| Release executable | 659,968 bytes |
| Limit | 1,474,560 bytes |
| Headroom | 814,592 bytes |
| Warnings | 0 |
| Errors | 0 |

Stage 4B sign-off target, established by the final S4B-08 build and required to
be reproduced by the completion commit:

| Metric | Stage 4B |
| --- | ---: |
| Release executable | 667,648 bytes |
| Change from Stage 3 | +7,680 bytes |
| Limit | 1,474,560 bytes |
| Remaining headroom | 806,912 bytes |
| Core warnings/errors | 0 / 0 |
| Win32 platform warnings/errors | 0 / 0 |
| Final application warnings/errors | 0 / 0 |

The Core/Platform library split itself added zero bytes to the S4B-07 payload.

## Remaining portability debt

### Persistence codec and Windows file I/O are still mixed

`game/src/floppy144_persistence.c` still combines portable binary
encode/decode/checksum/schema logic with Windows/MSVC file operations including
`MoveFileExA`, `DeleteFileA`, `GetFileAttributesA`, `MAX_PATH` and
`fopen_s`.

It is therefore deliberately excluded from `Floppy144Core` and quarantined
in the Windows application target.

A future portability pass should split the byte codec from file operations and
move read/exists/atomic-replace behind the platform storage contract.

### Top-level coordinator remains partly Win32-owned

`floppy144_main.c` still combines `WinMain`, message-loop duties and some
top-level game screen/session coordination.

All migrated services use the F144 boundaries, but a future non-Windows
launcher will still benefit from extracting the remaining portable coordinator
policy.

### Legacy presenter/runtime support

The Windows platform library still contains older F144 runtime helpers and
Win32-native runtime structures required by the existing presenter.

They no longer leak into Core. They should not be copied into another platform
backend merely to mimic the Windows implementation.

### Audio remains intentionally silent

The Stage 4B Win32 audio backend is a safe no-op implementation because the
audited baseline had no shipping audio backend or generated audio content.
Actual music/SFX generation/playback is later Stage 4 work, not a Stage 4B
integration defect.

### Mobile lifecycle policy is not yet defined

Suspend/resume concepts and a safe autosave-request route now exist, but
Windows does not invent mobile-style pause/background policy. That policy
belongs to a later target-specific/mobile task.

## Known limitations

- CI validates launch/lifecycle/input/persistence behaviour through deterministic
  harnesses and source/build gates rather than GUI automation of an interactive
  Windows desktop session.
- The legacy workflow filename remains `.github/workflows/stage3c-ci.yml` for
  repository continuity even though it now validates the complete Stage 4B
  branch.
- The current workspace still exposes only the Windows Premake platform. This
  is intentional until a future platform implementation is actually added.

## Stage 4C readiness

The completion gate has reproduced all PASS results and the Stage 4B
size/build metrics above.

The platform abstraction work is therefore integrated and stable enough for
**Stage 4C Profile, Settings and Player Identity** work to begin.
