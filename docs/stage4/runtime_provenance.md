# Stage 4 Runtime Source-Provenance and Licensing Audit

**Audit date:** 2026-10-06  
**Branch audited:** `Stage4`  
**Audit source HEAD:** `1b4dc576a5682b77c30f295cee05f2dfa4727e8a`  
**Purpose:** Establish source provenance and redistribution questions before any Stage 4 cross-platform runtime refactor.

> This is a technical source-provenance audit, not legal advice. Where repository evidence does not establish ownership, relicensing authority, or platform-store compatibility, the issue is explicitly marked for external/legal confirmation.

## Executive status

**REVIEW REQUIRED — currently shipped FLOPPY//144 code includes identifiable River2D-derived GPLv3 runtime material.**

The current Windows executable does **not** depend on an external River2D DLL, imgsurf library, puddle submodule, or other vendor runtime at execution time. However, this does not mean River2D provenance has been removed.

The present build statically compiles an `F144 Runtime` whose source history and code structure show that it was produced by trimming and renaming River2D runtime code. The strongest evidence is:

- historical project documentation explicitly states that the River2D software renderer was compiled directly into `Floppy144.exe`;
- commit `709c76c109d09e775ef72785b1b44efc240b7cce` is titled **`refactor: rename River2D runtime to F144`**;
- that commit adds the F144 files while deleting/renaming their River2D counterparts;
- direct comparison with the recorded River2D baseline commit `dab1612e82053c91d818d8ec98677c0632297238` shows substantial retained code after identifier renaming;
- the repository's root `LICENSE` is GPLv3 material inherited from River2D and still contains the River2D/BadAcronym application notice;
- the same GPLv3 licence blob appears in the recorded upstream River2D baseline.

Therefore the F144 name should **not** be treated as evidence of an independently reimplemented runtime.

The main unresolved question is whether the FLOPPY//144 project has any separate written permission, assignment, dual-licence grant, or other relicensing right from the River2D copyright holder beyond the public GPLv3 terms. The repository does not establish that.

## Audit scope

The following were inspected:

- current `Stage4` repository tree;
- runtime source and headers;
- game source and headers;
- root licence;
- README and historical runtime design notes;
- Git history for the runtime migration, licence, submodules, and utility extraction;
- current Premake build wiring;
- generation/build/test scripts;
- generated source/data;
- stale submodule metadata;
- embedded font data;
- audio-related source/build references;
- tracked build binaries;
- recorded River2D baseline source and upstream repository metadata.

No code was removed, replaced, relicensed, or refactored during this audit.

## Important historical evidence

### River2D baseline recorded by this repository

`design/river2d-stock-baseline.md` identifies:

- upstream repository: `BadAcronym/river2D`;
- upstream branch: `gh-stable`;
- source commit: `dab1612e82053c91d818d8ec98677c0632297238`;
- stock Windows runtime using `river2Dsoftware.dll`.

`design/floppy144-stock-runtime.md` then records the first FLOPPY//144 build using River2D's dynamically loaded software renderer.

`design/floppy144-static-runtime.md` explicitly states:

> The river2D software renderer is compiled directly into Floppy144.exe.

It also records that the external `river2Dsoftware.dll` was no longer required after the static transition.

This is dependency removal, not source-provenance removal.

### Runtime migration history

Relevant repository commits include:

| Commit | Message | Provenance significance |
| --- | --- | --- |
| `24d413b626c493504a440f08a7f3db8edc49b5fe` | `refactor: remove imgsurf and puddle dependencies` | Removes active vendor use and introduces a small local `string_view` implementation. |
| `709c76c109d09e775ef72785b1b44efc240b7cce` | `refactor: rename River2D runtime to F144` | Directly establishes that F144 originated as a renamed River2D runtime. |
| `601c0600a8ed045716b1613359dbec686607316a` | `chore: finish F144 runtime cleanup and add size gate` | Further trims/cleans the renamed runtime. |
| `1090bf05ab86904c01f89024be4d77837a63427c` | `update LICENSE` | Adds the current GPLv3 licence; commit is authored by the BadAcronym account. |

### Direct source comparison

