# Beta.2 runtime migration status
Last updated: 21 October 2026. **Current stage: B2-04 production runtime cutover integrated, Windows Release compiled and executable size verified.** Full legacy regression and visual/provenance acceptance are not yet green.

## B2-04 production integration (current)

- **Development:** `WeeJubya/Floppy144` existing branch `development/Beta-2-runtime`; **release repository** `GreyDoorRepublik/Floppy144-beta` was not modified.
- **New native host:** `include/floppy144_win32_host.h` and `src/floppy144_win32_host.c` bind the unchanged game-facing 15-slot platform API, existing persistence/calendar/WinMM procedural sound, framebuffer, Win32 paint and orderly quit. Rewritten `src/f144_win32_timing.c` obtains the native HWND from the new host.
- **Coordinator:** `game/src/floppy144_main.c` now owns a new independent host instead of F144Runtime/F144Image/private planes/callback assignments. The B2-03 GDI display is the **sole production presenter**. Existing game renderers, save/progression, generated content, native input and audio remain unchanged.
- **Premake:** three original targets retained. Five inherited engine source/header files and `string_view` are absent from the compiled platform target; only their repository history remains for B2-05 audit. Static MSVC CRT removes the VCRUNTIME140 redistributable dependency.
- **Actual Windows tests:** [run 39570329426](https://github.com/WeeJubya/Floppy144/actions/runs/39570329426) at integrated code commit `38afcfe5b6ed8ac5da08039479c205ab883239d9`: 79 independent display assertions **PASS**; 25 native host/API assertions **PASS**; architecture, input, timing/lifecycle, storage, audio and whole-game presentation source checks **PASS**.
- **Release:** strict MSVC x64 VS2022 Rebuild **PASS**, 0 C warnings/0 errors, executable **973,312** of **1,474,560 bytes**, **501,248 remaining**; SHA256 `5C50762B78DA8E59E6772B08E4025E02D0FB63124B5FBDDB7267256933491096` for that measured code build. Imports only USER32, KERNEL32, GDI32, SHELL32 and WINMM system DLLs.
- **Regression not fully green:** `testbuild.ps1` fails at the known Stage 4F intro-label assertion (`floppy label is unlettered near the drive`, exit 43) already recorded during B2-01. The Stage 4D+ independent source sweep is **14/15**. The broad CI steps were deliberately non-blocking to preserve focused/Release evidence; their failures remain explicit. Full `cleanbuild.ps1` not claimed.
- **Updated root Release check:** `run.ps1 -build release` now runs in the dedicated workflow, result to be recorded from the next completed run.
- **Outstanding:** desktop screenshot/audio/resize comparison across all screens, verified Beta.1 reference hash, byte-identical save fixture round-trip, separate provenance review, fully green root regression/cleanbuild after legitimate intro-test resolution and GDR GPLv3 LICENSE publication correction. No game/audio/save/content alteration intended or approved.
- **Source provenance caveat:** earlier B2-01 exposure to some inherited renderer source was disclosed. B2-04 did not inspect the five forbidden files for code/design. New native code is **not formally clean-room certified**; independent B2-05 reviewer sign-off is essential.

Full evidence and remaining gates: `docs/beta2/04_integration_report.md`.

## B2-03 isolated module record (historical snapshot)


## B2-03 isolated implementation
- New native display declaration: `include/floppy144_win32_display.h`; new implementation: `src/floppy144_win32_display.c`. Completely separate from the existing production build and `F144PlatformApi` binding.
- `tools/beta2_win32_display_tests.c`: standalone memory, allocator failure, cleanup, 11 viewport geometries, invalid dimensions, pixel ordering, Win32 GDI offscreen transfers, presentation bands and optional real-window paint/resize smoke. Uses only permitted portable `Floppy144Surface` contract and Windows SDK.
- `tools/test_beta2_win32_display.ps1`: isolated MSVC /W4 /WX /std:c11 build/test runner. `.github/workflows/beta2-display-ci.yml`: Windows 2022 validation of these files only; branch-specific push trigger.
- No existing `premake5.lua` target changed, no old runtime source touched, no game drawing/audio/persistence/content generated file modified.
- Native backend owns a 921,600-byte, 640×360, 32-bit 0x00RRGGBB top-down frame. Window client viewport is aspect-fitted by integer arithmetic; GDI StretchDIBits uses 32-bit top-down BI_RGB and crisp COLORONCOLOR. Unused bands are painted only after the live image, without full-client preclear.
- **Actual Linux validation:** GCC -std=c99 -Wall -Wextra -Werror -pedantic and Clang -fsanitize=address,undefined both compile and run 27 portable standalone assertions: PASS, zero compiler warnings/errors and no reported sanitizer issues. Windows GDI/window tests were **skipped** because this local environment runs Linux and has no MSVC/MinGW/Wine; this is **not** Windows build verification.
- **Actual Windows CI validation:** GitHub Actions [isolated display job 39519083190](https://github.com/WeeJubya/Floppy144/actions/runs/39519083190) at commit `9ce0fbbb1ac7f04b8a1296b255d40b0672570e0a` **PASS: 79 assertions**, including offscreen GDI pixel/colour/order/bands, zero-size/invalid client handling, and dispatched Win32 window paint/resize smoke (no `SKIP` in the final run). Compiled using MSVC x64 `/std:c11 /W4 /WX` with zero reported C compilation warnings/errors. GitHub emitted a **separate workflow warning** that Node.js 20 actions were being forced onto Node.js 24 (checkout/toolchain action deprecation); this is not a C compiler diagnostic. The earlier run 39518899648 passed 75 assertions but skipped its hidden-window paint smoke; the later 79-assertion run supersedes it.
- **Provenance disclosure:** B2-01 auditing exposed the assistant to the forbidden inherited F144/River2D source and glue earlier in this conversation, before B2-03. During B2-03 those files were deliberately not consulted. The new code is authored against the B2-02 specification, project-owned public contract and Microsoft docs, but **formal clean-room independence is NOT claimed**. Independent human source-expression review is required.
- Outstanding: human Beta.1 screenshot/video/parity and black-flash acceptance, independent source-expression/provenance review, production integration (future authorised stage only), existing intro-label regression and GPLv3-publication verification. No full-game Release build or size remeasurement was performed in B2-03 because the module is not yet linked into the production target. Existing published Beta.1 size metadata remains 822,272 bytes with 652,288 bytes budget headroom.

## Historical B2-01 and B2-02 handover


## Source of truth
- Working repository: https://github.com/WeeJubya/Floppy144
- Dedicated migration branch: `development/Beta-2-runtime`
- Branch was created from the latest `development/Stage-4` commit `46c2a81e097abf69734a4b2f0b7bc91f9142c256`, not from obsolete WeeJubya/main.
- Published immutable Beta.1 reference: https://github.com/GreyDoorRepublik/Floppy144-beta, `main` / tag `Floppy144` at `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`. GDR is for releases and is **not** the development/write target.
- Beta.1 published EXE: 822,272 bytes and GitHub-reported sha256 `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749`. Independent download hash pending.
- **Licence clarification from project owner (20 October):** GPLv3 is the intended Beta.1 licence. Its root `LICENSE` was accidentally omitted from the GDR public repository and is due to be added on 21 October 2026. Treat GPLv3 as intended in B2-01; publication of the missing file and any package/compliance verification remain outstanding. Do not modify the GDR repository in this stage.
- Starting Stage-4 HEAD Windows Actions: main RC regression fails intro floppy-label assertion; independent audio/Release job succeeds with **0 warnings, 0 errors**, EXE 822,272 bytes. This is **not** a clean full regression pass and was built from Stage-4, not Beta.1.

## Deliverables
- `docs/beta2/01_baseline.md` — repository/build/test and behavioural/capture baseline
- `docs/beta2/01_provenance_inventory.md` — derived files, consumers, history, evidence and third-party uncertainties
- `docs/beta2/01_risk_register.md` — risk severities, blockers and owners
- `docs/beta2/STATUS.md` — current handover and future verification list

## Verification and blockers
- Completed: direct inspection of both repository trees and SHA comparisons; confirmed public Beta.1 release/tag/asset metadata; inspected current build, runtime, adapters and original historical audit; reviewed existing Windows Actions steps/logs for exact Stage-4 source base.
- Not performed: local Windows regression run, fresh Beta.2-branch CI, independently downloaded EXE hash, desktop launch, framebuffer/screenshots, human visual/audio acceptance. Do not claim any of these as complete.
- **Rights/compliance review:** River2D GPLv3-derived runtime still ships and GPLv3 is the owner-confirmed intended licence for Beta.1; no separate non-GPL relicensing permission has been evidenced. Root `LICENSE` is actually absent from the inspected GDR public source tree but is an acknowledged accidental omission scheduled for correction on 21 October; confirm completion and actual package notices/corresponding source, without recasting Beta.1 as unlicensed.
- **Regression mismatch**: existing GDR floppy-label assertion fails on Stage-4; verify expected unlettered floppy scene and repair test/implementation separately after human sign-off.
- **Source drift**: Beta.1 source and Stage-4 base differ in seven files; check whether user expects Beta.2 to inherit post-release GUEST login changes.
- Captures: protocol drafted in baseline, no image or framebuffer asset was generated.

## B2-02 deliverables (2026-10-21)
- `docs/beta2/02_runtime_specification.md`: Win32/GDI, fixed 640x360 top-down pixels, deterministic client fit, paint, memory, lifecycle, failure and native service requirements.
- `docs/beta2/02_api_contract.md`: all **15** unmodified `F144PlatformApi` callback slots, game-owned data and native event service boundaries.
- `docs/beta2/02_acceptance_matrix.md`: L0 pure, L1 Windows, L2 regression/Release and L3 human screenshot/audio checks, with MEM/PIX/GEO/RND/INP/LIFE/API/CLK/STO/INST/AUD/BUILD test IDs. All new tests are **unrun**.
- `docs/beta2/02_implementation_handover.md`: self-contained clean-room brief, permitted public headers and project-owned service interfaces, forbidden River2D-derived files, independent implementer/integrator/reviewer division of responsibility.

**B2-02 documentation review:** The four files were screened for inherited runtime function invocations, private F144 struct declarations, copied implementation statements and engine private-member accesses; none were detected. This is a limited textual screen, not independent source-code authorship certification. Public platform contract information and Win32 API semantics remain authorised.

**Specification readiness:** READY FOR INDEPENDENT IMPLEMENTATION DESIGN, **not** approved for final parity or production replacement. A C99 native Windows developer can use B2-02 documents, permitted game-facing headers and Microsoft API references without consulting forbidden runtime source. No actual independent implementation build was performed at B2-02. B2-01 Beta.1 EXE hash and screenshot/audio acceptance **remain unverified**, the prior Stage-4 intro assertion fails, and no full new Windows regression was run on the Beta.2 branch.

## Next stage: B2-03 (not started)
- Verify the separately planned GPLv3 file correction in the **GDR release repository**, and review actual notice/source delivery. As last checked, the GDR public `main` tree was still missing `LICENSE`; the project owner identified an accidental omission, not a new licensing choice.
- Independently download/hash the published Beta.1 EXE and capture reproducible client/framebuffer screenshots/audio evidence under a controlled Windows environment.
- Investigate existing Stage-4 intro floppy-label regression against approved scene behaviour **in a separate authorised task**; do not weaken assertions to invent a pass.
- Confirm explicit choices/compatibility for DPI awareness, minimum Windows version and any pixel-rounding reference discrepancy. Keep the 640x360, 0x00RRGGBB, unchanged ABI, procedural sound and save format as fixed requirements.
- Only after approval, assign clean implementer/integrator/reviewer roles and develop wholly original native backend on `development/Beta-2-runtime` without accessing derived source. Require L0-L3 gate, strict Release, size ≤1,474,560, no derived link inputs, commit/push/report.

**Repo discipline:** development in WeeJubya/Floppy144 on the existing Beta.2 branch. GreyDoorRepublik/Floppy144-beta is for public releases and was not modified by B2-01 or B2-02.
