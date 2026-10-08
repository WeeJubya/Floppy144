# S4G-01: Orphan floorplan record

## Player discovery

After restoring **DR-01**, the terminal's `LIST DR-01` last page
includes one surplus index row:

- **DR-00-RS-0144**: *Unallocated Floor Area Notice*

The record is an independent authored document in the existing
`Floppy144DocumentDefinition` registry. It is intentionally **not** part
of the canonical JSON or the 35 generated collection IDs. The extra
terminal row is a pointer, not a recovered DR-01 document slot or a
recovery target. `OPEN DR-00-RS-0144` uses the same viewer as every
other recovered authored document.

The anomalous floorplan, maintenance-lamp discrepancy and disputed
corridor wall are the only hints. There is no explicit unlock message.

## One-shot contract

`Floppy144RunState.grey_door_state` is strictly per recovery run:

| Value | Meaning |
| --- | --- |
| 0 | unavailable |
| 1 | available/unseen |
| 2 | encounter completed |

Opening the record after DR-01 recovery changes 0 to 1 and marks the
run dirty. Later views have no side effects, including after 1 becomes 2.
Completion's 1-to-2 transition will be invoked by the future encounter
implementation, not by this task. Nothing touches the normal collection,
trigger, evidence, notebook, recovery-capacity, profile or ending fields.

## Persistence and deterministic placement

Save **V3** adds one byte after the original V2 notebook data.
Legacy V1/V2 decoders remain intact and initialise the new byte to zero.
New saves use V3; old saves remain readable. Neither existing file content
nor existing profile/settings schemas need conversion.

The future geometry pass calls
`Floppy144RunStateGreyDoorPlacementSlot(state, candidate_count)`.
This hashes the persisted recovery seed in the independent
`GREY_DOOR / CORRIDOR_WALL_V1` feature domain from Stage 4E. The
result indexes a **fixed, deterministic list of safe wall candidates**.
No coordinate is stored or rerolled on reload.

**S4G-02 responsibility:** validate a stable list of candidate corridor
wall segments against doors, wall fixtures, required interactions and
reachability, then render the selected door. If no safe candidate exists,
do not materialise a door. Keep the candidate ordering stable for saved
runs once deployed. S4G-01 does not change world geometry.

## Regression

`TestGreyDoorDiscovery` is part of the existing Windows headless
persistence regression. It tests ID lookup, authored viewer identity,
DR-01 access gating, first-view availability, repeat-view stability,
capacity/evidence/notebook/profile isolation, V3 save/reload, completion
irreversibility and legacy V2 decode. The S4G source audit additionally
checks that the special record is absent from canonical collection data.

Builds and the 1,474,560-byte size gate are checked by the Stage4
GitHub Actions release CI on push.
