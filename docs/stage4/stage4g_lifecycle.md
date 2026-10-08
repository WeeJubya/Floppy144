# S4G-04: One-shot Grey Door lifecycle hardening

## The only valid run-level transitions

```
UNAVAILABLE (0)  --first successful view of DR-00-RS-0144-->  AVAILABLE (1)
AVAILABLE   (1)  --finish the GREY DOOR REPUBLIC encounter--> COMPLETED (2)
COMPLETED   (2)  --anything in the same run-->             COMPLETED (2)
```

These are **run-state-only** transitions. The ordinary DR-01 record
list may contain the orphan archive clue, but the document itself is
not a normal collection and adds no recovered capacity, evidence,
notebook entry, statistic or Profile event. Opening it again after
completion does nothing, including leaving dirty=0 on a clean saved run.

## What the player can see

- **Before discovery:** no Grey Door overlay, no Grey Door interaction,
  no collider, no altered Directory, room counts or object list.
- **Available:** one visual overlay on a single generated, vetted
  corridor wall selected deterministically by `GREY_DOOR /
  CORRIDOR_WALL_V1`. Nearby A/I use the existing controls. The overlay
  never inserts a Site rect or walkable cell. Other rooms, objects and
  Site Directory retain their normal interactions.
- **After completion:** the overlay query, interaction and proximity
  labels return false. Neither room changes, another terminal session,
  repeated view of the orphan record nor reloading a completed V3 save
  can re-enable it. Original wall rendering is **pixel-identical** at
  the same foot position, seed, camera and restored rooms.

Ordinary player-facing Profile, completion, credits, notebook, recovery
and directory modules are checked for stray Grey Door references.
No achievement, marker, menu option or message is created.

## Recorded-session checkpoint edge case

S4G-03 suppressed autosave inside the ephemeral vignette. S4G-04 makes
the one-shot transaction durable on exit: after
`Floppy144RunStateGreyDoorComplete`, the coordinator immediately
writes the unchanged world state plus the completed bit to the normal
autosave file, without adding a Profile statistic or scene data.

Normal Session Control prefers an explicit manual checkpoint over
autosave. That can inadvertently resurrect an AVAILABLE Door if the
manual checkpoint predates the anomaly and autosave was completed.
The narrowly scoped
`Floppy144RunStateGreyDoorCompletedAutosavePreferred` predicate
therefore selects a **completed autosave of the same recovery seed**
over a non-completed manual checkpoint. All other checkpoint
combinations retain existing manual-first precedence. A different-seed
recovery can never borrow the completed flag.

The recovery seed is the current V3 run identity. An explicit **new
recovery run** calls `Floppy144RunStateBegin`, resets to UNAVAILABLE,
and may discover a new Door; the Profile has no once-ever flag.
The optional deterministic test-seed override can deliberately create
two saves with the *same* seed, and this current save format cannot
distinguish their identities. Avoid using a reused test seed with
mixed manual/autosave records when testing an independent new run.
A unique session instance ID would require a future save-schema
migration, and is intentionally not added by this hardening patch.

If writing the completion autosave fails, the standard autosave
persistence-warning state is set. No Easter egg acknowledgement is
ever displayed. A failed filesystem write cannot promise persistence.

## Regression gates

1. Unavailable, available and completed save/reload.
2. All valid wall placements and two wall orientations (S4G-02/03).
3. A fixed seed always maps to the same safe wall, including after
   reconstructing other rooms and navigating away/back.
4. Pre/post room and fixture counts and solid wall collision.
5. Same-seed completed autosave preference; different-seed and ordinary
   manual precedence.
6. Repeated hidden-document views and forbidden backward transitions.
7. Fresh recovery with a new seed, and fresh reset under a controlled
   reused seed; no Profile spillover.
8. Direct framebuffer equality for UNAVAILABLE vs COMPLETED and
   visible pixel difference while AVAILABLE. Site Directory pixels
   must always match.
9. Source guard against hidden debug/player-facing acknowledgements.
10. Full Windows gameplay regressions, clean Release rebuild, and the
    1,474,560-byte decompressed payload limit.

The complete runtime intentionally does not add new collections,
geometry, keys or narrative after the Developer's final glitch.
