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
$encounter = Read-Source "game\src\floppy144_grey_encounter.c"
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
Require $main 'FLOPPY144_SCREEN_GREY_ENCOUNTER' 'Transient encounter screen missing.'
Require $main 'Floppy144GreyEncounterBegin' 'Access does not open the encounter.'
Require $main 'Floppy144GreyEncounterFinished' 'Return cleanup missing.'
Require $main 'global_screen == FLOPPY144_SCREEN_GREY_ENCOUNTER' 'Autosave suppression missing.'
Require $encounter '"GREY DOOR REPUBLIK"' 'Office sign missing.'
Require $encounter 'DEVELOPER_INSPECT_RADIUS 24' 'Developer inspection must use close range.'
Require $encounter 'Floppy144PlayerVisualDraw(' 'Shared player renderer not used.'
Require $encounter 'Floppy144PlayerVisualDrawDeveloper(' 'Developer does not share the vector character construction.'
Require $encounter 'Floppy144GreyEncounterInspectNearby(' 'Developer prompt and target have diverged.'
Require $encounter 'Floppy144GreyEncounterDisplayPercent(' 'Grey Door percentage animation is missing.'
Require $encounter 'Floppy144GreyEncounterPercentFlash(' 'Over-capacity red flash gating missing.'
Require $main 'Floppy144Site2DDrawHeader(' 'Normal shared Site header not used for Grey Door.'
Require $main 'FLOPPY144_SITE_MOVE_STEP_X16' 'Grey Door no longer follows fixed-point Site steps.'
Require $main 'global_grey_encounter.body_style' 'Grey Door ignores the selected operator style.'
Require $main 'FLOPPY144_GREY_BLACKOUT' 'Full-black frame must skip CRT and HUD rendering.'

