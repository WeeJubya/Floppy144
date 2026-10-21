# Beta.2 runtime migration status
Last updated: 2026-10-21. Project: Floppy//144. **Current stage: B2-03 isolated Windows display module implemented and undergoing native verification.** No production executable linking or gameplay modifications.

## B2-03 isolated implementation
- New native display declaration: `include/floppy144_win32_display.h`; new implementation: `src/floppy144_win32_display.c`. Completely separate from the existing production build and `F144PlatformApi` binding.
- `tools/beta2_win32_display_tests.c`: standalone memory, allocator failure, cleanup, 11 viewport geometries, invalid dimensions, pixel ordering, Win32 GDI offscreen transfers, presentation bands and optional real-window paint/resize smoke. Uses only permitted portable `Floppy144Surface` contract and Windows SDK.
- `tools/test_beta2_win32_display.ps1`: isolated MSVC /W4 /WX /std:c11 build/test runner. `.github/workflows/beta2-display-ci.yml`: Windows 2022 validation of these files only; branch-specific push trigger.
- No existing `premake5.lua` target changed, no old runtime source touched, no game drawing/audio/persistence/content generated file modified.
- Native backend owns a 921,600-byte, 640×360, 32-bit 0x00RRGGBB top-down frame. Window client viewport is aspect-fitted by integer arithmetic; GDI StretchDIBits uses 32-bit top-down BI_RGB and crisp COLORONCOLOR. Unused bands are painted only after the live image, without full-client preclear.
- **Actual Linux validation:** GCC -std=c99 -Wall -Wextra -Werror -pedantic and Clang -fsanitize=address,undefined both compile and run 27 portable standalone assertions: PASS, zero compiler warnings/errors and no reported sanitizer issues. Windows GDI/window tests were **skipped** because this local environment runs Linux and has no MSVC/MinGW/Wine; this is **not** Windows build verification.
- **Windows validation:** isolated Windows Actions test outcome to be recorded separately after completion; do not claim it passed before inspecting the actual job steps/log.
- **Provenance disclosure:** B2-01 auditing exposed the assistant to the forbidden inherited F144/River2D source and glue earlier in this conversation, before B2-03. During B2-03 those files were deliberately not consulted. The new code is authored against the B2-02 specification, project-owned public contract and Microsoft docs, but **formal clean-room independence is NOT claimed**. Independent human source-expression review is required.
- Outstanding: Windows native CI result; human video/screenshot parity from hash-verified Beta.1, black-flash/resize acceptance, production integration to be done only in a future authorised stage; existing intro-label regression and GPLv3-publication verification from B2-01 remain open.

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
