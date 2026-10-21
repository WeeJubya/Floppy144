# DRAFT PULL REQUEST (NOT OPENED)

> **Proposed target, only after formal rights and manual-acceptance clearance:** `GreyDoorRepublik/Floppy144-beta:main`, from an **approved curated Beta.2 staging branch in that same repository or an authorised fork**. The existing `WeeJubya/Floppy144:development/Beta-2-runtime` is a separate repository, not a branch of `GreyDoorRepublik/Floppy144-beta`; do not misrepresent this document as a live GitHub PR. **Keep the eventual PR in draft until approved.**

## Suggested PR title

`Beta.2: replace legacy Windows presenter with native Win32 runtime (REVIEW ONLY)`

## Suggested PR description

### Summary

Replaces the historic River2D-derived F144 display and Win32 presenter with a small independently implemented Win32/GDI 640×360 software renderer and host, while retaining the portable game-facing `F144PlatformApi`, procedural WinMM audio, profile/settings/save locations, narrative and gameplay.

The superseded derived renderer implementation and its unused string-view helper are removed from current source and the production build. Old commits and historical provenance/licence records remain preserved.

This is a **review-only** staging PR. Do not merge, tag or publish until the release checklist and explicit studio approval are complete.

### Verification (engineering)

- [x] B2-06 full `testbuild.ps1`: 46 Stage 2/3/4/BF groups pass.
- [x] Independent Stage 4D+ suite: 15/15 pass.
- [x] Canonical generation and drift validation: pass.
- [x] `cleanbuild.ps1` and `run.ps1 -build release`: pass.
- [x] B2-03 framebuffer/GDI: 79 Windows assertions pass.
- [x] B2-04 native host: 25 Windows assertions pass.
- [x] B2-05 active source/compiled archive/runtime-import audit: pass; derived engine objects absent.
- [x] Strict MSVC x64 Release: 0 errors, 0 production compiler warnings.
- [x] EXE size **973,312 / 1,474,560 bytes**, **501,248 remaining**, pass.
- [x] Imported system DLLs only: USER32, KERNEL32, GDI32, SHELL32, WINMM.
- [x] Published Beta.1 EXE actually downloaded and verified: 822,272 bytes, SHA-256 `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749`; 13 stable game modules hash-match.
- [x] Current developer build checks at `514b159197bcf4e9e2f2289cbc5b645d2b9defb4`, [Windows run 39580254575](https://github.com/WeeJubya/Floppy144/actions/runs/39580254575): three jobs pass. Strict binary at that build SHA-256 `C478E6D33B52E6E12D8786B048C8AB7A3CD2388FA3E87C48046C9CEEF311980F`.

**Hashes are specific to actual CI binaries**. Check again when a particular public build and staging source SHA are selected.

### Mandatory blockers (DO NOT MERGE)

- [ ] Independent source-unexposed reviewer and rights/licensing specialist approve the remaining game code, contributors, font glyph patterns, icon and any historically entangled coordinator expression. Earlier assistant source exposure explicitly prevents a formal clean-room declaration.
- [ ] Copyright holders approve the proposed **specific** licence for the **specific** candidate. An “All Rights Reserved” copyright notice is **not** assumed to replace GPLv3 obligations or prior licences.
- [ ] Beta.1 GPLv3 public repository `LICENSE` omission is addressed separately, with advice on source/distribution obligations; preserve all old historical notices/rights.
- [ ] Matched-seed Windows Beta.1/Beta.2 GUI screenshot/video walkthrough and black-flash/resize checks (VIS-01..11) are signed off.
- [ ] Actual Windows WinMM audio playback/listening checks (AUD-H01..04) are signed off.
- [ ] Genuine published Beta.1-created manual/autosave/profile/settings load/save/migration tested safely and signed off.
- [ ] The published source tree is **curated**: no internal audit dossiers, historical user save files, debug/capture artifacts, accidentally tracked binaries or unlicensed assets. Include **all source/notices required by the legally approved licence**.
- [ ] Inspect the PR file list against published Beta.1 and current reviewed source, ensuring no unintended gameplay, narrative,/generated-data or resource edits.
- [ ] Re-run the same full tests, strict Release/imports and size checks against the exact curated PR source HEAD. Capture SHA-256 of the **exact** candidate EXE.
- [ ] Grey Door Republik release owner explicitly approves the final merge **and separate public Beta.2 tag/release**, with named exact Git SHA, binary SHA and licence/asset manifest.

### Candidate package (NOT AUTHORISED)

- `Floppy144.exe` Windows x64 built with static MSVC CRT.
- Appropriate verified `LICENSE`, `NOTICES` / third-party acknowledgements and source distribution obligations once cleared.
- Player-facing `README.md` and Beta.2 release notes after review.
- SHA-256/size manifest tied to the exact uploaded artifact.
- Excludes `.obj`, intermediate test executables, arbitrary `.sav`/`.dat`, captures and internal notes.

### Risk register

1. **High/blocking:** no verified proprietary/all-rights-reserved relicensing authority; unknown source/asset authorship.
2. **High/blocking:** missing human Windows visual/audio evidence and public Beta.1-created compatibility fixtures.
3. **High/blocking:** earlier public Beta.1 GPLv3 distribution issue remains unresolved.
4. **Medium:** the GDR public repository is not a fork and has no current Beta.2 source branch; source/tree review and explicit staging publication is required rather than an unreviewed raw branch merge.
5. **Medium:** the source delta contains 95 developer-only files and internal source/test fixtures; a naïve merge might leak or distribute material not suitable for release.

**No reviewer approval requested by this document, and no live PR exists.**
