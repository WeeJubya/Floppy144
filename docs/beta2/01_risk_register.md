# B2-01: Beta.2 runtime migration risk register
20 October 2026. Statuses distinguish audit evidence, actions not yet taken, and rights decisions reserved for the relevant human owner. No legal advice or relicensing decision is implied.

| ID | Severity | Risk / verified evidence | Mitigation / owner / current status |
|---|---|---|---|
| R01 | CRITICAL | Public Beta.1 release is a single compiled EXE containing statically linked River2D-lineage GPLv3 source, according to rename history, source comparisons and active build wiring. Separate licensing/assignment is not documented. | **Legal/rights owner: urgent manual review** of redistribution authority, any grants, GPL obligations and published package. Open. |
| R02 | HIGH | Public Beta.1 repository has **no root LICENSE**; source `premake5.lua` expects to copy one, and credits claim notices are in a distributed licence. Release asset list shows EXE only. | Review existing release immediately for corresponding source, licence notices and delivery mechanism. Do not automatically publish/replace a licence. Open. |
| R03 | HIGH | WeeJubya/main is very old; Beta.1 public HEAD and current development/Stage-4 are not identical (seven changed files). A comparison of different baselines can conceal regressions. | Lock and name both exact SHAs, record 7-file drift, use immutable Beta.1 asset for visual acceptance and development SHA for CI. Open until independent binary verification. |
| R04 | HIGH | Full Windows regression CI at development base SHA **fails** the Stage 4F floppy-label assertion (exit 43). Later full-gate build steps skipped. | Reconcile acceptance intent with current scene art and test in a separate authorised change. No test weakening in B2-01. Open. |
| R05 | HIGH | Legacy `F144Runtime` binds window, GDI framebuffer, callbacks and platform state; simple text renaming risks copying derived design or breaking application lifecycle. | Isolate clean-room new implementation, preserve platform API behaviour, independent code review and line-of-origin trace. Not started. |
| R06 | HIGH | Save/profile/settings migration and old paths may regress when moving Win32 storage across the seam. | Fixture-based before/after tests on real Windows, inspect path and save format, preserve backwards compatibility. Not started. |
| R07 | MEDIUM | No independent SHA-256 verification of Beta.1 EXE; metadata digest only. Stage-4 CI build equal size does not establish byte equivalence. | Download release asset to controlled Windows machine; hash, record PE metadata and source tag, archive read-only. Open. |
| R08 | MEDIUM | No baseline screenshots, raw-frame captures or human audio comparison; hardware/time/seed vary. | Execute documented capture matrix on fixed Windows VM with controlled save/seed/date and record per-capture hashes. Open. |
| R09 | MEDIUM | Puddle-influenced `string_view` and embedded 5x7 glyph source/attribution uncertain. | Trace upstream history/authorship or reauthor after rights review. Open. |
| R10 | MEDIUM | Reimplementation can change aspect ratio, pixel format, hard scaling, letterboxes, repaint, input repeats, focus handling or timer scheduling. | Define independent external behaviour tests and exact baselines before B2-02. Open. |
| R11 | MEDIUM | WinMM procedural MIDI/waveOut, volume, device failure and shutdown can regress, violating no stored-audio policy. | Keep generated audio; retain semantic cue mapping, exercise missing-device and long-play tests. Open. |
| R12 | MEDIUM | Native Release size may grow past 1,474,560 bytes, leaving only 652,288 bytes at published Beta.1 size. | Require zero-warning strict clean VS2022 Release, hash, byte gate on each implementation milestone. Not started. |
| R13 | MEDIUM | Branch-specific CI workflow filters currently exclude `development/Beta-2-runtime`; documentation-only push may not run full checks. | Extend CI triggers later by separately authorised change or use existing manual dispatch when supported. No CI pass assumed. |
| R14 | LOW | Tracked site compiler binary and generated-data provenance can complicate public source packaging. | Compare/rebuild tool and review canonical inputs separately. Open. |

## Immediate stop/go conditions
- **B2-01 documentation** can be checked in without changing source or licences; this does not remove R01/R02.
- **B2-02 engineering** requires a confirmed rights/compliance handling plan, independent baseline EXE hash, reconciliation of the pre-existing intro regression, documented behavioural test oracle and acceptance of a clean-room separation protocol.
- Preserve GDR release repository `main` untouched. Keep all Beta.2 commits on WeeJubya `development/Beta-2-runtime`.
- Changes to existing public release assets, licensing or attribution require an explicit human decision. Do not silently change those as a by-product of runtime migration.
