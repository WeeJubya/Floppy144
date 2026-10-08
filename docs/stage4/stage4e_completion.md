# Stage 4E integration, determinism and replay acceptance (S4E-10)

**Status: HOLD / NOT SIGNED OFF (as inspected on 2026-10-08).**
**Scope:** `Stage4` at `ff2a02679f6b8ecf23ac16c266b21d22f3b38655`.
**Rule:** No Stage 4F work or new feature work is accepted through this gate.

This record distinguishes **verified green regression/build evidence** from **unmet
S4E-10 acceptance criteria**. A clean CI result does not, on its own, prove
the omitted DR-04 seeded A/B arrangement or two complete end-to-end runs.

## Commit and evidence provenance

- Immutable Stage 3 source: `fe2239123f98346ed06b48bb500672a5e6e2e172`.
- Stage 4D completed: `1ce97d226`; Release baseline **695,808 bytes**.
- First Stage 4E variation-service commit: `a0991e76d`.
- Stage 4E content changes: `088108824` calendar; `010f18b60` /
  `cfa4b4a70` noticeboard; `cc4cf2d58` takeaway; `95ffa9445`
  crossword; `170f4b43e` paperback; `cfdc86d70` through
  `a411836d2` archive expansion/regression corrections.
- S4E-09 optional-collection decision: `1df5e32e9` /
  `ff2a02679` (both documentary).
- There is **no identifiable S4E-07/DR-04 swap commit or specific swap
  regression** in the inspected linear `Stage4` history between S4E-06 and
  S4E-08. This states what is in this branch, not whether equivalent work was
  completed in another workspace.
