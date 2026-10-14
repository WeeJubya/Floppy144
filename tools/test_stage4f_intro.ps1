$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$introPath = Join-Path $root "game\src\floppy144_intro.c"
$introHeaderPath = Join-Path $root "game\src\floppy144_intro.h"
$mainPath = Join-Path $root "game\src\floppy144_main.c"
$creditsPath = Join-Path $root "game\src\floppy144_credits_view.c"
$audioPath = Join-Path $root "src\f144_win32_audio.c"

Write-Host "=== S4F-02 INTRO SOURCE / ARCHITECTURE AUDIT ==="

$intro = Get-Content -LiteralPath $introPath -Raw
$introHeader = Get-Content -LiteralPath $introHeaderPath -Raw
$main = Get-Content -LiteralPath $mainPath -Raw
$credits = Get-Content -LiteralPath $creditsPath -Raw
$audio = Get-Content -LiteralPath $audioPath -Raw

# BF20 latest four-screen storyboard supersedes S4F's earlier two-screen material.
foreach($required in @(
    "FLOPPY144_INTRO_DISCOVERY",
    "FLOPPY144_INTRO_INSERTION",
    "FLOPPY144_INTRO_PROGRAM_START",
    "FLOPPY144_INTRO_GDR_CONNECTION",
    "FLOPPY144_INTRO_DURATION_MS",
    "SITE RECOVERY DISK",
    "DISK 144/233",
    "BOOTSTRAP",
    "MEDIA HEADER",
    "RECOVERY EXECUTABLE",
    "INITIALISING",
    "SYSTEM READY",
    "NETWORK INTERFACE",
    "SEARCHING",
    "REMOTE SESSION ACCEPTED",
    "LOGIN:"
))
{
    if($intro -notmatch [regex]::Escape($required) -and
       $introHeader -notmatch [regex]::Escape($required))
    {
        throw "BF20 storyboard beat/content missing: $required"
    }
}

foreach($forbidden in @(
    "Floppy144RunState",
    "Floppy144DiscoveryProfile",
    "Floppy144Persistence",
    "f144PlatformPersistence",
    "windows.h",
    "GetTickCount",
    "SetTimer",
    "PlaySound",
    "waveOut",
    "mciSend"
))
{
    if($intro -match [regex]::Escape($forbidden) -or $introHeader -match [regex]::Escape($forbidden))
    {
        throw "Intro presentation crossed a forbidden state/platform boundary: $forbidden"
    }
}

foreach($required in @(
    "Floppy144IntroDrawForProfile",
    "Floppy144IntroLoginReady",
    "Floppy144CommitIntroLogin",
    "Floppy144TimingStartSplash",
    "FLOPPY144_INTRO_DURATION_MS",
    "Floppy144OpenMainMenu",
    "Floppy144PersistenceSaveProfile"
))
{
    if($main -notmatch [regex]::Escape($required))
    {
        throw "BF20 coordinator integration missing: $required"
    }
}
if($main -match 'intro_elapsed_ms\s*>=\s*FLOPPY144_INTRO_DURATION_MS[\s\S]{0,150}Floppy144OpenMainMenu')
{
    throw "BF20 unguarded menu transition bypasses first-time LOGIN"
}
if($intro -notmatch 'return false;' -or
   $intro -notmatch 'Floppy144IntroActionSkips')
{
    throw "BF20 essential scenes must not be bypassed"
}

if($main -notmatch 'FLOPPY144_SCREEN_CREDITS:[\s\S]*F144_ACTION_CONFIRM[\s\S]*FLOPPY144_SCREEN_SPLASH[\s\S]*Floppy144TimingStartSplash')
{
    throw "Credits replay route is missing."
}

if($credits -notmatch [regex]::Escape("ENTER REPLAY INTRO"))
{
    throw "Credits does not advertise intro replay."
}

if($main -notmatch [regex]::Escape("Floppy144SettingsTextElapsedMs"))
{
    throw "Coordinator does not translate persisted text speed for the intro."
}

if($main -notmatch [regex]::Escape("Floppy144SettingsApplyCrtFilter"))
{
    throw "Intro/main presentation no longer passes through the CRT filter."
}

# The WinMM F144MIDI/F144SFX boundary replaced Stage 4B's silent stub.
# Intro remains presentation-only, and native audio belongs to the platform.
foreach($required in @('F144_AudioInit','F144_SFXInit','F144_MusicSetAct','F144_SFXPlay')) {
    if($audio -notmatch [regex]::Escape($required)) {
        throw "F144 audio platform integration missing: $required"
    }
}

if($intro -match "f144PlatformPlaySfx|f144PlatformPlayMusic")
{
    throw "Intro dispatches semantic audio despite the current no-cue silent backend."
}

Write-Host "S4F-02 INTRO SOURCE / ARCHITECTURE AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD S4F-02 INTRO REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4f-intro"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4f-intro"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4f_intro_tests.exe"

$args = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    (Join-Path $PSScriptRoot "stage4f_intro_tests.c"),
    $introPath,
    (Join-Path $root "game\src\floppy144_draw.c"),
    (Join-Path $root "game\src\floppy144_settings.c"),
    (Join-Path $root "game\src\floppy144_settings_runtime.c"),
    (Join-Path $root "game\src\floppy144_timing.c"),
    ("/I" + (Join-Path $root "include")),
    ("/I" + (Join-Path $root "game\src")),
    ("/Fe" + $exePath)
)

& cl.exe @args

if($LASTEXITCODE -ne 0)
{
    throw "S4F-02 intro regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN S4F-02 INTRO REGRESSION ==="

& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "S4F-02 intro regression failed with exit code $LASTEXITCODE."
}

Write-Host "S4F-02 INTRO REGRESSION: PASS"
