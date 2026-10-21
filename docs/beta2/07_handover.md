# B2-07: Release preparation, Git handover and hold register

**Date:** 21 October 2026. **Current state:** engineering test and package preparation complete at B2-06; **Beta.2 public release is blocked** by unsatisfied licensing/independent-provenance/manual Windows/real save-file acceptance.

## 1. Repository topology and Git state

- **Developer:** `WeeJubya/Floppy144`, existing branch `development/Beta-2-runtime`. B2-06 verified commit: `514b159197bcf4e9e2f2289cbc5b645d2b9defb4`. B2-07 paperwork is committed on this same branch; refer to live Git HEAD after that documentation commit.
- **Studio public release repository:** `GreyDoorRepublik/Floppy144-beta`, `main` = `9640d2873a0e3f38451d3e953a60f291a721b153` after a separately pushed GPLv3 `LICENSE` correction. The original Beta.1 tag remains `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`. The repository is **not configured as a fork**. At audit it had one branch, `main`, and no open PR. The developer Beta.2 branch is **not present** in this public repository.
- A literal GitHub PR from `WeeJubya/Floppy144:development/Beta-2-runtime` into `GreyDoorRepublik/Floppy144-beta:main` is **not currently a normal same-repository/fork-network PR**. Do not invent a PR URL or assume the public main can be safely fast-forwarded. A curated public staging branch/PR would require a separate authorised repository import/publication step after rights clearance, performed through the release owner's accepted workflow.
- **No B2-07 merge, public branch creation, tag, release asset upload or publication took place.** No historical commit, tag or licence was rewritten.

## 2. Requested deliverables and destinations

