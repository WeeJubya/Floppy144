# B2-06: Windows x64 Beta.2 release-candidate assessment

**Date:** 21 October 2026. **Repository:** `WeeJubya/Floppy144`; branch `development/Beta-2-runtime`. **Public release repository** `GreyDoorRepublik/Floppy144-beta` was not modified. **Decision: HOLD release approval**, irrespective of compiler/test pass, because mandatory desktop visual/audio and authentic published Beta.1 persistence fixtures are unavailable, while independent provenance/licence sign-off remains open.

## 1. Actually verified reference and candidate identity

| Item | Published Beta.1 | Beta.2 engineering candidate |
|---|---|---|
| Source | Public tag `Floppy144` at `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf` | `development/Beta-2-runtime`; B2-06 engineering-test commit `ba3674cc4f7e744e6b2c2c557955b09b9db77f50` |
| Executable | `Floppy144.0.4.0-beta.1.exe`, actual download verified | `bin/release/Floppy144.exe`, strict MSVC x64 |
| Bytes | **822,272**, verified | **973,312**, measured |
| SHA-256 | `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749` | `E24B27EC4B4B97ACE9D5545001CC65C25BE2241490963CF9DDF7385BC751FFFB` in strict Release job of [run 39579270827](https://github.com/WeeJubya/Floppy144/actions/runs/39579270827) |
| Independent verification | Downloaded released asset, checked SHA-256 **and** exact 822,272-byte size, checkout pinned tag and confirm commit; **PASS** | VS2022 strict Windows Release linked and executable hashed; **PASS** |
| Display backend | River2D-derived F144 implementation | Independent B2-03 Win32/GDI frame store + B2-04 host; inherited source deleted by B2-05 |
| Asset/source drift | 13 stable game-facing, drawing, audio, profile, persistence, site/Grey Door modules match published Beta.1 byte-for-byte | Intentional launcher/runtime/test differences only in cutover stage; game-owned stable modules unchanged |

Hash is attached to the measured CI artifact **only**; later native rebuilds, including after documentation commits, can generate a different SHA-256 even for the same source without implying content drift. Always publish the hash for the exact binary selected for distribution.

## 2. Mandatory engineering gates

| Gate | Result / evidence |
|---|---|
| Canonical authored-game-data and Site regeneration | **PASS**, Windows scripts `tools/build_game_data.ps1` and `tools/build_site.ps1`, drift test rejects changes to `game/src` / `data` |
| Full Stage 2/Stage 3/Stage 4 `testbuild.ps1` | Final result in `06_test_results.md`; it must be fully green before release approval |
| `cleanbuild.ps1` | Final result in `06_test_results.md`; must include full regressions then root Release |
| `run.ps1 -build release` | Run as part of canonical cleanbuild; do not claim independently until the full workflow succeeds |
| Win32 offscreen framebuffer/GDI | 79 assertions from B2-03 Windows tests, including scaling/color/orientation; confirmation in B2-06 logs |
| Native host, WinMM and persistence contract | 25 B2-04 host assertions plus entire Stage 4 suite |
| Strict MSVC x64 `/W4`, warnings-as-errors Release Rebuild | **PASS**, actual strict job reports 0 warnings, 0 errors |
| One-file EXE size | **PASS**: 973,312 of maximum **1,474,560**, **501,248 bytes remaining** |
| Windows imported DLLs | `USER32.DLL`, `KERNEL32.DLL`, `GDI32.DLL`, `SHELL32.DLL`, `WINMM.DLL`; **no non-system DLL** |
| Runtime compiled-object inventory | **PASS**: new `floppy144_win32_display.obj` and `floppy144_win32_host.obj` present, inherited engine objects absent |
| Built application icon | **PASS**, seven resolutions/resources from existing `test_stage4f_application_icon.ps1` |
| Produced artifacts | CI archives only `Floppy144.exe` and companion `LICENSE` for verification; **not an authorised public release** |

## 3. Verified versus unverified compatibility

Existing public Beta.1 and current source hashes match for thirteen stable game systems, including save codec `game/src/floppy144_persistence.c`, storage/migration, profile/settings, drawing, isometric Site, Grey Door and procedural sound. Those **prove unchanged source**, not live file compatibility or output-identical pixels.

The **public Beta.1 repo contains no sample manual/autosave/profile/settings files**. The development repository has four older samples:
- manual save V1 payload **64 bytes** and autosave V1 payload **68 bytes**; both predate and are **incompatible with** Beta.1's V1 save payload **72 bytes**. The current unchanged decoder safely rejects them, and the B2-06 regression asserts failure without mutation. They are **not evidence of a Beta.2 newly introduced regression**, but are a real unresolved *legacy import* scope decision.
- historical profile and settings samples load and survive safe current-format save/reload; their original file bytes are unchanged by tests.

No compatibility claim for **authentic published Beta.1-created saves** can be made until suitable fixtures are produced, verified, backed up and tested. No user-profile directory was touched by headless test harnesses; `F144_TEST_ROOT` isolates test writes.

## 4. Release-blocking acceptance

1. **Manual Windows desktop inspection is absent**: no verified Beta.1/Beta.2 matched-seed video/screenshot A/B across intro, terminal, office/isometric site, Notebook, Grey Door, completion, resizing, repaint/flicker. PPM captures are non-desktop supporting evidence only.
2. **Audio listening is absent**: automated WinMM contract and scheduling tests are not an actual sound-device audition.
3. **Authentic public Beta.1 save/profile/settings fixtures absent**. Identify whether the 64/68-byte ancient unsupported prototypes should remain out of scope or require a separate backwards-compatibility migration.
4. **Provenance and legal rights are not independently signed off**: B2-05 noted author-exposure/clean-room limitation, embedded 5×7 font patterns and icon/source ownership questions.
5. **GPLv3 release publication discrepancy**: the owner's intended GPLv3 root `LICENSE` file remains absent from the GDR public Beta.1 repository as checked 21 October. Confirm separate correction before external approval.
6. Final full `testbuild.ps1` and cleanbuild job logs must be recorded accurately, not inferred from a more permissive B2-04 workflow.

**Release status:** **NOT APPROVED / NO TAG / NO PUSH TO GREY DOOR REPUBLIK RELEASE REPO**. Engineering integration and runtime-size verification do not override outstanding mandatory acceptance.

## 5. Handover to human acceptance/release

Use `docs/beta2/06_visual_acceptance.md` for exact client dimensions, screen-by-screen and audible checks and sign-off sheet. Use `06_test_results.md` for script/run IDs and any remaining regressions. Record the precise candidate executable SHA, run matched Windows VM snapshots with isolated AppData and seed, capture complete videos and sound, exercise published Beta.1-created saves, have an independent provenance reviewer adjudicate font/icon/license, and require **zero unexplained failures** before release authorisation. No Beta.2 production executable was published by B2-06.
