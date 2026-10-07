# Stage 4C - Bespoke Recovery Completion

S4C-08 changes completion presentation only. Stage 3 remains the authority for
what counts as a completed recovery and for the persistence consequences of
that state.

## Completion logic retained

The current authored core-resolution chain is:

- physical item P-114 is inspected through I-033 and establishes E-022;
- automatic synthesis I-034 requires E-020, E-021 and E-022;
- I-034 establishes E-023 and applies SET_ACT COMPLETE;
- E-023 is the evidence record classified as Core resolution.

The terminal-exit coordinator independently evaluates two established ending
axes:

1. Floppy144GameDataEvidenceResolved, which detects established authored Core
   resolution evidence;
2. Floppy144RunStateAvailableRecoveryCapacityExhausted, which detects that at
   least one currently available unrestored collection exists but none can fit
   in the remaining capacity.

The Completion presentation is entered when either axis is true. They remain
separate so evidence-resolved, capacity-exhausted and simultaneous outcomes can
all be represented.

## Frozen completion semantics

Before presentation, the coordinator retains the Stage 3 sequence:

1. merge the achieved RunState into the discovery profile;
2. record one completion snapshot, including evidence mask, outcome flags and
   recovered KB;
3. save the profile;
4. clear RunState dirty state;
5. mark the recovery session inactive;
6. enter Completion.

The Completion renderer never calls persistence or progression APIs. Completion
navigation cannot return to Office or Terminal. Manual save continues to require
an active session, and autosave returns immediately when the session is
inactive, so the frozen completed run remains unsaveable.

Starting a later recovery still uses the ordinary Session Control Initiate path
and increments recovery history through Floppy144DiscoveryProfileBeginRecovery.
Viewing Completion, Final Note or Credits does not record another completion.

## Completion summary

The new summary presents only values already tracked by the completed RunState
and Profile:

- operator name, or UNASSIGNED if the profile has no name;
- collections actually restored in this run;
- evidence actually established in this run;
- free recovery capacity in KB and its derived percentage;
- the existing ending flavour.

The three current status variants are kept distinct:

- EVIDENCE RESOLVED;
- RECOVERY CAPACITY EXHAUSTED;
- EVIDENCE RESOLVED / CAPACITY EXHAUSTED.

Context text describes only the relationship between those existing predicates.
It does not substitute a new narrative conclusion.

## Final Note

VIEW FINAL NOTE preserves the old completion evidence ledger as a dedicated
subview. The authored Core resolution record is discovered from game data rather
than hard-coding E-023 into the renderer.

When Core resolution was established, its authored notebook/evidence prose is
shown first as FINAL, followed by the full numbered evidence record. When
capacity ends the run before Core resolution, the view explicitly reports that
the core resolution was not established and leaves unrecovered evidence
obscured. No conclusion is invented.

The evidence record remains scrollable with Up/Down and Page Up/Page Down.
Backspace or Enter returns to the Completion summary.

## Navigation

The summary offers:

- VIEW FINAL NOTE;
- VIEW CREDITS;
- RETURN TO MAIN MENU.

Credits remembers whether it was opened from Settings or Completion, so
Backspace returns to the correct parent. Returning to the main menu leaves no
active recovery, therefore Return to Active Site and Record Current Session
remain unavailable.

Escape retains the application's established universal route to Session Control;
it still cannot resume completed gameplay.