| Deliverable | Repository staging location | Status |
|---|---|---|
| Engineering, rights and final-release sign-off | `docs/beta2/07_final_signoff.md` | Prepared, **release decision BLOCKED** |
| Gate handover, packaging/PR route | `docs/beta2/07_handover.md` | Prepared |
| Public README candidate (no spoilers) | `docs/beta2/07_public_readme_draft.md` | Draft only; do **not** copy to public root README pending final gates |
| Player-facing Beta.2 release notes | `docs/beta2/07_release_notes_draft.md` | Draft only; withheld from public GitHub Releases |
| Curated proposed PR body | `docs/beta2/07_pr_draft.md` | Draft only; no live public PR while provenance and licence blocked |
| Current stage status | `docs/beta2/STATUS.md` | Updated |
| Verified Windows binary | [B2-06 RC verification artifact](https://github.com/WeeJubya/Floppy144/actions/runs/39580254575) | Built/hashed but **NOT PUBLICLY RELEASED** |
| Appropriate source licence/notices and distributable source package | Requires qualified rights review | **BLOCKED**, cannot choose “All Rights Reserved” or assert corresponding-source scope yet |

## 3. Technical handover from verified Beta.1

| Metric | Exact verified reference |
|---|---|
| Beta.1 public EXE | **822,272 bytes**, SHA-256 `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749` |
| Beta.1 source/tag | `GreyDoorRepublik/Floppy144-beta@0981e4ab3694a1cdb5b6338c199e0262a6ed14bf` |
| B2-06 verified code commit | `WeeJubya/Floppy144@514b159197bcf4e9e2f2289cbc5b645d2b9defb4` |
| B2-06 latest strict Windows build at that SHA | **973,312 bytes**, SHA-256 `C478E6D33B52E6E12D8786B048C8AB7A3CD2388FA3E87C48046C9CEEF311980F`; final B2-07 doc-only build may produce a different binary hash |
| Executable ceiling/headroom | 1,474,560 bytes / 501,248 bytes remaining (**PASS**) |
| DLL import list | USER32, KERNEL32, GDI32, SHELL32, WINMM only |
| Compiler | Windows MSVC x64 strict Release; 0 production compiler errors / warnings |
| Canonical tests | 46 ordered full-suite groups PASS, Stage 4D+ 15/15; cleanbuild and root Release PASS |
| Independent graphics/host | 79 display assertions and 25 host assertions PASS |
| Source audit | 212 active C/header/generated/test files PASS; original River2D engine removed, no old linker objects |

The cited hashes represent specific CI binaries. At publication time obtain **a new SHA-256 for the exact selected EXE** and verify it against a reproduced manifest and intended source commit. Do not reuse a previous hash based on file size alone.

## 4. Curated public tree, not wholesale merger

A direct tracked-blob comparison with Beta.1 found **95 developer-only additions, seven removed paths and 12 differing shared paths** before B2-07 documentation. Additions include `docs/beta2` and other engineering/provenance dossiers, local-development `.sav`/`.dat` fixtures, generated/test tooling and build artefacts. Preserve the full *developer* history privately, but **do not blindly mirror the development tree into the public player repository**.

Proposed gated publication steps, after written rights approval:
1. Freeze the approved development SHA and binary identity; capture signed tests, desk-review video/audio and authentic Beta.1 migration fixtures.
2. Decide and document precisely which **licence** legally applies to each code/asset/contribution and public source distribution. Record that the GPLv3 root `LICENSE` was separately added to current public `main` in `9640d287...` and verify any remaining past-release notice/corresponding-source obligations, without rewriting the original Beta.1 tag.
3. Prepare a **curated public source tree**: the required buildable game source, headers, canonical generation inputs, script/tool source as needed for corresponding-source obligations, resources, approved documentation and copyright/licence notices. Verify legal source-distribution obligations before deciding if full/partial source is public.
4. Exclude **user save/profile/settings instances**, temporary binaries/objects, diagnostic PPMs, unreleased source notes and internal audit reports unless explicitly required by law or approved for publication. Do not exclude licence-required material on the pretext of tidiness.
5. Stage player-facing README and drafted release notes from `07_public_readme_draft.md` and `07_release_notes_draft.md`; preserve Grey Door Republik studio credit and WeeJubya developer credit, avoid spoilers.
6. Only once public publication is authorised, create the **existing-repo release staging branch** (or an approved fork/PR path) in `GreyDoorRepublik/Floppy144-beta`, push just the curated, auditable changes, open a **draft PR targeting main** with the `07_pr_draft.md` body and correct test evidence, obtain independent reviewers and rights sign-off.
7. Confirm GitHub diff does not import irrelevant old saves, internal/proprietary files or stale code. Inspect generated-file drift and developer-resource accidental inclusions in the PR.
8. Obtain a **fresh, explicit final release approval** specifying public SHA, exact EXE SHA, licensing/notices and any source package.
9. Only then merge through the approved workflow, create the **agreed** Beta.2 tag/version, upload approved assets and checksums, and independently re-fetch/tag-hash every public item.
10. Update this handover with actual PR URL, merge commit, tag, release URL, packaged sizes, SHA-256 and acceptance signers; do not mark “complete” before verifying public assets.

## 5. Mandatory blocking items

| Blocker | Evidence that would close it | Current status |
|---|---|---|
| Original-source/relicensing rights | Independent qualified reviewer and copyright holders' documented grants/assignments for all retained components | **OPEN** |
| 5×7 glyphs/icon and any other asset authorship | Verified design originals, explicit licence/permission or separately approved non-regressing replacement | **OPEN** |
| Beta.1 GPLv3 historical obligations | Root `LICENSE` now exists at current public `main` (identical to development); evaluate whether past binary distribution, notices and corresponding source met GPLv3 requirements and document any required remediation | **OPEN (file added; legal review pending)** |
| Beta.1→Beta.2 manual/autosave/profile/settings | Actual Beta.1-created files backed up, controlled Beta.2 load/save/relaunch, reviewer signature | **OPEN** |
| Real Windows visual parity | Completed `06_visual_acceptance.md` VIS-01..11, matched seeded captures and observed issues triaged | **OPEN** |
| Real WinMM audible parity | AUD-H01..04 on a Windows sound device, evidence and signature | **OPEN** |
| Licensed packaging/source manifest | Rights-approved licence, attribution file, necessary source artefacts and reproducible public manifest | **OPEN** |
| Explicit studio-owner release approval | Written confirmation of exact code HEAD, license and final binary hash | **NOT GIVEN** |

The historical pre-Beta.1 64/68-byte V1 save examples are **unsupported even by Beta.1**; they are not validated compatibility fixtures. Imported Beta.1 format requires 72-byte V1 payloads; no “fixed” or “preserved all historical save formats” claim should be made in the public release notes.

## 6. Post-gate acceptance records (intentionally blank)

- Independent provenance reviewer / date / supporting evidence: **NOT SIGNED**
- Copyright/licence signatory and documented scope: **NOT SIGNED**
- Visual reviewer / run/video hashes / VIS statuses: **NOT SIGNED**
- Audio reviewer / output device / AUD statuses: **NOT SIGNED**
- Genuine Beta.1 save migration tester / file hashes: **NOT SIGNED**
- Release owner's exact-final SHA and public licence approval: **NOT SIGNED**
- Public branch and PR URL: **NONE**
- Merge commit, release tag and GitHub Release URL: **NONE**
- Uploaded public EXE/source/checksum and independent verification: **NONE**

**Next action:** close rights and mandatory human verification gates **before** any public branch/PR/package. The final status is **PREPARED, NOT APPROVED**.
