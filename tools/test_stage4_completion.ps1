$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$mainPath = Join-Path $root "game\src\floppy144_main.c"
$viewPath = Join-Path $root "game\src\floppy144_completion_view.c"
$viewHeaderPath = Join-Path $root "game\src\floppy144_completion_view.h"
$runStateHeaderPath = Join-Path $root "game\src\floppy144_run_state.h"
$dataPath = Join-Path $root "game\src\floppy144_game_data.generated.inc"

$main = Get-Content -LiteralPath $mainPath -Raw
$view = Get-Content -LiteralPath $viewPath -Raw
$viewHeader = Get-Content -LiteralPath $viewHeaderPath -Raw
$runStateHeader = Get-Content -LiteralPath $runStateHeaderPath -Raw
$data = Get-Content -LiteralPath $dataPath -Raw

Write-Host "=== STAGE 4C BESPOKE COMPLETION BOUNDARY AUDIT ==="

foreach($required in @(
    'FLOPPY144_RUN_ACT_COMPLETE',
    '"I-033", "Synthesis", "P-114"',
    '"I-034", "Synthesis", "E-020/E-021/E-022"',
    '"I-034", "SET_ACT", "COMPLETE"',
    '"E-023"',
    '"Core resolution"'
))
{
    if($data -notmatch [regex]::Escape($required) -and
       $main -notmatch [regex]::Escape($required) -and
       $runStateHeader -notmatch [regex]::Escape($required))
    {
        throw "Authoritative completion chain is missing: $required"
    }
}

if(
    $main -notmatch
        'global_completion_evidence_resolved\s*=\s*\r?\n\s*Floppy144GameDataEvidenceResolved' -or
    $main -notmatch
        'global_completion_capacity_exhausted\s*=\s*\r?\n\s*Floppy144RunStateAvailableRecoveryCapacityExhausted' -or
    $main -notmatch
        'global_completion_evidence_resolved\s*\|\|\s*\r?\n\s*global_completion_capacity_exhausted'
)
{
    throw "Completion presentation no longer uses the established Stage 3 ending predicates."
}

$recordCompletionCount =
    [regex]::Matches(
        $main,
        'Floppy144DiscoveryProfileRecordCompletion\s*\('
    ).Count

if($recordCompletionCount -ne 1)
{
    throw "Profile completion must be recorded from exactly one coordinator site; found $recordCompletionCount."
}

$completionTransitionStart =
    $main.IndexOf(
        "global_completion_evidence_resolved="
    )

$completionTransitionEnd =
    $main.IndexOf(
        "else if(",
        $completionTransitionStart
    )

if(
    $completionTransitionStart -lt 0 -or
    $completionTransitionEnd -le $completionTransitionStart
)
{
    throw "Could not isolate completion transition."
}

$completionTransition =
    $main.Substring(
        $completionTransitionStart,
        $completionTransitionEnd - $completionTransitionStart
    )

foreach($required in @(
    'Floppy144DiscoveryProfileMergeRunState',
    'Floppy144DiscoveryProfileRecordCompletion',
    'Floppy144PersistenceSaveProfile',
    'global_run_state.dirty=0U',
    'global_session_active=false',
    'Floppy144CompletionViewReset',
    'FLOPPY144_SCREEN_COMPLETION'
))
{
    if($completionTransition -notmatch [regex]::Escape($required))
    {
        throw "Frozen completion transition lost Stage 3 behaviour: $required"
    }
}

$completionCaseStart =
    $main.LastIndexOf(
        "case FLOPPY144_SCREEN_COMPLETION:"
    )

$completionCaseEnd =
    $main.IndexOf(
        "case FLOPPY144_SCREEN_NOTEBOOK:",
        $completionCaseStart
    )

if(
    $completionCaseStart -lt 0 -or
    $completionCaseEnd -le $completionCaseStart
)
{
    throw "Could not isolate Completion screen navigation."
}

$completionCase =
    $main.Substring(
        $completionCaseStart,
        $completionCaseEnd - $completionCaseStart
    )