A line-level technical comparison was made between current F144 runtime files and the repository-recorded River2D baseline at `dab1612e...`.

For this comparison only, obvious `River`/`river` and `F144`/`f144` identifier naming was normalised before counting exact retained lines.

| Current file | River2D baseline source | Current lines matching after name normalisation |
| --- | --- | ---: |
| `include/f144_runtime.h` | `include/river2D_main.h` | 67.5% |
| `src/f144_runtime.c` | `src/river2Dcommon_main.c` | 87.8% |
| `src/f144_win32_runtime.c` | `src/win32_river2Dcommon.c` | 58.9% |
| `src/f144_win32_platform.c` | `src/win32_river2Dsoftware_platform.c` | 46.0% |

These percentages are **not legal tests** and should not be treated as such. They are technical provenance evidence. Combined with the explicit rename commit and historical design notes, they strongly support classification of the F144 runtime as **derived code**.

## Current shipping build

`premake5.lua` currently builds two relevant targets.

### F144 Runtime static library

Compiled from:

- `src/f144_runtime.c`
- `src/f144_win32_runtime.c`
- `src/string_view.c`
- `include/f144_runtime.h`
- `include/string_view.h`

### Floppy144 application

Compiled from:

- `game/src/**.c`
- `game/src/**.h`
- `src/f144_win32_platform.c`
- `include/f144_win32_platform.h`

Linked against:

- `F144 Runtime`
- Windows `user32`
- Windows `gdi32`

Therefore River2D-derived F144 code is currently present in the shipped executable even though no River2D-named DLL is shipped.

## Component provenance table

