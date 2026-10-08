# Stage 4G completion: Grey Door Republic end-case

**Gate:** S4G-05 final integration acceptance  
**Scope:** only the one-shot Grey Door anomaly; no new content or gameplay redesign  
**Status: STAGE 4G: PASS (automated release gate)**  
**Baseline:** completed Stage 3 source `fe2239123f98346ed06b48bb500672a5e6e2e172`  
**Pre-4G head:** `c15a0966bcb0b0575d15457eb5a6a8a74d13501d`  
**Stage 4G commit range:** `77dbebf4f3bc3b275049c550bc86f3cbfbbe13f1..HEAD` (final Stage4 HEAD is the commit containing the signed-off handover)

## Design and discoverability

The secret is not a generated collection or normal evidence record. Once DR-01 is available,
its terminal LIST provides one surplus *orphan* authored index pointer:
`DR-00-RS-0144`, *Unallocated Floor Area Notice*. `OPEN` goes through the normal
document viewer. Only the **first** successful view flips an isolated
`Floppy144RunState.grey_door_state` from `UNAVAILABLE` (0) to `AVAILABLE` (1).
The document hints at impossible floor area without naming the Grey Door Republic.
Reopening it never repeats the effect or updates the Notebook.

## Corridor overlay and seed selection

The Stage 4E stateless `Floppy144VariationRange` hashes the saved
`recovery_seed` in namespace `GREY_DOOR / CORRIDOR_WALL_V1`.
A stable, cached enumeration of **22 candidate wall slots** derives from generated
Corridor floor boundaries. Every candidate is a 4U x 1U or 1U x 4U door-shaped
wall overlay, validated against the actual walkable Corridor stance and a 2U
protected-geometry halo. The validator rejects overlaps with normal doors,
room labels, wall-hangings, Directory, fixtures and other authored non-floor
rectangles. There is no fallback for zero or overflowing safe candidates.
One selected candidate is rendered at a time; no candidate enters the actual Site
rectangle inventory or collision system. The original Site Directory does not
know this overlay exists.

The exhaustive geometry regression checks **all 22 slots**, including both
orientations. The final seed-route matrix uses
`0, 1, 42, 144, 146, 2026, 65535, 0x12345678, 0xffffffff, 9441,
271828, 314159` and requires two seeds to select different valid candidates.

## Temporary scene

`floppy144_grey_encounter.{h,c}` owns a completely transient
`Floppy144GreyEncounter` state and draws to the existing **640x360 software
framebuffer** with procedural rectangles/text. It is not a normal reconstructed
Room or a new renderer and introduces no Win32 API to game-core logic.

The coordinator opens `FLOPPY144_SCREEN_GREY_ENCOUNTER` from the existing
`A ACCESS` action and accepts existing logical movement and `I INSPECT`.
The Stage 4B monotonic presentation tick advances all timed phases.
The regular CRT filter is intentionally omitted for the bright modern office.
The room contains the GREY DOOR REPUBLIC sign, desks, original future-game
concept thumbnails, code monitor, unexplained half-eaten sandwich and a seated
Developer facing away. Inspection reveals `DEVELOPER`, followed by the slow
turn and exact text `You're not supposed to be able to get in here.`.
The isolated scene then draws **RESTORATION CAPACITY: 144%**, runs a short
horizontal CRT glitch and returns to the original Corridor foot position.

The 144% text appears **only in framebuffer presentation**. The source audit
guards against that string in authoritative run-state and persistence modules.
The test matrix compares `Floppy144RunStateRecoveredKb`, the real
`Floppy144RunStateRecoveredPercent` (at most 100% under legal capacity),
completion predicates, collection/evidence/notebook masks, the entire
`Floppy144RunState` and the entire `Floppy144DiscoveryProfile` before
and after the vignette. The scene itself never receives a mutable run-state
pointer, so it cannot alter normal progression.

## One-shot lifecycle and persistence

```
UNAVAILABLE (0) --first secret record view--> AVAILABLE (1)
AVAILABLE (1) --finish the vignette--> COMPLETED (2)
COMPLETED (2) --all subsequent actions--> COMPLETED (2)
```

After the glitch, the coordinator marks only the hidden completion flag and
ordinary `dirty` bit, saves to the existing autosave path immediately, and
resumes the original Corridor render/collision. Manual and lifecycle-triggered
autosaving are suppressed **during** the transient scene so a temporary room
can never be serialised. Normal autosave resumes on exit.

If a pre-encounter manual checkpoint and a completed autosave belong to the
**same saved recovery seed**, the completed autosave takes precedence over
the otherwise usual manual-first policy. Different-seed saved runs never
inherit one another's hidden state. V3 stores the three-state byte; V1/V2
migration defaults it to UNAVAILABLE without changing Profile/settings schema.
A genuinely new recovery calls `Floppy144RunStateBegin` and offers a fresh
run-scoped opportunity, not a lifetime Profile lock. The existing file schema
uses recovery seed as run identity, so consciously reusing identical debug
test seeds across unrelated checkpoints is a documented test-only ambiguity.

Upon COMPLETED, renderer/hitbox/prompt queries fail closed, repeated secret
record view is inert, and changing rooms or re-entering a terminal cannot
reactivate it. A pixel-diff regression proves the ordinary Corridor rendering
is byte-identical in UNAVAILABLE and COMPLETED states. The Site Directory is
unchanged at all three stages and physical movement/collision is unchanged.

