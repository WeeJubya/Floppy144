$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$core = Join-Path $root "game\src"
$source = Get-Content -LiteralPath (Join-Path $core "floppy144_takeaway.c") -Raw
$cabinet = Get-Content -LiteralPath (Join-Path $core "floppy144_cabinet.c") -Raw
$canonical = Get-Content -LiteralPath (Join-Path $core "floppy144_game_data.generated.inc") -Raw
$runtimeTest = Get-Content -LiteralPath (Join-Path $PSScriptRoot "stage3b_cabinet_tests.c") -Raw
$stage3b = Get-Content -LiteralPath (Join-Path $PSScriptRoot "test_stage3b.ps1") -Raw

Write-Host "=== STAGE 4E TAKEAWAY CORE / GAMEPLAY BOUNDARY AUDIT ==="
if($source -match '(?i)\b(rand|srand|time|GetTickCount|GetLocalTime)\s*\(|#\s*include\s*[<"]windows\.h[>"]')
{
    throw "Takeaway generator must not use platform clock or sequential randomness."
}
if($source -notmatch 'Floppy144VariationRange' -or
   $source -notmatch 'takeaway\.name\.modifier\.v1' -or
   $source -notmatch 'takeaway\.special\.v1' -or
   $source -notmatch 'takeaway\.tagline\.v1')
{
    throw "Takeaway generator is not independently keyed through S4E-01 variation."
}
if($canonical -notmatch 'FLOPPY144_DATA_PHYSICAL_ITEM, "P-330", "Takeaway menu", "STAFF_ROOM", "STAFF_ROOM_NOTICEBOARD"')
{
    throw "Canonical P-330 physical item was removed or altered."
}
foreach($required in @(
    'sGeneratedTakeaway.pszF',
    'Floppy144TakeawayMenuGenerate',
    'Floppy144SitePhysicalItemVisible',
    'Floppy144CabinetVisibleContentAt'
))
{
    if($cabinet -notmatch [regex]::Escape($required))
    {
        throw "Cabinet menu presentation contract missing: $required"
    }
}
foreach($required in @(
    'Floppy144TestTakeawayMenuPresentation',
    'Floppy144PersistenceEncodeRunState',
    'Floppy144PersistenceDecodeRunState',
    'memcmp(&snapshot, &run, sizeof(run))'
))
{
    if($runtimeTest -notmatch [regex]::Escape($required))
    {
        throw "Missing actual generated-data and save/reload menu coverage: $required"
    }
}
if($stage3b -notmatch 'floppy144_takeaway.c')
{
    throw "Stage 3B runtime regression omits the generator translation unit."
}
Write-Host "STAGE 4E TAKEAWAY CORE / GAMEPLAY BOUNDARY AUDIT: PASS"

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4e-takeaway"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4e-takeaway"
}
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4_takeaway_tests.exe"

Write-Host "=== BUILD STAGE 4E TAKEAWAY GENERATOR REGRESSION ==="
$compileArgs = @(
    "/nologo", "/TC", "/std:c11", "/W4", "/WX", "/utf-8",
    (Join-Path $PSScriptRoot "stage4_takeaway_tests.c"),
    (Join-Path $core "floppy144_takeaway.c"),
    (Join-Path $core "floppy144_variation.c"),
    ("/I" + $core),
    "/Fe:$exePath"
)
& cl.exe @compileArgs
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4E takeaway test build failed with exit code $LASTEXITCODE."
}

Write-Host "=== RUN STAGE 4E TAKEAWAY GENERATOR REGRESSION ==="
& $exePath
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4E takeaway regression failed with exit code $LASTEXITCODE."
}
