$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$core = Join-Path $root "game\src"
$source = Get-Content -LiteralPath (Join-Path $core "floppy144_variation.c") -Raw
$persistence = Get-Content -LiteralPath (Join-Path $core "floppy144_persistence.c") -Raw

Write-Host "=== STAGE 4E PORTABLE VARIATION AUDIT ==="
if($source -match '(?i)\b(rand|srand|time|GetTickCount|QueryPerformanceCounter)\s*\(|#\s*include\s*[<"]windows\.h[>"]')
{
    throw "Stage 4E variation depends on OS/runtime random or timing facilities."
}
if($persistence -notmatch 'Floppy144PersistenceWriteU32\(&payload\[offset\],state->recovery_seed\)' -or
   $persistence -notmatch 'decoded\.recovery_seed=Floppy144PersistenceReadU32\(&payload\[offset\]\)')
{
    throw "Existing run-seed persistence contract changed unexpectedly."
}
Write-Host "STAGE 4E PORTABLE VARIATION AUDIT: PASS"

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4e-variation"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4e-variation"
}
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4_variation_tests.exe"

Write-Host "=== BUILD STAGE 4E VARIATION REGRESSION ==="
$compileArgs = @(
    "/nologo", "/TC", "/std:c11", "/W4", "/WX", "/utf-8",
    (Join-Path $PSScriptRoot "stage4_variation_tests.c"),
    (Join-Path $core "floppy144_variation.c"),
    (Join-Path $root "src\f144_startup_config.c"),
    ("/I" + $core),
    ("/I" + (Join-Path $root "include")),
    "/Fe:$exePath"
)
& cl.exe @compileArgs
if($LASTEXITCODE -ne 0) {
    throw "Stage 4E variation build failed with exit code $LASTEXITCODE."
}

Write-Host "=== RUN STAGE 4E VARIATION REGRESSION ==="
& $exePath
if($LASTEXITCODE -ne 0) {
    throw "Stage 4E variation tests failed with exit code $LASTEXITCODE."
}
