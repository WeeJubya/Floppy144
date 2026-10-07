$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$mainPath = Join-Path $root "game\src\floppy144_main.c"
$winTimingPath = Join-Path $root "src\f144_win32_timing.c"
$winLifecyclePath = Join-Path $root "src\f144_win32_lifecycle.c"
$testSource = Join-Path $PSScriptRoot "stage4_timing_lifecycle_tests.c"

Write-Host "=== STAGE 4 TIMING/LIFECYCLE BOUNDARY AUDIT ==="

$mainSource = Get-Content -LiteralPath $mainPath -Raw
$winTiming = Get-Content -LiteralPath $winTimingPath -Raw
$winLifecycle = Get-Content -LiteralPath $winLifecyclePath -Raw
$platformHeader = Get-Content -LiteralPath (Join-Path $root "include\f144_platform.h") -Raw

foreach($pattern in @(
    '\bGetTickCount(?:64)?\s*\(',
    '\bSetTimer\s*\(',
    '\bKillTimer\s*\(',
    '\bPostQuitMessage\s*\(',
    '\bPostMessageA\s*\(',
    '\bWM_TIMER\b',
    '\bWM_CLOSE\b',
    '\bWM_DESTROY\b',
    '\bWM_ACTIVATEAPP\b'
))
{
    if($mainSource -match $pattern)
    {
        throw "Game coordinator still contains migrated Win32 timing/lifecycle policy: $pattern"
    }
}

foreach($required in @(
    'f144PlatformMonotonicMs',
    'F144LifecycleEvent',
    'F144_LIFECYCLE_SUSPEND',
    'F144_LIFECYCLE_RESUME',
    'f144PlatformQuit'
))
{
    if($platformHeader -notmatch [regex]::Escape($required))
    {
        throw "Platform timing/lifecycle contract is missing: $required"
    }
}

foreach($required in @(
    'GetTickCount64',
    'SetTimer',
    'KillTimer',
    'WM_TIMER'
))
{
    if($winTiming -notmatch [regex]::Escape($required))
    {
        throw "Win32 wake/clock adapter is missing: $required"
    }
}

foreach($required in @(
    'WM_ACTIVATEAPP',
    'WM_CLOSE',
    'WM_DESTROY',
    'F144_LIFECYCLE_ACTIVE',
    'F144_LIFECYCLE_INACTIVE',
    'F144_LIFECYCLE_SHUTDOWN_REQUESTED',
    'F144_LIFECYCLE_SHUTDOWN'
))
{
    if($winLifecycle -notmatch [regex]::Escape($required))
    {
        throw "Win32 lifecycle adapter is missing mapping: $required"
    }
}

foreach($required in @(
    'Floppy144TimingAdvance',
    'Floppy144AutosaveIfNeeded',
    'event->request_autosave',
    'Floppy144LifecycleApply',
    'f144Win32TimingIsWakeMessage',
    'f144Win32TranslateLifecycleEvent'
))
{
    if($mainSource -notmatch [regex]::Escape($required))
    {
        throw "Game coordinator timing/lifecycle wiring is missing: $required"
    }
}

Write-Host "STAGE 4 TIMING/LIFECYCLE BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4 TIMING/LIFECYCLE REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-timing-lifecycle"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-timing-lifecycle"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4_timing_lifecycle_tests.exe"
$includePlatform = "/I" + (Join-Path $root "include")
$includeGame = "/I" + (Join-Path $root "game\src")

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    $testSource,
    (Join-Path $root "src\f144_platform.c"),
    (Join-Path $root "game\src\floppy144_timing.c"),
    (Join-Path $root "game\src\floppy144_lifecycle.c"),
    (Join-Path $root "game\src\floppy144_settings.c"),
    $includePlatform,
    $includeGame,
    "/Fe$exePath"
)

& cl.exe @compileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 timing/lifecycle regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4 TIMING/LIFECYCLE REGRESSION ==="

& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 timing/lifecycle regression failed with exit code $LASTEXITCODE."
}
