# Stage 4E integration, determinism and replay acceptance (S4E-10)

**Status: S4E-07 IMPLEMENTED / AUTOMATED PASS; S4E-10 MANUAL REPLAY SIGN-OFF STILL PENDING.**
**Scope:** `Stage4` at tested code commit `5368f2c8976631b89de8b74b2503ee48c1be21e7`.
**Rule:** No Stage 4F work or new feature work is accepted through this gate.

This record distinguishes **verified green regression/build evidence** from **unmet
S4E-10 acceptance criteria**. The updated suite does prove both DR-04
permutations, both initial branch choices and two seed-class full-site
reconstruction routes. It does not replace two manual interactive playthroughs.

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
- S4E-07 was omitted from the original S4E-06 to S4E-08 sequence. It was
  subsequently implemented on top of S4E-08's renumbered archive by
  `4f9b47aa5`; tests `85ed6b4e5`, and cross-suite/dual-seed
  reconstruction `5368f2c89`.
- Current production-equivalent Windows CI:
  [run 37769519595](https://github.com/WeeJubya/Floppy144/actions/runs/37769519595),
  job `113285337591`, **success** on `5368f2c89`.
  Stage 2, 3A, 3B, 4B, 4C, 4D, all 4E checks and clean Release passed.
- Implementation details, seed cases and safeguards:
  [stage4e_dr04_swap.md](stage4e_dr04_swap.md).
- This report records passing automated routes; no claim is made that a
  human-operated, uninterrupted full-game session has been completed.

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
| DR-04 at fixed RS-0216 | Technology (T-011) | Technology (T-011) |
| DR-04 at fixed RS-0147 | Records (T-010) | Records (T-010) |
| DR-04 swap class | Swapped | Swapped |

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

**S4E-07 correction and seeded arrangement:** The two workstreams are
located by immutable T-010 (Records) and T-011 (Technology) authored
identities. The post-S4E-08 *slot numbers* remain RS-0216 and RS-0147.
`Floppy144VariationRange(seed,"dr04.workstream-swap.v1","DR-04",2)`
decides which payload is displayed in each slot. The seed value is saved,
not regenerated after reinstatement. The terminal LIST, OPEN, viewer,
scrolling, post-open recommendation, trigger-access gate and effect
application now share this identity-based resolution.

Two **opposite** deterministic seed classes were needed: 144 and 145
happen to select the *same* swapped class, so the explicit A/B tests use
**146 (not swapped)** and **144 (swapped)**:

| Fixed record ID | Seed 146: no swap | Seed 144: swapped |
| --- | --- | --- |
| DR-04-RS-0216 | Records, T-010 | Technology, T-011 |
| DR-04-RS-0147 | Technology, T-011 | Records, T-010 |

The real Terminal and persisted RunState fixtures test both seed classes
with **both Records-first and Technology-first choices** (four paths),
LIST pages, OPEN, viewer resolution, branch gating, V2 save/reload and
terminal re-entry. The canonical room progression and Act II branch
fixtures additionally run under both permutation classes. No canonical JSON,
generated document output, trigger definition, evidence or notebook text
was changed. No existing player guidance hardcodes either swapped slot.

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

**Still requiring manual sign-off:** two uninterrupted, interactive
fresh-profile/alternate-route end-to-end playthroughs under opposite seed
classes, with in-game mid-route save/reload, visual notebook inspection
and human acceptance of the completed endings. The automated four-path
seeded OPEN regression, two-seed full-site reconstruction, all existing
Stage 3B ending fixtures and Stage 4C smoke now pass. Automated coverage
does not claim a complete human-operated playthrough.

## Production build and size

Latest completed production-equivalent Windows x64 Release, on exact
`5368f2c89`: MSBuild Core **0 warnings / 0 errors**; Platform
**0 / 0**; clean rebuilt application **0 / 0**.

| Metric | Bytes |
| --- | ---: |
| Competition ceiling | 1,474,560 |
| Stage 3 baseline Release | 659,968 |
| Stage 4D Release | 695,808 |
| Stage 4E Release with S4E-07 | **709,632** |
| Stage 4E delta vs Stage 3 | **+49,664** |
| Stage 4E delta vs Stage 4D | **+13,824** |
| S4E-07 delta vs pre-swap S4E-09 | **+512** |
| S4E-06 Release before archive expansion | 707,584 |
| Observed archive-expansion delta vs S4E-06 | **+1,536** |
| Remaining to absolute cap | **764,928** |
| Protected Stage 4F / final QA reserve | **350,000** |
| Headroom above protected reserve | **414,928** |

Both size gates **PASS**. This figure is the compiled
`Floppy144.exe` payload, not the much larger on-disk canonical JSON
or generated C source file lengths.

## Remaining sign-off actions

1. Perform and document an actual seed-146 fresh-profile run and seed-144
   alternate-route *interactive* playthrough, with mid-route save/reload.
2. Confirm visual notebook wording and dynamically generated terminal
   recommendations in both sessions, and reach both endings normally.
3. Confirm completion snapshots survive profile reinstatement and sign
   the S4E-10 human visual/replay gate.

**Decision: S4E-07 automated gate PASS; S4E-10 manual replay HOLD.**
The branch passes both compiled deterministic arrangements, existing
progression, clean Release, absolute size and protected reserve. Stage 4F
is not authorised by this report until the outstanding interactive acceptance
cases have been recorded.
