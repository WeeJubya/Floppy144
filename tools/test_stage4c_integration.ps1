$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$sourceDir = Join-Path $root "game\src"

function Read-Source([string]$relative)
{
    return Get-Content -LiteralPath (Join-Path $root $relative) -Raw
}

$main = Read-Source "game\src\floppy144_main.c"
$profile = Read-Source "game\src\floppy144_profile.c"
$profileH = Read-Source "game\src\floppy144_profile.h"
$profileEdit = Read-Source "game\src\floppy144_profile_edit.c"
$profileEditH = Read-Source "game\src\floppy144_profile_edit.h"
$profileView = Read-Source "game\src\floppy144_profile_view.c"
$settings = Read-Source "game\src\floppy144_settings.c"
$settingsH = Read-Source "game\src\floppy144_settings.h"
$settingsRuntime = Read-Source "game\src\floppy144_settings_runtime.c"
$settingsView = Read-Source "game\src\floppy144_settings_view.c"
$terminal = Read-Source "game\src\floppy144_terminal.c"
$terminalH = Read-Source "game\src\floppy144_terminal.h"
$credits = Read-Source "game\src\floppy144_credits_view.c"
$completion = Read-Source "game\src\floppy144_completion_view.c"
$completionH = Read-Source "game\src\floppy144_completion_view.h"
$recovery = Read-Source "game\src\floppy144_recovery.c"
$persistence = Read-Source "game\src\floppy144_persistence.c"
$runStateH = Read-Source "game\src\floppy144_run_state.h"
$workflow = Read-Source ".github\workflows\stage3c-ci.yml"

Write-Host "=== STAGE 4C INTEGRATION / ARCHITECTURE AUDIT ==="

$portableUiSources = @{
    "profile.c" = $profile
    "profile.h" = $profileH
    "profile_edit.c" = $profileEdit
    "profile_edit.h" = $profileEditH
    "profile_view.c" = $profileView
    "settings.c" = $settings
    "settings.h" = $settingsH
    "settings_runtime.c" = $settingsRuntime
    "settings_view.c" = $settingsView
    "terminal.c" = $terminal
    "terminal.h" = $terminalH
    "credits_view.c" = $credits
    "completion_view.c" = $completion
    "completion_view.h" = $completionH
    "recovery.c" = $recovery
}

$win32LeakPattern = '(?i)#\s*include\s*[<"]windows\.h[>"]|\bHWND\b|\bGetTickCount\w*\b|\bCreateFile\w*\b|\bReadFile\b|\bWriteFile\b|\bMoveFileEx\w*\b|\bDeleteFile\w*\b|\bGetFileAttributes\w*\b|\bPlaySound\w*\b|\bwaveOut\w*\b|\bmidiOut\w*\b'

foreach($entry in $portableUiSources.GetEnumerator())
{
    if($entry.Value -match $win32LeakPattern)
    {
        throw "Stage 4C portable UI acquired a direct Win32 dependency: $($entry.Key)"
    }
}

$stage4cImplementation = ($portableUiSources.Values -join [Environment]::NewLine)

if($stage4cImplementation -match '(?im)\b(TODO|FIXME|HACK)\b')
{
    throw "Stage 4C implementation contains a TODO/FIXME/HACK marker."
}

if($stage4cImplementation -match '(?i)\b(strcpy|strcat|sprintf|vsprintf|gets)\s*\(')
{
    throw "Stage 4C implementation contains an unsafe unbounded C string call."
}

foreach($required in @(
    'Floppy144DiscoveryProfileSetOperatorName',
    'Floppy144DiscoveryProfileSetBodyStyle',
    'Floppy144DiscoveryProfileBeginRecovery',
    'Floppy144DiscoveryProfileRecordCompletion',
    'latest_completion_evidence',
    'completed_recoveries'
))
{
    if($profile -notmatch [regex]::Escape($required) -and
       $profileH -notmatch [regex]::Escape($required))
    {
        throw "Profile/history contract is missing: $required"
    }
}

if($profileEdit -match 'Floppy144Persistence' -or
   $profileEditH -match 'Floppy144Persistence')
{
    throw "Transient name editor acquired persistence ownership."
}

if($terminal -match '(?i)floppy144_profile\.h|Floppy144DiscoveryProfile' -or
   $terminalH -match '(?i)floppy144_profile\.h|Floppy144DiscoveryProfile' -or
   $terminal -match 'Floppy144Persistence')
{
    throw "Terminal identity/personalisation leaked across its Profile/persistence boundary."
}

