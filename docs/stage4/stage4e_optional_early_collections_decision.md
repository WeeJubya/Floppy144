# S4E-09: Optional early collections, capacity and pacing decision

**Decision:** `S4E-09: NO ADDITIONAL COLLECTIONS`  
**Result:** Outcome B, intentional PASS.  
**Reason:** Existing early optional content is already sufficient; additional recovery choices would compete with essential routes and with stronger authored work in existing empty collections. The executable has space, so **this is not a claim of insufficient payload capacity**.

## Verified starting position (post-S4E-08)

S4E-08 `Stage4` HEAD `a411836d2872ff703ed4b2a1f09f4df373483033`
and Windows GitHub Actions run
[37760614118](https://github.com/WeeJubya/Floppy144/actions/runs/37760614118)
were fully green when this decision was made.

| Measurement | Value |
| --- | ---: |
| Competition executable hard cap | 1,474,560 bytes |
| Verified clean Windows Release before S4E-09 | **709,120 bytes** |
| Headroom before S4E-09 | **765,440 bytes** |
| Existing enforced minimum Stage 4F/QA reserve | **350,000 bytes** |
| Remaining above hard planning reserve | **415,440 bytes** |
| Preferred planning cushion until Stage 4F, Stage 4I and final QA | **500,000 bytes** |
| Remaining above preferred cushion | **265,440 bytes** |
| Existing collections | 35 |
| Authored documents | 170 |
| Reconstructed/generated index-only records | 1,401 |
| Total catalogue records | 1,571 |
| Stage 4E procedural subject/form title combinations | 864 |
| Compiler warnings and errors | 0 and 0 |

This decision has **no executable-byte cost** and allocates none of the reserve.
The executable after S4E-09 remains **709,120 bytes** as long as the
Release source and toolchain stay unchanged. No gameplay source or canonical
data is altered in S4E-09.

## In-game recovery budget: a separate, more important constraint

The working archive has a **1,440 KiB** restoration capacity.
The existing 35 collections have a cumulative recovery cost of
**2,519 KiB**, deliberately making complete recovery impossible in one
run. Collections currently marked `required_for_completion` sum to
**1,137 KiB**, leaving only **303 KiB** in an all-required-collection
working set for optional exploration. These sizes are recovery mechanics
and **must not be confused with the compiled executable payload**.

Prologue and Act I already contain:

| Collection | Era | Optional? | Recovery cost | Authored / total records |
| --- | --- | --- | ---: | ---: |
| DR-01: Disk Recovery Index | Prologue | No | 58 KiB | 4 / 15 |
| DR-02: Terminal Operations Guide | Prologue | Yes | 14 KiB | 4 / 15 |
| DR-03: Site Reconstruction Guide | Prologue | Yes | 14 KiB | 4 / 15 |
| HR-01: Site Establishment | Act I | Yes | 101 KiB | 6 / 25 |
| FM-04: Site Infrastructure & Maintenance | Act I | No | 115 KiB | 6 / 25 |
| FM-07: Asset Management & Decommissioning | Act I | Yes | 101 KiB | 6 / 25 |
| FM-13: Site Safety & Suppression Systems | Act I | Yes | 101 KiB | 6 / 25 |

These early optional holdings already represent **331 KiB** of potential
optional spend. DR-01's existing `T-001` enables **DR-02, DR-03 and HR-01**
at the first post-Prologue choice point. Giving the player yet another
optional collection in this window is not harmless: a believable extra
14-43 KiB restoration could displace later optional exploration or make
a route more dependent on the player's understanding of the capacity
budget. S4E-08 has just expanded the existing early archive to 15/25
records per collection. It does not need another early branch solely to
increase variety.

## Candidate editorial review (not implemented)

These are hypothetical pitches for the go/no-go assessment, **not**
new collection IDs in game data, and none is treated as existing or
available.

| Candidate | Possible access point | Sketch | Potential cost | Decision |
| --- | --- | --- | --- | --- |
| **DR-05, Duplicate Forms & Unclaimed Correspondence** | Reception/Records terminal after DR-01 | Four authored departmental circulars, 11 index-only fillers; snarky routing, duplication and disposal disputes | ~14–29 KiB recovery, ~4–9 KB executable (unmeasured estimate) | Fun but overlaps DR-02/DR-03 and the existing document-generator tone |
| **HR-02, Induction & Competency Reviews** | Main Office/HR source after HR-01 availability | Four authored personnel induction documents, 21 index-only fillers; procedural rituals and questionable training | ~29–43 KiB recovery, ~5–11 KB executable (unmeasured estimate) | Too close to **HR-14 Learning, Development & Competency**, an already registered optional collection awaiting authored content |
| **FM-02, Stationery & Approved Consumables** | Facilities terminal on initial Facilities access | Four authored inventories/approval notices, 21 index-only fillers; absurd stationery procurement | ~29–43 KiB recovery, ~4–10 KB executable (unmeasured estimate) | Overlaps existing FM-07 asset/decommissioning context and Staff Room flavour |

A strong candidate would need an unused subject, believable terminal or
physical source, multiple deliberately authored records, a justified
recovery cost, clear early-game gating and end-to-end fresh/alternate
route testing. None of the above currently clears the **marginal
narrative value versus early-game pressure** threshold.

## Prefer existing Stage 4I content obligations

Four *already registered*, currently empty, non-required holdings have
their own planned authoring pass:

- HR-14: Learning, Development & Competency
- HR-27: Payroll, Allowances & Closure Payments
- HR-36: Personnel Files & Postings
- FM-32: Utilities, Plant & Environmental Services

Filling these in their dedicated Stage 4I task increases authored world
depth without adding a new collection identity, another early unlock,
new trigger/evidence requirements or a new source of save-schema risk.
Do **not** implement or re-time these collections under S4E-09.

## Stage 4F, stability and reserve

Stage 4F still requires icon/intro integration; other future presentation
assets, QA fixes and compiler/linker variation have not yet received
final byte measurements. The existing CI minimum **350,000-byte reserve**
remains compulsory, and the more conservative **500,000-byte planning
cushion** should be protected while those tasks remain unmeasured.

Adding a collection also changes the generated collection enum. Inserting
one between existing entries would reindex the stored restoration bitset
and risk old-save compatibility. Appending a new identity and designing a
new availability gate could avoid that hazard, but still requires
regression coverage in capacity calculations, terminal availability,
notebook, profile history and completion logic. That is a substantial
cost for a weak story improvement.

## Verification and repository scope

No new collection, authored document, trigger, evidence item, dependency,
terminal permission, physical-item source, save field, capacity cost,
profile statistic or recommended route is introduced. Consequently
**every fresh, completed and alternate save remains governed by the exact
same gameplay and content** as the S4E-08 validated baseline.

The existing S4E-08 Windows CI run passed Stage 2, Stage 3A, Stage 3B,
Stage 4B/C/D, all S4E variation and archive audits, independent Core,
Win32, and clean Release builds, size and reserve gates with **0 compiler
warnings, 0 errors**.

**S4E-09 changes exactly one documentation file.** All gameplay,
data, generated registries, tests and workflow files are intentionally
unchanged. A docs-only push triggers the same full CI workflow for final
confirmation.

**Final disposition: S4E-09 PASS (Outcome B, no collections added).**
