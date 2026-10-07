$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root =
    Split-Path -Parent $PSScriptRoot

$sourceDir =
    Join-Path $root "game\src"

$mainPath =
    Join-Path $sourceDir "floppy144_main.c"

$terminalHeaderPath =
    Join-Path $sourceDir "floppy144_terminal.h"

$terminalSourcePath =
    Join-Path $sourceDir "floppy144_terminal.c"

$runStateHeaderPath =
    Join-Path $sourceDir "floppy144_run_state.h"

$persistenceSourcePath =
    Join-Path $sourceDir "floppy144_persistence.c"

$testSource =
    Join-Path $PSScriptRoot "stage4_terminal_auth_tests.c"

Write-Host "=== STAGE 4C TERMINAL AUTHENTICATION BOUNDARY AUDIT ==="

$mainSource =
    Get-Content -LiteralPath $mainPath -Raw

$terminalHeader =
    Get-Content -LiteralPath $terminalHeaderPath -Raw

$terminalSource =
    Get-Content -LiteralPath $terminalSourcePath -Raw

$runStateHeader =
    Get-Content -LiteralPath $runStateHeaderPath -Raw

$persistenceSource =
    Get-Content -LiteralPath $persistenceSourcePath -Raw

foreach($required in @(
    'Floppy144TerminalApplyOperatorIdentity',
    'GDR NETWORK ACCESS',
    'VERIFYING PROFILE...',
    'ACCESS ACCEPTED',
    'GDR OPERATOR'
))
{
    if(
        $terminalSource -notmatch
        [regex]::Escape($required)
    )
    {
        throw "Terminal authentication presentation is missing: $required"
    }
}

foreach($required in @(
    'global_terminal_authentication_complete',
    'Floppy144DiscoveryProfileOperatorName',
    'Floppy144TerminalApplyOperatorIdentity'
))
{
    if(
        $mainSource -notmatch
        [regex]::Escape($required)
    )
    {
        throw "Terminal/profile authentication wiring is missing: $required"
    }
}

if(
    $terminalHeader -match
        '(?i)floppy144_profile\.h|Floppy144DiscoveryProfile' -or
    $terminalSource -match
        '(?i)floppy144_profile\.h|Floppy144DiscoveryProfile'
)
{
    throw "Terminal module should receive an operator-name string, not own profile persistence."
}

if(
    $runStateHeader -match
        '(?i)terminal_authentication|operator_name' -or
    $persistenceSource -match
        '(?i)terminal_authentication_complete'
)
{
    throw "Terminal authentication leaked into recovery/save-state persistence."
}

if(
    $mainSource -notmatch
        'FLOPPY144_MAIN_MENU_INITIATE_SESSION[\s\S]*?global_terminal_authentication_complete\s*=\s*false[\s\S]*?Floppy144TerminalReset[\s\S]*?Floppy144ConfigureTerminalSession\(\s*true\s*\)'
)
{
    throw "Fresh recovery does not establish one authenticated terminal session."
}

if(
    $mainSource -notmatch
        'FLOPPY144_MAIN_MENU_REINSTATE_SESSION[\s\S]*?global_terminal_authentication_complete\s*=\s*false[\s\S]*?Floppy144TerminalReset(?:AtRoom)?[\s\S]*?Floppy144ConfigureTerminalSession\(\s*true\s*\)'
)
{
    throw "Reinstated recovery does not establish one authenticated application session."
}

if(
    $mainSource -notmatch
        'Floppy144SiteAccessTerminalRoom[\s\S]*?Floppy144TerminalResetAtRoom[\s\S]*?Floppy144ConfigureTerminalSession\(\s*true\s*\)'
)
{
    throw "Physical terminal access does not refresh operator identity."
}

if(
    $mainSource -notmatch
        'Floppy144TerminalReset\([\s\S]*?Floppy144ConfigureTerminalSession\(\s*false\s*\)[\s\S]*?Floppy144CatalogueReset'
)
{
    throw "Application startup unexpectedly authenticates before a recovery session exists."
}

Write-Host "STAGE 4C TERMINAL AUTHENTICATION BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4C TERMINAL AUTHENTICATION REGRESSION ==="

$tempDir =
    if($env:RUNNER_TEMP)
    {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4-terminal-auth"
    }
    else
    {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-terminal-auth"
    }

if(Test-Path -LiteralPath $tempDir)
{
    Remove-Item -LiteralPath $tempDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $tempDir |
    Out-Null

$includePlatform =
    "/I" +
    (Join-Path $root "include")

$includeGame =
    "/I" +
    $sourceDir

$sources = @(
    $testSource,
    (Join-Path $sourceDir "floppy144_game_data.c"),
    (Join-Path $sourceDir "floppy144_trigger_engine.c"),
    (Join-Path $sourceDir "floppy144_interaction_engine.c"),
    (Join-Path $sourceDir "floppy144_run_state.c"),
    (Join-Path $sourceDir "floppy144_world.c"),
    (Join-Path $sourceDir "floppy144_terminal.c"),
    (Join-Path $sourceDir "floppy144_recovery.c"),
    (Join-Path $sourceDir "floppy144_catalogue.c"),
    (Join-Path $sourceDir "floppy144_document.c"),
    (Join-Path $sourceDir "floppy144_effect.c"),
    (Join-Path $sourceDir "floppy144_draw.c"),
    (Join-Path $sourceDir "floppy144_site.c"),
    (Join-Path $sourceDir "floppy144_site_rooms.c"),
    (Join-Path $sourceDir "floppy144_site_object.c"),
    (Join-Path $sourceDir "floppy144_object_registry.c"),
    (Join-Path $sourceDir "floppy144_collection_registry.c")
)

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/wd4505",
    "/utf-8",
    $includePlatform,
    $includeGame
) + $sources + @(
    "/Festage4_terminal_auth_tests.exe"
)

Push-Location $tempDir
try
{
    & cl.exe @compileArgs

    if($LASTEXITCODE -ne 0)
    {
        throw "Stage 4C terminal-auth regression build failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    Write-Host "=== RUN STAGE 4C TERMINAL AUTHENTICATION REGRESSION ==="

    & ".\stage4_terminal_auth_tests.exe"

    if($LASTEXITCODE -ne 0)
    {
        throw "Stage 4C terminal-auth regression failed with exit code $LASTEXITCODE."
    }
}
finally
{
    Pop-Location
}