| Component / files | Purpose | Classification | Apparent origin and evidence | Licence / notice present | Compiled into shipped game? | Redistribution concern | Action |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `include/f144_runtime.h`, `src/f144_runtime.c` | Core image/runtime/time/types and renderer-facing API | **Derived code** | Explicit River2D -> F144 rename commit; strong source similarity to `river2D_main.h` and `river2Dcommon_main.c` | No per-file notice. Root GPLv3 licence inherited from River2D. | **Yes**, via `F144 Runtime` static library | **High if non-GPL/proprietary relicensing is intended** | Confirm rights/permission or plan replacement; preserve River2D attribution. |
| `src/f144_win32_runtime.c` | Win32 common runtime support, image creation, timing, wrappers | **Derived code** with later FLOPPY modifications | Created by River2D -> F144 migration; substantial source match to `win32_river2Dcommon.c`; later Stage 3 edits exist | No per-file notice | **Yes** | Same GPL/relicensing question as core runtime | Confirm rights or replace as separately agreed work. |
| `include/f144_win32_platform.h`, `src/f144_win32_platform.c` | Win32 software framebuffer/presenter and composition | **Derived code** with significant later modification | Rename commit shows direct River2D renderer/platform rename; historical static-runtime document says River2D software renderer was compiled into the EXE | No per-file notice | **Yes**, directly in application target | Same GPL/relicensing question; this is the most platform-specific piece likely to be replaced during cross-platform work | Do not merely port this code to more platforms until licensing strategy is agreed. |
| `src/string_view.c`, `include/string_view.h` | Minimal string-view utility | **Rewritten/reimplemented code with historical third-party lineage** | Introduced by commit `24d413b...` while removing puddle dependency. API names and type originated in the prior puddle-based build, but current implementation is a small local subset and differs from current puddle implementation | No per-file notice | **Yes**, in F144 Runtime | **Medium/unclear for relicensing**, though less significant than the F144 runtime itself | Confirm whether implementation was independently written; if uncertain, trivial to replace later with a clearly original utility. |
| `game/src/floppy144_main.c` | Win32 entry point and top-level game coordinator | **Original FLOPPY//144 code with derived-runtime integration glue** | Project-specific game flow; history shows it previously included River2D headers and was updated to F144 types during rename | No per-file licence notice | **Yes** | Depends on derived F144 API; some binding/platform glue is historically entangled | Preserve as game code but isolate from replacement runtime in future task. |
| Other hand-written `game/src/floppy144_*.c/.h` | Game state, terminal, catalogue, site, persistence, UI, gameplay | **Original FLOPPY//144 code**, based on repository evidence | Project-specific names/data/logic; no River2D/BadAcronym/puddle/GPL attribution markers found during current source scan | No per-file licence notice | **Yes** | Root licence scope and combined-work obligations need confirmation, but no separate upstream source provenance was found | Add explicit project copyright/licensing headers once licensing strategy is decided. |
| `game/src/floppy144_draw.c` embedded 5x7 glyph table | Bitmap text glyph shapes and drawing primitives | **Provenance unclear / investigation required** for glyph data; drawing code otherwise appears project-specific | File says font is embedded but gives no font source or attribution. No external font asset exists in tree. | None | **Yes** | Low-to-medium, especially if future relicensing is desired | Confirm glyph table was authored for FLOPPY//144; otherwise replace with an explicitly original/public-domain-compatible table. |
| `data/floppy144_game_data.json` | Canonical game/content data | **Original FLOPPY//144 data**, based on repository evidence | Current project canonical source; no external code dependency | No separate notice | Indirectly, after generation | No specific third-party code issue found; content ownership remains a normal project concern | Keep as authoritative provenance source. |
| `game/src/*.generated.inc`, `*.generated.def`, generated site JSON/definitions | Runtime tables and generated content | **Generated code/data** | Scripts explicitly mark files as generated from canonical FLOPPY//144 JSON/site data | Generated-file comments, no separate licence | **Yes**, where included by compiled sources | Inherits provenance of generator inputs; no independent third-party source found | Continue marking generated files and retain canonical inputs. |
| `tools/game_data_compiler.c`, `tools/game_data_emit_runtime.inc`, `tools/site_compiler.c` | Development-time compilers/generators | **Original FLOPPY//144 build-support code**, based on repository evidence | Project-specific canonical-data/site pipeline; no River2D/vendor references found | No per-file notice | **No** | Low current shipping risk | Add project licence header later. |
| `game_data_compiler.c` at repository root | Older/dormant game-data compiler | **Original/legacy FLOPPY//144 build support** | Not used by current `tools/build_game_data.ps1`; contains stale Stage 3 counts | No notice | **No** | No runtime concern; repository housekeeping issue | Consider removing/archive in separate cleanup task after confirming no external use. |
| `tools/site_compiler.exe`, `tools/site_compiler.obj` | Tracked build artefacts | **Generated build artefacts** | Source `tools/site_compiler.c` is present and current build script recompiles the tool | None | **No** | Not a shipped-game dependency, but unnecessary binary provenance surface in source repository | Consider deleting from version control in a separate cleanup task. |
| `.gitmodules` entry for `vendor/puddle` | Historical submodule metadata | **Historical third-party reference; currently inactive** | File still points to `BadAcronym/puddle`; current repository tree contains no `vendor/puddle` gitlink/directory; current Premake does not reference it | Upstream puddle reports GPLv3 | **No** | No current binary dependency; stale metadata can mislead future audits | Remove stale `.gitmodules` entry in separate housekeeping task after confirmation. |
| Historical imgsurf dependency | Image loading in old River2D build | **Historical third-party dependency; removed** | River2D baseline used imgsurf; commit `24d413b...` removes imgsurf; no current vendor tree/build link exists | Not present in current tree | **No** | No current runtime concern found | No action beyond documenting removal. |
| Windows `user32`, `gdi32`, Kernel/CRT APIs | OS windowing/presentation/runtime | **Platform/system libraries** | Explicit Premake links and ordinary Win32 API calls | External Microsoft terms, not repository source | **Yes**, dynamically/system-linked | Normal platform packaging/licensing consideration, not River2D provenance | Reassess per target platform during architecture work. |
| Premake 5 and GitHub Actions | Build/project-generation/CI tooling | **Third-party build tooling** | Workflow downloads Premake; Actions referenced by workflow | Their own external licences | **No** | Not shipped in game executable | Keep versions pinned/documented. |
| Save/profile/settings files tracked at repository root | Example/runtime state artefacts | **FLOPPY//144 generated/runtime state** | Produced by game persistence systems | None | **No**, unless mistakenly packaged | Packaging hygiene only | Exclude from release packages unless intentionally required. |

