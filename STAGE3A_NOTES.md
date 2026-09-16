# Floppy//144 Stage 3A Prologue Notes

## Purpose

Stage 3A proves one complete player journey across the data, runtime, interface
and persistence layers. A new recovery can initialise archive services, restore
DR-01, open the Disk Recovery Index, execute T-001, reconstruct Reception and
Corridor, record the opening notebook fact, enter the Site, and retain that
state after save/reinstate.

## Data-driven progression

The canonical T-001 effect list now includes:

```text
RECORD_NOTEBOOK_FACT DR-01
```

This fact is derived from the persistent fired-trigger ledger. It therefore
survives save/reinstate without adding a second copy of the same state or a
T-001-specific runtime function.

## Next-action resolver

`Floppy144TerminalPrintNextAction()` derives a useful action from current
persistent state in this order:

1. initialise archive services;
2. open an eligible trigger document belonging to a restored collection;
3. exit to the Site when at least one room has been reconstructed, while also
   naming the next available collection so later terminal visits do not stall;
4. restore the first available unrestored collection when no Site exists yet;
5. report that no recovery action is currently available.

`Floppy144DocumentFirstPendingTrigger()` joins generated document order to the
generic trigger eligibility engine. Neither helper embeds a collection,
document or trigger ID.

## Regression contract

`tools/stage3a_prologue_tests.c` drives public gameplay APIs and verifies:

- a new run begins in the Prologue with archive services offline;
- the objective resolver requests `INITIATE`;
- terminal `INITIATE` updates authoritative and hydrated state;
- DR-01 is selected as the first available collection;
- terminal `RESTORE DR-01` consumes the correct four-percent quota;
- the pending trigger-document query selects the Disk Recovery Index;
- terminal `OPEN DR-01-RD-0001` resolves the canonical record;
- graphical catalogue opening reaches the same authored-document path;
- opening that document fires T-001 once;
- Reception and Corridor are reconstructed;
- DR-02, DR-03 and HR-01 become available;
- the DR-01 notebook fact is recorded;
- the terminal hands the player to Site exploration;
- `EXIT` requests the Site screen transition;
- on Windows, save/reinstate retains the collection, trigger, rooms, notebook
  fact and archive-service state.

The Stage 2 suite remains in the build and runs first, preventing Stage 3A from
weakening the established data, Site, evidence, drawing or isometric contracts.
