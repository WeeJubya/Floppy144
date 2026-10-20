# B2-01: Beta.1 baseline and verification
Audit date: 20 October 2026. Documentation-only source/provenance baseline. No executable, gameplay, runtime or licence changes.

## Repository ownership and immutable points
- Development repository: https://github.com/WeeJubya/Floppy144
- Migration branch: `development/Beta-2-runtime`, initially created at `46c2a81e097abf69734a4b2f0b7bc91f9142c256`, from `development/Stage-4` (not WeeJubya/main).
- WeeJubya/main at audit: `fda69241a4e81e3099df140eaef0fdf2993e018d`; it is **704 commits behind** Stage-4. Using it as a Beta.2 starting point would omit most of the game.
- Published Beta.1 reference: https://github.com/GreyDoorRepublik/Floppy144-beta, `main` and tag `Floppy144` both at `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`. **Do not edit that release branch**.
- Release: `Floppy//144 0.4.0-beta.1`, pre-release published 2026-10-20 18:51:45 UTC: https://github.com/GreyDoorRepublik/Floppy144-beta/releases/tag/Floppy144
- Asset: `Floppy144.0.4.0-beta.1.exe` (the published release contains one EXE asset, not a ZIP).
- GitHub-provided asset size: **822,272 bytes**. Budget 1,474,560 bytes; headroom **652,288 bytes**; metadata size gate **PASS**.
- GitHub-provided asset SHA-256: `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749`. This is a release metadata digest, **not** an independent hash calculated from downloaded bytes. Independent download/hash verification is outstanding.
- Source comparison by Git blob SHA: 299 public Beta.1 blobs; 292 identical to the current Stage-4 tree, seven different, none unique to Beta.1; Stage-4 has 368 blobs. Seven changed paths: `.gitignore`, `README.md`, `game/src/floppy144_intro.c`, `game/src/floppy144_intro.h`, `game/src/floppy144_main.c`, `tools/stage4f_intro_tests.c`, `tools/test_stage4f_intro.ps1`. Therefore **Stage-4 is not byte-identical to the released Beta.1 source**. Do not describe its CI output as testing the released Beta.1 commit.

## Source/build map
- `premake5.lua`: C99, Windows x64, Visual Studio 2022; `Floppy144Core` portable static library; `Floppy144PlatformWin32` native static library (contains River2D-lineage support); `Floppy144` windowed executable/launcher and Win32 persistence glue.
- Native system links: `user32`, `gdi32`, `shell32`, `winmm`. No active external River2D DLL/imgsurf/puddle runtime is in the compiled target list. WinMM plays project-generated MIDI and wave audio.
- Canonical inputs: `data/floppy144_game_data.json`, `site_layout.jsonc`; `tools/build_game_data.ps1` and `tools/build_site.ps1` generate tables/site definitions. Generated files are not an independent source provenance claim.
- Build scripts: `testbuild.ps1` runs ordered regressions; `run.ps1 -build release` generates via Premake, uses MSBuild, enforces EXE size; `cleanbuild.ps1` checks diff, removes build artefacts, runs regressions then Release build. `.github/workflows/stage3c-ci.yml` runs on `windows-2022`, configures MSVC x64, downloads Premake v5.0.0-beta8, builds VS2022 targets.
- Relevant source: `game/src/` game/UI, `include/` API/headers, `src/` native support and audio, `platform/win32/` resources, `tools/` compilers and tests. A compiled `tools/site_compiler.exe` is tracked in both source trees and is **not** the released game executable.
- Original/project-specific modules, subject to normal rights verification: gameplay state, world/site, terminal, notebook, menus, recovery, persistence, draw/2.5D presentation, generators, procedural MIDI/SFX. Derived runtime and uncertain utility/glyph provenance are separated in `01_provenance_inventory.md`.

## Actual Windows checks available at the exact development starting SHA
The audit environment had no Windows MSBuild, Premake, PowerShell, Win32 desktop session or external Git clone transport. It **did not run new local regressions, compile a new build, launch the EXE or capture screenshots**.

