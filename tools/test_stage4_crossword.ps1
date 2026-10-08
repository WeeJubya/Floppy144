$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$core = Join-Path $root "game\src"
$source = Get-Content -LiteralPath (Join-Path $core "floppy144_crossword.c") -Raw
$cabinet = Get-Content -LiteralPath (Join-Path $core "floppy144_cabinet.c") -Raw
$canonical = Get-Content -LiteralPath (Join-Path $core "floppy144_game_data.generated.inc") -Raw
$runtime = Get-Content -LiteralPath (Join-Path $PSScriptRoot "stage3b_cabinet_tests.c") -Raw
$stage3b = Get-Content -LiteralPath (Join-Path $PSScriptRoot "test_stage3b.ps1") -Raw

Write-Host "=== STAGE 4E CROSSWORD GAMEPLAY BOUNDARY AUDIT ==="
if($source -match '(?i)\b(rand|srand|time|GetTickCount|GetLocalTime)\s*\(|#\s*include\s*[<"]windows\.h[>"]')
{
    throw "Crossword generator must not query platform time or sequential randomness."
}
foreach($required in @(
    'Floppy144VariationRange',
    'staff.crossword.pattern.v1',
    'staff.crossword.across-fill.v1',
    'staff.crossword.down-fill.v1',
    'staff.crossword.scribble.v1'
))
{
    if($source -notmatch [regex]::Escape($required))
    {
        throw "Missing independently keyed crossword variation: $required"
    }
}
if($canonical -notmatch 'FLOPPY144_DATA_PHYSICAL_ITEM, "P-074", "Half-finished crossword", "STAFF_ROOM", "STAFF_ROOM_COFFEE_TABLE_01"' -or
   $canonical -notmatch 'FLOPPY144_DATA_TRIGGER_EFFECT, "T-012", "REVEAL_PHYSICAL_ITEM", "P-074"' -or
   $canonical -notmatch 'FLOPPY144_DATA_RELATIONSHIP, "HR-05", "P-074"')
{
    throw "The original P-074 identity, reveal or HR-05 relationship has changed."
}
foreach($required in @(
    'sGeneratedCrossword',
    'Floppy144CrosswordGenerate',
    'Floppy144SitePhysicalItemVisible',
    'pItem == &pCabinet->sGeneratedCrossword'
))
{
    if($cabinet -notmatch [regex]::Escape($required))
    {
        throw "Crossword UI or progression separation missing: $required"
    }
}
foreach($required in @(
    'Floppy144TestCrosswordPresentation',
    'Floppy144PersistenceEncodeRunState',
    'Floppy144PersistenceDecodeRunState',
    'memcmp(&before, &run, sizeof(run))'
))
{
    if($runtime -notmatch [regex]::Escape($required))
    {
        throw "Missing actual Cabinet/RunState crossword test: $required"
    }
}
if($stage3b -notmatch 'floppy144_crossword.c')
{
    throw "Stage 3B.5 runtime regression omits the crossword source."
}
Write-Host "STAGE 4E CROSSWORD GAMEPLAY BOUNDARY AUDIT: PASS"

$temp = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4e-crossword"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4e-crossword"
}
New-Item -ItemType Directory -Force -Path $temp | Out-Null
$exe = Join-Path $temp "stage4_crossword_tests.exe"

Write-Host "=== BUILD STAGE 4E CROSSWORD TESTS ==="
$compile = @(
    "/nologo", "/TC", "/std:c11", "/W4", "/WX", "/utf-8",
    (Join-Path $PSScriptRoot "stage4_crossword_tests.c"),
    (Join-Path $core "floppy144_crossword.c"),
    (Join-Path $core "floppy144_variation.c"),
    ("/I" + $core),
    "/Fe:$exe"
)
& cl.exe @compile
if($LASTEXITCODE -ne 0)
{
    throw "Crossword strict-warning regression build failed: $LASTEXITCODE."
}
Write-Host "=== RUN STAGE 4E CROSSWORD TESTS ==="
& $exe
if($LASTEXITCODE -ne 0)
{
    throw "Crossword regression failed: $LASTEXITCODE."
}
