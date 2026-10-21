# B2-05: Dependency and provenance inventory

**Audit date:** 21 October 2026. **Repository:** `WeeJubya/Floppy144`, `development/Beta-2-runtime`. **Public release repository:** `GreyDoorRepublik/Floppy144-beta`, unchanged. This inventory describes **current compilation**, historical source origins and unresolved attribution, not a legal clearance certificate.

## Classification key

- **Independent implementation (technical):** a new B2-03/B2-04 implementation based on the published game-facing interface and B2-02 specification; authorship still needs independent third-party review due to disclosed earlier source exposure.
- **Retained original game code:** project-owned/game-specific implementation with no established direct River2D source lineage in the inspected evidence; this is not a universal ownership warranty.
- **Generated code:** produced by the canonical Floppy//144 input pipeline; provenance of underlying authored content is distinct from provenance of the generator.
- **Third-party licensed/toolchain:** independently licensed external platform, tool or historical upstream software. Presence in a source history does not imply inclusion in the current executable.
- **Removed code:** absent from the current branch but preserved by unmodified Git history and historical attribution documentation.
- **Unresolved:** evidence does not settle source author, specific rights, independently created expression or distribution obligations.

## 1. Shipping build: active source and binaries

| Component | Files / specific boundary | Classification | Actually compiled or linked? | Evidence / remaining question |
|---|---|---|---|---|
| Independent pixel store and GDI presentation | `include/floppy144_win32_display.h`; `src/floppy144_win32_display.c` | Independent implementation (technical) | **Yes**, PlatformWin32 | B2-03 specification-derived C99 640×360 / 0x00RRGGBB / top-down BI_RGB. B2-03 Windows tests 79 assertions pass. **No formal clean-room certification.** |
| Independent Win32 host and platform dispatch | `include/floppy144_win32_host.h`; `src/floppy144_win32_host.c` | Independent implementation (technical) | **Yes**, PlatformWin32 | 15 existing public F144PlatformApi slots. Exposes HWND/HDC in private Windows-only header. B2-04 Windows host tests 25 assertions pass. Shared public callback names/order are contractually required, not automatically copied expression. |
| Native clock/wake bridge | `src/f144_win32_timing.c`; `include/f144_win32_timing.h` | Independently rewritten implementation / retained public declaration | **Yes**, PlatformWin32 | Native GetTickCount64/SetTimer/KillTimer uses new host HWND accessor; no private F144Runtime cast. Lifecycle and timing regression pass. |
| Portable platform API and calendar/startup | `include/f144_platform.h`; `src/f144_platform.c`; `src/f144_calendar.c`; `src/f144_startup_config.c` | Retained original game-owned code | **Yes**, Core | Game-facing API is unchanged; the F144 prefix is a project naming convention, not evidence of River2D copying. |
| Game screen/coordinator | `game/src/floppy144_main.c` | Retained original game code with historically entangled renderer integration now removed; **provenance caveat** | **Yes**, executable | B2-04 replaced private renderer initialisation and callback wiring. Individual older Win32/coordinator portions still need source-by-source independent authorship review; file cannot be wholly certified by this mechanical migration. |
| Game rendering and world systems | `game/src/floppy144_*.c/.h` (site/isometric/office/Notebook/terminal/profile/Grey Door/completion and drawing helpers) | Retained original game code | **Yes**, Core or executable by Premake | Game-specific systems remain unchanged by B2-05; deterministic regressions cover the established rendering/game behaviour. The embedded font glyph **patterns** are separately unresolved below. |
| Persistence and migration | `game/src/floppy144_persistence.c`; `game/src/floppy144_storage.c`; `src/f144_win32_storage.c` and headers | Retained original game code | **Yes** | AppData location and legacy migration, profile/settings/manual/autosave binary formats preserved, source path tests pass; full byte-for-byte Beta.1 fixture round-trip still pending. |
| Input/lifecycle/instance/config | `src/f144_win32_input.c`; `src/f144_win32_lifecycle.c`; `src/f144_win32_single_instance.c`; `src/f144_win32_startup_config.c` and headers | Retained original game-owned/native services | **Yes** | Event and single-instance semantics left intact and regression tested. |
| Procedural audio and native WinMM | `src/f144audio.c`, `src/f144sfx.c`, `src/f144_win32_audio.c`, `game/src/floppy144_audio.c` and related headers | Retained original game-specific synthesis/service implementation | **Yes** | MIDI/waveOut generates sound programmatically; no stored SFX. Existing audio contract tests pass; formal source authorship and listening parity not independently signed off. |
| Canonical generated definitions | `game/src/*.generated.inc`, `*.generated.def`, `floppy144_site_generated.def`, generated layout data and checked-in derived .def names | Generated code | **Yes** where included by game source | `tools/build_game_data.ps1` uses `tools/game_data_compiler.c`, `tools/build_site.ps1` and `tools/site_compiler.c`; `tools/compile_intro_vectors.py` generates intro vectors. Canonical project assets/inputs remain the authorship source; no generated definitions changed in B2-05. |
| Win32 app icon and resource | `platform/win32/floppy144.ico`, `floppy144_app.rc` / resource header | Retained game artwork/resource, **specific art provenance unresolved** | **Yes**, Windows resource | Icon remains materially part of EXE; require original design/source or explicit rights record for definitive asset sign-off. |
| Embedded 5×7 bitmap font | `game/src/floppy144_draw.c`: 60 character cases with 5-bit rows packed into glyph values | **Unresolved provenance** within retained game code | **Yes**, compiled into Core/EXE | Direct table inspection found **60 case-based patterns**, no font source, author/licence reference or documented independent derivation. Similarity to common 5×7 glyphs is insufficient to identify author or conclude public domain. Preserve visual identity pending provenance clearance. |
| Windows OS APIs | Win32/GDI/user32/kernel32/shell32/winmm | Third-party/platform APIs licensed under applicable Microsoft terms | **System DLL imports only** | Actual Windows `dumpbin` imports are `USER32.DLL`, `KERNEL32.DLL`, `GDI32.DLL`, `SHELL32.DLL`, `WINMM.DLL`. Calling documented Win32 functions is not River2D source copying. |
| Build generator/compiler | Premake 5.0.0-beta8; Microsoft Visual C++ 2022, GitHub-hosted Windows runner/actions | Third-party toolchain, not game runtime | **Build-time**, static CRT selected | `premake5.lua` retains the three original projects, no vendor engine. GitHub Actions dependencies emit a Node.js deprecation warning, not a C compiler warning. |