## Integration and regression coverage

- **S4G-01:** orphan document provenance, access gating and V3/legacy decoding
- **S4G-02:** exhaustive 22 candidate placements, collision/halo checks and seed determinism
- **S4G-03:** 22 candidate entrance simulations, all timed encounter phases,
  Developer inspect, no saving inside the scene, exact return and no progression changes
- **S4G-04:** one-shot reversibility rejection, fresh-run scope, stale
  manual/autosave precedence, pixel-level disappearance, Directory non-interference
- **S4G-05:** twelve fixed seeds x four early/mid/late/low-capacity snapshots
  (48 complete headless routes), real physical save/reload at unseen, available
  and completed boundaries, route predicates and Profile invariance
- Existing Stage 2, Stage 3A/3B/3C, Stage 4B/4C/4D/4E/4F
  regressions, portable Core, Win32 and clean x64 Release build remain in
  the standard `.github/workflows/stage3c-ci.yml` gate.

The Stage 4G source audit also forbids disclosure in normal Help, Profile,
Settings, Completion, Site Directory, Notebook, Intro, credits *other than
the existing game design company name*, and additional shipping diagnostics.
The only deliberately player-readable clue is the orphan DR-01 record.

## Final integration evidence and fixed-seed locations

The production-equivalent acceptance ran on **2026-10-08**:
**[GitHub Actions run 37810016677](https://github.com/WeeJubya/Floppy144/actions/runs/37810016677)**
at shipping-source commit `e1fc849f70877fc6cd081ed43f07b5342147341a`.
It finished **SUCCESS** across Stage 2/3, Stage 4B-F, the existing
Stage 4G tests, the new 48-case S4G-05 journey, portable Core, Win32,
application icon, strict Release and the final size gate.

The S4G-05 matrix reported these deterministic baseline wall planes
(in canonical 100x100 Site units; width x height). Every seed was also
tested in four distinct synthetic recovery contexts.

| Recovery seed | Slot | Corridor wall rectangle |
| ---: | ---: | --- |
| 0 | 18 | (47,46) 4x1 |
| 1 | 6 | (66,67) 1x4 |
| 42 | 11 | (66,72) 1x4 |
| 144 | 16 | (45,46) 4x1 |
| 146 | 9 | (66,70) 1x4 |
| 2026 | 16 | (45,46) 4x1 |
| 65535 | 18 | (47,46) 4x1 |
| 0x12345678 (305419896) | 21 | (50,46) 4x1 |
| 0xffffffff (4294967295) | 6 | (66,67) 1x4 |
| 9441 | 17 | (46,46) 4x1 |
| 271828 | 15 | (44,46) 4x1 |
| 314159 | 3 | (66,64) 1x4 |

The matrix proved at least two seeds choose different safe locations,
and that both candidate orientations are selected. The low-free-space
scenarios were bounded by **less than 250 KB remaining recovery
capacity** using real canonical collection sizes. The entire run-state
snapshot, evidence/completion-derived predicates, actual recovery
capacity, Profile and final percentages were identical through
temporary-scene rendering. The only difference on return was the
one-shot lifecycle value (and normal run dirty flag).

All physical V3 boundary tests (UNAVAILABLE, AVAILABLE, COMPLETED)
passed, including the completed autosave's precedence over an older
same-seed manual checkpoint. No negative regression, duplicate door,
notebook message or player-facing post-encounter reference was found.

## Executable budget and provenance

| Metric | Size |
| --- | ---: |
| Immutable Stage 3 Release baseline | 659,968 bytes |
| Last successful Stage 4F / pre-4G Release (`c15a0966`, run `37778993691`) | 716,800 bytes |
| Stage 4G production Release | 725,504 bytes |
| **Stage 4G cost** | **+8,704 bytes** |
| **Total Stage 3-to-Stage 4 growth** | **+65,536 bytes** |
| Maximum decompressed Release payload | 1,474,560 bytes |
| **Remaining headroom** | **749,056 bytes** |

Project production builds use MSVC `/W4` with warnings-as-errors;
the successful release has **zero warnings and zero errors**.
The broader historical Stage 3B terminal/cabinet test compilations still
emit four existing C4701 warnings, separate from the Release builds.

**Acceptance scope:** These are automated headless state, collision,
framebuffer and compile/regression checks plus a production-equivalent
Windows build. They are not a claim of a physically observed human
playthrough. The Stage 4E integration record already notes a pending
human-observed alternate-ending replay check; that separate QA obligation
remains visible rather than being silently declared performed.

## Reproduction

Push to `Stage4`; the standard Windows Actions workflow regenerates canonical
data, runs the Stage 2-4 suite including
`tools/test_stage4_persistence.ps1` and
`tools/test_stage4g_grey_door.ps1`, builds all three portability tiers,
performs the clean Release rebuild and enforces the 1,474,560-byte
payload limit. All Stage 4G technical documents are internal and must
not be linked from player-facing Help, credits or the game executable.

**S4G-05 signed-off run and final HEAD:** see the Stage 4 completion handover
(`docs/stage4/stage4_completion.md`) accompanying this file.
