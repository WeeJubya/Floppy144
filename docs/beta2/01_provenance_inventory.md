# B2-01: Runtime provenance and dependency inventory
Audit date: 20 October 2026. This document is **historical/source evidence**, not an implementation recipe or clean-room specification.

## Evidence chain
1. Original `BadAcronym/river2D` baseline `gh-stable` commit `dab1612e82053c91d818d8ec98677c0632297238` is recorded in both repositories' `design/river2d-stock-baseline.md`.
2. `design/floppy144-static-runtime.md` records compiling the River2D software renderer into the game EXE, removing the need for the separate DLL, **not** eliminating source provenance.
3. WeeJubya commit `709c76c109d09e775ef72785b1b44efc240b7cce` explicitly renamed River2D runtime files to F144. Prior commit `24d413b626c493504a440f08a7f3db8edc49b5fe` removed imgsurf/puddle active dependencies; `601c0600a8ed045716b1613359dbec686607316a` later trimmed the renamed runtime.
4. Historical `docs/stage4/runtime_provenance.md` (audit dated 2026-10-06 at `1b4dc576a5682b77c30f295cee05f2dfa4727e8a`) reports, after River/F144 identifier normalisation, retained matching lines of 67.5% (`include/f144_runtime.h`), 87.8% (`src/f144_runtime.c`), 58.9% (`src/f144_win32_runtime.c`), and 46.0% (`src/f144_win32_platform.c`). These are **historical technical comparisons, not newly reproduced percentages or legal thresholds**.
5. Fresh B2-01 public Beta.1 tree and source inspection confirms the same named derived source files, symbols, retained F144 layout, Win32 adapter and live Premake inclusion. The source files and headers in this group have identical Git blob SHAs between the Beta.1 release tree and Stage-4 starting tree. The earlier audit is corroborating evidence, **not** a substitute for checking this published source.
6. WeeJubya retains a root GNU GPL version 3 licence with BadAcronym/River2D lineage. The project owner confirms **GPLv3 was intended for Beta.1**, and the absent `LICENSE` in the Grey Door Republik public source tree is an accidental omission with correction planned for 21 October 2026. At audit time the file is not there and the published release consists of one EXE; this is a publication-state discrepancy, not a change of intended licence. There is no documented separate non-GPL relicensing grant in the inspected repositories.

## Demonstrably River2D-derived shipping code
| File | Role and provenance | Migration disposition |
|---|---|---|
| `include/f144_runtime.h` | Renamed River2D core data model and API: `F144Runtime`, `F144Image`, `F144Time`, `F144Controls`, `F144Config`, Win32 window/DC/cursor fields, composition operations, legacy key constants | **Replace implementation and derived definitions**, but catalogue compatibility contracts without copying layouts |
| `src/f144_runtime.c` | Renamed runtime timing-delta and image append/destroy functionality | **Replace** |
| `src/f144_win32_runtime.c` | Renamed Win32 image creation, counter time, cursor, window client size and dispatch wrappers | **Replace** |
| `src/f144_win32_platform.c` | Directly renamed River2D Win32 software presenter/compositor/text loader with later original Stage-4 platform glue. Allocates `runtime->backbuffer`, calls `StretchDIBits`, handles client-area scaling/letterbox and binds platform services | **Separate/replace derived portions**; preserve independently introduced platform-service behaviour |
| `include/f144_win32_platform.h` | Win32 runtime/presenter declarations and later platform binding; historical rename lineage and direct `F144Runtime` dependence | **Replace derived declarations/legacy API references** |

The original River2D symbols/types are **not proof of origin on their own**; the direct rename history and historical source comparison are the grounds for the derived classification. Statically linking these files does not cleanse provenance. Avoid rewriting them line-by-line under a new name.

