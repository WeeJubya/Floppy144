# Stage 4 Completion and Stage 5 Handover

**Repository:** `WeeJubya/Floppy144`  
**Branch:** `Stage4`  
**Scope:** Stage 4A through Stage 4G, including the Grey Door Republic
one-shot end-case. The gate described here is the automated engineering
integration gate, not a replacement for the separate manual replay QA.

## Immutable starting point

Stage 3 source baseline: `fe2239123f98346ed06b48bb500672a5e6e2e172`.
Stage 3 Release executable: **659,968 bytes**.
The completed Stage 3 rules, progression logic, data identity and
1,474,560-byte standalone Windows executable limit remain authoritative.
See `stage3_baseline.md`.

## Stage 4 delivered scope

| Phase | Delivery and verification |
| --- | --- |
| **4A: Baseline/architecture** | Immutable Stage 3 provenance, dependency inventory and platform separation proposal |
| **4B: Platform** | Portable Core/Win32 contract, normalised logical actions, AppData paths, audio/timing/lifecycle, single instance, independent tests; `stage4b_completion.md` |
| **4C: Player identity** | Profile, operator name/body style, Settings, Credits, terminal authentication, reinstatement and bespoke Completion; `stage4c_completion.md` |
| **4D: Presentation** | Help layout, camera scale, player character, inspection 2.5D, UI consistency; `stage4d_completion.md` |
| **4E: Replay/atmosphere** | Deterministic variation, dates and noticeboards, takeaway, crossword, paperback, DR-04 seeded swap, document expansion and reserve decision; `stage4e_completion.md` |
| **4F: Application journey** | Win32 icon, launch intro and complete menu-to-completion presentation pass; `stage4f_presentation_journey.md` |
| **4G: Secret end-case** | Orphan document, seeded physical Grey Door, modern impossible office, Developer sequence, silent one-shot erasure, persistence hardening and S4G-05 full matrix; `stage4g_completion.md` |

## Final Stage 4G gate

**STAGE 4G: PASS.** The final source acceptance completed in
[Windows GitHub Actions run 37810016677](https://github.com/WeeJubya/Floppy144/actions/runs/37810016677)
against `e1fc849f70877fc6cd081ed43f07b5342147341a`.
The end-case consists of **22** checked corridor candidates and
**48** complete headless journeys over 12 seeds and four pre-completion
recovery contexts. Every physical persistence boundary and all
Stage 3/4 core regressions passed. Production Core/Win32/Release
builds passed with **0 warnings, 0 errors**. Four pre-existing C4701
warnings still occur solely in two legacy Stage 3B test harnesses.

The authoritative Stage 4G handover is
`docs/stage4/stage4g_completion.md`. It records trigger, world geometry,
every one-shot state, complete scene, save/reload, progression independence,
secrecy and the 12-seed x four-context end-to-end test matrix.

The normal CI entry is `.github/workflows/stage3c-ci.yml` on `Stage4`.
It runs the complete Stage 3 and Stage 4B/4C/4D/4E/4F/4G suite, plus
strict warning-clean Debug/Core/Win32/Release builds, icon/resource checks,
generated-data drift and final decompressed executable size gate.

## Stage 4 release measurements

| Item | Bytes |
| --- | ---: |
| Stage 3 immutable baseline | 659,968 |
| Pre-Stage 4G Release | 716,800 |
| Final Stage 4G Release | 725,504 |
| Stage 4G incremental cost | +8,704 |
| Total Stage 4 growth | +65,536 |
| Hard executable-size limit | 1,474,560 |
| Remaining floppy headroom | **749,056** |

**Runtime architecture:** the Easter egg adds no new Site rooms, authored
world geometry, collision layer, Profile fields or actual recovered-capacity
value. It uses Stage 4E variation, Stage 4B monotonic timing and logical
actions and the existing 640x360 framebuffer. A matching-run completed
autosave wins over an older manual checkpoint only to prevent replay of a
completed anomaly. All ordinary progression and ending calculations retain
their Stage 3/4 ownership.

**Secrecy:** no achievement, Notebook/Completion/Profile entry, Site
Directory marker or menu acknowledgement. The pre-existing *GREY DOOR
REPUBLIC* company credit is studio identification, not a spoiler about
the encounter; the secret is not advertised by the game or Help.

## Outstanding non-gate QA

The automated pipeline simulates complete Grey Door routes and checks
rendered pixels, but cannot independently certify subjective visual
timing from a manually played Windows session. The Stage 4E sign-off
also documented a hold on two human-observed full alternate-ending
replays. Neither has been misreported as a manual test.

The Stage 4 implementation and automated integration gate can be
complete with these explicitly documented presentation/release
QA observations still needed before public submission. Stage 5 can
address final release QA and packaging without further Stage 4
feature expansion.

## Final repository state

The final signed-off SHA and Windows Actions run are the commit/run
that ships this record on `Stage4`.
GitHub connector commits update the remote tree atomically; no
separate local checkout or working-tree modifications are created
by this process. A local `git status` is therefore not asserted.

**Engineering handover:** STAGE 4 COMPLETE / GREY DOOR CLOSED /
READY FOR STAGE 5, subject to the separate human replay QA above.

**Handover source commit tested:** `e1fc849f70877fc6cd081ed43f07b5342147341a`.
**Signed documentation:** the final Stage4 commit containing these two
handover records. The documentation-only follow-up is revalidated by
the normal Stage4 workflow before declaring a final branch HEAD.
