$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$core = Join-Path $root "game\src"
function Read-F144File([string]$path)
{
    Get-Content -LiteralPath (Join-Path $root $path) -Raw
}
function Require-Text([string]$content, [string]$literal, [string]$reason)
{
    if(-not $content.Contains($literal)) { throw $reason }
}

Write-Host "=== S4E-10 DETERMINISM / REPLAY / ARCHITECTURE GATE ==="

$variation = Read-F144File "game\src\floppy144_variation.c"
$document = Read-F144File "game\src\floppy144_document.c"
$catalogue = Read-F144File "game\src\floppy144_catalogue.c"
$terminal = Read-F144File "game\src\floppy144_terminal.c"
$cabinet = Read-F144File "game\src\floppy144_cabinet.c"
$reconstruction = Read-F144File "tools\stage3b_reconstruction_tests.c"
$terminalTests = Read-F144File "tools\stage3b_terminal_tests.c"
$cabinetTests = Read-F144File "tools\stage3b_cabinet_tests.c"
$calendar = Read-F144File "src\f144_calendar.c"
$persistence = Read-F144File "game\src\floppy144_persistence.c"
$workflow = Read-F144File ".github\workflows\stage3c-ci.yml"
$decision = Read-F144File "docs\stage4\stage4e_optional_early_collections_decision.md"
$authored = Get-Content -LiteralPath (Join-Path $root "data\floppy144_game_data.json") -Raw |
    ConvertFrom-Json

$portable = @(
    "game\src\floppy144_variation.c",
    "game\src\floppy144_noticeboard.c",
    "game\src\floppy144_takeaway.c",
    "game\src\floppy144_crossword.c",
    "game\src\floppy144_paperback.c",
    "game\src\floppy144_document.c",
    "game\src\floppy144_catalogue.c",
    "game\src\floppy144_terminal.c",
    "src\f144_calendar.c"
)
$forbidden = '(?i)#\s*include\s*[<"]windows\.h[>"]|\b(rand|srand|time|localtime|gmtime|GetLocalTime|GetTickCount|QueryPerformanceCounter)\s*\('
foreach($path in $portable)
{
    if((Read-F144File $path) -match $forbidden)
    {
        throw "Non-deterministic clock/PRNG or direct Win32 dependency in $path."
    }
}
Require-Text $variation 'FNV-1a' 'Stateless variation algorithm lost its documented hash contract.'
Require-Text $document 'dr04.workstream-swap.v1' 'Seeded DR-04 arrangement namespace missing.'
Require-Text $document 'Floppy144DocumentDr04Partner' 'DR-04 mapping is not keyed by authored trigger identity.'
Require-Text $document 'Floppy144DocumentGetForSeed' 'Seed-dependent authored document resolver missing.'
if($document -match 'DR-04-RS-(0216|0147|0063|0087)')
{
    throw "Core hardcodes a DR-04 generated/obsolete record slot."
}
foreach($required in @(
    'Floppy144CatalogueBuildRecordForSeed',
    'Floppy144DocumentGetForSeed'
))
{
    Require-Text $catalogue $required "Catalogue missing seeded presentation: $required"
}
foreach($required in @(
    'Floppy144DocumentRecordIdForSeed',
    'Floppy144DocumentGetForSeed',
    'Floppy144CatalogueBuildRecordForSeed',
    'run_state->recovery_seed'
))
{
    Require-Text $terminal $required "Terminal missing seeded document contract: $required"
}
Require-Text $persistence 'state->recovery_seed' 'Persisted run seed was removed.'
Require-Text $persistence 'decoded.recovery_seed' 'Save decode no longer restores run seed.'
Require-Text $calendar 'f144StartupConfigDebugEnabled(config)' 'Fixed-date override no longer requires developer mode.'
Require-Text $calendar 'f144StartupConfigFixedDateOverride(config' 'Portable fixed-date override was removed.'
Require-Text $cabinet 'Floppy144NoticeboardSelect' 'Contextual noticeboard lost date-aware selection.'

if(@($authored.collections).Count -ne 35 -or @($authored.documents).Count -ne 170)
{
    throw "S4E-08 canonical authored-document/collection identity count changed."
}
foreach($required in @(
    '144U, 146U',
    'Floppy144DocumentSlotForSeed',
    'Floppy144DocumentApplyEffects',
    'Floppy144PersistenceEncodeRunState',
    'Floppy144PersistenceDecodeRunState',
    'Floppy144TestTerminalContains'
))
{
    Require-Text $terminalTests $required "S4E-07 A/B/reload regression missing: $required"
}
foreach($required in @(
    'g_recoveryFixtureSeed = 146U',
    'Floppy144TestOpenDr04Workstream',
    'Floppy144TestCanonicalRoomProgression'
))
{
    Require-Text $reconstruction $required "Full-site dual-seed document route missing: $required"
}
foreach($date in @(
    '2026U, 1U, 15U', '2026U, 4U, 21U',
    '2026U, 7U, 15U', '2026U, 9U, 15U',
    '2026U, 10U, 31U', '2026U, 11U, 5U',
    '2026U, 12U, 20U'
))
{
    Require-Text $cabinetTests $date "Stage 4E date matrix missing: $date"
}
Require-Text $decision 'NO ADDITIONAL COLLECTIONS' 'S4E-09 conditional decision missing.'
foreach($step in @(
    'test_stage2.ps1', 'test_stage3a.ps1', 'test_stage3b.ps1',
    'test_stage4b_integration.ps1', 'test_stage4c_integration.ps1',
    'test_stage4d_integration.ps1', 'test_stage4_archive.ps1',
    'test_stage4e_integration.ps1',
    'Stage 4E reserve for Stage 4F and final QA'
))
{
    Require-Text $workflow $step "Production CI omitted inherited/Stage 4E check: $step"
}
Write-Host "S4E-10 SOURCE / MATRIX / ARCHITECTURE AUDIT: PASS"

$temp = if($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { [System.IO.Path]::GetTempPath() }
$dir = Join-Path $temp "floppy144-stage4e-integration"
New-Item -ItemType Directory -Force -Path $dir | Out-Null
$exe = Join-Path $dir "stage4e_integration_tests.exe"
& cl.exe @(
    "/nologo", "/TC", "/std:c11", "/W4", "/WX", "/utf-8",
    (Join-Path $PSScriptRoot "stage4e_integration_tests.c"),
    (Join-Path $core "floppy144_variation.c"),
    (Join-Path $core "floppy144_takeaway.c"),
    (Join-Path $core "floppy144_crossword.c"),
    (Join-Path $core "floppy144_paperback.c"),
    ("/I" + $core),
    ("/Fe:$exe")
)
if($LASTEXITCODE -ne 0) { throw "S4E-10 golden matrix compile failed." }
& $exe
if($LASTEXITCODE -ne 0) { throw "S4E-10 golden matrix regression failed." }
Write-Host "STAGE 4E INTEGRATION GATE: PASS"
