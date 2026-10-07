$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$mainPath = Join-Path $root "game\src\floppy144_main.c"
$recoveryPath = Join-Path $root "game\src\floppy144_recovery.c"

$main = Get-Content -LiteralPath $mainPath -Raw
$recovery = Get-Content -LiteralPath $recoveryPath -Raw

Write-Host "=== STAGE 4C RESTORED-SESSION PRESENTATION AUDIT ==="

foreach($required in @(
    '"GDR SESSION CONTROL SYSTEM"',
    '"GOVERNMENT DEPARTMENT OF RECORDS"',
    '"SITE RECONSTRUCTION ENVIRONMENT"',
    '"SESSION RESTORED"',
    '"RECORDED RECOVERY STATE REINSTATED"',
    '"RECOVERY STATUS AVAILABLE AFTER ACKNOWLEDGEMENT"',
    '"PRESS ANY KEY TO CONTINUE"',
    '"RECOVERY SEED: %u"'
))
{
    if($recovery -notmatch [regex]::Escape($required))
    {
        throw "Restored-session/menu-family presentation is missing: $required"
    }
}

foreach($required in @(
    '48U,\s*164U,\s*544U,\s*154U',
    '56U,\s*174U,\s*528U,\s*24U',
    '56U,\s*254U,\s*528U,\s*1U'
))
{
    if($recovery -notmatch $required)
    {
        throw "Restored-session presentation no longer shares the main-menu inner geometry: $required"
    }
}

if(
    $recovery -notmatch
        '(?s)if\(reinstate_confirmation\).*?panel_dark.*?border.*?panel.*?green'
)
{
    throw "Restored-session presentation no longer reuses the established menu palette/framing."
}

$reinstateStart = $main.IndexOf("case FLOPPY144_MAIN_MENU_REINSTATE_SESSION:")
$reinstateEnd = $main.IndexOf("case FLOPPY144_MAIN_MENU_OPERATOR_PROFILE:",$reinstateStart)

if($reinstateStart -lt 0 -or $reinstateEnd -le $reinstateStart)
{
    throw "Could not isolate the successful/failed reinstate coordinator flow."
}

$reinstate = $main.Substring($reinstateStart,$reinstateEnd-$reinstateStart)

foreach($required in @(
    'Floppy144PersistenceLoadRunState',
    'global_recorded_session_is_autosave',
    'F144_PERSISTENCE_AUTOSAVE',
    'F144_PERSISTENCE_MANUAL_SAVE',
    'Floppy144WorldHydrateFromRunState',
    'global_terminal.suppress_next_character',
    'global_resume_screen',
    'FLOPPY144_SCREEN_TERMINAL',
    'global_reinstate_confirmation_pending',
    'global_persistence_warnings'
))
{
    if($reinstate -notmatch [regex]::Escape($required))
    {
        throw "Reinstate semantics drifted while changing presentation: $required"
    }
}

if(
    $reinstate -notmatch
        'global_recorded_session_available\s*=\s*false' -or
    $reinstate -notmatch
        'FLOPPY144_PERSISTENCE_WARNING_SAVE'
)
{
    throw "Invalid/corrupt-save failure behaviour is no longer preserved."
}

if(
    $recovery -notmatch
        '(?s)case FLOPPY144_MAIN_MENU_REINSTATE_SESSION:\s*\{\s*return recorded_session_available;'
)
{
    throw "No-save behaviour changed: Reinstate must still depend solely on recorded-session availability."
}

if(
    $main -notmatch
        '(?s)global_reinstate_confirmation_pending\s*=\s*false;.*?global_reinstate_continue_on_keyup\s*=\s*true;.*?global_reinstate_continue_key\s*=\s*pEvent->physical_token;.*?UpdateWindow\s*\(\s*window\s*\)'
)
{
    throw "Key-down acknowledgement no longer paints the restored-percentage frame."
}

if(
    $main -notmatch
        '(?s)pEvent->physical_token\s*==\s*global_reinstate_continue_key.*?global_screen\s*=\s*global_resume_screen'
)
{
    throw "Matching key-up no longer continues to the restored gameplay screen."
}

if(
    $recovery -match 'Floppy144Persistence' -or
    $recovery -match 'Floppy144WorldHydrate' -or
    $recovery -match 'autosave' -or
    $recovery -match 'DiscoveryProfile'
)
{
    throw "Presentation renderer has acquired persistence/game-state mutation responsibilities."
}

Write-Host "STAGE 4C RESTORED-SESSION PRESENTATION AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4C RESTORED-SESSION RENDERER REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-reinstate-presentation"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-reinstate-presentation"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exe = Join-Path $tempDir "stage4_reinstate_presentation_tests.exe"

$args = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    (Join-Path $PSScriptRoot "stage4_reinstate_presentation_tests.c"),
    (Join-Path $root "game\src\floppy144_recovery.c"),
    (Join-Path $root "game\src\floppy144_draw.c"),
    ("/I"+(Join-Path $root "include")),
    ("/I"+(Join-Path $root "game\src")),
    ("/Fe"+$exe)
)

& cl.exe @args
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C restored-session presentation regression build failed."
}

& $exe
if($LASTEXITCODE -ne 0)
{
    throw "Stage 4C restored-session presentation regression failed."
}
