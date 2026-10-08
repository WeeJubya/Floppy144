# S4E-10 — Stage 4E Integration, Determinism and Replay Gate

**Date:** 2026-10-08  
**Branch:** `Stage4`  
**Decision:** **AUTOMATED GATE PASS; FINAL INTERACTIVE REPLAY HOLD.**  
**Next phase:** Do **not** begin Stage 4F until the remaining human-observed fresh-profile and alternate-route playthroughs are signed off.

This is an integration/acceptance record, not permission to add content or features.
The established Stage 3 gameplay contract, canonical JSON, generated registries,
save schema, authored clues and Site geometry were not changed by S4E-10.
A clean CI run and two seed-class reconstruction fixtures are genuine evidence,
but are **not** two uninterrupted, manually observed game completions.

## 1. Provenance and exact tested build

- Immutable Stage 3 baseline: `fe2239123f98346ed06b48bb500672a5e6e2e172`
  (659,968-byte Release).
- Stage 4D sign-off: `1ce97d226` (695,808-byte Release).
- Stage 4E implementation begins: `a0991e76d` (portable keyed variation).
- S4E-02: `088108824` (calendar/fixed-date provider).
- S4E-03: `010f18b60`, `cfa4b4a70` (seasonal noticeboard and gate fix).
- S4E-04: `cc4cf2d58` (takeaway).
- S4E-05: `95ffa9445` (crossword).
- S4E-06: `170f4b43e` (paperback).
- S4E-08: `cfdc86d70` through `a411836d2` (expanded catalogue and fixes).
- S4E-09: `1df5e32e9`, `ff2a02679` (no added optional collections).
- S4E-07, implemented *after* S4E-08: `4f9b47aa5` (swap),
  `85ed6b4e5` (four A/B trigger routes),
  `5368f2c89` (linker fixtures, two-seed reconstruction).
