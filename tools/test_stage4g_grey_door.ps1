$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
function Read-Source([string]$relative) {
    Get-Content -LiteralPath (Join-Path $root $relative) -Raw
}
function Require([string]$text,[string]$needle,[string]$message) {
    if(-not $text.Contains($needle)) { throw $message }
}
Write-Host "=== S4G-01 ORPHAN RECORD / SAVE-SCHEMA ISOLATION ==="

$doc = Read-Source "game\src\floppy144_document.c"
$grey = Read-Source "game\src\floppy144_grey_door.c"
$site2d = Read-Source "game\src\floppy144_site_2d.c"
$main = Read-Source "game\src\floppy144_main.c"
$header = Read-Source "game\src\floppy144_document.h"
$run = Read-Source "game\src\floppy144_run_state.c"
$runHeader = Read-Source "game\src\floppy144_run_state.h"
$catalogue = Read-Source "game\src\floppy144_catalogue.c"
$terminal = Read-Source "game\src\floppy144_terminal.c"
$persist = Read-Source "game\src\floppy144_persistence.c"
$persistHeader = Read-Source "game\src\floppy144_persistence.h"
$tests = Read-Source "tools\stage4_persistence_paths_tests.c"
$canonical = Read-Source "data\floppy144_game_data.json"

Require $header 'DR-00-RS-0144' 'Orphan authored record ID is missing.'
Require $header 'FLOPPY144_COLLECTION_COUNT' 'Orphan record is no longer outside normal collections.'
Require $doc 'Unallocated Floor Area Notice' 'In-world authored record is missing.'
Require $doc 'Floppy144RunStateGreyDoorDiscover' 'Reading the record does not enable the one-shot state.'
Require $doc 'bypass generic evidence' 'Grey Door reading may touch normal evidence.'
Require $catalogue 'FLOPPY144_GREY_DOOR_RECORD_COLLECTION' 'Ordinary catalogue viewer does not accept orphan records.'
Require $terminal 'DR-01' 'Discoverable archive index link is missing.'
Require $terminal 'FLOPPY144_GREY_DOOR_RECORD_INDEX' 'DR-01 LIST has no orphan pointer.'
Require $terminal 'No post-OPEN recovery breadcrumb' 'The secret record must not announce an unlock.'
Require $runHeader 'FLOPPY144_GREY_DOOR_COMPLETED' 'Missing completed state.'
Require $run 'Floppy144RunStateGreyDoorComplete' 'Door cannot be permanently consumed.'
Require $grey '"GREY_DOOR", "CORRIDOR_WALL_V1"' 'Missing independent placement namespace.'
Require $grey 'Floppy144GreyDoorCandidateSafe' 'Exhaustive candidate filtering missing.'
Require $site2d 'Floppy144Site2DDrawGreyDoor' 'The site overlay is missing.'
Require $main 'Floppy144GreyDoorNearby' 'Normal Site input is not wired.'
Require $tests 'TestGreyDoorPlacement(root)' 'No all-candidates/seed/reload geometry test.'
Require $persistHeader 'FLOPPY144_SAVE_VERSION_V2' 'Legacy V2 save format is not recognised.'
Require $persistHeader 'FLOPPY144_SAVE_PAYLOAD_V3_SIZE' 'No V3 byte for hidden state.'
Require $persist 'decoded.grey_door_state=payload[offset++]' 'V3 decoder does not preserve the one-shot state.'
Require $tests 'TestGreyDoorDiscovery(root)' 'Headless one-shot / save / stats regression not scheduled.'
if($canonical.Contains('DR-00-RS-0144')) {
    throw 'The orphan was added to normal generated collection accounting.'
}
if($run -match '(?i)\b(rand|srand|time|GetTickCount)\s*\(') {
    throw 'Grey Door run-state logic must not use a sequential random generator.'
}
Write-Host "S4G-01 ISOLATION / SCHEMA AUDIT: PASS"
Write-Host "Behavioural round-trip cases run in test_stage4_persistence.ps1 (CI)."
