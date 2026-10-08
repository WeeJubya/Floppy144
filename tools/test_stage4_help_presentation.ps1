param()

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$SourceDir = Join-Path $Root "game\src"
$IncludeDir = Join-Path $Root "include"

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw "MSVC compiler is not configured for the Stage 4D Help regression."
}

Write-Host "=== STAGE 4D HELP PRESENTATION REGRESSION ==="

$TerminalSource = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_terminal.c") -Raw

foreach($Required in @(
    'FLOPPY144_TERMINAL_HELP_TEXT_WIDTH',
    'FLOPPY144_TERMINAL_HELP_ROW_GAP',
    'Floppy144TerminalPushHelpPageContent',
    'Floppy144TerminalHelpPagerActive',
    'Floppy144TerminalMoveHelpPager',
    'Floppy144TerminalCloseHelpPager'
)) {
    if($TerminalSource -notmatch [regex]::Escape($Required)) {
        throw "Help presentation wiring missing: $Required"
    }
}

$Sources = @(
    (Join-Path $ScriptDir "stage4_help_presentation_tests.c"),
    (Join-Path $SourceDir "floppy144_game_data.c"),
    (Join-Path $SourceDir "floppy144_trigger_engine.c"),
    (Join-Path $SourceDir "floppy144_interaction_engine.c"),
    (Join-Path $SourceDir "floppy144_run_state.c"),
    (Join-Path $SourceDir "floppy144_world.c"),
    (Join-Path $SourceDir "floppy144_terminal.c"),
    (Join-Path $SourceDir "floppy144_recovery.c"),
    (Join-Path $SourceDir "floppy144_catalogue.c"),
    (Join-Path $SourceDir "floppy144_document.c"),
    (Join-Path $SourceDir "floppy144_variation.c"),
    (Join-Path $SourceDir "floppy144_effect.c"),
    (Join-Path $SourceDir "floppy144_draw.c"),
    (Join-Path $SourceDir "floppy144_site.c"),
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_object.c"),
    (Join-Path $SourceDir "floppy144_object_registry.c"),
    (Join-Path $SourceDir "floppy144_collection_registry.c")
)

foreach($Source in $Sources) {
    if(-not (Test-Path $Source)) {
        throw "Required Stage 4D Help test source is missing: $Source"
    }
}

$BuildDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4d-help"
}
else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4d-help"
}

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
$ExePath = Join-Path $BuildDir "stage4_help_presentation_tests.exe"

$ClArgs = @(
    "/nologo",
    "/std:c11",
    "/W4",
    "/WX",
    "/O1",
    "/MT",
    "/D_CRT_SECURE_NO_WARNINGS",
    ("/I" + $SourceDir),
    ("/I" + $IncludeDir)
)
$ClArgs += $Sources
$ClArgs += ("/Fe:" + $ExePath)

Push-Location $BuildDir
try {
    & cl.exe @ClArgs

    if($LASTEXITCODE -ne 0) {
        throw "Stage 4D Help presentation regression build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

$TestOutput = @(
    & $ExePath 2>&1
)
$TestExitCode = $LASTEXITCODE

foreach($Line in $TestOutput) {
    Write-Host $Line
}

if($TestExitCode -ne 0) {
    $AssertionFailures = @(
        $TestOutput |
            Where-Object { $_ -match '^FAIL:' } |
            ForEach-Object { $_.ToString().Trim() }
    )

    if($AssertionFailures.Count -gt 0) {
        throw (
            "Stage 4D Help presentation regression failed. Assertions: " +
            ($AssertionFailures -join " | ")
        )
    }

    throw "Stage 4D Help presentation regression failed with exit code $TestExitCode."
}

Write-Host "STAGE 4D HELP PRESENTATION REGRESSION: PASS"