- Previous audit baseline: `3dbff116f` (correctly held pending S4E-07).
- S4E-10 executable/regression implementation: **`16f2bd24bf8d153aa2011cf598e7940d64776992`**.
- **Exact tested Windows Actions run:** [37770899275](https://github.com/WeeJubya/Floppy144/actions/runs/37770899275),
  job `113289925028`, **SUCCESS** on that commit.

S4E-10 adds a standalone integrated seed-matrix executable,
`tools/stage4e_integration_tests.c`, an architecture/data/CI audit
`tools/test_stage4e_integration.ps1`, and a permanent CI step.
It also strengthens the inherited full-site reconstruction fixture to open
the actual *seed-selected* DR-04 document (via
`Floppy144DocumentApplyEffects`) for both T-010 and T-011 rather than
bypassing the swap with direct calls to `Floppy144TriggerTryFire`.
No shipping gameplay or data files changed in S4E-10.

## 2. Deterministic variation guarantee

`Floppy144VariationValue/Range/Chance` is pure, stateless unsigned
32-bit hashing, domain-separated by a versioned feature key and item ID.
There is no shared sequential PRNG stream and no call-order-dependent
consumption. `RunState.recovery_seed` is persisted in the existing V1
and V2 run-state codecs; the seed-derived presentation can be recreated
after terminal re-entry and file load without a new save field.
There is no direct Win32/clock/random API dependency in the variation,
noticeboard, takeaway, crossword, paperback, document, catalogue or
terminal Core modules.

**Limit of guarantee:** identical seed + feature namespace + immutable
content options + same build yields identical selection. A deliberate
future change to option arrays or hash namespaces need not preserve
bit-identical flavour across different game versions.

### Verified matrix (same seeds across all generators)

| Item | Seed A: **146** | Seed B: **144** |
| --- | --- | --- |
| Takeaway business, P-330 | THE OFFICIAL FERRET CHIP SHOP | THE PENDING HEDGEHOG PIE OFFICE |
| Crossword P-074 | Variant 3, QUEUE / SHELF | Variant 4, ORDER / INDEX |
| Paperback P-073 | A PAPERCLIP TOO CONFIDENTIAL (Crispin Bumblewick) | SENIOR LINE MANAGER (Crispin Quibble) |
| DR-04 permutation bit | 0, canonical | 1, swapped |
| Fixed DR-04-RS-0216 | Records, T-010 | Technology, T-011 |
| Fixed DR-04-RS-0147 | Technology, T-011 | Records, T-010 |

Both matrix outcomes are asserted by the new combined C executable.
It re-invokes all flavour generators after unrelated variation calls and
compares the full regenerated takeaway and paperback texts and crossword
grid, answers and marginalia byte-for-byte.
It also checks the opposite DR-04 permutation bits.
Output from the exact CI run:

```text
S4E-10 SEED 146: takeaway=FERRET CHIP SHOP crossword=QUEUE/SHELF paperback=TITLE: A PAPERCLIP TOO CONFIDENTIAL swap=0
S4E-10 SEED 144: takeaway=HEDGEHOG PIE OFFICE crossword=ORDER/INDEX paperback=TITLE: SENIOR LINE MANAGER swap=1
STAGE 4E INTEGRATION VECTORS: PASS
STAGE 4E INTEGRATION GATE: PASS
```

**Why not 144/145?** Those two seeds happen to choose the *same* DR-04
permutation, even though the flavour generators differ. Seeds **146 and
144** deliberately exercise both permutations. Pre-existing generator
golden tests for 144/145 also remain green.

## 3. Dates, seasons and temporary presentation

Date provider: portable `f144PlatformCalendarDate`, injected through the
F144 platform interface; Win32 `GetLocalTime` remains in the adapter.
`-date YYYY-MM-DD` is only honoured with the current developer/debug
configuration enabled. Invalid dates fail closed.

These representative dates are exercised through the actual Cabinet/notice
regressions (including permanent entries, access gates and layout/canaries):

| Date | Purpose | Expected contextual item |
| --- | --- | --- |
| 2026-01-15 | Ordinary winter | AMB-NB-08 |
| 2026-04-21 | Ordinary spring | AMB-NB-08 |
| 2026-07-15 | Summer | AMB-NB-04 |
| 2026-09-15 | Ordinary autumn | AMB-NB-08 |
| 2026-10-31 | Halloween | AMB-NB-05 |
| 2026-11-05 | Bonfire | AMB-NB-06 |
| 2026-12-20 | Christmas | AMB-NB-07 |
| 2026-01-01 / 2026-12-31 | New Year | AMB-NB-01 |
| 2026-02-14 / 2026-03-31 | Valentine / spring notice | AMB-NB-02 / AMB-NB-03 |

All nine canonical Staff Room noticeboard physical items remain
present and three original T-012-dependent items keep their gates.
The seasonal flyer is transient, read-only atmosphere and cannot
alter evidence, triggers, restoration, notebook or completion.
The spring notice is an authored fixed window, not a movable-Easter
date calculation. The real-world local date may legitimately change
seasonal atmosphere if a session crosses midnight; it does not reshuffle
run-seed choices or gameplay.

## 4. DR-04 placement, access and replay

The old `DR-04-RS-0063` / `RS-0087` figures predate S4E-08. The
post-expansion slots are permanently:

- **RS-0216:** original Records payload, *Physical Archive Reconciliation
  Workstream*, T-010.
- **RS-0147:** original Technology payload, *Terminal Network Remediation
  Workstream*, T-011.

The fixed **record numbers/IDs never swap**; the displayed authored
document title/body and its associated trigger move between those slots,
based on `Floppy144VariationRange(seed,
"dr04.workstream-swap.v1","DR-04",2)`.
Implementation traces a workstream by authored trigger identity, not
a hardcoded generated slot. Terminal LIST, OPEN, document viewer,
scrolling, debug recovery recommendations, access gates and effect
application use the consistent mapping.

**A/B integration results:**

- `tools/stage3b_terminal_tests.c` exercises seed 146 and 144, each
  selecting Records-first and Technology-first: **four routes**.
- Every case checks both fixed record numbers, seeded LIST content,
  OPEN lookup, viewer/authored trigger identity, branch commit,
  deferred alternate access, V2 save/reload, and terminal re-entry.
- `tools/stage3b_reconstruction_tests.c` now runs its canonical
  full-site reconstruction route under **both** seed classes,
  actually applying the seeded Records document to activate T-010
  and the seeded Technology document to activate T-011 after T-028
  releases the alternate workstream.
- Existing Stage 3B evidence, notebook, world/room, trigger release,
  capacity-exhaustion and completion regressions remain passing.
- No canonical notebook or authored content references the old slot
  figures. Generated player recommendations resolve the current slot
  for the authored trigger. No fixed-slot source assumption is left
  in the DR-04 runtime mapping.

These are **headless automated fixtures with some mid-Act-II progression
prerequisites represented by fixture state**. They are not a claim that
two human-operated, end-to-end fresh-profile completions have been observed.
The two seed-class room-reconstruction runs are both Records-first; the
Technology-first choice is covered as a shorter branch/reload fixture,
not as a full uninterrupted alternate-route playthrough.

## 5. Document and collection expansion

| Measure | Before S4E-08 | After S4E-08 / S4E-10 |
| --- | ---: | ---: |
| Collections | 35 | **35** |
| Canonical authored documents | 170 | **170** |
| Procedural index-only records | 348 | **1,401** |
| Total player-facing archive entries | 518 | **1,571** |

S4E-08 increased procedural catalogue density by **1,053 entries**
and preserved all 170 canonical authored documents. All 1,571 displayed
IDs are unique in the archive audit, compiled authored positions are
valid, and canonical registries regenerate cleanly from JSON. The new
word pools produce deterministic administrative entries, not fake
authorised clues. Procedural entries have no authored body, trigger,
or evidence effects. Stage 3 terminal LIST/page/document-scroll tests,
archive density/identity checks and generated-registry drift gate pass.
The compiled Release grew **1,536 bytes** from S4E-06 through S4E-08,
then **512 bytes** for S4E-07.

### Optional collections (S4E-09)

**NO ADDITIONAL COLLECTIONS — PASS.** The four previously declared
empty shells (HR-14, HR-27, HR-36, FM-32) are outside the stretch-task
decision. This stage did not introduce any new optional collection or
additional player capacity/progression pressure.
The [S4E-09 decision](stage4e_optional_early_collections_decision.md)
preserves a **350,000-byte minimum reserve** for Stage 4F and final QA.
A with/without-*new*-collection playthrough is not applicable.

## 6. Full regression, platform architecture and size

**Exact commit `16f2bd24b`, CI run 37770899275: SUCCESS.**

| Gate | Result |
| --- | --- |
| Stage 2 and Stage 3A regressions | PASS |
| Stage 3B terminal, cabinet, doors, reconstruction and progression | PASS |
| Stage 4B platform, input, persistence, audio, timing, configuration and integration | PASS |
| Stage 4C Profile, Settings, terminal, reinstatement, completion and integration smoke | PASS |
| Stage 4D Help, camera, player, 2.5D, consistency and integration | PASS |
| Stage 4E variation, calendar, notices, three flavour generators, archive | PASS |
| **New S4E-10 integrated seed matrix / boundary audit** | **PASS** |
| Portable Core Release, strict warnings | PASS, 0 warnings, 0 errors |
| Win32 platform Release, strict warnings | PASS, 0 warnings, 0 errors |
| Clean production-equivalent x64 Release rebuild | PASS, 0 warnings, 0 errors |
| Competition size and protected reserve | PASS |

Architecture remains content -> deterministic variation/date services ->
portable Core interfaces -> F144 platform abstraction -> Win32 adapter.
No Stage 4E flavour generator calls Win32 directly and no uncontrolled
`rand()`/`srand()`/`time()` is used to choose persisted flavour.
The new `test_stage4e_integration.ps1` CI audit enforces this boundary
and asserts the seed-aware document/terminal wiring, date override
guard, A/B fixtures, seasonal matrix and inherited regression coverage.

| Release size metric | Bytes |
| --- | ---: |
| Hard ceiling | **1,474,560** |
| Stage 3 immutable baseline | 659,968 |
| Stage 4D baseline | 695,808 |
| S4E-09 immediately before swap | 709,120 |
| **S4E-10 production Release** | **709,632** |
| Delta vs Stage 3 | **+49,664** |
| Delta vs Stage 4D | **+13,824** |
| Delta vs S4E-09 | **+512** |
| **Remaining headroom** | **764,928** |
| **Protected Stage 4F / QA reserve** | **350,000** |
| Uncommitted headroom beyond reserve | **414,928** |

This is the actual Release `Floppy144.exe` executable length, not source
or generated registry text size. No new shipping code was added by S4E-10,
so its measured payload equals the S4E-07 production build.

## 7. Limitations and final readiness

**Automated sign-off: PASS.** Seeded flavour is repeatable, DR-04
position swapping survives V2 save/reload, both workstream triggers
continue to drive the intended reconstruction, archive expansion retains
its authored clues, and the final production executable remains comfortably
under the competition limit and preserved reserve.

**Manual interactive replay: HOLD.** Remaining acceptance evidence:

1. Start a **new profile with developer seed 146**, play from the
   Prologue through the complete Records-first route, inspect both DR-04
   records and notebook hints, save/reinstate midway, and capture the ending.
2. Start a **fresh/second profile with seed 144**, follow the
   Technology-first alternate route to its ending, inspecting the
   swapped titles/body, recommendations, notebook, evidence and restoration.
   Save/reinstate at least once and check the swapped slots remain fixed.
3. Verify both completion screens, recovery totals and persisted
   profile-completion snapshots, and record screenshot/manual sign-off.

The headless full-site and branching tests are not substitutes for
real user-facing visual/interactive end-to-end playthroughs. Until those
three observations are recorded, **S4E-10 is not finally signed off and
Stage 4F remains on hold.** This is an acceptance limitation rather than
an identified integration defect. No new features or Stage 4F code were
introduced.