- Freshest examined exact-HEAD GitHub Actions Windows workflow:
  [run 37767175122](https://github.com/WeeJubya/Floppy144/actions/runs/37767175122),
  job `113277569837`, **success**, at `ff2a02679`.
  Source: `.github/workflows/stage3c-ci.yml`.
- This S4E-10 report is documentary; its creation is not a claim that an
  additional playable end-to-end acceptance route was executed.

## Deterministic service and save semantics

`Floppy144VariationValue/Range/Chance` in
`game/src/floppy144_variation.c` use 32-bit unsigned, stateless,
versioned FNV-1a plus avalanche, keyed by saved recovery seed, feature
namespace and canonical item ID. No global PRNG stream or traversal-dependent
consumption is present. `Floppy144RunState.recovery_seed` is encoded and
decoded in V1 and V2 saved-run payloads. The ordinary initial seed is sourced
by the platform coordinator; seeded choices themselves do not consult time.

Same seed + same build + same feature/key -> repeatable choice, irrespective
of calls to unrelated generators. Cross-build choice identity is not
guaranteed if option arrays or namespace versions deliberately change.
`-debug -seed` is explicitly gated by developer configuration.

### Fixed seed matrix (actual CI golden fixtures)

| Content | Seed A = 144 | Seed B = 145 |
| --- | --- | --- |
| Takeaway business, P-330 | THE PENDING HEDGEHOG PIE OFFICE | THE OFFICIAL SQUIRREL DUMPLING DEPOT |
| Takeaway special | CLASSIFIED BEEF IN A BOX | STAPLED-TOGETHER NOODLES |
| Crossword, P-074 | ORDER / INDEX | QUEUE / SHELF |
| Paperback, P-073 | SENIOR LINE MANAGER / CRISPIN QUIBBLE | HEARTBROKEN ARCHIVE CLERK / MILLICENT MUDDLE |
| DR-04 Records title at compiled slot | Physical Archive Reconciliation Workstream: RS-0216 | **Same RS-0216** |
| DR-04 Technology title at compiled slot | Terminal Network Remediation Workstream: RS-0147 | **Same RS-0147** |
| DR-04 arrangement A/B | **Not demonstrated** | **Not demonstrated** |

Takeaway and paperback have byte-exact known-seed assertions. Crossword
tests also assert the five-row grid and clue/answer crossing. All three
demonstrably change between the two seeds while preserving unrelated
feature choices. The Cabinet regression regenerates flavour after V2
save/reload and checks canonical identity and read-only behaviour.

**Important S4E-08 identity change:** `RS-0063` and `RS-0087` were
the Stage 3 DR-04 branch positions; S4E-08 redistributed authored
documents. Do not hardcode those obsolete numbers in a new acceptance test.
Test the canonical authored identities and T-010/T-011 effects, then resolve
the *current* player-facing IDs.

**DR-04 blocker:** In the inspected branch,
`Floppy144DocumentGet(collection, record_index)` reads fixed generated
document definitions, `Floppy144CatalogueBuildRecord(collection, index,...)`
resolves those definitions without run state, and the Terminal and core
coordinator have no seeded remap for the two authored workstream records.
Neither seed 144 nor seed 145 can exchange their placement in the current
generated catalogue. Static T-010/T-011 branch-choice progression tests do
not establish the requested S4E-07 presentation permutation.

Before PASS, either reconcile an actually implemented S4E-07 commit into
`Stage4`, or repair this missing mapping through the proper runtime
identity layer, without changing the canonical authored trigger identity
or authoring a new feature. Add exact A/B tests for LIST, OPEN,
document viewer, notebook, recommendations, trigger/evidence/restoration,
V1/V2 save/reload, both routes and completion. Keep tests ID-agnostic across
future authored catalogue distributions.

## Platform-neutral date and seasonal matrix

Production calendar calls `f144PlatformCalendarDate` in portable
`src/f144_calendar.c`. The OS local date and `GetLocalTime` stay inside
the Win32 adapter. `-debug -date YYYY-MM-DD` or portable injected overrides
are honoured only with debug enabled. Invalid dates and absent providers
fail safely. The live calendar intentionally re-queries the current date;
a change across local midnight may change the next opened seasonal flyer
without altering progress.

These representative fixtures are exercised in
`tools/stage3b_cabinet_tests.c`, with CI success:

| Date | Expected eligible notice |
| --- | --- |
| 2026-01-15, winter ordinary | AMB-NB-08 (routine) |
| 2026-04-21, spring ordinary | AMB-NB-08 (routine) |
| 2026-07-15, summer | AMB-NB-04 |
| 2026-09-15, autumn ordinary | AMB-NB-08 (routine) |
| 2026-10-31, Halloween | AMB-NB-05 |
| 2026-11-05, Bonfire | AMB-NB-06 |
| 2026-12-20, Christmas | AMB-NB-07 |
| 2026-01-01 / 2026-12-31 | AMB-NB-01 (New Year) |
| 2026-02-14 / 2026-03-31 | AMB-NB-02 / AMB-NB-03 |
| Unrelated ordinary date, e.g. 2026-01-15 | AMB-NB-08 (control) |

All **nine** authored Staff Room noticeboard PIs remain canonical; three
original T-012-gated items retain their gates. Seasonal context is an
additional transient read-only entry: no permanent notice can be displaced,
no trigger/evidence/notebook state is awarded, and tests exercise detail
layout, scrolling, Backspace and framebuffer canaries. The Easter period
is a fixed authored spring window, not calculated Easter Sunday.

## Flavour, browsing and generated-document acceptance

- P-330 takeaway: three bounded lines; 792 business names and 95,040
  theoretical complete menus; known seed/repeat/small-buffer, large-seed
  sweep, original-PI identity and V2 reload checked.
- P-074 crossword: six five-letter crossings, independent fill masks,
  six annotations; T-012 gate, read-only inspection, save/reload and
  guarded 640x360 rendering checked. Not an interactive scored puzzle.
- P-073 paperback: five templates, 305 titles, 64 bylines, eight
  marginalia; **156,160** combinations exhaustively validated; longest
  line **48** characters against a 54-character limit; V2 regeneration
  and T-012 gate checked.
- Authored document identities: **170 canonical authored** before and
  after S4E-08, same as the Stage 3 baseline; canonical JSON remains
  the source of truth. S4E-08 restored missing generated registry entries
  for OS-18, rather than adding new authored prose.
- Catalogue rows: **518 -> 1,571**; procedural index-only records:
  **348 -> 1,401**, an addition of **1,053**. Collections remain **35**.
  All 1,571 player-facing IDs are unique in the archive audit; every
  authored document has a unique valid compiled slot. Generation uses
  36 neutral subject stems and 24 administrative forms, with numeric
  sort/cache and deterministic per-collection indexing.
- Procedural records deliberately have no authored body, evidence or
  trigger: they must not masquerade as required recovered clues.
  Stage 3 terminal/pager/scroll regressions and the Stage 4E archive
  audit passed on the examined exact-HEAD run.
- Four previously declared optional unbudgeted shells
  (HR-14, HR-27, HR-36, FM-32) are still empty; OS-41 has one authored
  item. S4E-09 **added no new collection** and its
  `stage4e_optional_early_collections_decision.md` reserves capacity.
  Consequently an added-versus-unrestored optional-collection route is
  **not applicable**, not an unperformed test.
- No direct Win32 or clock/global random dependency was identified in
  `floppy144_variation.c`, `floppy144_takeaway.c`,
  `floppy144_crossword.c`, `floppy144_paperback.c` or
  `floppy144_noticeboard.c`. Existing platform boundary and calendar
  tests pass. The dependency direction remains content -> portable
  deterministic/date services -> platform API -> Win32 adapter.

## Regression evidence and limitations

Exact-HEAD CI run 37767175122 succeeded at all configured steps:

- Stage 2; Stage 3A; Stage 3B.1–3B.5 including reconstruction, alternate
  branch trigger fixtures, terminal, physical-item and door tests.
- Stage 4B input, persistence/path, audio, timing/lifecycle, single
  instance, developer config, architecture and integration checks.
- Stage 4C Profile, operator name, terminal authentication, Settings,
  Credits, reinstatement, completion, integration audit and headless
  fresh-profile smoke.
- Stage 4D Help, camera, player, 2.5D inspection, consistency,
  integration/visual audit.
- Stage 4E service, calendar, noticeboard, takeaway, crossword, paperback
  and archive density/identity audit.
- Independent portable Core and Win32 builds, clean production-equivalent
  Release rebuild, hard size gate and 350,000-byte reserve gate.

**Not verified in this S4E-10 pass:** two distinct DR-04 seeded
permutations (and the required complete A/B story routes); two full
fresh-profile/alternate-route *interactive end-to-end* playthroughs with
mid-route save/reload; a standalone Stage 4E integration test proving all
replay surfaces simultaneously. The existing Stage 3B fixture routes and
Stage 4C smoke are genuine passing tests, but are not substitutes for
those outstanding criteria. Prior CI success cannot clear this gate.

## Production build and size

Latest examined production-equivalent Windows x64 Release, on exact
`ff2a02679`: MSBuild Core **0 warnings / 0 errors**; Platform
**0 / 0**; clean rebuilt application **0 / 0**.

| Metric | Bytes |
| --- | ---: |
| Competition ceiling | 1,474,560 |
| Stage 3 baseline Release | 659,968 |
| Stage 4D Release | 695,808 |
| Stage 4E Release | **709,120** |
| Stage 4E delta vs Stage 3 | **+49,152** |
| Stage 4E delta vs Stage 4D | **+13,312** |
| S4E-06 Release before archive expansion | 707,584 |
| Observed archive-expansion delta vs S4E-06 | **+1,536** |
| Remaining to absolute cap | **765,440** |
| Protected Stage 4F / final QA reserve | **350,000** |
| Headroom above protected reserve | **415,440** |

Both size gates **PASS**. This figure is the compiled
`Floppy144.exe` payload, not the much larger on-disk canonical JSON
or generated C source file lengths.

## Remaining sign-off actions

1. Find/reconcile the S4E-07 implementation in the active `Stage4`
   branch or correct the actual missing seeded DR-04 arrangement.
2. Implement A/B golden-vector assertions against **current authored
   identity and triggers**, without relying on S4E-08-invalidated
   `RS-0063` and `RS-0087` numbering.
3. Re-test LIST, viewer, notebook, recommendations, evidence,
   restoration, save/reload and completion through both permutations.
4. Execute/document a true seed-144 fresh-profile end-to-end playthrough
   and seed-145 alternate route, restoring from mid-run saves.
5. Rerun complete Windows CI and the strict 1,474,560-byte and
   350,000-byte reserve gates at the eventual sign-off commit.

**Decision: S4E-10 HOLD. Stage 4F is NOT authorised by this record.**
A successful inherited suite and comfortable payload are positive
evidence, but deterministic A/B workstream replay remains an unproven
mandatory acceptance criterion in the checked-in branch.
