# Floppy//144 - Stage 3A Prologue Vertical Slice

This is the self-contained Stage 3A build of Floppy//144. It preserves the
completed Stage 2 data-driven refactor and makes the complete Prologue route a
tested player-facing vertical slice.

The central design rule is deliberately simple:

> **Game facts live in JSON. C implements reusable verbs.**

`data/floppy144_game_data.json` is the canonical source for collections,
documents and body text, triggers, interactions, evidence, rooms, connections,
furniture, fixtures, physical items, relationships, colours, drawing recipes
and ambient/seasonal content.

The released executable does **not** need to parse JSON. During development the
C99 `tools/game_data_compiler.c` validates the approved JSON and generates
compact C tables. Runtime systems interpret generic conditions and effects such
as `RECONSTRUCT_ROOM`, `UNLOCK_CONNECTION`, `ENABLE_COLLECTION`,
`REVEAL_PHYSICAL_ITEM`, `SET_ACT`, `SET_BRANCH`, `SET_PROJECTION`,
`GRANT_CAPABILITY` and `COMPLETE_INTERACTION`.

This means T-001, T-025, I-004, E-004 and their neighbours are data, not bespoke
C functions.

## Stage 2 scope

Stage 2 includes:

- canonical JSON -> generated C game data;
- stable collection, trigger, interaction and evidence IDs;
- generic trigger condition/effect interpretation;
- generic physical interaction interpretation;
- order-independent evidence synthesis;
- generated document catalogue metadata and all 157 readable document bodies;
- data-derived room connections, collection availability and physical reveals;
- generated Site geometry and validation;
- generic JSON drawing-recipe interpretation;
- persistent 2D / 2.5D projection state;
- FM-23 `SET_PROJECTION: ISOMETRIC` support;
- a lightweight isometric Site renderer over the same canonical geometry;
- Stage 2 headless regression tests;
- the 1,474,560-byte release size gate.

## Stage 3A scope

Stage 3A adds:

- a data-derived next-action prompt in the archive terminal;
- the complete `INITIATE -> RESTORE -> OPEN -> EXIT` Prologue hand-off;
- equivalent progression when the Disk Recovery Index is opened through the
  graphical catalogue instead of the direct terminal command;
- a persistent DR-01 notebook fact emitted by T-001 through the existing
  generic `RECORD_NOTEBOOK_FACT` effect;
- Stage 3A end-to-end regression through the public terminal and document APIs;
- Windows save/reinstate verification for completed Prologue state.

Stage 3A does not add a bespoke DR-01 or T-001 C function. The Prologue remains
defined by canonical JSON and interpreted by reusable runtime systems.

The remaining Stage 3 gameplay integration, Stage 4 presentation/audio work
and Stage 5 finalisation are intentionally outside this package.

## Folder location

The ZIP is packaged with a top-level `refactor` folder. Extract it into:

```text
C:\Dev\Floppy144
```

and the project will land at:

```text
C:\Dev\Floppy144\refactor
```

If you instead open an already-created `C:\Dev\Floppy144\refactor` folder in an
archive tool, extract the *contents* of the ZIP's `refactor` folder there.

## Build prerequisites

Use a Visual Studio Developer PowerShell / Developer Command Prompt with:

- Visual Studio 2022 C/C++ build tools (`cl.exe`, `MSBuild`);
- Premake 5 (`premake5`) on `PATH`.

No River2D source or Python runtime is required. The JSON compiler and Site
compiler are C source included in this package.

## Build and test

From the project root:

```powershell
.\run.ps1 -build release
```

The build performs these gates in order:

1. compile the C game-data compiler;
2. validate and compile `data/floppy144_game_data.json`;
3. synchronise generated stable-ID definition files;
4. compile and validate generated Site geometry;
5. compile and run the Stage 2 headless regression suite;
6. compile and run the Stage 3A Prologue regression suite;
7. generate the VS2022 solution with Premake;
8. build Floppy144 with MSBuild;
9. enforce the 1.44 MB / 1,474,560-byte release executable limit.

A failure in an earlier gate stops the build.

To run only the data generation:

```powershell
.\tools\build_game_data.ps1
```

To run only the Stage 2 headless regression:

```powershell
.\tools\test_stage2.ps1
```

