# Beta.2 runtime migration status
Last updated: 2026-10-20. Project: Floppy//144. Stage: **B2-01 investigation and baseline documents**; no runtime implementation is authorised here.

## Source of truth
- Working repository: https://github.com/WeeJubya/Floppy144
- Dedicated migration branch: `development/Beta-2-runtime`
- Branch was created from the latest `development/Stage-4` commit `46c2a81e097abf69734a4b2f0b7bc91f9142c256`, not from obsolete WeeJubya/main.
- Published immutable Beta.1 reference: https://github.com/GreyDoorRepublik/Floppy144-beta, `main` / tag `Floppy144` at `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`. GDR is for releases and is **not** the development/write target.
- Beta.1 published EXE: 822,272 bytes and GitHub-reported sha256 `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749`. Independent download hash pending.
- Starting Stage-4 HEAD Windows Actions: main RC regression fails intro floppy-label assertion; independent audio/Release job succeeds with **0 warnings, 0 errors**, EXE 822,272 bytes. This is **not** a clean full regression pass and was built from Stage-4, not Beta.1.

## Deliverables
- `docs/beta2/01_baseline.md` — repository/build/test and behavioural/capture baseline
- `docs/beta2/01_provenance_inventory.md` — derived files, consumers, history, evidence and third-party uncertainties
- `docs/beta2/01_risk_register.md` — risk severities, blockers and owners
- `docs/beta2/STATUS.md` — current handover and future verification list

## Verification and blockers
- Completed: direct inspection of both repository trees and SHA comparisons; confirmed public Beta.1 release/tag/asset metadata; inspected current build, runtime, adapters and original historical audit; reviewed existing Windows Actions steps/logs for exact Stage-4 source base.
- Not performed: local Windows regression run, fresh Beta.2-branch CI, independently downloaded EXE hash, desktop launch, framebuffer/screenshots, human visual/audio acceptance. Do not claim any of these as complete.
- **Urgent rights/compliance review**: demonstrably derived River2D GPLv3 runtime still ships, root LICENSE missing from published Beta source and only an EXE is listed in release assets. No separate redistribution waiver/rights identified.
- **Regression mismatch**: existing GDR floppy-label assertion fails on Stage-4; verify expected unlettered floppy scene and repair test/implementation separately after human sign-off.
- **Source drift**: Beta.1 source and Stage-4 base differ in seven files; check whether user expects Beta.2 to inherit post-release GUEST login changes.
- Captures: protocol drafted in baseline, no image or framebuffer asset was generated.

## Next stage
B2-02 is **not started / not yet cleared to implement**. Obtain human legal/rights guidance, independently hash the published EXE, establish labelled capture fixtures, resolve existing test expectation and explicitly approve a behavioural-only clean-room implementation brief. Future code changes must preserve procedural audio, save compatibility and the 1,474,560-byte size cap. Never use historical derived source as a copy template.