Existing GitHub Actions run at Stage-4 SHA `46c2a81e097abf69734a4b2f0b7bc91f9142c256`:
- https://github.com/WeeJubya/Floppy144/actions/runs/39486754386
- Main `Stage 4 Release Candidate Gate`: **FAIL**. Canonical data regeneration and the earlier source/regression steps through the Stage 4F intro source test succeeded; `Stage 4F GDR connection intro` failed with `FAIL: floppy label is unlettered near the drive`, test exit 43. Subsequent full-gate Release steps were **skipped**. This is a pre-existing baseline discrepancy requiring a decision on whether the test expectation is stale.
- Independent Stage 4D+ sweep: **FAIL** on the same intro assertion.
- Separate `Audio integration / release build` job: **PASS**; MSVC 2022 / Premake / VS2022; clean Release `/t:Rebuild`, `WarningLevel=Level4`, `TreatWarningsAsErrors=true`; log summary **0 warnings, 0 errors**, and EXE size **822,272 bytes**. That is an independently compiled **Stage-4** executable and is not proof of a clean published Beta.1 build.
- Other workflow jobs at the same SHA: BF10, BF18, BF25 and BF28 **success**. No B2-01 branch-specific Windows CI run was triggered by this documentation change; workflow branch filters do not include `development/Beta-2-runtime`.
- The `testbuild.ps1` *standalone* complete sequence and `cleanbuild.ps1` *standalone* orchestrator were **not executed in this audit**. Full-suite PASS is not established.
- Beta release SHA-256 is from GitHub metadata only; not verified against the Stage-4 CI EXE.

## Behavioural preservation acceptance matrix
| Area | Beta.1 characteristics to reproduce | Existing coverage / outstanding evidence |
|---|---|---|
| Framebuffer | Software 32-bit pixel surface, legacy BGRA/Windows DIB semantics, allocation/resize, clear/composite and native viewport dimensions | Platform/drawing tests; pixel-format and resize captures outstanding |
| Presentation | 640x360 game drawing space; integer/aspect-correct scaling and client-area letterbox bars; hard pixel edges/no black flash; correct repaint | Stage 4 platform and presentation tests; actual Win32 screenshots outstanding |
| Input | Physical key down/up identity, movement/navigation, repeat, text separate from actions, focus loss, mouse where used | Stage 4 logical-input tests; keyboard/IME/focus manual checks outstanding |
| Window/lifecycle | Correct entry, initialisation, WM_PAINT, WM_CLOSE, WM_DESTROY, suspend/inactive, quit and resource destruction | Lifecycle tests passed in existing CI; actual close/reopen and handle checks outstanding |
| Timing | Monotonic millisecond clock, timer wake cadence, frame progression, deterministic gameplay variation | Stage 4 timing tests passed; real frame pacing and long-run stability outstanding |
| Audio | Procedural MIDI plus waveOut SFX, menu and completion cues, scene changes, volume/mute, cleanup; no stored audio dependency | Separate audio tests and Release job passed; listening/audio-device verification outstanding |
| Storage | Manual save, autosave, profile, settings, legacy paths, atomic/recoverable data and migration | Stage 4 persistence/profile/settings tests passed; real clean-user and upgrade checks outstanding |
| Shutdown | Stop timers/audio, free surfaces, release Win32 resources, leave no lock/ghost process | Source tests; real close/crash/second-launch behaviour outstanding |

## Repeatable visual baseline protocol (NOT YET EXECUTED)
Use a clean Windows 10/11 x64 VM with screen scaling 100%, a documented desktop resolution, and the **downloaded Beta.1 release asset** verified to the recorded SHA-256. Record Windows build, monitor/scaling, GPU, session settings, application window client size, audio setting and capture method. Use a copied, known save/profile fixture (record SHA-256), never overwrite a tester's real save. Record difficulty, deterministic seed/fixed date if explicitly supported by the existing developer settings, elapsed time and input sequence per capture. Collect **lossless client-area screenshots and, where instrumented independently, raw 640x360 surface BGRA captures** for: intro scenes including floppy approaching drive, login/main menu, session control, profile, settings, credits, terminal, document reader, notebook/ledger, 2D site, isometric site, inspection cabinet/2.5D furniture, Grey Room, completion, restore session, and error/exit paths. Preserve PNG/RAW file SHA-256 plus sidecar capture metadata and steps under a future `docs/beta2/baseline_captures/` directory; do not generate or commit placeholder images. Verify colour channels, edge scaling at native / 2x / non-integer window sizes, resize repaint, letterboxing, audio by human listening and shutdown. No screenshots, framebuffer hashes or human acceptance are claimed here.

## B2-01 limits
A source-audit classification is evidence about provenance, **not** a legal determination. Historical source matching remains only in this inventory; future independently written specifications must describe observable behaviour, not copy source implementation patterns. See `01_risk_register.md` and `STATUS.md`.
