# Document / evidence / Notebook correlation

Reviewed 2026-09-25 against `development/stage-3c-in-universe-documents`,
base commit `e0ea8a089858a4c607df803dcc20300fee76d0bf`.
Corrections are on `development/stage-3c-notebook-correlation`.

## Scope and result

Read all 157 rewritten document bodies in `data/floppy144_game_data.json` and
compared them with all 23 evidence definitions, all 40 interaction records,
the 35 collection Notebook entries, and their trigger/inspection dependencies.
The content correction changes 26 strings only: `statement` and `notebook_entry`
for eight evidence records, plus ten interaction `notebook` strings.

No document body, ID, title, classification, source order, relationship,
collection allocation, physical source, synthesis condition, prerequisite,
trigger, effect, metadata or runtime code changed. No generated game output or
legacy root-level JSON copy is part of this change.

The governing distinction is between what a record reports, what an inspection
establishes, and what a combination supports. Absence of a recorded visitor is
not proof of no unrecorded movement; authorisation is not presence; a manual
electrical channel is not proof of a person operating a switch.

## Evidence correlation

Document references below use `collection / RS suffix`; for example
`DR-07 / 0003` means `DR-07-RS-0003`. Supporting optional records are not assumed
to have been opened. Physical observations come from the existing inspections.

| Evidence | Document support and physical acquisition | Decision |
|---|---|---|
| E-001 | FM-13 / 0047, 0002; I-001 inspects P-012 after T-006. | Retain: physically connected control, no claim about cause. |
| E-002 | FM-07 / 0001–0002; I-002 inspects retained-services register P-033. | Retain: deliberate retention is explicit; it is not deliberate discharge. |
| E-003 | DR-04 / 0001–0006; I-003 inspects handover folder P-014 revealed by I-002. | Change “handover was attempted” to “ahead of the planned ... handover.” A checklist/target does not establish an actual attempt. DR-04 is enabled by this inspection, so the finding also relies on the folder itself. |
| E-004 | DR-07 / 0001–0006, particularly 0003; I-004 + I-005 compare P-040/P-041. | Retain: location on trolley awaiting refiling, not disappearance or custody clearance. |
| E-005 | DR-29 / 0001–0006; I-006 + I-007 compare transfer box and withdrawal marker. | Retain: staged transfer, not completed transfer, loss or theft. |
| E-006 | DR-31 / 0001–0006, especially assignment 0002; I-008/P-052. | Retain: assigned verification responsibility, not responsibility for the incident. |
| E-007 | TS-02 / 0001–0006; I-009/P-017 after diagnostic T-020. | Retain: serviceable endpoint and upstream network fault; does not identify the riser as cause yet. |
| E-008 | TS-06 / 0001–0006; I-010/P-057 after T-021/T-022. | Retain: correctly patched route toward Server Room, not end-to-end service restored. |
| E-009 | TS-10 / 0001–0006; I-011/P-066. | Retain: preparation complete, live verification blocked. I-012/media case remains optional corroboration. |
| E-010 | DR-47 / 0001–0006; I-013/P-021. | Retain: closure suspended for review and unfinished work, not a finding of cause or blame. |
| E-011 | DR-53 / 0001–0006; I-014/P-080 with cabinet access. | Retain: the Director expressly distinguishes the planned centralisation from an emergency/security shutdown. |
| E-012 | OS-18 / 0001–0002; I-015 + I-016/P-159/P-160. | Retain: standing and temporary permissions only; no inference that permission proves entry. |
| E-013 | OS-07 / 0001–0006; I-017 + I-018/P-001/P-002. | Specify the incident window and explicitly preserve the possibility of unrecorded movement. |
| E-014 | HR-09 / 0001–0006; I-019 + I-020/P-025/P-026. | Limit attendance to start of duty and distinguish assigned duties from later locations. |
| E-015 | OS-26 / 0001–0002, corroborated by 0007; I-021 + I-022/P-085/P-086. | Evidence already says injuries were “recorded”; retain it. Qualify I-022's shorter “no injuries” as “no injuries reported.” |
| E-016 | OS-26 / 0002–0003; I-022 + I-023/P-086/P-087. | Attribute the location to Security's report and retain “in or immediately around ... route before the alarm,” rather than asserting precise presence at evacuation. |
| E-017 | OS-48 / 0001–0006 + OS-41 / 0001; I-024 + I-025 + I-026/P-096/P-097/P-161. | Limit the entry claim to IT Support and include the cable-riser blind spot. No conclusion about interior activity or cause. |
| E-018 | FM-18 / 0001–0006; I-027 + I-028/P-037/P-103. | Evidence already limits absence to detection; retain it. Align I-028 with recorded sensor states and ventilation isolation. |
| E-019 | TS-14 / 0001–0006; I-029 + I-030/P-106/P-107. | Retain: live server infrastructure for backup/verification, not completed archive verification. |
| E-020 | TS-18 / 0001, corroborated by 0004; I-031/P-112. | Retain: physical cable work immediately before incident, without yet asserting causation. |
| E-021 | FM-13 / 0047 via E-001; TS-18 / 0001–0002 via T-048/T-049; I-032/P-113. TS-18 / 0004 and 0007 corroborate. | Describe the shared, still-connected circuit and work area without treating co-location as proof of the release mechanism. Does not require E-020 first. |
| E-022 | FM-18 / 0001–0002 through E-018; TS-18 / 0002 via T-049; I-033/P-114. | Explicitly identify the electrical channel and state that it does not establish switch operation by a person. |
| E-023 | I-034 automatically combines E-020 + E-021 + E-022; earlier FM-13 / 0047 reports instability under cable movement. TS-18 / 0005 and 0008 contain the stronger isolated-test/final-report support. | Replace “establish” with a supported accidental-activation reconstruction; no attribution of individual responsibility. Do not claim the optional isolated test was read. |

