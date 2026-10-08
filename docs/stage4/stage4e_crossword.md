# S4E-05: Half-finished Staff Room crossword

## Canonical data and existing gameplay boundary

The item is `P-074`, **Half-finished crossword**, parent
`STAFF_ROOM_COFFEE_TABLE_01`, with original generated description
"Half-finished crossword. Recovered in Staff Room."

The Stage 3 canonical data also carries `AMB-STAFF-XWORD-A`, a separate
flavour inspection annotation, and an `HR-05` contextual relationship.
`T-012` **reveals P-074**. The reveal, relationship, physical ID, list
ordering and original authored source text are untouched.

P-137 is a different, completed crossword on Coffee Table 02 and is
not affected. No new player input or story progression is introduced.

## Implementation and vocabulary

Six safe, hand-authored five-letter Across/Down pairs share a letter in the
middle of a tiny five-by-five grid, rather than relying on runtime puzzle
placement or introducing a document-pager workflow:

| Across | Down | Clue concepts |
| --- | --- | --- |
| FORMS | CARDS | Mandatory paperwork; boxed index slips |
| STAMP | DRAFT | Ink approval; provisional writing |
| DESKS | ASSET | Office furniture; inventory labels |
| QUEUE | SHELF | Lines requiring forms; storage |
| ORDER | INDEX | Countersigned instructions; alphabets |
| PAPER | TYPOS | Administrative media; printing errors |

The grid uses ASCII only. Unoccupied cells are shaded, entered cells
contain letters, and unfinished squares show underscores. Four compatible
fill masks apply to each direction, with at least two unsolved squares
per answer and the shared crossing filled. Six short pencil annotations
provide administrative grumbling rather than any story clues.

All parts are chosen with independent `Floppy144VariationRange()` namespaces
and canonical item key `P-074` using the existing
`Floppy144RunState.recovery_seed`. The effective selection space is
**6 crossings x 4 Across masks x 4 Down masks x 6 annotations = 576**
potential combinations. No global PRNG, date, clock or new persistence field.

## Display

The existing **640x360 Cabinet Interior physical-item detail panel** uses a
500x224 overlay. S4E-05 reserves an inspection-only branch for P-074:
five 17x15 cell boxes in a 5x5 grid at x98..195, y135..222, two concise
clues at x230 and pencil annotation beneath them. The permanent physical
item heading and Backspace navigation remain unchanged.

This is not a formal crossword challenge: there is no cursor, typed
answer, scoring, notebook entry, unlock, trigger or completion effect.
No new scrolling state is needed because all content fits within the
existing detail panel. The filled/empty grid is deliberately a scribbled
snapshot, not something a player edits or solves in-game.

A shallow immutable `Floppy144DataRecord` copy in transient Cabinet state
preserves the P-074 ID, parent, title and authored fields. Returning this
copy from `Floppy144CabinetVisibleContentAt()` at P-074's existing seeded
list position lets the existing inspection UI recognise it without touching
the saved run or generated game data. T-012 visibility is checked *before*
the variant is created, and reset/reload simply regenerates it by seed.

## Regression requirements

- Strict C11 `/W4 /WX` test of seeds 144 and 145, exact clue/answer
  pairings, intersecting letter consistency, partial-fill completeness,
  ASCII-safe glyphs, every variant and 10,000 run seeds.
- Real Stage 3B.5 Cabinet tests for gated P-074 before/after T-012,
  canonical identity and HR-05 relation, repeated inspection, no state
  mutation, V2 save encode/decode and regeneration, different seeds,
  Backspace navigation and guarded 640x360 framebuffer rendering.
- CI source audit confirms no time, Win32 date, global random, changed
  generated dataset or shortcut around the original reveal gate.
- All inherited Stage 2–4D and S4E-01–04 regression/build/size gates
  remain mandatory, including the 1,474,560-byte executable size limit.
