$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$mainPath = Join-Path $root "game\src\floppy144_main.c"
$terminalPath = Join-Path $root "game\src\floppy144_terminal.c"
$winConfigPath = Join-Path $root "src\f144_win32_startup_config.c"
$testSource = Join-Path $PSScriptRoot "stage4_config_tests.c"

Write-Host "=== STAGE 4 CONFIGURATION BOUNDARY AUDIT ==="

$mainSource = Get-Content -LiteralPath $mainPath -Raw
$terminalSource = Get-Content -LiteralPath $terminalPath -Raw
$winConfigSource = Get-Content -LiteralPath $winConfigPath -Raw

foreach($forbidden in @(
    'Floppy144CommandLineHasSwitch',
    'global_debug_guidance',
    'GetCommandLineA',
    'GetCommandLineW'
))
{
    if($mainSource -match [regex]::Escape($forbidden))
    {
        throw "Game-facing configuration still depends on legacy/raw Windows argument handling: $forbidden"
    }
}

foreach($required in @(
    'F144StartupConfig global_config',
    'f144Win32StartupConfigFromCommandLine',
    'f144StartupConfigDebugEnabled',
    'f144StartupConfigRecoverySeedOverride'
))
{
    if($mainSource -notmatch [regex]::Escape($required))
    {
        throw "Portable configuration wiring is missing from game coordinator: $required"
    }
}

foreach($required in @(
    '"-GDR-CinderEllie"',
    '"-GDR-Hathaway"',
    '"-seed"',
    '"-date"',
    'f144StartupConfigSetRecoverySeedOverride',
    'f144StartupConfigSetFixedDateOverride'
))
{
    if($winConfigSource -notmatch [regex]::Escape($required))
    {
        throw "Win32 configuration acquisition is missing developer option support: $required"
    }
}

foreach($required in @(
    'pTerminal->debug_guidance',
    '!pTerminal->debug_guidance',
    'NO DEBUG RECOVERY ACTION AVAILABLE'
))
{
    if($terminalSource -notmatch [regex]::Escape($required))
    {
        throw "Existing terminal debug-guidance contract is missing: $required"
    }
}

if($mainSource -match '"-debug"')
{
    throw "The game coordinator must not know the raw -debug spelling."
}

# Stage 4H: no legacy alias, portable semantic inspection state.
foreach($required in @(
    'f144StartupConfigVisualInspectionEnabled',
    'Floppy144RunStateEnableHathawayInspection',
    'INSPECTION MODE - SAVING DISABLED'
)) {
    if($mainSource -notmatch [regex]::Escape($required)) {
        throw "Hathaway mode not wired into coordinator: $required"
    }
}
if($winConfigSource -match '"-debug"' -or
   $winConfigSource -match 'StrStrI|_stricmp|CompareString') {
    throw "Legacy or case-insensitive developer switch is still active."
}
Write-Host "STAGE 4 CONFIGURATION BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4 CONFIGURATION REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-config"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-config"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4_config_tests.exe"
$includePlatform = "/I" + (Join-Path $root "include")

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    $testSource,
    (Join-Path $root "src\f144_startup_config.c"),
    (Join-Path $root "src\f144_win32_startup_config.c"),
    $includePlatform,
    "/Fe$exePath"
)

& cl.exe @compileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 configuration regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4 CONFIGURATION REGRESSION ==="

& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 configuration regression failed with exit code $LASTEXITCODE."
}