## 2. Removed from current Git tree in B2-05

| Path | Classification and reason | Reproducibility |
|---|---|---|
| `include/f144_runtime.h` | Removed known River2D-derived header with F144Runtime/F144Image private layouts | Historical blob and rename commit preserved |
| `src/f144_runtime.c` | Removed derived core image/runtime implementation | Historical source preserved in Git |
| `src/f144_win32_runtime.c` | Removed derived Win32 runtime helper | Historical source preserved |
| `src/f144_win32_platform.c` | Removed superseded River2D-lineage Win32 presenter/adapter (not the new `floppy144_win32_host.c`) | Historical source preserved |
| `include/f144_win32_platform.h` | Removed derived Win32 presenter header | Historical source preserved |
| `src/string_view.c` / `include/string_view.h` | Removed utility whose provenance was uncertain after puddle removal and which was already unlinked since B2-04 | Historical source preserved |
| `.gitmodules` | Removed inactive metadata referring to `vendor/puddle` | No vendor directory, no tracked Gitlink (mode 160000) and no production build reference existed; older submodule history preserved |

**Source manifest:** the currently active platform build includes the new host/display and retained owned native services only. A static compiled-object check uses `lib.exe /list` against `floppy144platformwin32.lib`; the legacy objects are absent, and their independent replacement objects are present. `tools/test_beta2_provenance.ps1` also checks that removed files cannot reappear unnoticed and scans 212 active C/header/generated/test files for old symbols and includes.

## 3. Historical code and licences retained without shipping

| Component | Classification | Current state |
|---|---|---|
| Original `BadAcronym/river2D` at `dab1612e82053c91d818d8ec98677c0632297238` | Historical third-party GPLv3 River2D implementation | **Not** compiled/linked. Direct lineage established by project commit `709c76c109d09e775ef72785b1b44efc240b7cce`, which replaced River2D-named paths with F144-named ones. |
| `docs/stage4/runtime_provenance.md`, `docs/beta2/01_provenance_inventory.md`, original design notes | Historical attribution/evidence | **Preserved verbatim**. Existing historical percentage comparisons describe source lineage, not copyright-law thresholds or the new source. |
| Root `LICENSE` | GPLv3, including inherited BadAcronym / river2D notice within the licence text | **Preserved**. No attempt to relicense, suppress notices or rewrite history. Retention does not settle redistribution or ownership questions by itself. |
| Public Beta.1 `GreyDoorRepublik/Floppy144-beta` | Released historical product under intended GPLv3 | Release repo main remained at `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`, with root `LICENSE` not yet committed as of this audit. Publication obligations require an owner/legal check. |

## 4. Unresolved build artefacts and assets not linked into Floppy144.exe

- `tools/site_compiler.exe` (174,080 bytes) and `tools/site_compiler.obj` (93,914 bytes) are tracked binary artefacts with unverified individual binary provenance. **They are not linked**. Actual `tools/build_site.ps1` compiles `tools/site_compiler.c` into `obj/tools/site_compiler.exe` and runs that fresh output. The tracked copies are not required by the canonical build script; review removal and redistribution scope separately without conflating these binaries with the executable runtime.
- Historical sample `floppy144_auto.sav`, `floppy144_manual.sav`, `floppy144_profile.dat` and `floppy144_settings.dat` are tracked test/data fixtures, not engine source or runtime DLLs. Retained because compatibility testing may require them.
- `platform/win32/floppy144.ico` and the embedded glyph table require individual asset-origin records, not merely a source-code namespace scan.
- Generated content source rights remain distinct from technical correctness of generators. No project-wide assignment or full external-asset bill of materials has been independently authenticated.

**Inventory conclusion:** demonstrably River2D-derived F144 runtime source is absent from the production build **and** removed from the current working tree. No remaining compiled River2D engine dependency was identified by the source/archive/import checks. **Unresolved provenance persists in embedded glyphs, possible historical coordinator glue, ancillary assets/build binaries and formal independently verified authorship/redistribution rights.**


## B2-07 ownership clarification (later addendum)

The game creator states that the remaining game-specific code, glyph patterns, icon, music, story and original assets were made by the creator with ChatGPT assistance. The former River2D engine was the externally sourced runtime and was removed. Therefore references above to **unknown glyph/icon author** represent B2-05's earlier *lack of creator testimony*, now superseded by `07_creator_declaration.md`. Independent legal/expression review remains a separate question, as discussed in `07_expression_recheck.md`. Historical original records and Beta.1 GPLv3 permissions are not changed.
