$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$main = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_main.c") -Raw
$recovery = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_recovery.c") -Raw
$credits = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_credits_view.c") -Raw
$completion = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_completion_view.c") -Raw
$audio = Get-Content -LiteralPath (Join-Path $root "src\f144_win32_audio.c") -Raw

Write-Host "=== S4F-03 WHOLE-GAME PRESENTATION JOURNEY AUDIT ==="

# Startup: one intro route, one canonical menu opener, CRT applied after drawing.
foreach($required in @(
    "Floppy144IntroDraw",
    "Floppy144IntroActionSkips",
    "Floppy144TimingStopSplash",
    "Floppy144OpenMainMenu",
    "Floppy144SettingsApplyCrtFilter"
))
{
    if($main -notmatch [regex]::Escape($required))
    {
        throw "Startup/presentation wiring is missing: $required"
    }
}

$introSkip = $main.IndexOf("Floppy144IntroActionSkips")
$genericMenu = $main.IndexOf("if(eAction == F144_ACTION_MENU)",$introSkip)
if($introSkip -lt 0 -or $genericMenu -le $introSkip)
{
    throw "Intro skip must be handled before the universal Session Control route."
}

# The universal MENU route is the sole screen-to-Session-Control policy.
$menuIfCount = [regex]::Matches($main,'eAction\s*==\s*F144_ACTION_MENU').Count
$menuCaseCount = [regex]::Matches($main,'case\s+F144_ACTION_MENU\s*:').Count
if($menuIfCount -ne 1 -or $menuCaseCount -ne 0)
{
    throw "Duplicate screen-specific MENU transition paths remain: if=$menuIfCount case=$menuCaseCount"
}

# Session Control remains compact. Credits stays beneath Settings because an
# eighth row would collide with the established status/divider geometry.
foreach($label in @(
    '"INITIATE NEW RECOVERY SESSION"',
    '"RETURN TO ACTIVE SITE"',
    '"RECORD CURRENT SESSION"',
    '"REINSTATE RECORDED SESSION"',
    '"OPERATOR PROFILE"',
    '"SETTINGS"',
    '"TERMINATE RECOVERY ENVIRONMENT"'
))
{
    if($recovery -notmatch [regex]::Escape($label))
    {
        throw "Session Control option missing: $label"
    }
}

if($recovery -match '"CREDITS"')
{
    throw "Credits unexpectedly moved onto the already-full Session Control option stack."
}

if(
    $recovery -notmatch '170U\s*\+\s*\r?\n\s*option_index\s*\*\s*12U' -or
    $recovery -notmatch '48U,\s*\r?\n\s*252U,\s*\r?\n\s*544U'
)
{
    throw "Session Control geometry changed; re-evaluate the seven-row hierarchy decision."
}

# Settings/Profile child routes deliberately restore their parent selection.
foreach($required in @(
    'global_main_menu_option =\s*\r?\n\s*FLOPPY144_MAIN_MENU_SETTINGS',
    'global_main_menu_option =\s*\r?\n\s*FLOPPY144_MAIN_MENU_OPERATOR_PROFILE'
))
{
    if($main -notmatch $required)
    {
        throw "Child-screen return selection contract is missing: $required"
    }
}

# Credits has two contexts. Settings-owned Credits offers Intro Replay;
# Completion-owned Credits does not interrupt the ending journey.
foreach($required in @(
    '"ENTER REPLAY INTRO   BACKSPACE BACK"',
    '"BACKSPACE BACK"',
    'Floppy144CreditsViewFooter'
))
{
    if($credits -notmatch [regex]::Escape($required))
    {
        throw "Credits footer policy is missing: $required"
    }
}

if(
    $main -notmatch '(?s)Floppy144CreditsViewDraw\s*\(\s*pSurface,\s*global_credits_return_screen\s*!=\s*FLOPPY144_SCREEN_COMPLETION' -or
    $main -notmatch '(?s)eAction\s*==\s*F144_ACTION_CONFIRM\s*&&\s*global_credits_return_screen\s*!=\s*FLOPPY144_SCREEN_COMPLETION.*?Floppy144TimingStartSplash'
)
{
    throw "Credits Intro Replay is not correctly limited to the non-Completion route."
}

