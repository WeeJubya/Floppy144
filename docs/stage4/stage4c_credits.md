# Stage 4C - Credits and Runtime Attribution

S4C-06 adds a compact one-page Credits/About screen reached from the Settings
screen. It deliberately does not add another top-level Session Control entry.

The wording is grounded in `docs/stage4/runtime_provenance.md`, the root
`LICENSE`, the current Premake build, and the creator/company information
provided for this stage.

The screen includes:

- FLOPPY//144;
- Game Design Company: Grey Door Republic;
- Main Designer / Developer: WeeJubya;
- acknowledgement that F144 contains River2D-derived runtime material;
- River2D copyright (C) 2026 BadAcronym;
- GNU GPL version 3 identification for the derived runtime;
- a concise no-warranty notice;
- a pointer to the full root `LICENSE` distributed with the project.

The provenance audit does not establish a whole-project FLOPPY//144 copyright
holder or settle whether independently authored game material is intended to be
licensed under the inherited GPLv3 file. The in-game screen therefore does not
invent a whole-game copyright or licensing claim. It says only what the audit
supports and notes that broader licensing-scope questions remain documented in
the project provenance record.

No authoritative game version/build identifier exists in the current runtime
or build metadata, so S4C-06 omits one rather than manufacturing a version
string.

Puddle and imgsurf are not credited as current runtime dependencies because the
audit records them as removed/inactive. Premake and GitHub Actions are build
tooling and are not shipped in the executable, so they are not presented as
runtime components.

All credit lines fit on the existing 640x360 framebuffer. No pager or scrolling
state is required. Backspace returns directly to Settings with Credits still
selected.
