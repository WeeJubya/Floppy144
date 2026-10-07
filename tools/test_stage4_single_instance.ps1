$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$mainPath = Join-Path $root "game\src\floppy144_main.c"
$singlePath = Join-Path $root "src\f144_win32_single_instance.c"
$testSource = Join-Path $PSScriptRoot "stage4_single_instance_tests.c"

Write-Host "=== STAGE 4 SINGLE-INSTANCE BOUNDARY AUDIT ==="

$mainSource = Get-Content -LiteralPath $mainPath -Raw
$singleSource = Get-Content -LiteralPath $singlePath -Raw

foreach($forbidden in @(
    'CreateMutexA',
    'ReleaseMutex',
    'CloseHandle\s*\(',
    'Global\\\\Floppy144\.Instance'
))
{
    if($mainSource -match $forbidden)
    {
        throw "Game coordinator contains Win32 single-instance implementation detail: $forbidden"
    }
}

foreach($required in @(
    'CreateMutexA',
    'ERROR_ALREADY_EXISTS',
    'ReleaseMutex',
    'CloseHandle',
    'SHGetFolderPathA',
    'CSIDL_APPDATA',
    'Global\\Floppy144.Instance'
))
{
    if($singleSource -notmatch [regex]::Escape($required))
    {
        throw "Win32 single-instance implementation is missing: $required"
    }
}

$acquireIndex = $mainSource.IndexOf('f144Win32SingleInstanceAcquire')
$debugIndex = $mainSource.IndexOf('f144Win32StartupConfigFromCommandLine')
$storageIndex = $mainSource.IndexOf('Floppy144StorageResolve')

if($acquireIndex -lt 0) {
    throw "WinMain does not acquire single-instance ownership."
}

if(
    $storageIndex -lt 0 -or
    $acquireIndex -gt $storageIndex
)
{
    throw "Single-instance ownership must be acquired before persistence resolution."
}

if(
    $debugIndex -lt 0 -or
    $acquireIndex -gt $debugIndex
)
{
    throw "Platform configuration acquisition must not bypass single-instance protection."
}

foreach($required in @(
    'F144_WIN32_SINGLE_INSTANCE_ALREADY_RUNNING',
    'already running for this Windows profile',
    'f144Win32SingleInstanceRelease'
))
{
    if($mainSource -notmatch [regex]::Escape($required))
    {
        throw "WinMain single-instance UX/lifecycle wiring is missing: $required"
    }
}

Write-Host "STAGE 4 SINGLE-INSTANCE BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4 SINGLE-INSTANCE REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-single-instance"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-single-instance"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4_single_instance_tests.exe"
$includePlatform = "/I" + (Join-Path $root "include")

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    $testSource,
    (Join-Path $root "src\f144_win32_single_instance.c"),
    $includePlatform,
    "/Fe$exePath",
    "/link",
    "shell32.lib"
)

& cl.exe @compileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 single-instance regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4 SINGLE-INSTANCE REGRESSION ==="

& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 single-instance regression failed with exit code $LASTEXITCODE."
}
