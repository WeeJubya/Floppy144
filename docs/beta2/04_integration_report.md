# B2-04: Independent native Win32 production runtime integration

Date: 21 October 2026. **Development repository:** `WeeJubya/Floppy144`, branch `development/Beta-2-runtime`. **Release repository:** `GreyDoorRepublik/Floppy144-beta`, deliberately unchanged. The inherited renderer was replaced in the **production build**; historical inherited files remain uncompiled for B2-05 review.

## 1. Source changes and responsibility

| File | Change |
|---|---|
| `include/floppy144_win32_host.h` | New Win32-only host state: HWND, transient paint HDC, independent display and closing flag; portable F144PlatformApi unchanged |
| `src/floppy144_win32_host.c` | New independently authored 15-slot platform binding: framebuffer, presenter, quit, and reuse of original non-derived storage, calendar, clock and procedural WinMM services |
| `src/f144_win32_timing.c` | Rewritten timer/clock adapter using host-owned HWND, not an inherited private runtime cast |
| `game/src/floppy144_main.c` | Replaced render callback assignment, private renderer and plane types, startup, WM_PAINT, quit and cleanup integration with the native host; preserved game screens, drawing, native input, saving and audio |
| `premake5.lua` | Retained Core, PlatformWin32 and Floppy144 targets. Removed all five derived runtime files and unused string_view from production compilation; added native display/host sources and headers. Selected static MSVC CRT across all projects to eliminate a VCRUNTIME140 redistributable dependency. |
| `tools/test_stage4_build_architecture.ps1` | Converted outdated required-inherited-sources checks into positive independent-source and negative legacy-source assertions, retaining all other architecture checks |
| `tools/beta2_win32_host_tests.c`; `tools/test_beta2_win32_host.ps1` | New isolated production host/ABI, framebuffer lifetime, quit/timer and mocked WinMM/calendar/storage service tests |
| `.github/workflows/beta2-cutover-ci.yml` | Dedicated Windows x64 integration, strict rebuild, import and size, established regression and root Release workflow coverage |
| `docs/beta2/04_integration_report.md`; `docs/beta2/STATUS.md` | Verification report and handover |

No intentional changes to any game screen, generated definition, gameplay, animation, progression, save format or procedural sound synthesis. `src/floppy144_win32_display.c` from B2-03 is now linked to the production executable; no SDL/Raylib/OpenGL/DirectX or additional graphics library was introduced.

## 2. Windows test and build evidence

