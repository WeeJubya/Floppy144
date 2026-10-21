# B2-07: Creator provenance declaration and classification update

**Recorded:** 21 October 2026  
**Source:** the creator's direct statements in the Floppy//144 project conversation.  
**Status:** **creator-attested**, not an independent legal or source-unexposed expert certification.

## 1. Declaration received from the creator

The creator stated:

> "Other than the R2D, all other components have been created by me, with your help."

The creator further clarified that all versions before the publicly released Beta.1 were local development builds, tested by the creator alone. Beta.1 was the first release distributed to other beta testers, and the planned Beta.2 is a new release.

**Interpretation authorised for this project:** the creator asserts original authorship of the remaining game-specific source code, pixel artwork, embedded 5×7 font glyph patterns, Windows icon artwork, content, narrative, game data, procedural sound and music, and ancillary game implementations, with ChatGPT assistance. River2D was the known external runtime origin and was replaced/removed in the Beta.2 migration. The declaration includes the previously unresolved glyph and icon provenance. It is **not** a statement that Windows OS APIs, Microsoft tooling, GNU GPL text, historical source records or external CI/build services were created by the game author.

Do not attribute original game content to unknown third parties merely because the earlier mechanical audit lacked artwork creation records. **The documented author claim is now present.** Conversely, do not mislabel an author claim as independently authenticated employment/assignment records, a search of all previous external publications, or proof that generated material is eligible for exclusive copyright in every jurisdiction.

## 2. Current provenance classifications

| Component | Updated evidence-based classification | Caveat |
|---|---|---|
| All retained original Floppy//144 gameplay, drawings, Site/UI, narrative, generated-data inputs and tools | **Creator-declared original work with ChatGPT assistance** | No independently authenticated third-party ownership audit of every source statement |
| Embedded 5×7 glyph data (60 character cases in `game/src/floppy144_draw.c`) | **Creator-declared original work**; historic B2-05 report lacked this testimony | Font expression eligibility and unintentional outside similarity are not technically proven |
| Original Windows application icon (compiled `platform/win32/floppy144.ico`) | **Creator-declared original artwork** | No original design-file metadata independently examined |
| Game audio and WinMM synthesis source | **Creator-declared original work** | Sound-device acceptance separate; system WinMM APIs are Microsoft platform services |
| New Win32/GDI display/host/timing code | **New Beta.2 implementation** with authorial source-exposure caveat and limited line comparison | Historical inherited source was inspected by the same assistant involved in implementing the replacement; cannot claim formal clean-room separation |
| Original River2D-derived five runtime paths, uncertain unused `string_view` utility and old submodule declaration | **Removed from active tree and production binary**; historical Git records preserved | Removal does not extinguish Beta.1 GPL rights or historic obligations |
| Retained GPLv3 `LICENSE` and historic BadAcronym notices | **Historical licence and attribution records** | The licence document is not creator-owned game artwork; new proprietary licence must be scoped to the new version and only rights-controlled components |

## 3. Historical-release boundary

The creator reports no third-party public distribution before Beta.1. Do not infer from this that no copyright obligations existed or that history should be deleted: keep the original source history, GPL notices and Beta.1 tag. The public Beta.1 `main` gained an identical GPLv3 `LICENSE` in separate commit `9640d2873a0e3f38451d3e953a60f291a721b153` on 21 October 2026; the original Beta.1 release tag and prior permissions remain unchanged.

The creator intends to explore **All Rights Reserved** licensing for Beta.2 and subsequent versions. This is an instruction to prepare a new release-specific rights and licence proposal, **not** proof that existing GPL rights for old versions can be withdrawn, nor an authorisation to rewrite past licences or merge/publish Beta.2.

## 4. Remaining bounded questions

1. **Ownership identity:** establish whether Grey Door Republik is an incorporated rights holder, a registered trading name or a studio credit used by the individual creator. Credit **Grey Door Republik** as studio and **WeeJubya** as creator; identify the actual legal copyright holder before issuing the proprietary licence.
2. **Licence scope:** ensure proposed Beta.2 binary and any public source/package consist only of rights-controlled current work and permitted platform dependencies. Do not introduce another's copyrighted code or claim legal clean-room certification solely on the basis of a diff. See `07_expression_recheck.md`.
3. **Prior rights:** retain Beta.1 GPLv3 terms, original tag, history and notices. Review actual Beta.1 binary corresponding-source/attribution obligations separately if needed.
4. **Publication gates:** human real-Windows visual/audio acceptance and authentic Beta.1-created save migration evidence are still absent. Those are not copyright disputes, but they continue blocking the release.

## 5. Attestation status

**Creator declaration recorded as direct first-party testimony.** No signed legal affidavit, transfer/assignment to a separately incorporated studio, specialist written legal opinion or independent source-unexposed expression certification was supplied.

**Updated assessment:** the *specific glyph/icon authorship unknown* finding from B2-05 is **addressed by creator declaration**, not left as an unanswered author-identification request. Formal copyright/relicensing and final release approval remain open on narrower grounds. No production runtime, game content or historical licence was altered by recording this declaration.