Require $encounter 'FLOPPY144_GREY_BLACKOUT' 'Blackout phase missing.'
Require $encounter 'return 2000U;' 'Two-second blackout hold missing.'
Require $encounter '"floppy144_run_state.c"' 'Source monitor missing.'
Require $encounter '"You''re not supposed to"' 'Speech above the Developer is missing its opening line.'
Require $encounter '"be able to get in here."' 'Speech above the Developer is missing its final line.'
Require $encounter 'RGB(255,255,255)' 'Developer speech must remain white.'
Require $encounter 'scene->restoration_start+elapsed' 'Restoration gag must rise from the actual saved percentage.'
Require $main 'Floppy144GreyEncounterDisplayPercent(' 'Restore gag is not connected to the actual Site header.'
Require $encounter 'Poster(s,60,245,0U)' 'Concept art must visibly sit on the near desk.'
Require $encounter 'Poster(s,135,245,1U)' 'Second future-game concept sheet is missing.'
Require $encounter 'SourceMonitor(s)' 'Source-code monitor is not composed.'
Require $encounter 'Sandwich(s)' 'Unexplained half-eaten sandwich is missing.'
Require $encounter 'Developer(s,scene)' 'Seated Developer is missing.'
Require $encounter 'static void Glitch(' 'CRT glitch renderer missing.'
Require $encounter 'Floppy144GreyEncounterSaveAllowed' 'Temporary save policy missing.'
Require $tests 'TestGreyEncounter(root)' 'Full encounter test missing.'
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
# S4G-04: enforce absence from EVERY ordinary player-facing summary and
# persistent progression presentation. The one fictionally authored orphan
# record is allowed before discovery and remains the only archive clue.
Require $run 'Floppy144RunStateGreyDoorCompletedAutosavePreferred' 'Finished run can be resurrected by a stale manual checkpoint.'
Require $main 'Floppy144RunStateGreyDoorCompletedAutosavePreferred' 'Recorded-session load path bypasses one-shot reconciliation.'
Require $main 'F144_PERSISTENCE_AUTOSAVE' 'Finished encounter is not checkpointed.'
Require $tests 'TestGreyDoorOneShotLifecycle(root)' 'Full three-state lifecycle regression missing.'
$visualTests = Read-Source "tools\stage3b_cabinet_tests.c"
Require $visualTests 'Floppy144TestGreyDoorNoScar();' 'Pixel-level one-shot restoration regression missing.'
Require $visualTests 'memcmp(before_pixels,after_pixels' 'No byte-for-byte original-wall verification.'
$noMention = @(
    "game\src\floppy144_site_directory.c",
    "game\src\floppy144_notebook_view.c",
    "game\src\floppy144_completion_view.c",
    "game\src\floppy144_profile.c",
    "game\src\floppy144_profile_view.c",
    "game\src\floppy144_credits_view.c",
    "game\src\floppy144_recovery.c",
    "game\src\floppy144_settings_view.c",
    "game\src\floppy144_intro.c"
)
foreach($relative in $noMention) {
    $text = Read-Source $relative
    if($relative -eq "game\src\floppy144_credits_view.c") {
        # Existing studio identity predates this anomaly. It is a company
        # credit, not a new achievement or acknowledgement of this event.
        $text = $text.Replace(
            "GAME DESIGN COMPANY: GREY DOOR REPUBLIC", ""
        )
    }
    if($text -match '(?i)(GREY[ _]DOOR|DR-00-RS-0144|GreyDoor)') {
        throw "Grey Door acknowledgement has leaked into ordinary player-facing content: $relative"
    }
}
if($main -match '(?i)printf\s*\([^;]*GREY[ _]DOOR') {
    throw "The shipping launcher is leaking Grey Door diagnostic output."
}
Write-Host "S4G-04 NON-ACKNOWLEDGEMENT / ONE-SHOT SOURCE AUDIT: PASS"
# S4G-05 release integration assertions. The secret has no native
# implementation dependency: only the established framebuffer, logical
# actions, monotonic Stage 4B tick, and Stage 4E variation service.
Require $tests 'TestStage4GCompleteJourney(root)' 'No complete save/exit/reload seed-matrix journey gate.'
Require $grey 'Floppy144VariationRange(' 'Grey Door seed does not use Stage 4E stateless variation.'
Require $run 'Floppy144RunStateGreyDoorCompletedAutosavePreferred' 'Same-run manual/autosave one-shot guard missing.'
Require $main 'Floppy144TimingAdvance(' 'Grey Door is not ticked by Stage 4B time service.'
Require $main 'Floppy144GreyEncounterAdvance(' 'Temporary scene timing not wired.'
Require $main 'F144_ACTION_INSPECT' 'Developer inspect must use platform-neutral actions.'
Require $main 'Floppy144GreyEncounterDraw(' 'Impossible office not using neutral framebuffer screen dispatch.'
Require $main 'Floppy144SettingsApplyCrtFilter(' 'Ordinary CRT presentation path missing.'
Require $encounter 'Floppy144DrawText(' 'Encounter dialogue must render in software.'
Require $encounter 'Floppy144DrawFillRect(' 'Encounter geometry must render in software.'
foreach($core in @($grey,$encounter)) {
    if($core -match '(?i)(windows\.h|win32|HWND|GetTickCount\s*\(|GetAsyncKeyState\s*\(|rand\s*\(|srand\s*\()') {
        throw 'Platform-specific dependency or uncontrolled randomness leaked into Grey Door Core.'
    }
}
if(($run + $persist) -match 'RESTORATION CAPACITY:\s*144%') {
    throw 'False 144% display leaked into authoritative capacity or persistence source.'
}
# The normal archive's single orphan document clue is intentional. Only the
# existing studio name in Credits is exempt; no further ordinary UI spoilers.
Write-Host "S4G-05 INTEGRATION / ARCHITECTURE / SECRECY SOURCE AUDIT: PASS"

Write-Host "S4G-01 ISOLATION / SCHEMA AUDIT: PASS"
Write-Host "Behavioural round-trip cases run in test_stage4_persistence.ps1 (CI)."
