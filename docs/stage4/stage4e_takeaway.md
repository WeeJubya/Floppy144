# S4E-04: Deterministic Staff Room takeaway menu

## Existing canonical physical item

**P-330, "Takeaway menu"** is one of the nine permanent physical children of
`STAFF_ROOM_NOTICEBOARD`. Canonical description (unchanged in JSON and the
generated database): "Takeaway menu left with this noticeboard in Staff Room."

It has no authored interaction, trigger or evidence relation, no reveal gate,
and no game-progression text. It is not S4E-03's separate date-window
`NOTICEBOARD_SEASONAL` flyer. The board's existing P-330 list entry remains
named `Takeaway menu` in its original seeded position.

## Generative design

`floppy144_takeaway.c` adds five small, immutable phrase arrays:

| Fragment | Count |
| --- | ---: |
| Administrative modifier | 12 |
| Mascot/object noun | 11 |
| Business type | 6 |
| Takeaway special | 12 |
| Approval/complaints tagline | 10 |

There are **12 x 11 x 6 = 792 possible business names**, and
**792 x 12 x 10 = 95,040 complete three-line menu permutations**.
No long static full-menu blocks or memory allocations are required.

The three lines are:
1. `[Modifier] [Subject] [Business type]`;
2. `SPECIAL: [Dish]`;
3. `[Tagline]`.

Every part is selected by a separate S4E-01
`Floppy144VariationRange()` namespace ending in `.v1`, with item ID
`P-330` and the saved `Floppy144RunState.recovery_seed`.
No iteration-order PRNG, random clock calls, profile-derived seed, date,
Settings or new save fields are involved. A new run may select a different
business; repeat inspection and save/reload give byte-identical text.

Golden vectors:

- Seed 144:
  `THE PENDING HEDGEHOG PIE OFFICE`,
  `SPECIAL: CLASSIFIED BEEF IN A BOX`,
  `CLOSING TIME IS UNDER CONSULTATION`.
- Seed 145:
  `THE OFFICIAL SQUIRREL DUMPLING DEPOT`,
  `SPECIAL: STAPLED-TOGETHER NOODLES`,
  `NO REFUNDS AFTER FINAL SIGN-OFF`.

## Integration and layout

On successful Staff Room noticeboard `Floppy144CabinetOpenParent()`, the
existing `P-330` generated record is located by canonical ID and verified
against that exact parent and its existing visibility policy. The code copies
the immutable record into **transient Cabinet state**, substituting only
`pszF` with a bounded generated 192-byte text buffer. The existing list
ordering and `P-330` ID/name are preserved. No generated JSON/data,
trigger code, interaction logic or persistence codec changes are made.

`Floppy144CabinetVisibleContentAt()` returns this presentation-only copy
in the same original ordered slot. The original 640x360 Cabinet item-detail
renderer displays it using its existing maximum three lines, 54 characters
per line. Source fragments are deliberately bounded so none exceeds that
width. The generator returns false and clears the destination on an
undersized buffer rather than displaying clipped text. In that case
the original authored P-330 description is shown.

The generator is used only for this single specific physical item;
unrelated furniture and all seasonal notices are untouched. The transient
Cabinet state is rebuilt at every open, including after loading a save.
No profile fields or normal settings are added.

## Regression

- Pure C11, `/W4 /WX`: golden vectors for seeds 144 and 145, repetition,
  unrelated variation calls, >10,000-seed line-length/printability tests,
  business diversity, null/small-output safeguards and buffer canaries.
- Stage 3B.5 generated-data Cabinet test: canonical `P-330` identity,
  no gameplay interaction, stable list count and ordering, open/reopen,
  read-only Inspect, actual Version 2 run-state encode/decode and reopen,
  different seed, original canonical description untouched, framebuffer
  canaries and Backspace navigation.
- CI boundary audit: no use of Win32/C-runtime time or random functions;
  confirms source, runtime and regression wiring.

The fully inherited Stage 2-4D and Stage 4E regression/build gates remain
mandatory, as does the 1,474,560-byte executable hard size limit.