## Dependent but not demonstrated copies
| File(s) | Dependency to remove or preserve without assuming derivation |
|---|---|
| `game/src/floppy144_main.c` | Project-specific coordinator/Win32 launcher; `F144Runtime runtime`, plane allocation, `f144Win32Init`, `f144Win32Shutdown`, `f144Win32BltBuffer`, `f144Win32LoadText`, `f144Win32CompositeImage` function-pointer assignments (about lines 307-313 in released source); `f144Win32PlatformBind` around line 4624; Win32 window creation, message loop and WM_PAINT. **Original gameplay/coordinator with derived API integration**, not wholesale River2D code. |
| `include/f144_platform.h`, `src/f144_platform.c` | Stage-4 platform-neutral `Floppy144Surface` and `F144PlatformApi` (framebuffer, present, input contracts, persistence, audio, monotonic clock, quit). Project-specific boundary to **retain behaviourally**; carefully audit indirect linkage. |
| `src/f144_win32_timing.c`, `include/f144_win32_timing.h` | Stage-4 timer-wake adapter casts `platform->state` to `F144Runtime *` and accesses HWND; must decouple from derived runtime. |
| `src/f144_win32_lifecycle.c`, `src/f144_win32_input.c`, corresponding headers | Native WM_CLOSE/WM_DESTROY and logical/typed key mapping. Preserve events; implementation origins appear project-specific and need normal provenance checks. |
| `src/f144_win32_single_instance.c`, `src/f144_win32_storage.c`, `src/f144_win32_startup_config.c` and headers | Independent Windows services requiring compatibility verification, not automatically classified as River2D-derived. |
| `game/src/floppy144_persistence.c`, `game/src/floppy144_storage.c` | Project save/path/migration glue, currently in application target; preserve old on-disk formats and paths. |
| `game/src/floppy144_draw.c`, `game/src/floppy144_drawing_runtime.c`, site, terminal, cabinet, notebook and 2.5D modules | Game-specific software drawing and scene logic consume `Floppy144Surface`; no direct copied River2D source established by the inspected evidence. Never discard simply because of a shared `f144` name. |
| `src/f144_win32_audio.c`, `src/f144audio.c`, `src/f144sfx.c`, `game/src/floppy144_audio.c` and headers | Newer original-looking project procedural MIDI/waveOut generation and WinMM adapter. This **exists in Beta.1**, unlike the older October 6 audit's audio snapshot. Preserve generated sound; rights check not otherwise settled. |

## Active build references and dependencies
`premake5.lua` compiles `src/f144_runtime.c`, `src/f144_win32_runtime.c`, `src/f144_win32_platform.c`, `src/string_view.c`, their headers, WinMM audio and other native services into `Floppy144PlatformWin32`; `Floppy144` links this static library and `Floppy144Core`. Native links are `user32`, `gdi32`, `shell32`, `winmm`. Existing `include/f144_runtime.h` also contains historical BUILD_LINUX fields, but the current Premake platform selection is Windows only. No external River2D DLL is required in the released EXE; this does not mean the EXE is independently implemented.

Direct adapter flow at Beta.1: the launcher creates/binds `F144Runtime`; the Stage-4 `F144PlatformApi` exposes the runtime's `backbuffer.data` as `Floppy144Surface.pixels`, calls the legacy `bltBuffer` callback on present, and the window procedure requests that presentation from `WM_PAINT`. Native wake/quit and audio/storage dispatch use associated platform state. Behaviour is a compatibility requirement, but the historical code itself **must not be copied into new implementation documents**.

## Uncertain and historical third parties
- `include/string_view.h`, `src/string_view.c`: local small utility introduced during removal of BadAcronym/puddle. Historical third-party API lineage; exact independent authorship/relicensing authority **unverified**. Review/replace separately if necessary, rather than assuming the tiny implementation is public domain.
- `game/src/floppy144_draw.c`: embedded 5x7 glyph bit patterns have no documented original source or attribution. The game drawing code appears project-specific; **glyph data provenance is unresolved**.
- `tools/site_compiler.exe`: tracked tool binary; audit binary origin/reproducibility before re-distributing any source archive; not a runtime module of the published game.
- `BadAcronym/puddle` and `imgsurf`: historical references; neither an active vendor tree nor active current game link was identified in the public Beta.1 Premake target. Do not label these as shipped DLLs.
- Generated `.generated.inc`/`.generated.def` and JSON/site tables: derived from project canonical authored data; inputs and generators still require their own authorship assurance, not a blanket external licence assumption.
- Toolchain Premake, GitHub Actions and Microsoft system libraries are development/platform dependencies, not imported River2D runtime source.

## Licence and published-release checks
**The project owner has confirmed GPLv3 as Beta.1's intended licence.** The public `LICENSE` omission was accidental; addition is planned for 21 October 2026. As inspected on 20 October, the public source tree still lacks that file, although `premake5.lua` has `postbuildcommands` copying `%{wks.location}/../LICENSE`, the credits source refers to a distributed `LICENSE`, and the GitHub release lists only an EXE asset. Treat the licence as intended, but verify the correction when published; do not claim the file was physically supplied or that all historical distribution obligations have already been met. Confirm corresponding-source and notice delivery for actual packages. GPLv3 intention is not evidence of a separate right to relicense derived River2D code under non-GPL terms. Obtain human/legal review when that right or compliance is in doubt.

**Separation rule:** This file and the dated Stage-4 provenance report are evidence for the historical audit only. B2-02 clean-room engineering should use independently authored, behavioural acceptance requirements from `01_baseline.md`, not source similarity, original internal routines or layout as instructions.