foreach($required in @(
    'global_terminal_authentication_complete',
    'Floppy144TerminalApplyOperatorIdentity',
    'Floppy144DiscoveryProfileOperatorName'
))
{
    if($main -notmatch [regex]::Escape($required))
    {
        throw "Terminal personalisation coordinator wiring is missing: $required"
    }
}

if($runStateH -match '(?i)terminal_authentication|operator_name' -or
   $persistence -match 'terminal_authentication_complete')
{
    throw "Transient terminal authentication leaked into RunState persistence."
}

foreach($required in @(
    'Floppy144SettingsApplyCrtFilter',
    'Floppy144SettingsTextElapsedMs',
    'f144PlatformSetMusicVolume',
    'f144PlatformSetSfxVolume',
    'Floppy144TimingSetAutosaveInterval'
))
{
    if($main -notmatch [regex]::Escape($required))
    {
        throw "Visible Settings control is not wired to its runtime consumer: $required"
    }
}

foreach($required in @(
    'FLOPPY144_SETTINGS_DEFAULT_CRT_MODE',
    'FLOPPY144_SETTINGS_DEFAULT_TEXT_SPEED',
    'FLOPPY144_SETTINGS_DEFAULT_MUSIC_VOLUME',
    'FLOPPY144_SETTINGS_DEFAULT_SFX_VOLUME',
    'FLOPPY144_SETTINGS_DEFAULT_AUTOSAVE_MODE'
))
{
    if($settingsH -notmatch [regex]::Escape($required))
    {
        throw "Settings default contract is missing: $required"
    }
}

if($settingsView -notmatch [regex]::Escape(
    'UP/DOWN SELECT   LEFT/RIGHT CHANGE   ENTER CHANGE/OPEN   BACKSPACE BACK'
))
{
    throw "Settings footer no longer describes both stepped values and Credits opening."
}

if(
    $main -notmatch '(?s)global_settings_option\s*==\s*FLOPPY144_SETTINGS_OPTION_CREDITS.*?global_credits_return_screen\s*=\s*FLOPPY144_SCREEN_SETTINGS' -or
    $main -notmatch '(?s)FLOPPY144_COMPLETION_OPTION_CREDITS.*?global_credits_return_screen\s*=\s*FLOPPY144_SCREEN_COMPLETION' -or
    $main -notmatch '(?s)case FLOPPY144_SCREEN_CREDITS:.*?global_credits_return_screen\s*==\s*FLOPPY144_SCREEN_COMPLETION'
)
{
    throw "Credits parent-screen navigation is inconsistent."
}

foreach($required in @(
    'BACKSPACE  BACK',
    'GREY DOOR REPUBLIC',
    'WEEJUBYA',
    'RIVER2D-DERIVED',
    'GNU GENERAL PUBLIC LICENSE VERSION 3'
))
{
    if($credits -notmatch [regex]::Escape($required))
    {
        throw "Credits presentation/provenance contract is missing: $required"
    }
}

if($credits -match '(?i)PAGE\s+[0-9]|SCROLL')
{
    throw "Credits unexpectedly acquired paging/scroll text despite remaining a one-page view."
}

foreach($required in @(
    '"SESSION RESTORED"',
    '"PRESS ANY KEY TO CONTINUE"',
    'global_reinstate_confirmation_pending',
    'global_reinstate_continue_on_keyup',
    'global_reinstate_continue_key'
))
{
    if($main -notmatch [regex]::Escape($required) -and
       $recovery -notmatch [regex]::Escape($required))
    {
        throw "Restored-session contract is missing: $required"
    }
}

$completionRecordCalls = [regex]::Matches(
    $main,
    'Floppy144DiscoveryProfileRecordCompletion\s*\('
).Count

if($completionRecordCalls -ne 1)
{
    throw "Completion history must be recorded at exactly one coordinator site; found $completionRecordCalls."
}

if($completion -match 'Floppy144Persistence' -or
   $completion -match 'Floppy144DiscoveryProfileRecordCompletion' -or
   $completion -match 'Floppy144DiscoveryProfileBeginRecovery')
{
    throw "Completion view acquired persistence/profile-history mutation responsibilities."
}

foreach($required in @(
    'FLOPPY144_COMPLETION_OPTION_FINAL_NOTE',
    'FLOPPY144_COMPLETION_OPTION_CREDITS',
    'FLOPPY144_COMPLETION_OPTION_MAIN_MENU',
    'Core resolution',
    'FINAL: [ CORE RESOLUTION NOT ESTABLISHED ]'
))
{
    if($completion -notmatch [regex]::Escape($required) -and
       $completionH -notmatch [regex]::Escape($required))
    {
        throw "Completion presentation contract is missing: $required"
    }
}

