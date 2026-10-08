# S4E-09: Optional early collection capacity decision

**Decision: S4E-09: NO ADDITIONAL COLLECTIONS. Outcome B: PASS.**

## Capacity and reserve

The last completed S4E-08 Windows CI, run 37760614118, reports a
Release executable of **709,120 bytes**, with **765,440 bytes** spare
under the 1,474,560-byte ceiling. The project already enforces a
**350,000-byte safety reserve** for Stage 4F icon/intro integration,
remaining content, QA, compiler/linker variation and emergency fixes.
Only 415,440 bytes are beyond that minimum reserve. Capacity alone
does not justify a new collection.

## Existing early-game coverage

The mandatory DR-01 prologue already enables DR-02 (Terminal Operations
Guide, 14 KB, 4 authored documents), DR-03 (Site Reconstruction Guide,
14 KB, 4 authored) and HR-01 (Site Establishment, 101 KB, 6 authored)
through T-001. Act I also provides FM-04 (required), FM-07 and FM-13
(optional), each with six authored documents. The recently expanded
archive now has 35 collections, 1,571 browsable entries, 170 authored
and 1,401 generated entries. Early exploration already has meaningful
optional choices and a substantially deeper archive.

Four declared collection shells remain unfilled: HR-14, HR-27, HR-36
and FM-32. Their existing presence and separate authored-content plan
are a better place to invest writing/QA effort than another collection ID.

## Candidates considered, not implemented

| Concept | Prospective source/unlock | Authored content and filler | Why not |
|---|---|---|---|
| DR-05, Reception Return Slips | Reception terminal after T-001 | 3-4 memos about misrouted mail and claims; 15-25 index records | Repeats DR-02/03; overcrowds tutorial choices |
| HR-02, Provisional Induction Pack | Main Office terminal after HR-01 | 3-4 notices about orientation and temporary staff; 25 index records | Overlaps HR-01 and undeveloped HR-14 |
| FM-02, Stationery Requisition Appeals | Facilities terminal after access | 3-4 notes on filing supplies, tea and stock counts; 25 index records | Overlaps FM-04/FM-07 and adds early terminal complexity |

Each sketch would likely cost a few KB of executable and an additional
in-game collection recovery cost, but none was authored or compiled,
so these are not measured cost estimates. No candidate offers a clear
new gameplay experience despite the potential for entertaining prose.

## Gameplay and compatibility

Adding an optional collection still changes the collection ledger and
availability model, recovered-percentage denominator, capacity spent,
possible capacity-exhaustion completion timing and save/profile checks.
New names and authored text would also require terminal, LIST, paging,
save/reload, notebook, branch and optional non-restoration regression.
No such changes are justified by the available concepts.

## Verification and sign-off

Baseline S4E-08 CI: https://github.com/WeeJubya/Floppy144/actions/runs/37760614118

That workflow passed Stage 2, 3A/3B, Stage 4B/C/D, Stage 4E variation/
content/archive regressions, Core/Win32 builds, clean Release rebuild
and size/reserve gates. Compiler warnings/errors: **0/0**.

**S4E-09 changes only this documentation file.** No game data,
runtime, build configuration, tests, IDs, seed outputs, save format,
progression or payload bytes change. The verified Release remains
709,120 bytes, leaving 765,440 bytes of headroom.

Revisit only if testing identifies a specific unserved early-game story
gap that merits the integration burden. This task is complete.
