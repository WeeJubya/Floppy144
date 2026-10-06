# Stage 3 Baseline for Stage 4

This document records the immutable Stage 3 comparison point for all FLOPPY//144 Stage 4 work.

The baseline is the completed Stage 3 source itself, not the commit that adds this document. Later Stage 4 phases must compare gameplay behaviour, generated-data integrity, build health, and size-budget movement against the source commit recorded below. Stage 3 gameplay behaviour must remain unchanged unless a Stage 4 requirement explicitly changes it.

## Baseline identity

| Item | Baseline |
| --- | --- |
| Source branch | `development/stage-3c-complete` |
| Source commit / HEAD | `fe2239123f98346ed06b48bb500672a5e6e2e172` |
| Source commit message | `Manual update of document content.` |
| Stage 4 branch starting point | `Stage4`, created from the same source commit |
| Baseline date | 2026-10-06 |
| Validation workflow | `.github/workflows/stage3c-ci.yml` |
| Validation run | GitHub Actions run `37494439775`, attempt `2` |
| Validation result | **PASS** |

The validation workflow was rerun against the exact source SHA above after the `Stage4` branch was created. Attempt 2 completed successfully and reproduced the original Stage 3-complete build and size result.

## Build configuration and toolchain

The repository CI establishes the release baseline on a `windows-2022` GitHub-hosted runner.

| Item | Value |
| --- | --- |
| Build configuration | `release` |
| Platform | `windows` |
| Architecture | x86-64 |
| Application | `Floppy144`, C99, WindowedApp |
| Runtime library project | `F144 Runtime`, C99 static library |
| Project generator | Premake `5.0.0-beta8` |
| IDE/toolchain | Visual Studio 2022 Enterprise |
| MSVC tools | `14.44.35207` |
| Windows SDK | `10.0.26100.0` |
| MSBuild | `17.14.60+43b635718` |
| CI release warning policy | `/W4` and `TreatWarningsAsErrors=true` |
| Game-data / site compilers | C11, `/W4 /WX /O2` |

The Release application is optimized for size by `premake5.lua`. The supporting runtime library is optimized for speed.

GitHub-hosted runner/action deprecation notices are infrastructure notices and are not counted as project compiler warnings. The project build itself produced zero compiler/linker warnings and zero errors.

## Canonical generation result

Canonical game-data generation completed successfully:

```text
Floppy//144 game data compiled: 35 collections, 170 documents, 50 triggers,
42 interactions, 23 evidence, 634 physical items; 1137/2519 KB.
```

Generated Site validation also completed with:

- warnings: **0**
- errors: **0**
- result: **PASS**
- game-data result: **PASS**

### Canonical release metrics

These values are taken directly from the current Stage 3-complete canonical JSON and its generated-data validation.

| Metric | Count |
| --- | ---: |
| Collections | 35 |
| Documents | 170 |
| Budgeted readable documents | 169 |
| Triggers | 50 |
| Interactions | 42 |
| Ambient interactions | 16 |
| Evidence entries | 23 |
| Physical items | 634 |
| Rooms | 11 |
| Furniture entries | 151 |
| Fixtures | 91 |
| Drawing definitions | 24 |
| Colours | 44 |
| Connections | 16 |
| Relationships | 1,249 |
| Budgeted reconstruction pool | 150% |
| Registered reconstruction weight | 175% |

### Document composition

The canonical source does not contain a simple binary `authored/generated` flag for documents, so an invented split is deliberately not recorded.

All 170 canonical document records contain bodies and are classified by `source_status` as:

| Source status | Count |
| --- | ---: |
| `setup-authored-identity` | 49 |
| `preserved-stage-1` | 5 |
| `authored-for-integration` | 116 |
| **Total** | **170** |

The runtime generator compiles all of these canonical records into generated tables. By document type, the same 170 records comprise 50 `trigger_record` entries and 120 `readable_record` entries. These schema classifications are recorded separately from the canonical `budgeted_readable_documents` metric above.

## Regression baseline

The fresh baseline validation run completed every existing Stage 3 automated regression gate successfully.

