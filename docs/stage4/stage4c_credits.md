# Stage 4C - Credits and Runtime Attribution

S4C-06 adds a compact one-page Credits/About screen reached from Settings. It does not add another top-level Session Control entry.

The wording is grounded in `docs/stage4/runtime_provenance.md`, the root `LICENSE`, current build metadata, and the creator/company information supplied for this stage.

The screen identifies FLOPPY//144, Grey Door Republic, WeeJubya, the River2D-derived F144 runtime lineage, River2D copyright (C) 2026 BadAcronym, GNU GPL version 3 for the derived runtime, redistribution/no-warranty notices, and the full `LICENSE` location.

The provenance audit does not establish a whole-project FLOPPY//144 copyright holder or settle the licence scope of independently authored game material. The screen therefore makes no invented whole-game copyright/licence claim and explicitly keeps broader scope questions in the provenance record.

No authoritative game version/build identifier exists in the current runtime/build metadata, so none is fabricated.

Puddle and imgsurf are not credited as current dependencies because the audit records them as inactive/removed. Premake and GitHub Actions are build tooling rather than shipped runtime components.

The root GPL text is copied beside the built executable as `bin/<configuration>/LICENSE`; the source package also retains the tracked root licence.

All credit lines fit the existing 640x360 framebuffer, so no scrolling/pager state is necessary. Backspace returns to Settings with Credits selected.
