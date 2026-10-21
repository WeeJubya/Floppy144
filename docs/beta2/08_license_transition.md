# B2-08: Apply All Rights Reserved to the Beta.2 development branch

**Requested:** 21 October 2026, by the project creator.  
**Repository/branch:** `WeeJubya/Floppy144` / `development/Beta-2`.  
**Change:** Explicitly adopt proprietary terms for this **current Beta.2 branch only**.  
**Public release:** Not authorised by this change.

## Authority and intended scope

The creator states all retained game-specific content, source code, 5x7 glyphs,
icon, narrative and procedural audio were created by the creator with ChatGPT
assistance, apart from the former River2D-derived runtime which B2-05 removed.
The creator explicitly instructed that the Beta.2 development branch should
use the All Rights Reserved licence going forward. This records the project
owner's licensing instruction, not an independent legal ownership opinion.

The copyright identity is printed as **Grey Door Republik**, retaining
**WeeJubya** as the creator/developer credit. No claim is made that a particular
corporate legal structure, assignment or outside rights opinion was checked.

## Licence files

- Root `LICENSE`: **operative Beta.2 proprietary All Rights Reserved text**,
  including limited permission for authorised players to install, play and
  report on the game. It does not generally license reuse or distribution of
  source code or the executable.
- `licenses/history/Beta1-GPL-3.0.txt`: **unchanged** original GPLv3 text,
  including historic River2D/BadAcronym notices, identical to the former root
  Git blob `b1b737ee5e3c21b4d2f41d38020d8214009019ae`.
- `LICENSING.md`: clear version boundary and location map.
- Existing Git history, historical audit reports and public Beta.1 GPLv3
  licence/release are **not rewritten or revoked**.
- `docs/beta2/07_proprietary_licence_draft.md` remains a historical
  *pre-authorisation draft*; the **operative** text is the root `LICENSE`.

## Build integration and guard rails

The existing `premake5.lua` postbuild command copies root `LICENSE` into
`bin/release/LICENSE`; because the root is updated, the Release bundle picks
up the proprietary Beta.2 terms automatically.

`tools/test_beta2_license.ps1` verifies:
- The active root licence explicitly identifies the current Beta.2 game,
  Grey Door Republik, WeeJubya and All Rights Reserved.
- Original Beta.1 GPLv3 text and copyright notice are retained byte-identically
  via Git blob SHA; absent/changed historic licence **fails the test**.
- Premake's packaging rule still copies **the operative root licence**.
- After a Release build, the packaged `bin/release/LICENSE` hashes identically
  to the proprietary root, not to the archived GPLv3 text.

CI runs the source-phase licence test as part of the full regression job
and the built-phase check within the strict x64 Release job. The unchanged
B2-05 test still excludes inherited runtime sources/objects; its preserved
Credits attribution pointer now accurately refers to the archival Beta.1 text.

## Release boundary

This action changes licence and documentation, not gameplay logic, saves,
world state, assets or procedural audio. The Windows compiler, full regression
suite, generated-data drift, link imports, executable size, binary SHA-256 and
licence packaging must be reverified in CI after commit.

**No public Beta.2 merge/tag/upload.** Human Windows visual/audio review,
real Beta.1-created saves and explicit final release approval are not
substituted by a licence change. The creator's instruction authorises the
current development licence, not public release.
