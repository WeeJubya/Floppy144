# FLOPPY//144 Stage 4 root build-tool separation

**Scope:** PowerShell orchestration only. No change to the game's source,
runtime behaviour, collections, generated-data compiler, compiler flags,
regression assertions or release payload.

## Original execution and dependencies

Before this refactor the actual root command path was:

```
cleanbuild.ps1
  Remove-Item .\build -Recurse -Force -ErrorAction SilentlyContinue
  Remove-Item .\obj   -Recurse -Force -ErrorAction SilentlyContinue
  Remove-Item .\bin   -Recurse -Force -ErrorAction SilentlyContinue
  git diff --check
  .\run.ps1 -build release
      Create obj/build/bin
      tools/build_game_data.ps1   [compile canonical JSON + Site; prerequisite]
      tools/test_stage2.ps1       [regression]
      tools/test_stage3a.ps1      [regression]
      tools/test_stage3b.ps1      [regressions including door, cabinet, visual audits]
      premake5 vs2022            [solution generation]
      MSBuild /t:Floppy144        [compilation]
      Check Release executable <= 1,474,560 bytes [post-build gate]
```

The `run.ps1` script did **not** launch the Windows executable before
this work, so it still does not launch the executable. Its existing
`-build debug|asan|release` argument is a **build configuration**, unrelated
to `-GDR-CinderEllie`/`-GDR-Hathaway`, which are game executable arguments.
Neither root script previously forwards flags to an executable.

`tools/build_game_data.ps1` compiles the canonical data compiler, generates
runtime tables and legacy .def aliases, and calls `tools/build_site.ps1`.
Those outputs are needed for BOTH headless regressions and a standalone
Windows build. Therefore this *preparation*, including its built-in
validation, legitimately appears in both `testbuild` and `run`;
it is **not** an extra replay of an individual regression script.

The Stage 3B runner contains its own established sequence of 3B.1 terminal,
3B.2 Site, 3B.3 reconstruction, 3B.4 exterior doors/coordination and
3B.5 cabinets, geometry, physical items and visual audits; that sequence
and its failure aggregation are unchanged.

## Final orchestration

```
cleanbuild.ps1
    git diff --check              [unchanged exact Git command, now first]
    Remove-Item build/obj/bin      [unchanged exact three statements]
    testbuild.ps1                 [38 ordered regression scripts]
    run.ps1 -build release        [only after ALL tests pass]

testbuild.ps1                    [standalone regression workflow]
    tools/build_game_data.ps1     [necessary canonical prerequisite]
    38 regression scripts        [exact original Stage 2->3A->3B order;
                                  rest in existing Stage 4 CI order]
    FULL REGRESSION SUITE: PASS   [no production exe build or launch]

run.ps1 -build release           [standalone build workflow]
    tools/build_game_data.ps1     [necessary canonical prerequisite]
    premake5 vs2022               [unchanged]
    MSBuild /t:Floppy144          [unchanged arguments]
    Release size check            [unchanged 1,474,560 byte threshold]
```

The CI-only **source release-candidate gate** requires a clean and pushed
Git checkout, so it stays in the CI source phase rather than breaking
direct local `testbuild.ps1` use with uncommitted developer changes.
The CI-only built-artifact gate, generated icon verification, strict
multi-target Release rebuild, packaging and reserve checks similarly
stay at their *actual build/release dependency points*. The Release size
gate also remains in `run.ps1`: the file does not exist until after MSBuild.
This is deliberate classification, not lost test coverage.

A native command's exit code is checked; thrown PowerShell exceptions
propagate immediately. `cleanbuild.ps1` cannot call `run.ps1` after a
failed test. The scripts resolve paths from `$PSScriptRoot` and restore
the caller's working directory. The ordinary standalone tests and
release build do not depend on cleanbuild-only variables.

## Exact test order and migration inventory

Tests 1–3 retain the original `run.ps1` sequence. The other 35 tests
were already in CI and are brought into the standalone test runner,
**without changing their existing CI order**. There are no duplicates
within the 38-script runner or among the three root scripts.

| Order | Regression/check | Previous location | New local owner |
| ---: | --- | --- | --- |
| 1 | `test_stage2.ps1` | `run.ps1` | `testbuild.ps1` |
| 2 | `test_stage3a.ps1` | `run.ps1` | `testbuild.ps1` |
| 3 | `test_stage3b.ps1` | `run.ps1` | `testbuild.ps1` |
| 4 | `test_stage4_platform.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 5 | `test_stage4_input.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 6 | `test_stage4_persistence.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 7 | `test_stage4_audio.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 8 | `test_stage4_timing_lifecycle.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 9 | `test_stage4_single_instance.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 10 | `test_stage4_config.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 11 | `test_stage4_variation.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 12 | `test_stage4_calendar.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 13 | `test_stage4_noticeboard.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 14 | `test_stage4_takeaway.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 15 | `test_stage4_crossword.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 16 | `test_stage4_paperback.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 17 | `test_stage4_archive.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 18 | `test_stage4_profile_screen.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 19 | `test_stage4_profile_name.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 20 | `test_stage4_terminal_auth.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 21 | `test_stage4_settings.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 22 | `test_stage4_credits.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 23 | `test_stage4_reinstate_presentation.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 24 | `test_stage4_completion.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 25 | `test_stage4_help_presentation.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 26 | `test_stage4_office_camera.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 27 | `test_stage4_player_visual.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 28 | `test_stage4_inspection_25d.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 29 | `test_stage4_presentation_consistency.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 30 | `test_stage4d_integration.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 31 | `test_stage4_build_architecture.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 32 | `test_stage4b_integration.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 33 | `test_stage4c_integration.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 34 | `test_stage4e_integration.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 35 | `test_stage4f_application_icon.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 36 | `test_stage4f_intro.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 37 | `test_stage4f_presentation_journey.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |
| 38 | `test_stage4g_grey_door.ps1` | Existing Stage 4 CI-only | `testbuild.ps1` |

## Validation

The normal Windows CI continues its existing source/compiled regression
and final Release gate. A separate root-orchestration CI acceptance
checks `testbuild.ps1`, `run.ps1`, and `cleanbuild.ps1` as independent
commands and injects a **temporary Stage 2 regression failure only in
the CI workspace**, restoring the original file in `finally`. It verifies
that no Visual Studio solution was generated after the injected failure
and that the modified fixture was restored byte-for-byte. No failing
test/fixture is committed.

This document is internal developer documentation, not player-facing.
