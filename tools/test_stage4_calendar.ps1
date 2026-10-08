$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$platformPath = Join-Path $root "src\f144_platform.c"
$calendarPath = Join-Path $root "src\f144_calendar.c"
$nativePath = Join-Path $root "src\f144_win32_calendar.c"
$configPath = Join-Path $root "src\f144_startup_config.c"
$gameSource = Join-Path $root "game\src"
$premake = Get-Content -LiteralPath (Join-Path $root "premake5.lua") -Raw
$workflow = Get-Content -LiteralPath (Join-Path $root ".github\workflows\stage3c-ci.yml") -Raw

Write-Host "=== STAGE 4E PLATFORM-NEUTRAL CALENDAR BOUNDARY AUDIT ==="
$scanPaths = @($platformPath, $calendarPath, $configPath) +
    @(Get-ChildItem -LiteralPath $gameSource -File -Filter "*.c" | Select-Object -ExpandProperty FullName)
foreach($path in $scanPaths)
{
    $source = Get-Content -LiteralPath $path -Raw
    if($source -match '(?i)\bGetLocalTime\s*\(|\bGetSystemTime\s*\(|\bSYSTEMTIME\b|\b(localtime|gmtime|time)\s*\(')
    {
        throw "Native calendar API leaked into portable/game-facing code: $path"
    }
}
$native = Get-Content -LiteralPath $nativePath -Raw
if($native -notmatch 'GetLocalTime' -or $native -notmatch 'SYSTEMTIME')
{
    throw "Win32 native calendar adapter does not query the local civil date."
}
if($premake -notmatch 'f144_win32_calendar.c' -or $premake -notmatch 'f144_win32_calendar.h')
{
    throw "Native calendar adapter is absent from the explicit Win32 build target."
}
if($workflow -notmatch 'test_stage4_calendar.ps1')
{
    throw "Full CI workflow is missing the Stage 4E calendar regression gate."
}
Write-Host "STAGE 4E PLATFORM-NEUTRAL CALENDAR BOUNDARY AUDIT: PASS"

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4e-calendar"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4e-calendar"
}
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exe = Join-Path $tempDir "stage4_calendar_tests.exe"
Write-Host "=== BUILD STAGE 4E CALENDAR REGRESSION ==="
$compileArgs = @(
    "/nologo", "/TC", "/std:c11", "/W4", "/WX", "/utf-8",
    (Join-Path $PSScriptRoot "stage4_calendar_tests.c"),
    $platformPath,
    $calendarPath,
    $configPath,
    $nativePath,
    (Join-Path $root "src\f144_win32_startup_config.c"),
    ("/I" + (Join-Path $root "include")),
    "/Fe:$exe"
)
& cl.exe @compileArgs
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4E calendar regression build failed with exit code $LASTEXITCODE."
}
Write-Host "=== RUN STAGE 4E CALENDAR REGRESSION ==="
& $exe
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4E calendar regression failed with exit code $LASTEXITCODE."
}