| Regression / audit | Result |
| --- | --- |
| Canonical game-data generation | **PASS** |
| Generated Site validation | **PASS** |
| Stage 2 headless tests | **PASS** |
| Stage 3A Prologue tests | **PASS** |
| Player-facing Act label audit | **PASS** |
| Stage 3B.1 Terminal tests | **PASS** |
| Stage 3B.2 Site Interaction tests | **PASS** |
| Stage 3B.3 Reconstruction tests | **PASS** |
| Stage 3B.4 Coordinator Wiring audit | **PASS** |
| Main Menu Record Feedback audit | **PASS** |
| Main Menu Reinstate Flow audit | **PASS** |
| Stage 3B.4 Door / Access tests | **PASS** |
| Stage 3B.5 Cabinet Coordinator Wiring audit | **PASS** |
| Physical Item Player-Facing Contract | **PASS** |
| Stage 3B.5 Cabinet tests | **PASS** |
| Stage 3B regression summary | **PASS** |

No gameplay or canonical-data changes were made to obtain these results.

## Release build baseline

| Item | Result |
| --- | ---: |
| Release build | **PASS** |
| MSBuild warnings | 0 |
| MSBuild errors | 0 |
| Compiler warnings found in validation log | 0 |
| Compiler errors found in validation log | 0 |
| Linker warnings found in validation log | 0 |
| Linker errors found in validation log | 0 |
| Release executable | `Floppy144.exe` |
| Release executable size | **659,968 bytes** |
| Floppy-size limit | **1,474,560 bytes** |
| Remaining size headroom | **814,592 bytes** |
| Size gate | **PASS** |

The hard size limit is defined by `run.ps1` and duplicated by the Stage 3C CI release-size gate.

## Stage 4 regression contract

### Gameplay regression criteria

Unless a Stage 4 requirement explicitly changes gameplay, Stage 4 work must preserve the behaviour covered by the Stage 2, Stage 3A, and Stage 3B automated suites, including progression, terminal behaviour, Site interaction, reconstruction, door/access behaviour, persistence/menu flows, physical-item behaviour, and the player-facing contracts already guarded by the audits.

A Stage 4 presentation change is not permission to silently alter trigger ordering, canonical IDs, collection availability, reconstruction rules, interaction completion, save/reinstate semantics, evidence behaviour, or physical-item gameplay.

If a Stage 4 requirement intentionally changes any protected behaviour, the relevant regression expectation must be updated deliberately and the change documented against this baseline.

### Presentation changes allowed in Stage 4

Stage 4 may deliberately change presentation systems when required by the Stage 4 specification, including rendering, UI/layout, visual treatment, animation, audio, presentation-oriented container views, and other presentation capabilities.

Such work may alter pixels, layout, assets, presentation code, or executable size. It must not alter Stage 3 gameplay semantics merely as a side effect unless the Stage 4 requirement explicitly calls for that gameplay change.

### Size-budget regression criteria

The immutable Stage 3 executable baseline is **659,968 bytes**.

Stage 4 is allowed to grow beyond that baseline because presentation work may require additional code and data. Every Stage 4 phase should nevertheless measure and report its executable-size delta against this baseline.

The absolute release gate remains **1,474,560 bytes**. A release above that limit is a hard failure. Unexpected or disproportionate size growth should be investigated even while the executable remains below the hard limit.

Baseline headroom available to Stage 4 is **814,592 bytes**.

## Reproduction path

The authoritative Stage 3 CI path performs the following against the baseline source:

1. configure the VS2022 x64 MSVC environment;
2. run `tools/build_game_data.ps1`;
3. run `tools/test_stage2.ps1`;
4. run `tools/test_stage3a.ps1`;
5. run `tools/test_stage3b.ps1`;
6. install Premake 5.0.0-beta8;
7. generate the VS2022 solution with `premake5 vs2022`;
8. build Release with MSBuild;
9. locate `Floppy144.exe` and enforce the 1,474,560-byte release gate.

For local release validation, `run.ps1 -build release` follows the same generation/regression/build/size-gate path using the locally installed toolchain.

## Baseline rule

**Commit `fe2239123f98346ed06b48bb500672a5e6e2e172` is the immutable Stage 3 gameplay/build/size comparison point for Stage 4.**

The `Stage4` branch may advance from it, but the baseline SHA itself must not be moved, rewritten, or reinterpreted.