## GPL findings

### Facts visible in the repository

1. The repository contains a root `LICENSE` whose content is GNU GPL version 3.
2. The current licence file still contains the application notice:
   - `A cross-platform 2D graphics engine.`
   - `Copyright (C) 2026 BadAcronym.`
   - an example runtime notice naming `river2D`.
3. The local `LICENSE` blob is the same blob recorded in the River2D baseline source tree.
4. Git history shows the licence was added in commit `1090bf05...`, authored by the BadAcronym account.
5. The recorded upstream `BadAcronym/river2D` repository identifies its licence as GPL-3.0.
6. Current F144 runtime files are demonstrably derived from that River2D source lineage.
7. Current FLOPPY//144 source files generally do **not** contain per-file copyright or SPDX/licence headers.

### Reasonable technical inference

Absent a separate permission grant, the safest technical assumption is that the River2D-derived runtime must continue to be treated as GPLv3-covered material.

Because that runtime is statically linked/compiled into the shipped `Floppy144.exe`, a future distribution plan cannot safely reason about FLOPPY//144 as though the River2D component were merely an optional external tool.

The presence of a root GPLv3 file alone would not prove the legal status of every independently authored FLOPPY//144 file. In this case, however, the derived runtime has separate provenance evidence tying it directly to the GPLv3 River2D source.

GPLv3 permits commercial distribution in principle; the key issue is compliance with its redistribution conditions, not whether money is charged. Store/device terms can add separate compatibility questions.

These are technical/provenance inferences, not a legal opinion.

### External/legal confirmation required

The repository cannot answer the following:

1. Does the FLOPPY//144 author hold separate written permission from BadAcronym to relicense, dual-license, or incorporate the River2D-derived code under non-GPL terms?
2. Was any copyright assignment or contributor agreement made outside Git?
3. Is the current root GPLv3 licence intentionally meant to cover the whole FLOPPY//144 project, or was it inherited solely from the original River2D codebase?
4. If FLOPPY//144 remains GPLv3, what exact source-offer, notice, attribution, modified-version, and installation-information obligations apply to each intended distribution channel?
5. Are the terms of the intended Windows/macOS stores, Google Play, Apple App Store, iOS signing/distribution model, and any future console/mobile channel compatible with the chosen GPLv3 distribution approach?
6. If proprietary or otherwise non-GPL distribution is desired, is a clean replacement of all River2D-derived code sufficient, or are additional historical contributions/assets subject to separate rights?

Do not infer answers to these from repository ownership or from the fact that the code is publicly visible.

## Third-party/submodule status

### Puddle

`.gitmodules` still contains:

`vendor/puddle -> git@github.com:badacronym/puddle`

However:

- no `vendor/puddle` entry exists in the current Stage4 Git tree;
- current Premake does not include or link puddle;
- current runtime uses local `string_view.c/.h`;
- commit history explicitly says the puddle dependency was removed.

**Conclusion:** puddle is not a current shipped dependency. The metadata is stale.

### imgsurf

The River2D baseline used imgsurf. FLOPPY//144 history subsequently removed the dependency, and current Premake/source tree does not contain or link imgsurf.

**Conclusion:** no current imgsurf runtime dependency found.

### Other vendor code

No active `vendor/` directory or third-party source tree is present in the current Stage4 tree.

No stb-style single-header libraries, external image loaders, SDL, OpenGL loader, audio library, or equivalent embedded third-party source was found.

## Audio provenance

The current Stage4 tree contains persistence/settings support for music volume, but no implemented audio playback backend was found.

Specifically, the current shipped build has no identified:

- WinMM link;
- MIDI playback source;
- wave output source;
- external audio library;
- embedded music file;
- embedded sound-effect file;
- audio asset directory.

Therefore any previously discussed WinMIDI/F144SFX work is **not part of the current Stage 3/Stage4 baseline source audited here**.

This is useful for Stage 4: future audio work starts as a new provenance surface and should receive explicit provenance/licence documentation as it is introduced.

## Rendering provenance

There are two distinct rendering layers and they must not be confused.

### Platform framebuffer/presenter