Changed evidence: E-003, E-013, E-014, E-016, E-017, E-021, E-022, E-023.
Changed interaction notes: I-003, I-018, I-020, I-022, I-023, I-026, I-028,
I-032, I-033, I-034. All evidence statements still exactly match their canonical
evidence Notebook entries.

## Disclosure and visibility

`tools/game_data_emit_runtime.inc` emits the canonical prose;
`Floppy144GameDataNotebookRecordVisible` in `game/src/floppy144_game_data.c`
selects entries from existing persistent state. Evidence-producing interaction
notes are suppressed in the Notebook to avoid duplicate findings. Their source
text was still aligned so the authored records do not contradict each other.

Paired evidence still requires its complete synthesis set, including the
order-independent handling in the runtime. No inspection or trigger is added.
E-021 and E-022 remain distinct from the E-023 synthesis. E-023 still completes
the existing route through I-034; it does not require a new optional document or
test gate. Its earlier wording implied greater certainty than its acquisition
requirements warranted. The prior service note, physical inspections, work
sequence and diagnostics support a reconstruction without pretending that the
optional isolated reproduction has been read.

All 35 collection notes are availability summaries, not narrative conclusions;
they were retained. DR-01's note is recorded by T-001. The separate Security
cabinet-code fact/capability remains governed by T-037 and existing cabinet
logic. I-035 (keys) and I-039 (cabinet register) describe acquired capabilities,
not documentary deductions, and were retained.

## Full document coverage

All documents in each listed collection were read, including non-trigger and
optional background documents. This is 157 documents in 30 populated collections.

| Collection | Documents | Correlation / role |
|---|---:|---|
| DR-01 | 4 | Recovery and catalogue limitations; availability note only. |
| DR-02 | 4 | Command and preservation guidance; no incident finding. |
| DR-03 | 4 | Reconstruction limits and explicit Notebook evidential guidance. |
| DR-04 | 6 | E-003; planned workstreams and incomplete handover. |
| DR-07 | 6 | E-004; location versus custody distinction. |
| DR-23 | 4 | Historical 1962 misfile; not evidence about final-Friday incident. |
| DR-29 | 6 | E-005; staged, unsigned/unsealed transfer controls. |
| DR-31 | 6 | E-006; unresolved exceptions and assignment. |
| DR-47 | 6 | E-010; suspension and questions for review, no cause finding. |
| DR-53 | 6 | E-011; centralisation authority and executive context. |
| HR-01 | 6 | Staffing/desks, not minute-by-minute movements. |
| HR-05 | 5 | Welfare/background; no incident conclusion imported. |
| HR-09 | 6 | E-014; attendance/duty and separately recorded evacuation. |
| FM-04 | 6 | Facilities/access/shared-route background; no cause finding. |
| FM-07 | 6 | E-002/E-003 context; retained services and handover dependencies. |
| FM-13 | 6 | E-001; live circuit and earlier movement-sensitive input. |
| FM-18 | 6 | E-018/E-022; sensor states, automatic responses, limits of diagnosis. |
| FM-23 | 1 | Isometric-plan request; no investigative conclusion. |
| OS-07 | 6 | E-013; visitor record scope and pass-return limitations. |
| OS-18 | 2 | E-012; authorisation versus actual entry. |
| OS-26 | 7 | E-015/E-016; preliminary incident, reported route, accountability. |
| OS-34 | 5 | Key/register/cabinet-code handling; capability notes. |
| OS-41 | 1 | E-017; manually entered duty-call chronology. |
| OS-48 | 6 | E-017; observed approaches and deliberate coverage gaps. |
| TS-02 | 6 | E-007; endpoint tests versus building route. |
| TS-06 | 6 | E-008; patching, intermittent riser fault, proposed work. |
| TS-08 | 4 | Historical support faults; not alternative facts about Friday. |
| TS-10 | 6 | E-009; prepared is not verified/transferred. |
| TS-14 | 6 | E-019; server availability versus network accessibility. |
| TS-18 | 8 | E-020–E-023; work, channel diagnosis, optional test and final report. |