foreach($forbidden in @(
    'Floppy144PersistenceSaveRunState',
    'Floppy144DiscoveryProfileRecordCompletion',
    'Floppy144DiscoveryProfileBeginRecovery',
    'FLOPPY144_SCREEN_OFFICE',
    'FLOPPY144_SCREEN_TERMINAL'
))
{
    if($completionCase -match [regex]::Escape($forbidden))
    {
        throw "Completion navigation can mutate/re-enter completed gameplay: $forbidden"
    }
}

foreach($required in @(
    'FLOPPY144_COMPLETION_OPTION_FINAL_NOTE',
    'FLOPPY144_COMPLETION_OPTION_CREDITS',
    'FLOPPY144_COMPLETION_OPTION_MAIN_MENU',
    'Floppy144CompletionViewOpenFinalNote',
    'FLOPPY144_SCREEN_CREDITS',
    'Floppy144OpenMainMenu'
))
{
    if($completionCase -notmatch [regex]::Escape($required))
    {
        throw "Completion navigation is missing: $required"
    }
}

if(
    $main -notmatch
        '(?s)static void Floppy144AutosaveIfNeeded\(.*?!global_session_active\s*\|\|\s*global_run_state\.dirty\s*==\s*0U' -or
    $main -notmatch
        '(?s)case FLOPPY144_MAIN_MENU_RECORD_SESSION:.*?global_session_active\s*&&\s*Floppy144PersistenceSaveRunState'
)
{
    throw "Completed-run autosave/manual-save restrictions have drifted."
}

if(
    $main -notmatch
        '(?s)case FLOPPY144_SCREEN_CREDITS:.*?global_credits_return_screen\s*==\s*FLOPPY144_SCREEN_COMPLETION.*?global_screen\s*=\s*FLOPPY144_SCREEN_COMPLETION' -or
    $main -notmatch
        '(?s)global_settings_option\s*==\s*FLOPPY144_SETTINGS_OPTION_CREDITS.*?global_credits_return_screen\s*=\s*FLOPPY144_SCREEN_SETTINGS'
)
{
    throw "Credits no longer returns to the screen that opened it."
}

if(
    $main -notmatch
        '(?s)case FLOPPY144_MAIN_MENU_INITIATE_SESSION:.*?Floppy144DiscoveryProfileBeginRecovery.*?global_session_active\s*=\s*true'
)
{
    throw "A subsequent recovery no longer begins cleanly on the same profile."
}

foreach($required in @(
    '"RECOVERY SESSION COMPLETE"',
    '"OPERATOR: %s"',
    '"COLLECTIONS RESTORED:  %u / %u"',
    '"EVIDENCE ESTABLISHED: %u / %u"',
    '"CAPACITY REMAINING:   %u KB (%u%%)"',
    '"FINAL ARCHIVAL STATUS"',
    '"VIEW FINAL NOTE"',
    '"VIEW CREDITS"',
    '"RETURN TO MAIN MENU"',
    '"Core resolution"',
    '"FINAL: [ CORE RESOLUTION NOT ESTABLISHED ]"',
    'Floppy144CompletionViewFinalConclusionText'
))
{
    if($view -notmatch [regex]::Escape($required) -and
       $viewHeader -notmatch [regex]::Escape($required))
    {
        throw "Bespoke Completion presentation is missing: $required"
    }
}

if(
    $view -match 'Floppy144Persistence' -or
    $view -match 'Floppy144DiscoveryProfileRecordCompletion' -or
    $view -match 'Floppy144DiscoveryProfileBeginRecovery'
)
{
    throw "Completion view acquired persistence/completion-authority responsibilities."
}

Write-Host "STAGE 4C BESPOKE COMPLETION BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4C BESPOKE COMPLETION REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-completion"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-completion"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exe = Join-Path $tempDir "stage4_completion_tests.exe"

$args = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    (Join-Path $PSScriptRoot "stage4_completion_tests.c"),
    (Join-Path $root "game\src\floppy144_completion_view.c"),
    (Join-Path $root "game\src\floppy144_draw.c"),
    ("/I"+(Join-Path $root "include")),
    ("/I"+(Join-Path $root "game\src")),
    ("/Fe"+$exe)
)

& cl.exe @args
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C bespoke completion regression build failed."
}

& $exe
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C bespoke completion regression failed."
}
