# S4E-08: Archive document variety and collection density

## Baseline and size policy

Pre-change verified S4E-06 Windows Release: **707,584 bytes**, leaving
**766,976 bytes** of the **1,474,560-byte** decompressed limit.

At this point there are **35 collections, 30 budgeted collections, 170
canonical authored documents and 348 procedural/index-only documents** across
**518 total catalogue entries**. Four optional unbudgeted collections remain
empty. OS-41 is an unbudgeted holding containing one authored document.
A stale checked-in document registry had only two of OS-18's four canonical
authored documents; regeneration from canonical JSON produces all **170**.
This is a generated-source synchronisation correction, *not* newly written
authored fiction.

The original inception targets were **Prologue 15, Act I 25, Act II 50,
Act III 100 records per collection**. The implemented targets restore the
first three eras in full while using **75** for Act III. This deliberately
makes later search substantial (7.5 pages per collection) without multiplying
the seven or eight authored late-game records into ten pages of nearly all
missing-body stubs.

Totals after: **1,571 catalogue entries, 170 authored, 1,401 procedural
index-only records**, across the same 35 collection identities. This is
**+1,053 generated records**. No collection sizes in KB, restoration costs,
capacity logic, evidence, triggers, interactions or required paths change.

## Per-collection source-of-truth comparison

The baseline totals come from the original Stage4 canonical collection
`content_budget.records` (clamped to authored count as the compiler does).
The after values come from the S4E-08 canonical JSON budgets.

| Collection | Act | Before | After | Authored | Generated before | Generated after |
|---|---|---:|---:|---:|---:|---:|
| DR-01 | Prologue | 8 | 15 | 4 | 4 | 11 |
| DR-02 | Prologue | 6 | 15 | 4 | 2 | 11 |
| DR-03 | Prologue | 6 | 15 | 4 | 2 | 11 |
| DR-04 | Act II | 18 | 50 | 6 | 12 | 44 |
| DR-07 | Act II | 24 | 50 | 6 | 18 | 44 |
| DR-23 | Act II | 12 | 50 | 4 | 8 | 46 |
| DR-29 | Act II | 20 | 50 | 6 | 14 | 44 |
| DR-31 | Act II | 18 | 50 | 6 | 12 | 44 |
| DR-47 | Act III | 20 | 75 | 6 | 14 | 69 |
| DR-53 | Act III | 18 | 75 | 6 | 12 | 69 |
| HR-01 | Act I | 18 | 25 | 6 | 12 | 19 |
| HR-05 | Act II | 16 | 50 | 5 | 11 | 45 |
| HR-09 | Act III | 18 | 75 | 6 | 12 | 69 |
| HR-14 | Unbudgeted | 0 | 0 | 0 | 0 | 0 |
| HR-27 | Unbudgeted | 0 | 0 | 0 | 0 | 0 |
| HR-36 | Unbudgeted | 0 | 0 | 0 | 0 | 0 |
| FM-04 | Act I | 25 | 25 | 6 | 19 | 19 |
| FM-07 | Act I | 20 | 25 | 6 | 14 | 19 |
| FM-13 | Act I | 22 | 25 | 6 | 16 | 19 |
| FM-18 | Act III | 18 | 75 | 6 | 12 | 69 |
| FM-23 | Act II | 20 | 50 | 6 | 14 | 44 |
| FM-32 | Unbudgeted | 0 | 0 | 0 | 0 | 0 |
| OS-07 | Act III | 20 | 75 | 6 | 14 | 69 |
| OS-18 | Act III | 10 | 75 | 4 | 6 | 71 |
| OS-26 | Act III | 22 | 75 | 7 | 15 | 68 |
| OS-34 | Act III | 16 | 75 | 5 | 11 | 70 |
| OS-41 | Unbudgeted | 1 | 1 | 1 | 0 | 0 |
| OS-48 | Act III | 18 | 75 | 6 | 12 | 69 |
| TS-02 | Act II | 20 | 50 | 6 | 14 | 44 |
| TS-06 | Act II | 22 | 50 | 6 | 16 | 44 |
| TS-08 | Act II | 12 | 50 | 4 | 8 | 46 |
| TS-10 | Act II | 20 | 50 | 6 | 14 | 44 |
| TS-14 | Act III | 20 | 75 | 6 | 14 | 69 |
| TS-18 | Act III | 24 | 75 | 8 | 16 | 67 |
| TS-27 | Act II | 6 | 50 | 6 | 0 | 44 |

## Generator and browsing design

- **Before:** 12 neutral subject stems x 10 document forms = 120 potential
  subject/form combinations. Subject and form choice followed visible
  modulo rhythms, so filler titles repeated frequently.
- **After:** **36 neutral subject stems x 24 administrative forms = 864
  subject/form combinations**. Reusable fragments include facilities,
  stationery, postage, attendance, desk circulars, filing returns,
  distribution copies, routing slips and service dockets.
- A collection-specific deterministic offset and index mixer break the
  obvious adjacent-word sequence. The existing irregular, **numerically
  sorted** record-number generator remains authoritative for all new
  filler records; record IDs are checked for global uniqueness by CI.
- The catalogue-number sort is cached **once per collection** in a compact
  transient lookup, rather than rescanning all candidate ranks on each
  terminal LIST/OPEN, which would be expensive at 75 records per collection.
- Generated entries remain intentionally **index-only**, explicitly stating
  that the record body is missing. No procedural filler creates misleading
  invented evidence, new searchable clues, recoverable witness statements,
  notebook facts or clickable progression opportunities.

## Authored redistribution and compatibility

The canonical `data/floppy144_game_data.json` authored documents, titles,
body texts, source order and stable canonical IDs remain unchanged.
The data compiler now assigns non-pinned authored records using a
**collection-offset coprime permutation of evenly spaced catalogue bands**.
This changes their visible order while spreading them throughout the larger
archive, and keeps their original source order for first-eligible trigger
choices and recovery guidance.

Three documented fixed landmarks retain their exact player-facing IDs:

- `DR-01-RS-0001`: bootstrap entry remains slot 0.
- `FM-13-RS-0047`: suppression service note remains slot 3.
- `HR-01-RS-0107`: Main Office desk allocation stays in its preserved slot.

Other player-facing authored IDs are regenerated consistently from the new
catalogue numbering and documented regression expectations are updated.
These player-facing references are *not persistence identifiers*: save data
keeps its existing recovery seed, flags and restored-collection IDs, and the
runtime builds LIST, OPEN and Notebook references against the same regenerated
registry. The compiler never renumbers JSON document identity, trigger IDs,
evidence IDs, notebook IDs or authored source order.

Same version + same saved game -> identical collection contents, ordering,
record IDs and titles. Changes between builds legitimately change index-only
titles/record numbers; the saved game does not persist individual list-row
indices or synthetic document bodies.

## Validation

The `tools/test_stage4_archive.ps1` CI gate audits every collection's
record budget, authored mapping, spread and inversion, generated/record-ID
uniqueness including all filler, expected contents and safe vocabulary.
Existing Stage3B terminal regressions exercise every record-ID resolution,
LIST pager, document scrolling, preserved access paths, notebook/evidence,
and completion. The complete Stage 2, 3A/3B and Stage 4B–4E suite plus
Windows Core, platform and clean Release builds remain mandatory.

A deliberate **350,000-byte executable headroom reserve** is enforced as a
separate Windows CI gate, in addition to the competition hard limit. This
reserve is for Stage 4F presentation/icon/intro integration, QA, compiler
variation and emergency fixes. The final incremental byte cost is reported
from the Windows Release build rather than inferred from generated text size.