HR-14, HR-27, HR-36, FM-32 and TS-27 have collection metadata but no authored
document bodies in this snapshot. They were not counted as missing rewritten
documents or used to support evidence.

## Source ambiguities retained

This pass does not silently reconcile minor chronology discrepancies in the
documents: DR-04 / 0005 lists a 16:00 Director review and 16:30 closure target,
then calls the latter a handover review; DR-53 / 0006 places an interruption
from Security at 14:25, while OS-41 records the alarm call at 14:29 (the earlier
call may be separate). FM-18 / 0003 describes ventilation isolation before the
permitted suppression sequence, whereas 0001 labels 14:28:49 as the start of
the release sequence before the 14:28:50 isolation command. Sequence initiation
and actual discharge may be different events. None of the Notebook entries
assert these disputed exact times or resolve these ambiguities by invention.

Some optional documents disclose more than the prerequisite trigger document
(notably TS-18 / 0008). That existing archive-reading behaviour is retained;
this change prevents the Notebook from presuming every optional report was read.

## Validation

Validation used a disposable copy under `obj/notebook-audit/native`, regenerated
from the edited canonical JSON. Tracked generated files in the branch were
untouched. Windows MSVC 14.51, SDK 10.0.26100.0 and installed Premake were used.

- JSON parse, stable counts and unique IDs: PASS (35 collections, 157 documents,
  50 triggers, 40 interactions, 23 evidence, 161 physical items).
- All document bodies nonempty; all 50 trigger IDs/title/collection mappings:
  PASS. Evidence physical/collection/interaction/additional-evidence references:
  PASS. All 23 statement/Notebook pairs and emitted evidence prose: PASS.
- Parsed comparison with the base commit after normalising only the permitted
  prose fields: PASS. Exactly 26 changed strings; every other value, array order
  and document body unchanged. `git diff --check`: PASS.
- `tools/build_game_data.ps1`: PASS. Recovery budget 1151 KB required / 2371 KB
  complete archive. Site compiler: 11 rooms, 16 doors, zero warnings/errors.
- `tools/test_stage2.ps1`: PASS.
- `tools/test_stage3a.ps1`: PASS, including its save/reinstate checks and
  player-facing act-label audit.
- `tools/test_stage3b.ps1`: PASS: terminal, Site interaction, reconstruction,
  door/access, cabinet suites and coordinator-wiring audits.
- `premake5 vs2022`, native MSBuild release: PASS, zero warnings/errors.
  Executable: **522,752 bytes**, below the **1,474,560-byte** limit.
- Independent read-only review of all evidence and 114 supporting documents:
  no further substantive mismatch. The main review also covered the other 43.

The initial release attempt encountered duplicate PATH/Path environment entries
and then sandbox denial in MSBuild FileTracker. A deduplicated child environment
and approved execution outside the sandbox allowed the release build to pass;
no repository build-script changes were required.

Still needed: interactive Windows play-through for Notebook wrapping/scrolling
of the longer sentences, chronological display after save/reinstate, and both
Records-first and Technology-first routes through final synthesis. In particular,
check E-021/E-022 before E-023 and compare a route that skips optional TS-18
reports with one that opens them. Automated Windows build/regression and the
release-size gate are complete; these visual/player acceptance checks are not.
