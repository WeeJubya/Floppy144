# B2-07: Targeted River2D expression recheck

**Date:** 21 October 2026. **Developer source at start:** `WeeJubya/Floppy144@e644914affacfa1aaa4c8e5b80392267a4c91163`. **Type:** source-overlap engineering check, **not** independent legal source audit or formal clean-room certificate.

## 1. Inputs and test scope

Historical derived runtime source is read only from immutable retained Git commit `38afcfe5b6ed8ac5da08039479c205ab883239d9`. Five files:

- `include/f144_runtime.h`, `src/f144_runtime.c`, `src/f144_win32_runtime.c`;
- `src/f144_win32_platform.c` and `include/f144_win32_platform.h`.

Current replacement and integration files examined:

- `src/floppy144_win32_display.c`, `src/floppy144_win32_host.c`, `src/f144_win32_timing.c`;
- `include/floppy144_win32_display.h`, `include/floppy144_win32_host.h` and `game/src/floppy144_main.c`.

The supplemental regression script is `tools/test_beta2_expression.ps1`; the canonical Beta.2 release-candidate workflow runs it with the existing `test_beta2_provenance.ps1` source and linked-object audits. The script uses `git show` to read the historical version without restoring it to the current tree. Changes introducing new exact overlaps fail by default.

**Comparison method:** trim and collapse spaces per line; ignore preprocessor directives, standalone comment lines and lines shorter than 24 characters; require **exact textual identity** against any line of the five historic source/header files. This deliberately limits false positives but is not exhaustive. Long intact source blocks spanning multiple lines of a changed single-line formatting style or semantic renaming may escape detection.

## 2. Direct source comparison observations

Manual tool-driven comparison of the pinned old/new files before adding the regression script:

| Current file | Identical substantive lines in five old files |
|---|---:|
| `src/floppy144_win32_display.c` | **0** |
| `src/floppy144_win32_host.c` | **8 distinct interface lines** |
| `src/f144_win32_timing.c` | **0** |
| `include/floppy144_win32_display.h` | **0** |
| `include/floppy144_win32_host.h` | **0** |
| `game/src/floppy144_main.c` | **0** |

The host's eight identical substantive lines are: a generic `return &platform->surface;` expression and seven references to existing public platform callback names for persistence, procedural audio and local calendar services. They do **not** implement frame rasterisation or copied private F144Runtime/F144Image structures. They are pinned as explicit exceptions rather than silently excluded by broad name filtering. The script will fail if an unexpected line appears or an expected exception disappears (so exceptions can be retired).

The retained platform core `src/f144_platform.c` was also sampled: it shares generic field assignments involving `platform->audio_initialized` and `platform->music_volume`, consistent with common platform API functionality. It is **not** part of the replaced renderer; B2-06 independently verified that this exact game-facing source file matches public Beta.1 byte-for-byte. The creator now specifically attests authorship of remaining game-specific components.

## 3. Independent shipping-build evidence

Prior B2-06 [Windows CI run 39589601998](https://github.com/WeeJubya/Floppy144/actions/runs/39589601998) verified:

- Removed five derived F144 runtime implementation/header files, `string_view` and stale submodule declaration absent from current tree.
- No old private runtime/engine tokens in **212 active source/header/generated/test/resource files** (historical attribution comment intentionally retained).
- Current binary uses new Win32 display/host objects, not old `f144_runtime.obj`, `f144_win32_platform.obj` or `f144_win32_runtime.obj`.
- Strict x64 Windows Release: **973,312 bytes**, **501,248 remaining** against 1,474,560; **0 compiler warnings/errors**.
- Imported DLLs: Windows system `USER32`, `KERNEL32`, `GDI32`, `SHELL32`, `WINMM` only.
- Testbuild, cleanbuild, 79 display and 25 host assertions green.

The latest B2-07 expression regression adds a **repeatable focused negative-control test**; refer to its own subsequent CI run when executed. No source parser or automated line-overlap heuristic can establish copyright ownership.

## 4. Interpretation and limits

**No unexpected copied identical long lines were found in the six examined replacement/coordinator files.** The current shipping build excludes the old compiled engine. This supports the creator's statement of intended independently authored replacement.

**But:** some inherited implementation was inspected before the replacement was written, so this reviewer does not satisfy source-unexposed clean-room separation. Exact-line absence cannot rule out substantively similar expression, third-party provenance of other works, generic API re-use or legal concerns. The original River2D Git history and Beta.1 GPLv3 rights are preserved.

**Authorship statement:** see `07_creator_declaration.md`. The creator states all original game components other than the old River2D runtime were made by the creator with ChatGPT assistance, expressly including glyphs and icon. Treat that as **creator attestation**, not verified source records from a third party.

**Conclusion:** limited code-level expression regression **no unexpected direct overlaps found**; shipping dependency removal **technical PASS**; formal independent copyright and proprietary licence approval **not determined here**. Neither source test nor declaration is a release authorisation.
