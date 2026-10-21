# B2-05: Derived runtime removal and technical independence audit

**Date:** 21 October 2026. **Development:** `WeeJubya/Floppy144`, `development/Beta-2-runtime`. **Public Beta.1:** `GreyDoorRepublik/Floppy144-beta`, unchanged. **Decision:** derived runtime removal from current source and linked executable is **TECHNICALLY VERIFIED**; **complete legal independence and historical/asset authorship sign-off are NOT CERTIFIED**.

## 1. Audit independence and evidence limitation

This review was performed by the same assistant that conducted B2-01 and wrote B2-03/B2-04. **The assistant had previously read some inherited River2D-derived runtime implementation and integration glue during B2-01**, before the new implementation was authored. This violates the ideal separation of source-exposed analyst from source-unexposed clean-room implementer. B2-04 deliberately did not consult the five forbidden implementations; B2-05 **did** inspect their historical contents **only for forensic comparison**, as explicitly requested by this stage. No code was copied or ported from them during B2-05.

Consequently this is a **technical and historical review with a documented conflict of independence**, **not an independent third-party clean-room audit, copyright clearance or legal opinion**. Require a separate qualified reviewer with access to original source histories, rights documents and author testimony before any formal non-derived/relicensing conclusion.

## 2. Historical source lineage: verified