To run only the Stage 3A Prologue regression:

```powershell
.\tools\test_stage3a.ps1
```

## Stage 3A manual acceptance route

After a successful release build:

1. Start `bin\release\Floppy144.exe` and initiate a new recovery.
2. Enter `INITIATE` at the terminal.
3. Confirm the prompt says `NEXT RECOVERY ACTION: RESTORE DR-01`.
4. Enter `RESTORE DR-01`.
5. Confirm reconstruction reaches 4 percent and the prompt names
   `DR-01-RS-0001`.
6. Enter `OPEN DR-01-RS-0001` and read the Disk Recovery Index.
7. Press Backspace to return to the terminal.
8. Confirm the prompt says `NEXT RECOVERY ACTION: EXIT TO SITE`.
9. Enter `EXIT` and confirm Reception and Corridor are reconstructed.
10. Press Escape, record the session, close the game, relaunch it and reinstate
    the recorded session.
11. Enter `EXIT` again and confirm the reconstructed Site state is retained.

The graphical alternative is also valid: after restoring DR-01, use
`LIST DR-01`, open the first record and follow the same return/exit route.

## Isometric state

Projection is stored once, as the existing persistent projection enum. Do not
store a second independent boolean. Code that wants the simple boolean view can
use:

```c
if(Floppy144RunStateIsIsometric(pRunState))
{
    /* FM-23 isometric projection is active. */
}
```

FM-23 reaches that state through its ordinary generated `SET_PROJECTION`
effect. There is no T-025-specific projection function.

## Editing game content

Edit **only** the canonical JSON for game-specific content wherever an existing
engine verb can express the desired behaviour. Generated files are marked as
generated and will be overwritten by the next build.

Add C code only when the design genuinely needs a new reusable engine verb or
new presentation capability.

See `STAGE2_REFACTOR_NOTES.md` and `data/DATA_SCHEMA_README.md` for additional
technical notes.

## Windows user data - Stage 4

Stage 4 no longer stores writable player data relative to the executable or
process working directory. The Windows platform implementation resolves the
roaming AppData folder with the Windows shell API and stores all production
persistence beneath:

```text
%APPDATA%\Floppy144\
    floppy144_manual.sav
    floppy144_auto.sav
    floppy144_profile.dat
    floppy144_settings.dat
```

The discovery profile also contains the most recent completion/profile-history
snapshot; there is no separate completion-history file.

On first use of a Stage 4 path, FLOPPY//144 performs conservative Stage 3
migration. If the AppData file already exists it always wins and is never
overwritten by an older copy. If it does not exist, the Windows layer supplies
the old Stage 3 working-directory path first and the executable directory as a
second compatibility probe. A legacy file is accepted only when the existing
Stage 3 codec can load and validate it. It is then re-saved atomically to the
AppData path and reloaded to verify the new copy. The legacy source is left in
place.

The platform-neutral game code requests conceptual manual-save, autosave,
profile and settings paths through `F144Platform`; it contains no
`%APPDATA%` or Windows user-name assumptions.

## Audio platform boundary - Stage 4

The verified Stage 4 baseline contains persisted music/SFX volume settings but
no shipping playback backend, WinMIDI/WinMM calls, external audio library, or
embedded production music/SFX data. S4B-04 therefore introduces the semantic
audio boundary without changing the current silent runtime.

Game-facing code can now initialise/shut down audio, play or replace a semantic
music cue, stop music, trigger an SFX cue, and set independent music/SFX volume
through `F144Platform`. Cue IDs remain platform-neutral. Future compact
runtime music/SFX composition must remain game/core-owned; only native playback
and device handling belong in the Win32 backend.

The current Win32 audio adapter is intentionally silent. This preserves the
audited behaviour exactly while leaving the platform seam ready for the
separately approved generated-audio feature and Stage 4C persisted volume UI.

## Timing and lifecycle platform boundary - Stage 4

FLOPPY//144 game timing now uses the platform-neutral
`f144PlatformMonotonicMs()` clock. Windows supplies that clock with
`GetTickCount64`, but game code no longer depends on `GetTickCount`,
`SetTimer`, `KillTimer` or `WM_TIMER` semantics.

