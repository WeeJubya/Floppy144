# BF29 / BF31 / BF32 / BF34: document catalogue and difficulty

- Canonical source: 170 authored bodies (and summaries), plus 1,401 generated catalogue entries in 35 collection definitions. No authored document ID, title, summary, cost or dependency is changed.
- BF29: generated subject plus form is checked word-by-word, case-insensitively; punctuation separates words. If a word repeats, search the existing 24-form vocabulary in deterministic cyclic order. A bounded unique fallback guarantees termination. Authored titles are untouched and IDs remain stable.
- Difficulty labels: NORMAL, EASY, REALLY SUPER EASY (I MEAN REALLY STUPIDLY EASY). The latter is rendered in full at scale 1 on its own new-session selector screen. New-session selection initially highlights EASY; legacy saves retain NORMAL.
- RunState difficulty is a persisted byte. Save V6 appends one byte to the V5 layout, preserving all earlier offsets and Notebook chronology. V1 to V5 load as NORMAL. Invalid V6 mode values fail decoding. The existing command-line debug option is independently ORed with the easiest selection.
- Size display is synthetic metadata only, never an actual byte-size measurement of document prose. A collection of `size_kb` is `size_kb * 1024` bytes, matching existing capacity units. EASY uses integer weights 13 for authored, 10 for generated. Easiest uses weights 0 for authored and 10 for generated. For each row, the allocation is `floor(total_bytes * cumulative_weight / total_weight) - floor(total_bytes * preceding_weight / total_weight)`, evaluated with 64-bit multiplication. These allocations telescope to exact collection totals and are stable without storing per-record bytes.
- NORMAL hides SIZE values; EASY shows all; easiest shows generated only. The OPENED column always uses existing RunState bits and is refreshed when the catalogue is drawn. List and viewer use the same public allocation function.
- Existing BF18 persistence and Notebook ordering remain untouched.

## Existing data edge cases

HR-14, HR-27, HR-36 and FM-32 have zero catalogue records. OS-41 has one authored document and zero generated records. It is mathematically impossible to distribute each of their nonzero collection totals across generated-only entries in easiest mode. This patch does **not** invent generated document identities or change authored records to hide that data conflict. Their easiest-mode SIZE values are blank; all collections with at least one generated row allocate exactly. A product/content decision is required if exact generated-only display totals must apply to the five exceptions as well.

## Verification

The existing Stage 4 persistence regression suite now covers all 1,571 entries and four presentation seeds, title-word uniqueness, size visibility, collection sums, authored weighting, V5 to NORMAL migration and V6 round-trips. `test_documents_difficulty.ps1` checks that the UI, debug and persistence wiring remain present. The Stage 4 release-candidate workflow runs both suites, Stage 3 regressions, Release build and executable-size gate. Manual Windows acceptance is still required for keyboard navigation, full-width names and pixel positioning.