- Original recorded upstream: `BadAcronym/river2D` at `dab1612e82053c91d818d8ec98677c0632297238` (historical `design/river2d-stock-baseline.md`).
- Historical commit [`709c76c109d09e775ef72785b1b44efc240b7cce`](https://github.com/WeeJubya/Floppy144/commit/709c76c109d09e775ef72785b1b44efc240b7cce), verified from GitHub: message `refactor: rename River2D runtime to F144`; adds `include/f144_runtime.h` and `src/f144_runtime.c`, renames `src/f144_win32_platform.c`, deletes River2D originals. Explicit rename establishes lineage independently of mere string similarity.
- Historical commit [`24d413b626c493504a440f08a7f3db8edc49b5fe`](https://github.com/WeeJubya/Floppy144/commit/24d413b626c493504a440f08a7f3db8edc49b5fe) removed imgsurf/puddle dependencies and introduced `include/string_view.h` and `src/string_view.c`. Their independent authorship was not settled; they were already unlinked, then deleted in B2-05.
- Historical `1090bf05ab86904c01f89024be4d77837a63427c` is a BadAcronym-linked GPLv3 `LICENSE` introduction (current licence retains a River2D/BadAcronym notice). Preserve that history and file.
- Prior `docs/stage4/runtime_provenance.md` reported name-normalised source line matches of **67.5%, 87.8%, 58.9%, 46.0%** for four derived files. These are **October 6 historical comparisons**, not recomputed B2-05 figures and not a legal percentage threshold.
- `docs/stage4/runtime_provenance.md`, `docs/beta2/01_provenance_inventory.md` and design/legacy baseline records remain in source control, including attribution and licence considerations. B2-05 did **not** rewrite Git history or remove the root `LICENSE`.

**Finding:** the five F144 engine/presenter paths were unambiguously derived historically; they must not be portrayed as independent solely because F144 symbols replaced River2D names.

## 3. Current cleanup and source-dependency inspection

**Deleted from current tree:**
1. `include/f144_runtime.h`
2. `src/f144_runtime.c`
3. `src/f144_win32_runtime.c`
4. `src/f144_win32_platform.c` (old derived renderer, **not** `src/floppy144_win32_host.c`)
5. `include/f144_win32_platform.h`
6. `src/string_view.c`
7. `include/string_view.h`
8. `.gitmodules` (obsolete `vendor/puddle` Git submodule declaration)

The tree previously contained **no Gitlink entries** (Git mode 160000), no `vendor/puddle` directory and no active reference in the three-target Premake build. The string-view utility had no compiled consumers after B2-04. Removal is against a normal Git commit; all history remains reachable.

The updated architecture test rejects restored superseded files as well as any attempt to add them to PlatformWin32. New `tools/test_beta2_provenance.ps1 -Phase Source` proves:
- All eight deleted paths remain absent, no vendor directory or Gitlink persists.
- Production `premake5.lua` uses the independent host/display/timing and static CRT in all three projects.
- **212 active C source, header, include, definition, resource and C test files**, including generated definitions, contain none of the forbidden private F144 runtime types, old callbacks, string_view API or direct River2D/vendor source references.
- Exactly **one permitted River2D mention** remains in the `floppy144_credits_view.c` *comment* pointing readers to historical licence/provenance notices. It is deliberately preserved for attribution and is **not** compiled engine code. The audit permits only this one exact credit; other occurrences still fail.
- `LICENSE` and both historical provenance documents remain present.

Review of the current Premake source manifest and dedicated build/test workflow shows no production inclusion of the deleted files. As in B2-04, `Floppy144Core`, `Floppy144PlatformWin32` and the sole `Floppy144.exe` remain. Gameplay, save, generated data, audio synthesis and UI/graphics modules were not rewritten.

## 4. Replacement expression comparison: limited evidence, no automatic legal inference

I examined the newly authored `floppy144_win32_display.c`, `floppy144_win32_host.c`, new private headers and rewritten `f144_win32_timing.c` against the five removed source/header texts **solely to identify suspect retained expression**.

A mechanical text check trimmed whitespace, omitted comment-only lines and lines shorter than 20 characters, then checked exact line identity. It found:
- **0** matching substantive-length lines between the independent display .c and any removed implementation/header.
- **21** matching line instances between the new Win32 host .c and the old derived Win32 platform .c, with up to **11 consecutive** matching lines. On inspection, these consist of retained public platform-service header inclusions, required ordered `F144PlatformApi` callback symbols, one `return &platform->surface;` and generic `F144Platform *platform` parameter declarations. They are explainable by the **public interface contract**, and contain no demonstrated copied framebuffer algorithm or private engine layout. Their presence is nevertheless recorded for independent reviewer examination, not dismissed solely because there is a benign explanation.
- New timing/source and private header overlap was limited to short/generic Win32 macros, type parameters and approved includes in this screen; exact longer structural copy was not established.

This elementary check **does not** detect renamed/reordered algorithms, semantic duplication, common ancestry concealed by refactoring or third-party code introduced through other pathways. It cannot replace human authorship evidence, prior-decision documentation or a legal assessment. Use of documented Win32 `CreateWindowEx`, `GetClientRect`, `StretchDIBits`, `SetTimer` and common scaling mathematics is itself ordinary API/algorithm use, **not proof** of copying. Conversely shared ABI field names are not a legal immunity for any actual copied code.

**Provenance construction record:** B2-02 documents `02_runtime_specification.md`, `02_api_contract.md`, `02_acceptance_matrix.md` and `02_implementation_handover.md` preceded B2-03. New source and test commits, including `66db29dc29fc58ad2bb0e42f91a678946ce13274` (B2-03) and `2bced07019b119d3cf5949273a743df89aa2b7e3` (B2-04 integration), are preserved. This sequence supports the stated specification-led methodology **without proving an unexposed author**.

## 5. Actual verification and executable provenance

Windows x64 Actions [run 39575610546](https://github.com/WeeJubya/Floppy144/actions/runs/39575610546) for source/test-fix commit `95bdfcd5864f7fc179ce3758c43142b8ff295c93` was inspected:

| Gate | Verified observation |
|---|---|
| B2-03 display, pixel/geometry/GDI/window smoke | **PASS**, 79 assertions |
| B2-04 native host/API/timing/quit | **PASS**, 25 assertions |
| B2-05 active-source scan | **PASS**, 212 C/header/generated/test/resource files |
| Updated architecture and active-source exclusion | **PASS** |
| `lib.exe /list` against Release `floppy144platformwin32.lib` | **PASS**: independent `floppy144_win32_display.obj` and `floppy144_win32_host.obj` present; old `f144_runtime.obj`, `f144_win32_runtime.obj`, `f144_win32_platform.obj` and `string_view.obj` absent |
| Strict VS2022 Windows x64 Release `/t:Rebuild`; root `run.ps1 -build release` | **PASS**, zero reported C warnings/errors |
| `dumpbin /dependents` | **PASS**: **USER32.DLL, KERNEL32.DLL, GDI32.DLL, SHELL32.DLL, WINMM.DLL** and no additional DLLs |
| Executable size | **973,312 bytes used; 501,248 remaining of 1,474,560; PASS** |
| SHA-256 for the measured CI binary | `84F9D9ADC1519F6FD377C4A248DB91BAE0419F0B6D2278C90F13046C79401825` |
| Focused input, timing/lifecycle, persistence, procedural audio and presentation source checks | **PASS** |
| Full existing `testbuild.ps1` | **FAIL** at pre-existing Stage 4F intro-label assertion `floppy label is unlettered near the drive`, exit 43 |
| Independent Stage 4D+ sweep | **14/15 PASSED**; same pre-existing intro-label discrepancy |
| `cleanbuild.ps1` | **Not declared passed** because it invokes the failing `testbuild.ps1` before compilation |
| Approved desktop Beta.1/Beta.2 captures, listening comparison and save fixture byte-for-byte tests | **NOT PERFORMED** |

The dedicated CI regression job deliberately marks the two broad historical suite steps non-blocking to permit other evidence collection. **A green overall CI workflow is not a green complete regression suite.** GitHub Actions also displays a Node.js action-runtime deprecation warning distinct from any C compiler warning.

The published Beta.1 executable's reported SHA-256 remains independently unverified. GUI screenshot and audio sign-off are unavailable at this stage.

## 6. Outstanding suspected authorship / licensing issues

| Item | Reason still unresolved | Disposition |
|---|---|---|
| **Embedded 5×7 glyph data** in `game/src/floppy144_draw.c` | 60 case-based glyph patterns observed; no author, originating typeface, source catalogue or licence recorded. Even commonplace glyph geometry cannot be conclusively attributed from appearance alone. **These patterns are compiled in the game.** | **Unresolved, material**. Request original artwork/font creation evidence or replace in a separately authorised visual-parity-preserving stage if rights require. |
| **Earlier game coordinator / ancillary game sources** | Historic integration with River2D types has been cut over, but this stage did not independently reconstruct original authorship for every older Win32 coordinator statement or game helper. Prior source exposure compromises reviewer independence. | **Unresolved** for formal independent authorship review; no *known compiled derived runtime* detected. |
| **App icon resource** | Pixel artwork is compiled into EXE; original designer/source and rights record not established by source history inspected here. | **Unresolved** asset-origin documentation, not evidence of River2D code. |
| **Tracked tool binaries** `tools/site_compiler.exe` / `tools/site_compiler.obj` | Historical binary provenance not independently authenticated. `tools/build_site.ps1` recompiles `tools/site_compiler.c` into `obj/tools` rather than using tracked copies; neither binary is linked into the game. | **Unresolved outside shipping EXE**; consider separate source-package cleanup, do not mislabel them runtime dependencies. |
| **Ownership/licence/attribution** | Retained GPLv3 `LICENSE` includes BadAcronym / river2D historical notice; absence of old compiled code does not determine ownership of all remaining code or licence duties for previously published builds. No verified assignment or non-GPL grant. | Seek specialist legal/licensing review before any proprietary/non-GPL representation. Preserve historical notices. |
| **Public release metadata** | As verified in the public GDR Beta.1 repository, root `LICENSE` remains absent despite owner-confirmed GPLv3 intent and promised correction. | **Owner/release compliance action outstanding.** Development cleanup does not change that repo or retroactively repair publication. |
| **Behavioural parity / failing intro test** | Same older Stage 4F test incorrectly expects disk lettering near drive, previously approved for removal; broad suite remains red. Full actual Windows desktop/screens/audio were not compared against approved captures. | Resolve test expectation in separately reviewed work and conduct visual/audio/manual sessions. |

## 7. Engineering conclusion and required next action

**Passed technical gate:** historical River2D-derived runtime implementations and the uncertain string_view utility are removed from the current tree; the production source manifest, actual native archive and Release EXE imports confirm the **independent Win32 display is the sole compiled/display path**, with no identified River2D engine object/DLL. Size, strict Windows Release and targeted regressions pass. All historical authorship and GPL notices remain recoverable.

**Not passed:** definitive legal independence, a truly source-unexposed independent auditor's sign-off, embedded glyph/icon ownership, public release licence compliance and complete regression/visual/audio parity. The correct status is **technical dependency removal complete, provenance/legal acceptance open**. Do not describe Floppy//144 as conclusively free of all possibly derived expression without additional independently established evidence.