The game owns deterministic deadlines for the existing behaviours: 16 ms splash
animation opportunities, 50 ms terminal-restoration quanta, 500 ms terminal
cursor blinking, and the persisted 5/10/30-minute (or off) autosave cadence.
The Win32 layer uses one native wake timer only to give the game opportunities
to advance those deadlines.

Lifecycle events are platform-neutral: start, active, inactive, suspend,
resume, shutdown-requested and shutdown. Windows currently maps activation and
orderly close/destroy messages into those events. Inactive/suspend/resume do
not pause or alter gameplay because Stage 3 had no such policy. A lifecycle
event can carry an explicit autosave request, ready for later mobile
background/suspend policy without exposing persistence internals to a platform
backend.

## Single-instance protection - Stage 4

The Windows launcher now acquires a platform-specific named mutex before it
creates the game window or resolves any writable persistence path. The mutex
name is derived from the current Roaming AppData profile environment, so two
FLOPPY//144 processes targeting the same `%APPDATA%\Floppy144` data set
cannot run concurrently, while separate Windows profile environments are not
needlessly coupled.

The mutex lives in the Windows `Global\` object namespace and is owned by the
process. Windows releases it automatically if the process exits or crashes, so
there is no persistent lock file to strand or repair. A second launch shows a
short "already running" message and exits before profile/settings/save files are
opened. If ownership cannot be established safely, the launcher fails closed.

`-debug` obeys the same protection. Headless/regression tools are separate
test executables and do not enter the production `WinMain`, so no test bypass
is required or exposed in release builds.

Future Linux/macOS launchers should implement their own process-exclusivity
mechanism at the equivalent launcher/platform boundary. Core/game modules
must remain unaware of the native lock primitive.

## Developer configuration - Stage 4

Raw launch-option parsing is platform-owned. Windows converts the WinMain
command-line tail into the portable `F144StartupConfig` object; game systems query
semantic values rather than parsing Windows arguments themselves.

Supported Windows developer options are:

```text
-debug
-debug -seed <non-zero uint32>
-debug -date <YYYY-MM-DD>
```

`-debug` retains its existing meaning: it enables diagnostic terminal
recovery breadcrumbs. There is no separate debug-document set, debug overlay,
developer keyboard shortcut or player-facing logging mode in the current
production game.

`-seed` and `-date` are deterministic developer/regression hooks prepared
for Stage 4E. They are ignored unless `-debug` is also present. The portable
configuration API can inject the same values directly in tests or future
platform launchers, so regression work does not depend on a real command line,
random environment or calendar clock.

The legacy F144 runtime still contains a handful of unconditional stderr error
messages for internal rendering/timestamp failures. They are implementation
diagnostics rather than `-debug` facilities and are unchanged by S4B-07.


## Final Stage 4B build separation

Premake now generates three production targets:

```text
Floppy144Core
Floppy144PlatformWin32
Floppy144
```

`Floppy144Core` contains the portable gameplay/state/rendering and semantic
platform contracts. `Floppy144PlatformWin32` contains the native Windows
implementation and legacy presenter support. `Floppy144` is the final Windows
application and links both static libraries plus `user32`, `gdi32` and
`shell32`.

No DLL dependency was introduced. No SDL3/Linux/macOS/mobile dependency was
added.

The remaining portability debt is explicit: `floppy144_persistence.c` still
mixes portable persistence encoding with Win32/MSVC file I/O, so it is
quarantined in the Windows application target instead of being mislabeled as
Core.

See `docs/stage4/build_architecture.md` for the full dependency map, test
strategy and future-platform attachment point.


## Operator Profile screen - Stage 4C

Stage 4C exposes the existing persistent discovery profile through an
always-available **OPERATOR PROFILE** entry in GDR Session Control.

The screen is deliberately read-only in S4C-01. It displays only data stored
in `Floppy144DiscoveryProfile`: operator name, body configuration, recoveries
begun, cumulative distinct collections restored, cumulative distinct evidence
established, completed-recovery count, and the latest completion snapshot.

Current recovery-session state is not passed to the Profile renderer. Active
run reconstruction percentage, current-run collections/evidence, recovery seed
and other transient session values therefore cannot be mistaken for permanent
operator history.

A fresh profile displays `UNASSIGNED` for an empty operator name and zeroed
history safely. Name editing and body-style editing remain reserved for
S4C-02/S4C-03.
