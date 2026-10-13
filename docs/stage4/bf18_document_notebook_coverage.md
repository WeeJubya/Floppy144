# BF18: Authored document to Notebook coverage

## Catalogue coverage

Source: `data/floppy144_game_data.json` at the BF18 implementation.
All summaries were written following review of the corresponding full authored
document bodies, rather than generated from titles. Every summary is exactly
one sentence prefixed at runtime by the canonical `<document ID>:` identifier.

**170 authored documents, 170 unique summaries, 31 populated collections,
4 collections without authored documents.** The former 157-document baseline
predates Stage 4 archive expansion.

| Collection | Authored bodies | Notebook summaries |
|---|---:|---:|
| DR-01 | 4 | 4 |
| DR-02 | 4 | 4 |
| DR-03 | 4 | 4 |
| DR-04 | 6 | 6 |
| DR-07 | 6 | 6 |
| DR-23 | 4 | 4 |
| DR-29 | 6 | 6 |
| DR-31 | 6 | 6 |
| DR-47 | 6 | 6 |
| DR-53 | 6 | 6 |
| HR-01 | 6 | 6 |
| HR-05 | 5 | 5 |
| HR-09 | 6 | 6 |
| HR-14 | 0 | 0 |
| HR-27 | 0 | 0 |
| HR-36 | 0 | 0 |
| FM-04 | 6 | 6 |
| FM-07 | 6 | 6 |
| FM-13 | 6 | 6 |
| FM-18 | 6 | 6 |
| FM-23 | 6 | 6 |
| FM-32 | 0 | 0 |
| OS-07 | 6 | 6 |
| OS-18 | 4 | 4 |
| OS-26 | 7 | 7 |
| OS-34 | 5 | 5 |
| OS-41 | 1 | 1 |
| OS-48 | 6 | 6 |
| TS-02 | 6 | 6 |
| TS-06 | 6 | 6 |
| TS-08 | 4 | 4 |
| TS-10 | 6 | 6 |
| TS-14 | 6 | 6 |
| TS-18 | 8 | 8 |
| TS-27 | 6 | 6 |
| **Total** | **170** | **170** |

## Semantics

- An OPENED bit is written **only when the viewer successfully opens** a record,
  whether reached via terminal OPEN or the catalogue list.
- The same per-record opened bitset covers authored and procedurally generated
  records for future BF34 status reporting; generated records do not invent an
  authored Notebook summary.
- The first authored OPEN records one matching Notebook summary, identified by
  the actual seed-presented document ID; repeated OPEN does not duplicate it.
- New, unread and legacy-loaded documents remain hidden in the Notebook.
- Existing factual, interaction and evidence Notebook entries and their legacy
  persisted 128-entry chronology are preserved. Document notes appear after
  those entries, in their own first-view order.
- Save V5 appends opened-state and document-note sequence data; V1–V4 decode
  without inventing a prior viewing history.

## Acceptance

- Test `tools/test_bugfix18_notebook.ps1`: 170 readable bodies, unique IDs,
  unique substantive sentences, matching compiled Notebook rows.
- Native persistence regression: all authored notes, first open and duplicate
  suppression, generated record status without a false summary, V4 decode,
  V5 save/reload, original Notebook history.
- Full Stage 3A, 3B, Stage 4 persistence and Release/size gates.
- Manual Windows acceptance: read records through terminal and catalogue,
  check Notebook scrolling and save/reinstate; automated tests do not establish
  visual sign-off.
