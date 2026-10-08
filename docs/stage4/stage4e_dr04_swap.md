# S4E-07: Seeded DR-04 authored workstream placement

**Implementation:** PASS (automated). **Branch:** `Stage4`.
**Windows CI:** [37769519595](https://github.com/WeeJubya/Floppy144/actions/runs/37769519595), success at `5368f2c89`.

## Why the historic IDs changed

The deferred story specified `DR-04-RS-0063` / `RS-0087`, which were
Stage 3 player-facing record numbers. S4E-08 redistributed authored documents
and expanded DR-04 to 50 catalogue rows. The canonical generated pair now is:

- **Records (Document A):** Physical Archive Reconciliation Workstream,
  trigger T-010, fixed base record `DR-04-RS-0216` (index 21).
- **Technology (Document B):** Terminal Network Remediation Workstream,
  trigger T-011, fixed base record `DR-04-RS-0147` (index 14).

Restoring the obsolete RS-0063/0087 numbers would break the completed
S4E-08 archive. Instead the existing record IDs remain fixed and *authored
payloads* swap according to the saved recovery seed.

## Seed choice and exact mappings

Pure portable Core `Floppy144VariationRange(
seed, "dr04.workstream-swap.v1", "DR-04", 2U)` selects 0 = canonical
or 1 = swapped. This is stateless, feature-keyed, independent of traversal
order and identical after save/reload. Seed 146 selects 0; seed 144 selects 1.
Seeds 144 and 145 *both* select 1, so they are not suitable opposing fixtures.

| Player-facing record | Seed 146 (Arrangement A) | Seed 144 (Arrangement B) |
| --- | --- | --- |
| DR-04-RS-0216 | Records, T-010, original Records body | Technology, T-011, original Technology body |
| DR-04-RS-0147 | Technology, T-011, original Technology body | Records, T-010, original Records body |

No authored JSON, generated registry, record numbering, persisted format or
collection storage cost was changed.

## Core integration and dependency safety

`floppy144_document.c` resolves partners by collection plus **trigger
identity** (T-010 or T-011), not hardcoded document indexes or RS numbers.
`Floppy144DocumentGetForSeed` exposes a borrowed immutable authored
definition for a fixed slot. `Floppy144DocumentSlotForSeed` and
`Floppy144DocumentRecordIdForSeed` map authored identity back to its current
slot for player-facing recommendations. Normal `Floppy144DocumentGet`
continues to expose the original compiled canonical registry for unseeded
build-time and data audits.

The same mapping is consumed by:
- Terminal LIST paging, post-open guidance and NEXT recovery actions;
- OPEN ID lookup (slot stays fixed), preview title and branch access gate;
- document viewer, wrapped-body scrolling and catalogue-row presentation;
- authored trigger application, evidence and restoration effects.

The terminal and catalogue keep only a transient copy of the run seed;
`Floppy144RunState.recovery_seed` remains the single persistent source.
Real V1/V2 save codecs already encode it. Reload reconstructs mapping
without storing a second random choice. Legacy zero seed follows canonical
arrangement. No Win32 calls or sequential PRNG are introduced in Core.

The canonical data, compiled notebook definitions and player-facing terminal
messages were audited for the old RS-0063/0087 and current fixed RS numbers:
no fixed-slot player clue required changing. The neutral DR-04 workstream
briefing now recommends whichever IDs hold T-010/T-011 on that seed.

## Automated acceptance

`tools/stage3b_terminal_tests.c` exercises seeds **146 and 144** across
**both initial choices each** (four cases). Assertions cover:
fixed record IDs, seed-selected title/body trigger identity, LIST pages 2/3,
OPEN routing, document view, exact T-010/T-011 branch commit, deferred
alternate workstream, actual V2 save/reload and re-entry by record ID.
The prior branch-choice/regression fixture was updated to expect
seed-aware guidance, not the static source identity.

`tools/stage3b_reconstruction_tests.c` now runs the canonical complete
Site reconstruction route for each seed class, additionally running the
Act II branch choice fixture under seed 146. Existing Stage 3B suites
exercise later branch release, evidence and automatic completion logic.
Tests are combined coverage, not a claim of two uninterrupted interactive
human playthroughs.

All direct-document test executables now link `floppy144_variation.c`.
The full Windows CI at `5368f2c89` passed Stage 2, 3A/3B, 4B/4C/4D,
4E flavour/date/archive suites and clean production builds.

## Payload

| Measurement | Bytes |
| --- | ---: |
| S4E-09 Release before swap | 709,120 |
| S4E-07 Release after swap | 709,632 |
| Incremental cost | **512** |
| Limit | 1,474,560 |
| Remaining | **764,928** |
| Protected Stage 4F / final QA reserve | **350,000** |

Independent portable Core build: **0 warnings, 0 errors**.
Win32 platform build: **0 warnings, 0 errors**.
Clean Release build: **0 warnings, 0 errors**.
Both absolute competition size and reserve gates: **PASS**.

Implementation commits:
`4f9b47aa5` (Core), `85ed6b4e5` (A/B terminal tests), and
`5368f2c89` (cross-suite linker wiring and two-seed reconstruction).

**Remaining manual qualification:** Two complete interactive A/B sessions
with real mid-route save/reload, visual notebook inspection and ending
screens are still required for the separate S4E-10 replay sign-off.
