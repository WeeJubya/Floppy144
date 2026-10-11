# Bug Fix 13 integration into Stage 4

## Merge inputs and resolution

- Target: `development/Stage-4`, `3ad09be4a7ad24d5d8e5d3ca9fc8ad279f7fda80`.
- Source: `bugfix/13-grey-door-republik`, `abfd75e21fad15f1214432b2ee17553c3a78d484`.
- Pull request: https://github.com/WeeJubya/Floppy144/pull/1.

The merge retains the complete shared player/Developer renderer, fixed-point
movement, close-range inspection, rotating chair, spatial speech, accelerating
presentation-only recovery percentage, shared Site HUD, deterministic corruption,
and two-second black return interval from Bug Fix 13. Stage 4 audio, canonical
company naming, chairs, Security geometry, and all other target-only changes
remain present. Canonical gameplay data is unchanged by this merge.

Three files required explicit conflict resolution:

- CI retains `Stage4`, `development/Stage-4`, and the Bug Fix 13 branch triggers.
- The office sign uses Bug Fix 13's lower position to clear the shared Site HUD,
  retaining the canonical `GREY DOOR REPUBLIK` spelling.
- The Grey Door source audit retains all Bug Fix 13 checks and Stage 4 naming checks.

Main, player visuals, Site 2D and related headers merged automatically and were
reviewed against both inputs. The six encounter/player/Site source and header
files match the Bug Fix 13 input exactly; main additionally retains newer audio
integration from Stage 4.

## Actual local verification

Windows x64, Visual Studio 2026 Community developer environment using MSVC
14.44.35207, Premake VS2022 projects. These are automated checks, not a human
playthrough.

| Check | Result |
| --- | --- |
| Canonical generation and Site validation | PASS, zero warnings/errors |
| Stage 4 persistence | PASS outside sandbox |
| Grey Door journeys | PASS, all 48 seeded scenarios, save boundaries and one-shot completion |
| Grey Door source/isolation audit | PASS |
| Shared player visual/animation regression | PASS |
| Logical input and developer configuration | PASS |
| Bug Fix 15 chairs and Bug Fix 17 Security geometry | PASS |
| Stage 4 audio contract | PASS |
| Stage 2 and Stage 3A | PASS |
| Application icon source and built executable | PASS |
| Release build | PASS, 3 warnings, 0 errors |
| Release size | 752,128 bytes; 722,432 bytes below the 1,474,560-byte limit |
| Stage 3B | FAIL, six pre-existing failing steps, reproduced on target input |
| Stage 4F presentation journey source audit | FAIL, pre-existing silent-audio expectation |

The first sandboxed persistence run failed at file writes; the unchanged suite
passed outside the sandbox. The first sandboxed build timed out with no compiler
diagnostics; an unrestricted single-worker build succeeded. Release emits C4090
at main's three unchanged inspection save wrappers (lines 139, 146, 153): their
const arguments are passed to mutable persistence APIs. Although the inherited
workflow supplies `TreatWarningsAsErrors=true`, the generated compiler command
uses `/WX-`; this result must not be described as warning-clean or strict `/WX`.

## Confirmed pre-existing blockers

An untouched archive of target commit `3ad09be` was generated and tested with
the same toolchain. Its Stage 3B summary matches the merged summary exactly:

1. Terminal: three Facilities authorisation/notebook persistence assertions.
2. Site interaction: opposite-side directory focus/prompt/activation assertion.
3. Reconstruction: five old diagonal dining/office, shelving-depth, and Security
   desk geometry assertions.
4. Cabinet coordinator source audit: missing proportional-scrollbar contract.
5. Physical-item contract: ledger contains 600 items versus expected 633.
6. Cabinet regression: diagonal Secretary desk inspection assertion.

The historical harness also emits four C4701 uninitialized-local warnings.
Both snapshots fail `test_stage4f_presentation_journey.ps1` with
`Stage 4B silent-audio baseline changed; re-audit transition audio.` The current
audio contract passes. These inherited tests were not weakened to obtain a pass.
The full historical CI suite is therefore not claimed green.

## Reproduction

In an x64 Visual Studio developer PowerShell, run `tools/build_game_data.ps1`,
then the focused scripts `test_stage4_persistence`, `test_stage4g_grey_door`,
`test_stage4_player_visual`, `test_stage4_input`, `test_stage4_config`,
`test_bugfix15_chairs`, `test_bugfix17_security`, `test_stage4_audio`,
`test_stage2`, and `test_stage3a` under `tools/` with `.ps1` suffixes.

Generate with `premake5 vs2022`, then build:

```powershell
MSBuild ./build/Floppy144.sln /t:Floppy144 /m:1 /p:Configuration=release /p:Platform=windows /p:WarningLevel=Level4 /p:TreatWarningsAsErrors=true
./tools/test_stage4f_application_icon.ps1 -Phase Built
```

Run `tools/test_stage3b.ps1` and `tools/test_stage4f_presentation_journey.ps1`
on the target input as well as the merge to reproduce the known blockers.
