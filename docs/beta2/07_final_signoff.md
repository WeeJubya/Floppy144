# B2-07: Final engineering and release sign-off

**Review date:** 21 October 2026 (UTC).  
**Status:** **RELEASE BLOCKED; NO FINAL AUTHORISATION**. This is an engineering and publication-readiness record, **not** independent legal advice, rights clearance, human audiovisual acceptance, a signed certificate or an instruction to publish.

## A. Exact review scope and baselines

| Item | Verified identity |
|---|---|
| Developer / studio | WeeJubya / **Grey Door Republik** |
| Working development repository | `WeeJubya/Floppy144` |
| Existing development branch reviewed | `development/Beta-2-runtime` |
| B2-06 final verified source HEAD before B2-07 paperwork | `514b159197bcf4e9e2f2289cbc5b645d2b9defb4` |
| Latest verified Windows RC CI | [GitHub Actions run 39580254575](https://github.com/WeeJubya/Floppy144/actions/runs/39580254575), **three jobs PASS** at that HEAD |
| Public repository | `GreyDoorRepublik/Floppy144-beta`: `main` = `9640d2873a0e3f38451d3e953a60f291a721b153` after a separate GPLv3 `LICENSE` addition; only branch present on recheck; **no Beta.2 branch or PR**. Original Beta.1 tag still points to `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf` |
| Public Beta.1 executable | `Floppy144.0.4.0-beta.1.exe`, **822,272 bytes**, SHA-256 `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749`, independently downloaded and checked in B2-06 CI |
| Proposed Beta.2 strict Release build | **973,312 bytes**, SHA-256 `C478E6D33B52E6E12D8786B048C8AB7A3CD2388FA3E87C48046C9CEEF311980F` in the strict Windows job of run 39580254575. Hash identifies **that exact CI binary only**. |
| Public Beta.1 source tag | `Floppy144` = `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`, verified in B2-06 baseline job |

**Repository separation:** Work through B2-06 was done on `WeeJubya/Floppy144`. The requested public `GreyDoorRepublik/Floppy144-beta` does **not** have the existing Beta.2 branch and is not declared a GitHub fork of WeeJubya. No unrelated branch was created or switched. No source was copied into the public release repository during B2-07 because rights/publication gates are blocked.

## B. B2-01 through B2-06 gate ledger

| Stage / mandatory gate | Actual status | Source |
|---|---|---|
| B2-01 original binary/source benchmark | **PASS** binary SHA/size and source tag identity, now independently verified, but historical visual/audio captures absent | `06_test_results.md`, B2-06 baseline job |
| B2-02 approved independent specification and ABI | **PASS: documentation prepared**, 15 public API services and deterministic display test matrix | `02_runtime_specification.md`, `02_api_contract.md`, `02_implementation_handover.md` |
| B2-03 independently designed framebuffer, memory and native GDI testing | **PASS engineering**, **79** Windows test assertions. **Formal source-unexposed clean-room condition NOT PASSED** due to earlier inherited-code exposure | `02_implementation_handover.md`, `06_test_results.md` |
| B2-04 production cutover and standalone launcher | **PASS**, `floppy144_win32_display.c` and `floppy144_win32_host.c` own production display/Win32 host. Existing 15-slot platform contract and native audio/storage preserved | `04_integration_report.md`, B2-06 strict build |
| B2-05 obsolete derived source and linked object removal | **PASS technical**, 212 active files scanned; derived private runtime files not present in tree/archive; root GPL licence and historical record preserved | `05_independence_audit.md` / `05_dependency_inventory.md` |
| B2-05 **independent** rights/provenance clearance | **BLOCKED**: same author/assistant previously saw inherited implementation; embedded 5×7 glyphs, app icon and contributions not fully attributed; no qualified independent sign-off | `05_independence_audit.md` |
| B2-06 complete Stage 2/3/4/BF regression | **PASS**, canonical `testbuild.ps1` (46 groups), Stage 4D+ 15/15, `cleanbuild.ps1` and root `run.ps1 -build release` | [run 39580254575](https://github.com/WeeJubya/Floppy144/actions/runs/39580254575) |
| B2-06 generation / generated drift | **PASS**, canonical game-data and Site compilers regenerated without tracked drift | Same CI run |
| B2-06 audio contract / deterministic timing | **PASS automated**; actual human playback **NOT VERIFIED** | `06_test_results.md` / `06_visual_acceptance.md` |
| B2-06 matched Windows desktop scene/resize/flicker validation | **BLOCKED**, no independently authenticated human Beta.1-vs-Beta.2 matched-seed screenshots or video | `06_visual_acceptance.md` |
| B2-06 published Beta.1-created manual/autosave/profile/settings import | **BLOCKED**, genuine Beta.1 fixtures unavailable. Older unsupported V1 64/68-byte developer samples are not Beta.1 files and are safely rejected; profile/settings historical examples round-trip | `06_test_results.md` |
| B2-07 licence, copyright and source-distribution clearance | **BLOCKED**, see Section E | Current licence and repository review |
| Explicit owner final release authorisation | **NOT REQUESTED OR GRANTED** until every preceding blocking gate is satisfied | This sign-off record |

**No final sign-off**: automated PASS does not override any **BLOCKED** or **UNVERIFIED** required criterion.

## C. Engineering re-verification

Strict x64 MSVC VS2022 Release / warnings-as-errors: **0 production compiler warnings, 0 compiler errors**. Standalone Windows executable: **973,312 / 1,474,560 bytes**, leaving **501,248 bytes**, **SIZE PASS**. Increase over published Beta.1: **151,040 bytes**. Compiled platform archive contains independent Win32 display/host objects and excludes old F144/River2D object files. The new runtime links no River2D/third-party graphics DLL.

Actual `dumpbin /dependents` reported only Windows system DLLs: `USER32.DLL`, `KERNEL32.DLL`, `GDI32.DLL`, `SHELL32.DLL` and `WINMM.DLL`. Static MSVC CRT avoids a VCRUNTIME140 redistributable dependency. No SDL, Raylib, OpenGL or DirectX graphics runtime was introduced.

**Source-diff summary between current developer tree and published Beta.1:** 95 files present only in developer tree, 7 old inherited/utility paths present only in Beta.1, and 12 different common Git blobs. These counts cover **all tracked blobs**, including private engineering docs and test/user-file fixtures, not just production code. The game-source differences include `game/src/floppy144_intro.c`, `floppy144_intro.h` and `floppy144_main.c`. Some differences reflect approved Stage-4 adjustments, e.g. an unlettered moving disk label and GUEST login. Do not assert pixel-by-pixel or all narrative parity without human comparison. Thirteen selected stable game-owned modules, including drawing, storage, profiles, settings, isometric site, Grey Door and procedural audio, were hash-matched to Beta.1 byte-for-byte in Windows CI.

**Package hygiene:** Do not release a raw zip or wholesale merge of the development checkout: its extra files include private `docs/beta2` audits, historic test save/profile/settings files, internal notes and development/compiler artefacts. A new public-tree manifest, spoiler/privacy review and appropriate validated rights/licence must precede public incorporation. A CI-verification artifact (`Floppy144.exe` plus a GPLv3 `LICENSE`) exists but has **not** been authorised as a public distributable.

## D. Actual CI evidence versus human checks

| Mandatory verification | Status |
|---|---|
| 79 framebuffer/Win32 GDI assertions and 25 native host/API assertions | PASS |
| Complete Stage 2/3/4, menu, terminal, persistence, office, isometric, notebook, Grey Door and completion regressions | PASS |
| Strict Windows build, native import/object checks, app icon | PASS |
| %APPDATA% path and legacy migration **source/test contract** | PASS |
| Real Windows launch, walking, resizing and alt-tab with video (VIS-01..11) | **UNVERIFIED** |
| Real hardware WinMM music, SFX, volume, shutdown (AUD-H01..04) | **UNVERIFIED** |
| Real Beta.1-created manual/autosave/profile/settings file migration (CMP-01/02) | **UNVERIFIED** |
| Full independently sourced asset/author rights ledger | **UNVERIFIED** |

Use `docs/beta2/06_visual_acceptance.md` as a signable manual checklist. A deterministic 640×360 PPM capture or successful headless offscreen test cannot demonstrate absence of black flashes or tearing in the actual Windows compositor.

## E. Licensing gate: **BLOCKED**, not “All Rights Reserved”

1. The development repository retains a full GPLv3 `LICENSE` with historic BadAcronym/River2D notices. Its presence, plus source provenance, needs proper author/contribution analysis. Removing a derived engine from the new link does **not** establish ownership or a proprietary licence for every remaining game component.
2. **Updated repository evidence:** while B2-07 was being prepared, WeeJubya separately added the intended root GPLv3 `LICENSE` to the public Beta.1 repository in commit `9640d2873a0e3f38451d3e953a60f291a721b153` (21 October 2026). Its Git blob SHA `b1b737ee5e3c21b4d2f41d38020d8214009019ae` **exactly matches** the development GPLv3 file and retains historical River2D/BadAcronym notices. **The missing-root-file issue is now corrected in current public `main`;** the original Beta.1 tag is unchanged, and separate verification of historical distributed-asset notices/corresponding-source obligations and any remediation remains open. Adding a file now does not itself establish all past compliance or proprietary relicensing rights.
3. No documented assignment or explicit grant for **all** surviving contributions, 5×7 bitmap glyph expressions, icon and other game assets has been verified. There was prior inherited-source exposure and no independently source-unexposed technical/legal assessment of the replacement.
4. **Do not** replace the licence with “All Rights Reserved”, claim exclusive source rights, remove prior notices, alter an earlier tag or publish a new source/binary package. Any new licence must be expressly approved on the evidence for all included material and the release history. Already granted GPL rights for old copies cannot be rescinded.
5. Confirm future source-package/corresponding-source obligations, copyright statements, contributor notices and third-party attributions with a qualified adviser; only then decide whether a defensible separate proprietary Beta.2 release is feasible.

For general, non-project-specific background, see the [GNU GPL FAQ](https://www.gnu.org/licenses/gpl-faq.en.html) on copyright-holder approval and prior licences. **This document does not issue a legal opinion.**

## F. Release decision and signatures

| Role / approval | Required evidence | State |
|---|---|---|
| Engineering verifier | Green `testbuild` + `cleanbuild`, strict Release, imports and size | **PASS (automated)** |
| Independent source-provenance reviewer | Attested comparison and expression/rights inventory | **NOT SIGNED** |
| Legal/licensing approver / rights owner(s) | Written scope of licence, contributions, glyph/icon origin, notices, Beta.1 remediation plan | **NOT SIGNED** |
| Windows visual reviewer | VIS-01..11, matched recordings, issue disposition | **NOT SIGNED** |
| Audio reviewer | AUD-H01..04 and device/session notes | **NOT SIGNED** |
| Persistence compatibility reviewer | Authentic Beta.1 files, controlled migration/rollback results | **NOT SIGNED** |
| Grey Door Republik release owner | Explicit final approval for *specific* source SHA, EXE SHA and public licence/package | **NOT SIGNED** |

**Gate outcome: RELEASE BLOCKED.** No public PR, merge, tag, GitHub release or downloadable Beta.2 publication authorised by this report. Do not infer consent from a build passing, or from preparation of the following public release-note drafts.


## B2-07 follow-up: creator testimony and focused expression check

The creator states that all remaining Floppy//144 code, assets, glyph patterns, icon, music and story content were made by the creator with ChatGPT assistance; River2D was the known prior third-party runtime. This first-party provenance statement is recorded in `07_creator_declaration.md`. The earlier unresolved glyph/icon *author identification* is now **creator-declared original work**, rather than wholly unattributed. The previous B2-05 technical observations and historic licence obligations are preserved.

`07_expression_recheck.md` documents direct comparison of five old engine sources with six current native/coordinator files. The display, timing, headers and coordinator had zero exact substantive matches. Eight host matches were existing public callback names and the generic surface-return statement. `tools/test_beta2_expression.ps1` adds a CI gate to reject additional exact overlaps. Exact textual checks cannot prove the absence of semantic derivation and the reviewer was historically source-exposed.

A candidate Beta.2 proprietary licence is drafted in `07_proprietary_licence_draft.md` for review. It is **not** the current root `LICENSE`, and the legal copyright holder or Grey Door Republik company/trading-name structure has not yet been verified. Creator testimony narrows the remaining legal assessment; it does not complete independent approval or human visual/audio and public Beta.1 save acceptance. **Release remains BLOCKED.**