[CI run 39570329426](https://github.com/WeeJubya/Floppy144/actions/runs/39570329426), for implementation commit `38afcfe5b6ed8ac5da08039479c205ab883239d9`:

| Check | Actual result |
|---|---|
| B2-03 independent framebuffer/GDI/window smoke | **PASS**, 79 assertions |
| B2-04 native host/platform API test | **PASS**, 25 assertions |
| Production build manifest, exclusion of forbidden source, unchanged public contract | **PASS** |
| Existing architecture, logical input, timing/lifecycle, storage paths, procedural audio | **PASS** |
| Existing whole-game presentation journey source audit | **PASS** |
| Stage 4D+ independent regression sweep | **14/15 pass**, one pre-existing Stage 4F intro test failure |
| `testbuild.ps1` | **FAIL**, `Stage 4F GDR connection intro`: `FAIL: floppy label is unlettered near the drive`, exit code 43, already present in B2-01 |
| Strict MSVC VS2022 Windows x64 Release rebuild (Level4, TreatWarningsAsErrors) | **PASS**; zero C compiler warnings/errors |
| `run.ps1 -build release` | **PASS** in [run 39571220788](https://github.com/WeeJubya/Floppy144/actions/runs/39571220788), including SIZE GATE PASS. |
| `cleanbuild.ps1` | Not passed or claimed: its prerequisite `testbuild.ps1` contains that existing failure |
| Actual Beta.1 versus Beta.2 visual/video/audio A/B | **Not performed**; approved baselines/human desktop evidence pending |

The regression CI job intentionally uses `continue-on-error` for broad legacy suites to allow independent subsequent checks to run, **not** to claim that the broad suite passed. Its overall green status does **not** erase those explicit failed test steps. No existing test assertions were weakened.

## 3. Actual Release executable measurement

For the above integrated code commit, strict clean MSVC rebuild:

- **973,312 bytes used**, versus **1,474,560-byte maximum**, leaving **501,248 bytes**; **PASS**.
- SHA-256: `5C50762B78DA8E59E6772B08E4025E02D0FB63124B5FBDDB7267256933491096` (specific measured build, not a promise for every future rebuild).
- `dumpbin /dependents` listed **USER32.dll, KERNEL32.dll, GDI32.dll, SHELL32.dll, WINMM.dll**, all Windows system DLLs. No `VCRUNTIME140.dll`, third-party engine DLL or stored audio asset.
- First integration attempt had linked `VCRUNTIME140.dll` via `/MD`; selecting Premake `staticruntime("On")` fixed the dependency while remaining safely inside the executable size limit.
- GitHub Actions issued a separate Node.js 20 deprecation warning for checkout/toolchain actions; this is **not** a C compiler warning.

The actual production link builds only the new display/host backend; old renderer files remain in the tree for audit but are excluded by Premake and audited by tests. Windows system libraries remain permitted.

## 4. Faults corrected during B2-04

1. New native host test runner originally incorrectly combined MSVC linker arguments, then omitted `f144_startup_config.c` needed for public calendar dispatch; both isolated harness issues were fixed and the 25-assertion test passed.
2. Existing static source tests require message-queue quit mechanics to reside in the native platform layer. `PostQuitMessage` was moved from the game coordinator into the new host window-destruction boundary; the timing/lifecycle test subsequently passed.
3. Dynamic MSVC CRT dependency eliminated with consistent static runtime selection across three targets.
4. No actual runtime/gameplay failures were resolved by copying or consulting a forbidden inherited implementation.

## 5. Remaining acceptance decisions and provenance

- **Outstanding historical regression:** the Stage 4F intro-label test still expects letters near the drive, despite an earlier approved removal of that intro text. The identical failure was recorded before B2-04; a separate approved test/behaviour review is required. Do not alter visuals or weaken tests merely to force a pass.
- **Human acceptance:** no verified Windows desktop comparison of the intro, profile, terminal, office movement, 2.5D rooms/inspection, Notebook, Grey Door and completion; no recorded black-flash stress/video capture or human audio comparison. Deterministic drawing source tests passed, but this is not the same as a real graphical parity check.
- **Compatibility:** existing storage and procedural audio code has not changed; focused tests passed, but byte-for-byte Beta.1 save/profile/settings round-trip against approved release fixtures is pending.
- **Licence/provenance:** the assistant had earlier exposure to some F144/River2D inherited source during B2-01. The five forbidden files were **not opened, used as templates, translated or ported** in B2-04. The game-owned coordinator was inspected only for necessary integration locations and the non-forbidden timing adapter for its service interface. New native code was authored from public game contracts, B2-02 specifications, independent B2-03 display and Win32 API documentation. **No formal clean-room independence claim.** B2-05 needs a separate reviewer and rights assessment.
- **GPLv3 publication:** the owner specified GPLv3 was intended for Beta.1, but the public GDR repository still lacked the accidentally omitted root `LICENSE` as checked on 21 October 2026. That release compliance correction was not part of this development cutover.
- **Final gate:** fully green `testbuild.ps1`/`cleanbuild.ps1`, approved visual/audio parity and B2-05 independent provenance sign-off remain pending. Do not claim they passed.

**Cutover assessment:** production code, link manifest, Windows unit/integration tests, Release compilation, standalone imports and executable size are **verified**. Full historical regression, actual visual/audio parity and independent provenance sign-off are **not verified**.
