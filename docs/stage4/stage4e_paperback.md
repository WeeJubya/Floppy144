# S4E-06: Dog-eared paperback title and byline

## Canonical item and gameplay boundaries

`P-073`, **Dog-eared paperback**, in Staff Room
`STAFF_ROOM_BOOKCASE_02`. Its authored physical-item description is
`Dog-eared paperback. Recovered in Staff Room.`

The canonical `T-012` effect reveals `P-073`, and `HR-05` has an authored
`restores_or_contextualises_item` relationship. These gameplay boundaries
remain fully intact. The permanent physical record, item ID, visible list
heading `Dog-eared paperback`, parent, order and original generated data are
unchanged.

The paperback is flavour and is not a puzzle, evidence source, new
interactive document, clue, notebook progress, access condition or
completion input.

## Compact generator

Pure C99/C11 `game/src/floppy144_paperback.c/.h`, driven only by the
persisted `Floppy144RunState.recovery_seed` and independently namespaced
`Floppy144VariationRange()` calls keyed by `P-073`:

| Form | Example structure | Combination count |
| --- | --- | ---: |
| 0 | THE [Object] OF [Location] | 8 x 8 = 64 |
| 1 | [Quality] [Profession] | 8 x 8 = 64 |
| 2 | A [Object] TOO [Quality] | 8 x 8 = 64 |
| 3 | THE [Ordinal] [Department] | 7 x 7 = 49 |
| 4 | [Verb] ME BEFORE [Event] | 8 x 8 = 64 |

305 possible titles. Two 8-word fictional-author pools supply 64
bylines; an 8-line marginalia pool adds damaged-book workplace observations.
There are **305 x 64 x 8 = 156,160 possible full covers**. Source contains **102 short reusable fragments**: 78 title-pool
fragments, 16 fictional-author fragments and 8 marginal annotations. No
whole-book assets or long pre-written descriptions are stored.

The body consists of exactly three explicit lines:

1. `TITLE: [fictional paperback title]`
2. `BY: [fictional given name] [fictional family name]`
3. `[note written on or in the book]`

Golden seed examples:

- 144: `SENIOR LINE MANAGER` by `CRISPIN QUIBBLE`, ending with
  `THE LAST PAGE IS A LEAVE REQUEST`.
- 145: `HEARTBROKEN ARCHIVE CLERK` by `MILLICENT MUDDLE`, same
  note coincidentally (selection remains independently keyed).
- 1: `ARCHIVE ME BEFORE THE REVIEW PANEL` by `NORA QUIBBLE`.

## Display and hard limits

The existing 640x360 Cabinet Interior detail overlay is 500x224 pixels.
The title appears **inside the item description** with the author and
note, not as a replacement for the canonical PI/list heading. The normal
`Floppy144CabinetDrawWrappedText` already handles newline-separated
three-line passages at up to **54 6-pixel ASCII glyphs per line**.

`Floppy144PaperbackCompose()` checks all indices, the title staging
buffer, formatted output capacity, exactly three non-empty lines and each
line's 54-character/ASCII constraint. On failure it clears the destination
rather than showing clipped text. `P-073` then falls back to its existing
authored description.

The runner exhaustively tests all **156,160** source combinations, including
maximal-length fragments; the maximum line is **48 characters**, leaving
six spare glyphs. `/W4 /WX` protects against compiler warnings.

## Integration and persistence

On successful `Floppy144CabinetOpenParent()` for the **exact** Staff Room
Bookcase 02 ID, the code finds canonical `P-073` and verifies the existing
`Floppy144SitePhysicalItemVisible()` rule. Only then does it create a
shallow copy of its canonical `Floppy144DataRecord` in *transient Cabinet
state*, replacing only `pszF` with a bounded 192-byte generated cover.
The existing `VisibleContentAt()` seeded list ordering is preserved; the
physical-item metadata and gameplay interaction routing are unchanged.

No extra save/profile field is stored: save/reload preserves the run seed,
and the cover is regenerated each time the Bookcase is opened. No runtime
clock, sequential PRNG, settings or seasonal date affects the title.

## Acceptance tests

- Fixed seeds 144, 145; deterministic cross-feature isolation.
- Every possible title/author/note combination, clean spacing, punctuation,
  line width, printable bitmap-font characters and 192-byte buffer bounds.
- 10,000-seed sweep; zero-length, undersized, null and invalid inputs.
- Real Stage 3B.5 fixture: physical item remains hidden before T-012,
  revealed only afterward, inspect/read-only behaviour, stable ordering,
  repeated opening, actual V2 encode/decode and reopen, different seed,
  Backspace and frame-buffer bounds.
- Full inherited Stage 2–4D and S4E-01–05 regression suites, strict builds,
  and the 1,474,560-byte decompressed executable cap.
