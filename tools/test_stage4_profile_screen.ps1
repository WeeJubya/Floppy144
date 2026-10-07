$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root =
    Split-Path -Parent $PSScriptRoot

$mainPath =
    Join-Path $root "game\src\floppy144_main.c"

$recoveryHeaderPath =
    Join-Path $root "game\src\floppy144_recovery.h"

$recoverySourcePath =
    Join-Path $root "game\src\floppy144_recovery.c"

$profileViewPath =
    Join-Path $root "game\src\floppy144_profile_view.c"

$playerVisualPath =
    Join-Path $root "game\src\floppy144_player_visual.c"

$profileHeaderPath =
    Join-Path $root "game\src\floppy144_profile.h"

$persistenceHeaderPath =
    Join-Path $root "game\src\floppy144_persistence.h"

$site2DHeaderPath =
    Join-Path $root "game\src\floppy144_site_2d.h"

$site2DSourcePath =
    Join-Path $root "game\src\floppy144_site_2d.c"

$siteGameplayPath =
    Join-Path $root "game\src\floppy144_site.c"

$runStatePath =
    Join-Path $root "game\src\floppy144_run_state.c"

$testSource =
    Join-Path $PSScriptRoot "stage4_profile_screen_tests.c"

Write-Host "=== STAGE 4C PROFILE SCREEN BOUNDARY AUDIT ==="

$mainSource =
    Get-Content -LiteralPath $mainPath -Raw

$recoveryHeader =
    Get-Content -LiteralPath $recoveryHeaderPath -Raw

$recoverySource =
    Get-Content -LiteralPath $recoverySourcePath -Raw

$profileViewSource =
    Get-Content -LiteralPath $profileViewPath -Raw

$playerVisualSource =
    Get-Content -LiteralPath $playerVisualPath -Raw

$profileHeader =
    Get-Content -LiteralPath $profileHeaderPath -Raw

$persistenceHeader =
    Get-Content -LiteralPath $persistenceHeaderPath -Raw

$site2DHeader =
    Get-Content -LiteralPath $site2DHeaderPath -Raw

$site2DSource =
    Get-Content -LiteralPath $site2DSourcePath -Raw

$siteGameplaySource =
    Get-Content -LiteralPath $siteGameplayPath -Raw

$runStateSource =
    Get-Content -LiteralPath $runStatePath -Raw

foreach($required in @(
    'FLOPPY144_SCREEN_PROFILE',
    'Floppy144ProfileViewDraw',
    'FLOPPY144_MAIN_MENU_OPERATOR_PROFILE'
))
{
    if($mainSource -notmatch [regex]::Escape($required))
    {
        throw "Profile screen coordinator wiring is missing: $required"
    }
}

foreach($required in @(
    'FLOPPY144_MAIN_MENU_OPERATOR_PROFILE',
    'FLOPPY144_MAIN_MENU_OPTION_COUNT'
))
{
    if($recoveryHeader -notmatch [regex]::Escape($required))
    {
        throw "Profile menu option is missing: $required"
    }
}

foreach($required in @(
    '"OPERATOR PROFILE"',
    'case FLOPPY144_MAIN_MENU_OPERATOR_PROFILE:'
))
{
    if($recoverySource -notmatch [regex]::Escape($required))
    {
        throw "Profile menu presentation/availability is missing: $required"
    }
}

if($profileViewSource -match 'Floppy144RunState')
{
    throw "Profile view must not consume current recovery-session state."
}

foreach($required in @(
    'FLOPPY144_OPERATOR_BODY_STYLE_A',
    'FLOPPY144_OPERATOR_BODY_STYLE_B',
    'FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT',
    'FLOPPY144_OPERATOR_BODY_STYLE_COUNT',
    'Floppy144DiscoveryProfileBodyStyle',
    'Floppy144DiscoveryProfileSetBodyStyle'
))
{
    if($profileHeader -notmatch [regex]::Escape($required))
    {
        throw "Body-style profile contract is missing: $required"
    }
}

if(
    $persistenceHeader -notmatch 'FLOPPY144_PROFILE_VERSION\s+1U' -or
    $persistenceHeader -notmatch 'FLOPPY144_PROFILE_PAYLOAD_V1_SIZE\s+64U'
)
{
    throw "S4C-03 unexpectedly changed the profile V1 persistence format."
}

