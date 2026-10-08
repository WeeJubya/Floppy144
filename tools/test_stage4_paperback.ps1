$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$core = Join-Path $root "game\src"
$source = Get-Content -LiteralPath (Join-Path $core "floppy144_paperback.c") -Raw
$cabinet = Get-Content -LiteralPath (Join-Path $core "floppy144_cabinet.c") -Raw
$canonical = Get-Content -LiteralPath (Join-Path $core "floppy144_game_data.generated.inc") -Raw
$runtime = Get-Content -LiteralPath (Join-Path $PSScriptRoot "stage3b_cabinet_tests.c") -Raw
$stage3b = Get-Content -LiteralPath (Join-Path $PSScriptRoot "test_stage3b.ps1") -Raw

Write-Host "=== STAGE 4E PAPERBACK GAMEPLAY / PLATFORM BOUNDARY AUDIT ==="
if($source -match '(?i)\b(rand|srand|time|GetTickCount|GetLocalTime)\s*\(|#\s*include\s*[<"]windows\.h[>"]')
{
    throw "Paperback generator must not query platform time or sequential randomness."
}
foreach($required in @(
    'Floppy144VariationRange',
    'staff.paperback.form.v1',
    'staff.paperback.fragment.first.v1',
    'staff.paperback.fragment.second.v1',
    'staff.paperback.author.given.v1',
    'staff.paperback.author.family.v1',
    'staff.paperback.marginalia.v1',
    'Floppy144PaperbackThreeLinesFit'
))
{
    if($source -notmatch [regex]::Escape($required))
    {
        throw "Missing seed-keyed/line-safe paperback behaviour: $required"
    }
}
foreach($marker in @(
    'FLOPPY144_DATA_PHYSICAL_ITEM, "P-073", "Dog-eared paperback", "STAFF_ROOM", "STAFF_ROOM_BOOKCASE_02"',
    'FLOPPY144_DATA_TRIGGER_EFFECT, "T-012", "REVEAL_PHYSICAL_ITEM", "P-073"',
    'FLOPPY144_DATA_RELATIONSHIP, "HR-05", "P-073"'
))
{
    if($canonical -notmatch [regex]::Escape($marker))
    {
        throw "Canonical paperback or Stage 3 reveal was modified: $marker"
    }
}
foreach($marker in @(
    'sGeneratedPaperback',
    'Floppy144PaperbackGenerate',
    'Floppy144SitePhysicalItemVisible',
    'pWinner->pszId'
))
{
    if($cabinet -notmatch [regex]::Escape($marker))
    {
        throw "Cabinet integration or original PI visibility is missing: $marker"
    }
}
foreach($marker in @(
    'Floppy144TestPaperbackPresentation',
    'Floppy144PersistenceEncodeRunState',
    'Floppy144PersistenceDecodeRunState',
    'memcmp(&before, &run, sizeof(run))'
))
{
    if($runtime -notmatch [regex]::Escape($marker))
    {
        throw "Actual Cabinet/save-reload regression is missing: $marker"
    }
}
if($stage3b -notmatch 'floppy144_paperback.c')
{
    throw "Stage 3B.5 test harness does not link the paperback generator."
}
Write-Host "STAGE 4E PAPERBACK GAMEPLAY / PLATFORM BOUNDARY AUDIT: PASS"

$temp = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4e-paperback"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4e-paperback"
}
New-Item -ItemType Directory -Force -Path $temp | Out-Null
$exe = Join-Path $temp "stage4_paperback_tests.exe"

Write-Host "=== BUILD STAGE 4E PAPERBACK REGRESSION ==="
$compile = @(
    "/nologo", "/TC", "/std:c11", "/W4", "/WX", "/utf-8",
    (Join-Path $PSScriptRoot "stage4_paperback_tests.c"),
    (Join-Path $core "floppy144_paperback.c"),
    (Join-Path $core "floppy144_variation.c"),
    ("/I" + $core),
    "/Fe:$exe"
)
& cl.exe @compile
if($LASTEXITCODE -ne 0)
{
    throw "Paperback strict-warning regression build failed: $LASTEXITCODE."
}
Write-Host "=== RUN STAGE 4E PAPERBACK REGRESSION ==="
& $exe
if($LASTEXITCODE -ne 0)
{
    throw "Paperback regression failed: $LASTEXITCODE."
}
