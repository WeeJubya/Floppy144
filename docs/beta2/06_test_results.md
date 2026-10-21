# B2-06: Comprehensive regression and release-candidate test results

**Audit date:** 21 October 2026. **Development repository:** `WeeJubya/Floppy144`, existing branch `development/Beta-2-runtime`. The public `GreyDoorRepublik/Floppy144-beta` release repository was **not** modified. **Automated engineering verification: PASS. Mandatory human visual/audio and genuine published Beta.1 save compatibility: OPEN. Release authorisation: HOLD.**

## 1. Reproducibility and evidence

**Primary Windows x64 Actions run:** [39579270827](https://github.com/WeeJubya/Floppy144/actions/runs/39579270827) at source commit `ba3674cc4f7e744e6b2c2c557955b09b9db77f50`. All **three jobs passed**: `beta1-baseline`, `full-regression-and-cleanbuild` and `strict-release-and-dependencies`. Windows Server 2022/MSVC VS2022; pinned Premake `5.0.0-beta8`; canonical generation and full original build scripts.

The previous test-development iteration [39578797148](https://github.com/WeeJubya/Floppy144/actions/runs/39578797148) **failed** because newly added optional screenshot code called `getenv` without `<stdlib.h>`, diagnosed by MSVC C4013/C4047 under `/W4 /WX`. This was fixed by an explicit standard-header include, independently rerun and verified. No production source or test behavioural assertion was removed.

Historical earlier [39577959365](https://github.com/WeeJubya/Floppy144/actions/runs/39577959365) found two old development save files unsupported by *both* the baseline's and current V1 decoder due to old payload lengths; the test now explicitly distinguishes safe rejection from supported formats, rather than changing the game's binary schema.

## 2. Independently verified Beta.1 comparison

The public Beta.1 release **executable was actually downloaded** on Windows, not trusted solely by metadata. Verified file: `Floppy144.0.4.0-beta.1.exe`, **822,272 bytes**, SHA-256 `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749`. Pinned public `Floppy144` source tag resolves to `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`.

B2-06 `tools/test_beta2_baseline_reference.ps1` compared **13 unchanged game-owned files byte-for-byte** between verified public Beta.1 source and Beta.2, including public platform ABI, `floppy144_draw.c`, save codec, storage migration, profile, settings, isometric site, Grey Door and WinMM procedural audio. **PASS, 13/13**. This excludes intentionally changed runtime/coordinator and intro behaviour; identical source is **not** evidence of human visual/audio parity.

**No published Beta.1 sample save/autosave/profile/settings fixture exists in that source tag**, independently confirmed by the baseline checker. Historical developer fixtures below are not incorrectly described as public Beta.1 files.

## 3. Canonical build and regression matrix

| Category | Invoked tests/scripts | Observed result |
|---|---|---|
| Canonical authored data/site | `tools/build_game_data.ps1` → `tools/build_site.ps1`, repeated before testing, cleanbuild and strict build; `git diff --exit-code -- game/src data` | **PASS**, no generated definition/site drift |
| Stage 2 | `tools/test_stage2.ps1` | **PASS**, headless recovery/world tests |
| Stage 3A | `tools/test_stage3a.ps1` | **PASS**, prologue |
| Stage 3B | `tools/test_stage3b.ps1` | **PASS**, terminal, site interactions, reconstruction, door/access, cabinet, coordinator |
| Stage 4 platform, input, timing | `test_stage4_platform.ps1`, `test_stage4_input.ps1`, `test_stage4_timing_lifecycle.ps1` | **PASS** |
| Save migration and compatibility | `test_stage4_persistence.ps1`, `test_stage4_single_instance.ps1` and existing profile/session tests | **PASS**, supported formats, legacy-location migration and safe failure paths; obsolete prototype formats treated separately below |
| Profile, Settings, authentication | `test_stage4_profile_screen.ps1`, `test_stage4_profile_name.ps1`, `test_stage4_terminal_auth.ps1`, `test_stage4_settings.ps1`, `test_stage4_config.ps1` | **PASS** |
| Procedural audio and scheduling | `test_stage4_audio.ps1`, `test_bugfix21_backspace.ps1`, `test_bugfix22_music.ps1`, `test_bugfix23_24_audio.ps1` and timing | **PASS**, contracts and deterministic sound scheduling (no actual listening) |
| Movement, office/ISO, inspection markers | `test_stage4_office_camera.ps1`, `test_stage4_player_visual.ps1`, `test_stage4_inspection_25d.ps1`, `test_stage4_presentation_consistency.ps1`, bugfix 15/17 | **PASS** |
| Notebook, documents/ledger, catalogue | `test_bugfix18_notebook.ps1`, `test_bugfix19_indicators.ps1`, content/integration suites | **PASS** |
| Intro, screens, Credits, completion | `test_stage4f_intro.ps1`, `test_bugfix20_intro.ps1`, `test_stage4f_presentation_journey.ps1`, `test_stage4_completion.ps1`, `test_stage4_credits.ps1` | **PASS**, corrected paper-label pixel oracle |
| Grey Door Easter Egg and save isolation | `test_stage4g_grey_door.ps1` and persistence Stage 4G cases | **PASS** |
| Stage 4D+ independent cross-check | `tools/test_stage4_post_bugfix.ps1` | **PASS, 15/15**, previously 14/15 due to faulty intro test sampling |
| Entire canonical suite | `.\testbuild.ps1` (46 ordered regression groups) | **PASS** |
| Native B2-03 framebuffer presenter | `.\tools\test_beta2_win32_display.ps1` | **PASS, 79 assertions** on Windows, pixel memory/format, geometry, aspect bands, GDI and window-smoke |
| B2-04 platform host/API | `.\tools\test_beta2_win32_host.ps1` | **PASS, 25 assertions**, framebuffer ownership, native quit/timer, 15-slot callbacks with test doubles |
| B2-05 derived source scan | `.\tools\test_beta2_provenance.ps1 -Phase Source` | **PASS**, 212 active C/header/generated/test/resource files scanned; historical notices preserved |
| Canonical clean validation | `.\cleanbuild.ps1`: clean build/obj/bin, rerun `testbuild.ps1`, then `run.ps1 -build release` | **PASS**, including second complete canonical suite and SIZE GATE |
| Strict independent Release | Premake VS2022, MSBuild `/t:Rebuild /p:Configuration=release /p:Platform=windows /p:WarningLevel=Level4 /p:TreatWarningsAsErrors=true` | **PASS**, **0 warnings, 0 errors** |
| Binary object/import/icon verification | `test_beta2_provenance.ps1 -Phase Linked`, `test_stage4f_application_icon.ps1 -Phase Built` | **PASS**, independent presenter objects present, old River2D objects absent, icon embedded |

## 4. Root cause and new regression coverage

**Longstanding Stage 4F disk-label test was incorrect**: its sample rectangle `x=325..394,y=171..179` included a dark rim *outside* the moving paper face in the program/insertion scene at 8,200ms, so it reported legitimate border pixels as "lettering". At the approved animation phase (1,800ms) the actual white inset occupies approximately `x=339..426,y=169..182`. The corrected focused pixel oracle checks `x=345..394,y=174..179`, **300 pixels required exactly `0x00F2F2F2`**. A reintroduced `GDR` glyph would fall inside that region and fail. The renderer, scene timing and artwork were **not altered**; only the incorrectly positioned test assertion was corrected. Stage 4D+ now passes 15/15, and the full cleanbuild reaches Release.

**Additional coverage:** four historical tracked development fixtures are loaded through the public persistence API with `F144_TEST_ROOT` and `F144_PRE_BETA2_FIXTURE_ROOT`; originals are checked byte-for-byte before/after, writes go only to a test-temp directory. Profile and settings load and safely round-trip. The old manual/auto prototype saves have **64/68-byte V1 payloads**, but the shipped Beta.1 decoder's V1 requires **72 bytes**. Correct behaviour for these *unsupported earlier prototypes* is safe rejection, no mutation; both are tested, and output prints `B2-06 COMPATIBILITY LIMIT`. Do not claim successful pre-Beta.1 prototype save migration. Genuine Beta.1-created save fixtures are **not supplied** and remain an acceptance blocker if required.

**Deterministic visual fixtures:** `tools/stage4f_intro_tests.c` can now emit 640×360 top-down P6 PPM frames of discovery, insertion, network and GUEST login to an isolated CI temp directory. Existing Stage 4G persistence test can emit a Grey Door office frame. The [deterministic-frame artifact](https://github.com/WeeJubya/Floppy144/actions/runs/39579270827/artifacts/12335268329) is **automated supporting evidence, not approved GUI screenshots**.

## 5. Windows Release executable, dependencies and packaging

| Metric | Actual verified B2-06 value |
|---|---|
| Target | Standalone `Floppy144.exe`, MSVC Windows x64 Release |
| Bytes used | **973,312** |
| Maximum bytes | **1,474,560** |
| Bytes remaining | **501,248** |
| Size status | **PASS** |
| Strict Release SHA-256 from build at `ba3674cc...` | `E24B27EC4B4B97ACE9D5545001CC65C25BE2241490963CF9DDF7385BC751FFFB` |
| Production compiler errors / warnings | **0 / 0** |
| Imports from `dumpbin /dependents` | **USER32.DLL, KERNEL32.DLL, GDI32.DLL, SHELL32.DLL, WINMM.DLL** |
| Imported external graphics/engine/CRT DLL | **None** |
| Actual native archive objects | Independent `floppy144_win32_display.obj` and `floppy144_win32_host.obj` present; old engine objects absent |
| Built Windows app icon | **PASS** |
| Generated-file drift | **None detected** |
| CI release-candidate artifact | [Windows x64 verification bundle](https://github.com/WeeJubya/Floppy144/actions/runs/39579270827/artifacts/12335547461), includes EXE and companion GPLv3 LICENSE, **not a public release** |

The SHA-256 is tied to the **exact binary measured in that CI run**. A fresh native build may have a different byte hash due to build metadata; independently measure any candidate before redistribution.

## 6. Warnings, limitations and sign-off

**All automated B2-06 checks passed.** Windows CI emitted a **GitHub Actions tooling warning** about older actions being forced from Node.js 20 to Node.js 24, plus a Node.js `url.parse` deprecation. These are **not production compiler warnings** and do not affect the measured EXE.

**Not performed/verified:** desktop compositor and human pixel/video review under matched Beta.1 seed; actual playback and listening on hardware; no transient black frames during walking/resize at every frame; actual published Beta.1-created manual/autosave/profile/settings round-trip; standalone executable launch interaction across all screens on a human desktop. Recorded headless checks cannot substitute for them.

**Independent provenance status remains OPEN:** B2-05 identified prior inherited-code exposure and unresolved bitmap glyph/icon provenance. The public Beta.1 repository's accidentally omitted GPLv3 `LICENSE` still requires release-owner correction and compliance check. No specialist legal independence sign-off is implied.

**Conclusion:** Beta.2 is **automated-regression and clean-Release verified** within the executable-size limit. **Release approval remains blocked** by mandatory manual/asset/provenance/real Beta.1 save evidence. See `06_visual_acceptance.md` and `06_release_candidate.md` for gates and reviewer checklists.