foreach($required in @(
    'Floppy144CommitProfileBodyStyle',
    'F144_ACTION_MOVE_LEFT',
    'F144_ACTION_MOVE_RIGHT',
    'Floppy144PersistenceSaveProfile',
    'Floppy144Site2DDrawForPlayerState'
))
{
    if($mainSource -notmatch [regex]::Escape($required))
    {
        throw "Body-style coordinator wiring is missing: $required"
    }
}

foreach($required in @(
    'LEFT/RIGHT SELECT',
    'LEFT/RIGHT  BODY STYLE',
    'Floppy144ProfileViewDrawBodyPreview'
))
{
    if($profileViewSource -notmatch [regex]::Escape($required))
    {
        throw "Body-style selector/preview presentation is missing: $required"
    }
}

if(
    $site2DHeader -notmatch [regex]::Escape('Floppy144Site2DDrawForPlayerState') -or
    $site2DSource -notmatch [regex]::Escape('Floppy144PlayerVisualDraw') -or
    $playerVisualSource -notmatch [regex]::Escape('FLOPPY144_OPERATOR_BODY_STYLE_B') -or
    $playerVisualSource -notmatch [regex]::Escape('sprite_width * 7 / 8')
)
{
    throw "Current player renderer is not consuming the cosmetic body style."
}

if(
    $siteGameplaySource -match '(?i)body_style' -or
    $runStateSource -match '(?i)body_style'
)
{
    throw "Cosmetic body style leaked into gameplay/run-state logic."
}

foreach($required in @(
    'Floppy144DiscoveryProfileCollectionsEverRestoredCount',
    'Floppy144DiscoveryProfileEvidenceEverEstablishedCount',
    'completed_recoveries',
    'latest_completion_evidence_percent',
    'latest_completion_recovered_kb',
    'operator_name',
    'body_style'
))
{
    if($profileViewSource -notmatch [regex]::Escape($required))
    {
        throw "Profile view does not display persistent field: $required"
    }
}

$profileCaseStart =
    $mainSource.IndexOf(
        'case FLOPPY144_MAIN_MENU_OPERATOR_PROFILE:'
    )

$terminateCaseStart =
    $mainSource.IndexOf(
        'case FLOPPY144_MAIN_MENU_TERMINATE:',
        $profileCaseStart
    )

if(
    $profileCaseStart -lt 0 -or
    $terminateCaseStart -lt 0
)
{
    throw "Profile activation block could not be isolated."
}

$profileActivation =
    $mainSource.Substring(
        $profileCaseStart,
        $terminateCaseStart -
        $profileCaseStart
    )

foreach($forbidden in @(
    'Floppy144PersistenceSaveProfile',
    'Floppy144DiscoveryProfileMergeRunState',
    'Floppy144DiscoveryProfileBeginRecovery',
    'Floppy144DiscoveryProfileRecordCompletion'
))
{
    if($profileActivation -match [regex]::Escape($forbidden))
    {
        throw "Opening Profile unexpectedly mutates or persists history: $forbidden"
    }
}

if(
    $mainSource -notmatch
    'global_screen\s*!=\s*FLOPPY144_SCREEN_PROFILE'
)
{
    throw "Profile can incorrectly replace the active recovery resume screen."
}

if(
    $mainSource -notmatch
    'case FLOPPY144_SCREEN_PROFILE:[\s\S]*?F144_ACTION_BACK[\s\S]*?FLOPPY144_SCREEN_MAIN_MENU'
)
{
    throw "Profile Backspace route to Session Control is missing."
}

Write-Host "STAGE 4C PROFILE SCREEN BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4C PROFILE SCREEN REGRESSION ==="

$tempDir =
    if($env:RUNNER_TEMP)
    {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4-profile"
    }
    else
    {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-profile"
    }

New-Item -ItemType Directory -Force -Path $tempDir |
    Out-Null

$exePath =
    Join-Path $tempDir "stage4_profile_screen_tests.exe"

$includePlatform =
    "/I" +
    (Join-Path $root "include")

$includeGame =
    "/I" +
    (Join-Path $root "game\src")

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    $testSource,
    (Join-Path $root "game\src\floppy144_profile.c"),
    (Join-Path $root "game\src\floppy144_profile_edit.c"),
    (Join-Path $root "game\src\floppy144_profile_view.c"),
    (Join-Path $root "game\src\floppy144_player_visual.c"),
    (Join-Path $root "game\src\floppy144_draw.c"),
    $includePlatform,
    $includeGame,
    "/Fe$exePath"
)

& cl.exe @compileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C profile-screen regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4C PROFILE SCREEN REGRESSION ==="

& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C profile-screen regression failed with exit code $LASTEXITCODE."
}
