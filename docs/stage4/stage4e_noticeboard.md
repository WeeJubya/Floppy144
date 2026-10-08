# S4E-03: Date-aware Staff Room Noticeboard

## Existing authored content: inventory

The canonical site has **one NOTICEBOARD fixture**:
`STAFF_ROOM_NOTICEBOARD`, in the Staff Room. It is rendered via the
existing Site 2D/2.5D fixture and inspected using the reusable Cabinet
Interior view, not the Document Viewer.

Nine permanent physical children remain authored in generated canonical
data and are **never replaced or hidden by seasonal selection**. Importantly,
the original progression gate remains: `P-075`, `P-078` and `P-138`
stay hidden until `T-012` reveals them. The contextual flyer does not depend
on, or bypass, that gate:

- `P-075` social poster (Friday tea);
- `P-078` birthday card;
- `P-138` final-week staff-social notice (marked `Dynamic worldbuilding`
  in its original ledger but retaining its existing fixed, authored PI text);
- `P-329` pinned memo and `P-330` takeaway menu;
- `P-331` staff photograph and `P-332` final-week calendar;
- `P-333` fire-drill card and `P-334` social notice.

The final-week calendar and other references remain immutable, including
the handover-related authored text. No narrative clue or object identity is
substituted. Other visually similar wall fixtures such as the Site Directory,
Suppression Panel and door notices are *not* noticeboards.

## Canonical contextual flyers

There were already **eight canonical AMBIENT entries** with type
`NOTICEBOARD_SEASONAL`, targeting `P-138`. They were previously dormant:
the data-only `Floppy144GameDataAmbientForDate()` function existed, but no
Cabinet/Noticeboard screen called it with the player's current calendar date.

S4E-03 exposes those eight existing full bulletin headlines using their
authored priority/date windows. No data compiler, JSON data, generated record,
save schema or existing physical item is modified.

| Authored ID | Eligible local dates | Flavour |
| --- | --- | --- |
| AMB-NB-01 | 27 December to 6 January | New Year form dating |
| AMB-NB-02 | 10 to 15 February | Valentine's anonymous filing |
| AMB-NB-03 | 20 March to 20 April | Easter chocolate amnesty |
| AMB-NB-04 | 1 June to 31 August | Summer staff picnic contingency |
| AMB-NB-05 | 25 October to 1 November | Halloween badge requirements |
| AMB-NB-06 | 2 to 7 November | Bonfire Night restrictions |
| AMB-NB-07 | 1 to 26 December | Christmas lunch options |
| AMB-NB-08 | All other dates | Routine staff social circular |

The Easter window is the existing **fixed spring approximation**, not a
computed Easter Sunday. Winter outside the New Year/Christmas windows and
autumn outside Halloween/Bonfire receive the normal default circular.

Eight additional short, optional *alternative marginalia* lines are defined
in `floppy144_noticeboard.c`, one per canonical flyer. For each canonical
flyer, S4E-01's stateless variation service chooses either the original
authored marginal annotation or its alternative. The bulletin headline
stays identical. Thus there are **eight existing canonical flyers with
two annotation variants each**, not sixteen new persistent objects.

## Selection and rendering

At successful parent inspection, the coordinator queries
`f144PlatformCalendarDate(&global_platform, &global_config, &today)`.
The portable Cabinet setter applies it only to the canonical fixture whose
authored variant is `NOTICEBOARD` and which owns the authored social notice
`P-138` in canonical data. That existing PI need not yet be revealed; its
visibility remains governed by `T-012` regardless of the seasonal flyer.

The calendar query is platform-neutral. Normal play uses the Win32 local
calendar; `-debug -date YYYY-MM-DD` and the test configuration force a
validated date. If the provider fails, the original nine notices still
appear unchanged, with no new context item.

The selected contextual flyer is a *transient*, synthetic
`Floppy144DataRecord` stored in `Floppy144CabinetState`. It is appended
after **all** existing physical children in their original seeded order,
and appears as `Seasonal staff circular` in the established scrolling
contents list. No original PI is removed or relabelled.

On inspection it uses a strictly **read-only** branch: the synthetic
`AMB-NB` identifier never enters the interaction engine or produces
triggers, notebook entries, evidence, unlocks or saved state changes.

The detail screen wraps the authored flyer headline and marginalia in two
distinct bounded areas inside the existing 500x224 dialog. Existing permanent
PI detail layout is untouched. The scrolling contents list already handles
more than ten entries; there is no fixed new list ceiling.

## Determinism and persistence

- Date window selection is canonical and based on month/day.
- Annotation variant is keyed by `recovery_seed`,
  `staff.noticeboard.annotation.v1` and canonical `AMB-NB` ID through
  `Floppy144VariationRange(..., 2U)`.
- Opening the board repeatedly in the same run/window gives the same choice.
- Save/reload retains `recovery_seed`; it does not persist duplicate
  contextual objects, preserving the prior save schema.
- Different local dates can legitimately change the flyer at the next open.
  Normal live calendar is not frozen for the lifetime of a run.

## Validation

The **real generated-data** Stage 3B.5 Cabinet regression adds explicit
forced-date cases for winter, New Year, Valentine, spring/Easter, summer,
ordinary autumn, Halloween, Bonfire, Christmas and year-end rollover.

It also proves the three T-012-gated noticeboard PIs remain hidden beforehand
and all nine authored PIs and original `P-138` content remain present
afterwards, seasonal content appends without reordering them, repeat
calls are deterministic, different seeds select different annotation text,
contextual inspection does not mutate `RunState`, back navigation works,
framebuffer guard pixels survive full detail rendering and unrelated furniture
receives no contextual notices.

`tools/test_stage4_noticeboard.ps1` audits wiring, canonical content
presence and Core/platform boundaries. The CI workflow retains all
Stage 2–4D suites, S4E-01/S4E-02, independent builds, clean Release and
the hard **1,474,560-byte** size gate.