`src/f144_win32_platform.c` is River2D-derived F144 runtime/platform code and is currently compiled directly into `Floppy144.exe`.

This layer allocates/presents the software framebuffer and contains Win32 rendering/platform functionality.

### FLOPPY//144 drawing/UI renderers

Files such as:

- `game/src/floppy144_draw.c`
- `game/src/floppy144_drawing_runtime.c`
- `game/src/floppy144_site_2d.c`
- `game/src/floppy144_site_isometric.c`
- `game/src/floppy144_cabinet.c`

contain project-specific procedural drawing and UI logic.

No direct River2D source markers were found in these modules during this audit. They use the F144 framebuffer/runtime interface but are not themselves established as River2D copies by current evidence.

The embedded 5x7 glyph shapes in `floppy144_draw.c` remain the one presentation-data item whose authorship is not documented in source.

## Cross-platform implications

The current Premake configuration supports only `windows`, despite residual `BUILD_LINUX` fields/types in the F144 header inherited from River2D history.

There is no current Linux, macOS, iOS, or Android platform backend in the FLOPPY//144 tree.

Before creating those backends, the project should decide whether it is:

1. intentionally continuing a GPLv3 River2D-derived runtime lineage; or
2. replacing the derived runtime/platform layer with clearly original FLOPPY//144 code (or a deliberately chosen third-party library under compatible terms); or
3. operating under a separately documented permission from the relevant copyright holder.

Porting the existing F144 runtime first and resolving provenance later would spread the unresolved derived-code surface across more platforms and make replacement harder.

## Recommended actions, in priority order

1. **Resolve River2D rights before the cross-platform refactor.**  
   Confirm whether there is any separate written licence, permission, assignment, or dual-licensing agreement with BadAcronym. Store that evidence with the project records.

2. **Choose the intended Stage 4 distribution/licensing model.**  
   Decide explicitly whether FLOPPY//144 is intended to remain GPLv3-compatible, or whether future distribution requires a non-GPL/proprietary-compatible runtime.

3. **If no suitable separate permission exists and non-GPL distribution is required, create a separate clean replacement task.**  
   Replace the River2D-derived F144 runtime/platform files rather than incrementally porting them. Keep this replacement isolated from gameplay work and validate against the immutable Stage 3 baseline.

4. **If GPLv3 distribution is retained, perform a channel-specific compliance review before mobile release.**  
   Cover source availability, notices, modified-version marking, installation/signing implications, and each storefront's then-current terms. Give iOS/App Store distribution particular attention rather than assuming compatibility.

5. **Document or replace the embedded 5x7 glyph table.**  
   Confirm it was authored for FLOPPY//144. If that cannot be established, replace it with clearly original glyph data during a separately approved presentation task.

6. **Clean stale provenance metadata after the licensing decision.**  
   Remove the inactive puddle `.gitmodules` entry and tracked compiler `.exe/.obj` artefacts in a dedicated housekeeping task. Do not mix that cleanup with this audit commit.

7. **Add explicit provenance/licence headers.**  
   Once the licensing strategy is settled, add SPDX identifiers and copyright/provenance notices that distinguish:
   - original FLOPPY//144 files;
   - River2D-derived files, if retained;
   - generated files;
   - third-party components.

8. **Apply the same audit discipline to new Stage 4 dependencies.**  
   Any future SDL, audio, font, image, controller, mobile, or platform library should enter the repository with its licence, version, source URL, usage mode, and shipped/not-shipped status documented immediately.

## Final status

# REVIEW REQUIRED

No evidence was found that FLOPPY//144 currently ships an external River2D DLL, puddle library, imgsurf library, or other third-party runtime binary.

However, there **is** strong evidence that currently compiled F144 runtime/platform code is River2D-derived and originates from a GPLv3 codebase. The repository does not establish any separate relicensing permission.

That unresolved rights/distribution decision should be closed **before** Stage 4 spreads the runtime across Windows, Linux, macOS, iOS, and Android.

This audit does not declare the project legally blocked. It declares that the provenance question is real, current, and material enough that proceeding with a cross-platform port before resolving it would create avoidable technical and licensing risk.