if(
    $main -notmatch '(?s)static void Floppy144AutosaveIfNeeded\(.*?!global_session_active\s*\|\|\s*global_run_state\.dirty\s*==\s*0U' -or
    $main -notmatch '(?s)case FLOPPY144_MAIN_MENU_RECORD_SESSION:.*?global_session_active\s*&&\s*Floppy144PersistenceSaveRunState'
)
{
    throw "Post-completion autosave/manual-save restrictions drifted."
}

foreach($requiredStep in @(
    'Stage 2 regression',
    'Stage 3A regression',
    'Stage 3B regression',
    'Stage 4 platform boundary',
    'Stage 4 logical input',
    'Stage 4 persistence paths',
    'Stage 4 audio contract',
    'Stage 4 timing and lifecycle',
    'Stage 4 single-instance protection',
    'Stage 4 developer configuration',
    'Stage 4C operator profile screen',
    'Stage 4C operator name entry',
    'Stage 4C terminal authentication',
    'Stage 4C persistent settings',
    'Stage 4C credits and attribution',
    'Stage 4C restored-session presentation',
    'Stage 4C bespoke completion presentation',
    'Stage 4 build architecture',
    'Stage 4B integration sign-off audit',
    'Stage 4C integration sign-off audit'
))
{
    if($workflow -notmatch [regex]::Escape($requiredStep))
    {
        throw "Stage 4C sign-off workflow is missing required gate: $requiredStep"
    }
}

Write-Host "STAGE 4C INTEGRATION / ARCHITECTURE AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4C FRESH-PROFILE INTEGRATION SMOKE ==="

$tempDir =
    if($env:RUNNER_TEMP)
    {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4c-integration"
    }
    else
    {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4c-integration"
    }

if(Test-Path -LiteralPath $tempDir)
{
    Remove-Item -LiteralPath $tempDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

$includePlatform = "/I" + (Join-Path $root "include")
$includeGame = "/I" + $sourceDir

$sources = @(
    (Join-Path $PSScriptRoot "stage4c_integration_smoke_tests.c"),
    (Join-Path $sourceDir "floppy144_game_data.c"),
    (Join-Path $sourceDir "floppy144_trigger_engine.c"),
    (Join-Path $sourceDir "floppy144_interaction_engine.c"),
    (Join-Path $sourceDir "floppy144_run_state.c"),
    (Join-Path $sourceDir "floppy144_world.c"),
    (Join-Path $sourceDir "floppy144_terminal.c"),
    (Join-Path $sourceDir "floppy144_recovery.c"),
    (Join-Path $sourceDir "floppy144_catalogue.c"),
    (Join-Path $sourceDir "floppy144_document.c"),
    (Join-Path $sourceDir "floppy144_effect.c"),
    (Join-Path $sourceDir "floppy144_draw.c"),
    (Join-Path $sourceDir "floppy144_site.c"),
    (Join-Path $sourceDir "floppy144_site_rooms.c"),
    (Join-Path $sourceDir "floppy144_site_object.c"),
    (Join-Path $sourceDir "floppy144_object_registry.c"),
    (Join-Path $sourceDir "floppy144_collection_registry.c"),
    (Join-Path $sourceDir "floppy144_profile.c"),
    (Join-Path $sourceDir "floppy144_profile_edit.c"),
    (Join-Path $sourceDir "floppy144_profile_view.c"),
    (Join-Path $sourceDir "floppy144_player_visual.c"),
    (Join-Path $sourceDir "floppy144_settings.c"),
    (Join-Path $sourceDir "floppy144_settings_runtime.c"),
    (Join-Path $sourceDir "floppy144_settings_view.c"),
    (Join-Path $sourceDir "floppy144_timing.c"),
    (Join-Path $sourceDir "floppy144_credits_view.c"),
    (Join-Path $sourceDir "floppy144_completion_view.c")
)

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/wd4505",
    "/utf-8",
    $includePlatform,
    $includeGame
) + $sources + @(
    "/Festage4c_integration_smoke_tests.exe"
)

Push-Location $tempDir
try
{
    & cl.exe @compileArgs

    if($LASTEXITCODE -ne 0)
    {
        throw "Stage 4C integration smoke build failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    Write-Host "=== RUN STAGE 4C FRESH-PROFILE INTEGRATION SMOKE ==="

    & ".\stage4c_integration_smoke_tests.exe"

    if($LASTEXITCODE -ne 0)
    {
        throw "Stage 4C integration smoke failed with exit code $LASTEXITCODE."
    }
}
finally
{
    Pop-Location
}

Write-Host ""
Write-Host "STAGE 4C INTEGRATION GATE: PASS"
