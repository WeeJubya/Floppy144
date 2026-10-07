# FLOPPY//144 Stage 4B Final Build Architecture

## Purpose

S4B-08 aligns the generated Visual Studio/Premake build with the platform
boundaries introduced throughout Stage 4B.

The build now has three production targets:

```text
Floppy144Core
      |
      +----------------------+
                             |
                   Floppy144PlatformWin32
                             |
                             +------+
                                    |
                              Floppy144
                         Windows application
```

The final executable remains a single `Floppy144.exe`. Both supporting
targets are static libraries, so the build introduces no runtime DLL
dependency.

## Previous build structure

Before S4B-08, Premake produced:

```text
F144 Runtime
    static library
    f144_runtime.c
    f144_win32_runtime.c
    string_view.c

Floppy144
    every game/src C file
    platform-neutral F144 dispatch/config
    every Win32 adapter
    Windows launcher
    persistence/storage
    links user32 + gdi32 + shell32
```

That produced a working executable, but project ownership did not reflect the
Stage 4 platform boundary. Most portable game code and all native Windows
adapters were compiled by the same executable target.

## Final production targets

### Floppy144Core

`Floppy144Core` is a C99 static library optimized for size in Release.

It contains:

- gameplay state and progression;
- world/Site state;
- generated-data interpretation;
- terminal/catalogue/notebook/cabinet behaviour;
- trigger/interaction/evidence logic;
- software rendering into `Floppy144Surface`;
- logical movement/input state;
- timing and lifecycle state machines;
- settings/profile/run-state structures;
- the platform-neutral `F144Platform` dispatch contract;
- portable `F144StartupConfig` storage/validation.

It deliberately excludes:

- `floppy144_main.c`;
- `floppy144_persistence.c`;
- `floppy144_storage.c`;
- all `f144_win32_*` implementation files;
- the legacy F144 native runtime.

The Core project does not define `BUILD_WINDOWS` and does not link
`user32`, `gdi32`, `shell32` or WinMM.

The S4B-08 CI audit scans every Core C translation unit for Win32 headers,
native handles, virtual-key constants and migrated Windows APIs. CI also builds
`Floppy144Core.vcxproj` independently with warnings-as-errors.

### Floppy144PlatformWin32

`Floppy144PlatformWin32` is a C99 static library containing the native Windows
implementation:

- framebuffer/presentation adapter;
- Win32 logical-key translation;
- Win32 lifecycle translation;
- monotonic clock/native wake timer;
- AppData/path resolution;
- single-instance mutex implementation;
- Win32 startup-command acquisition;
- current silent audio adapter;
- the legacy F144 software-runtime support used by the Windows presenter.

This target owns `BUILD_WINDOWS`.

The legacy `F144 Runtime` project no longer exists as a separate build
target. Its remaining runtime implementation is now clearly support code for
the Windows platform layer rather than an apparent cross-platform game
dependency.

### Floppy144

`Floppy144` is the Windows `WindowedApp` target.

It contains:

- `floppy144_main.c`, the current Windows application/coordinator;
- `floppy144_storage.c`, application-level persistence migration glue;
- `floppy144_persistence.c`, temporarily quarantined here because its portable
  codec and Win32 file I/O have not yet been separated.

The application links:

```text
Floppy144Core
Floppy144PlatformWin32
user32
gdi32
shell32
```

There is still no WinMM link because S4B-04 established a silent semantic audio
boundary without introducing a native multimedia backend.

## Dependency direction

The intended final direction is:

```text
Windows launcher/application
        |
        +----> Floppy144Core
        |
        +----> Floppy144PlatformWin32
```

Core depends only on portable C headers and the semantic F144 contracts.

A future native implementation should implement the same F144 platform
contract without requiring gameplay duplication:

```text
Floppy144Core + Floppy144PlatformSDL3 + SDL3 launcher
Floppy144Core + Floppy144PlatformMac  + macOS launcher
Floppy144Core + Floppy144PlatformIOS  + iOS launcher
```

S4B-08 adds none of those dependencies or launchers.

## Generated source and data build

Canonical data generation remains a prerequisite rather than a new runtime
dependency.

CI still runs `tools/build_game_data.ps1` before Premake/project generation.
The generated `.inc` and `.def` files are consumed by the portable Core
translation units exactly as before.

The S4B-08 architecture audit explicitly verifies that canonical generation
still precedes Visual Studio project generation/build.

## Tests and headless regressions

The Stage 2, Stage 3A and Stage 3B suites continue to compile the relevant Core
source directly into their small headless executables. They do not gain a
Windows launcher dependency.

Existing Stage 4 boundary tests continue to compile only the modules needed for
their contract tests.

S4B-08 adds two additional build proofs:

1. source audit of the declared Core ownership for forbidden Win32 coupling;
2. independent Release build of `Floppy144Core.vcxproj` before the Windows
   platform and final application are built.

The full solution then builds the Win32 platform library and final executable.

## Resources and icons

The repository currently contains no `.rc`, `.ico`, `.manifest` or
precompiled `.res` files.

S4B-08 therefore has no Windows resource ownership to move. A future Windows
resource file belongs with the Windows application/launcher target, not Core.

## Remaining portability debt

The build split is genuine, but portability is not claimed to be complete.

### Mixed persistence codec/file I/O

`game/src/floppy144_persistence.c` still contains both:

- portable binary encode/decode/checksum/schema compatibility; and
- Windows/MSVC file operations such as `MoveFileExA`, `DeleteFileA`,
  `GetFileAttributesA`, `MAX_PATH` and `fopen_s`.

For that reason it is intentionally **not** compiled into
`Floppy144Core`.

A future storage cleanup should separate the byte codec from platform file I/O,
then move the codec into Core and implement read/exists/atomic-replace through
the platform storage service.

### Application coordinator still shares WinMain

`floppy144_main.c` still contains both the Windows message-loop/application
entry point and top-level game screen/session coordination.

The lower-level gameplay/render/state modules are now independently built as
Core, but a future port will still need either:

- extraction of the remaining coordinator into a portable application module;
  or
- a new launcher that reproduces the thin top-level coordination against Core.

This debt is explicit and should be addressed before claiming that the complete
application coordinator is portable.

### Legacy F144 runtime

The Windows presenter still uses the existing legacy F144 runtime structures.
Those structures contain Win32-native fields under `BUILD_WINDOWS`.

They are now isolated inside the Windows platform/application side of the build
and are no longer a Core dependency.

The provenance constraints recorded in the Stage 4 audit still apply: do not
copy this implementation into a future SDL3/Linux/macOS/mobile backend merely
to make it compile elsewhere.

## System-library ownership

No platform-specific system library is linked by `Floppy144Core`.

The final Windows application link owns:

- `user32`;
- `gdi32`;
- `shell32`.

These satisfy the native implementation and launcher. If a future Win32 audio
backend genuinely requires another system library, it should be added on this
Windows-side final link rather than to Core.

## Payload rule

The architecture uses static libraries only. The linker still emits one
standalone Windows executable and can discard unreferenced object code.

The immutable Stage 3 comparison point remains:

```text
659,968 bytes
```

Every Stage 4B validation continues to enforce the absolute limit of
`1,474,560` bytes.