if(
    $main -notmatch '(?s)global_settings_option\s*==\s*FLOPPY144_SETTINGS_OPTION_CREDITS.*?global_credits_return_screen\s*=\s*FLOPPY144_SCREEN_SETTINGS' -or
    $main -notmatch '(?s)FLOPPY144_COMPLETION_OPTION_CREDITS.*?global_credits_return_screen\s*=\s*FLOPPY144_SCREEN_COMPLETION'
)
{
    throw "Credits parent-screen ownership drifted."
}

# Completion must reset its own view on entry and use the canonical menu opener
# on exit, so a stale active-session menu selection cannot survive completion.
if($main -notmatch [regex]::Escape("Floppy144CompletionViewReset"))
{
    throw "Completion view is not reset on completion transition."
}

$completionMenuStart = $main.IndexOf("case FLOPPY144_COMPLETION_OPTION_MAIN_MENU:")
$completionMenuEnd = $main.IndexOf("case FLOPPY144_COMPLETION_OPTION_COUNT:",$completionMenuStart)
if($completionMenuStart -lt 0 -or $completionMenuEnd -le $completionMenuStart)
{
    throw "Could not isolate Completion -> Main Menu route."
}
$completionMenuRoute = $main.Substring($completionMenuStart,$completionMenuEnd-$completionMenuStart)

if($completionMenuRoute -notmatch [regex]::Escape("Floppy144OpenMainMenu"))
{
    throw "Completion bypasses the canonical Session Control opener."
}
if($completionMenuRoute -match 'global_screen\s*=\s*FLOPPY144_SCREEN_MAIN_MENU')
{
    throw "Completion still performs a direct Main Menu screen assignment."
}

# New/restored sessions reset transient browsing state before entering play.
$initStart = $main.IndexOf("case FLOPPY144_MAIN_MENU_INITIATE_SESSION:")
$restoreStart = $main.IndexOf("case FLOPPY144_MAIN_MENU_REINSTATE_SESSION:")
$profileStart = $main.IndexOf("case FLOPPY144_MAIN_MENU_OPERATOR_PROFILE:")
if($initStart -lt 0 -or $restoreStart -le $initStart -or $profileStart -le $restoreStart)
{
    throw "Could not isolate new/restore Session Control routes."
}
$newRoute = $main.Substring($initStart,$restoreStart-$initStart)
$restoreRoute = $main.Substring($restoreStart,$profileStart-$restoreStart)

foreach($route in @($newRoute,$restoreRoute))
{
    foreach($required in @(
        "Floppy144CatalogueReset",
        "Floppy144NotebookViewReset",
        "global_office_notice",
        "global_catalogue_direct_document"
    ))
    {
        if($route -notmatch [regex]::Escape($required))
        {
            throw "Session start/restore transient-state reset is missing: $required"
        }
    }
}

# Completion remains a three-choice end screen with an explicit Main Menu exit.
foreach($required in @(
    '"VIEW FINAL NOTE"',
    '"VIEW CREDITS"',
    '"RETURN TO MAIN MENU"',
    '"UP/DOWN SELECT   ENTER CONFIRM"'
))
{
    if($completion -notmatch [regex]::Escape($required))
    {
        throw "Completion journey presentation is missing: $required"
    }
}

# Audio continuity is intentionally silence-safe until the approved generated
# backend exists. No presentation screen should have grown direct native audio.
if($audio -notmatch [regex]::Escape("intentionally silent"))
{
    throw "Stage 4B silent-audio baseline changed; re-audit transition audio."
}
if($main -match '(?i)\bPlaySound\w*\b|\bwaveOut\w*\b|\bmidiOut\w*\b|\bmciSend\w*\b')
{
    throw "Coordinator bypasses the Stage 4B audio boundary."
}
foreach($required in @(
    "f144PlatformSetMusicVolume",
    "f144PlatformSetSfxVolume"
))
{
    if($main -notmatch [regex]::Escape($required))
    {
        throw "Persisted audio volume is no longer applied: $required"
    }
}

Write-Host "S4F-03 WHOLE-GAME PRESENTATION JOURNEY AUDIT: PASS"
Write-Host "menu=7-row-session-control credits=settings-owned-replay completion-owned-back-only"
Write-Host "completion=canonical-menu-reset audio=silent-platform-boundary state=transient-resets-verified"
