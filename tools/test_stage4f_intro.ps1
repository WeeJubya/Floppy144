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

foreach($required in @(
    "FLOPPY144_INTRO_DISCOVERY",
    "FLOPPY144_INTRO_INSERTION",
    "FLOPPY144_INTRO_PROGRAM_START",
    "FLOPPY144_INTRO_GDR_CONNECTION",
    "FLOPPY144_INTRO_DURATION_MS",
    "UNIDENTIFIED REMOVABLE MEDIA",
    "MEDIA ENGAGED",
    "BOOTSTRAP FOUND",
    "INITIALISING...",
    "NETWORK INTERFACE ........ ONLINE",
    "GDR NETWORK .............. FOUND",
    "CONNECTING...",
    "REMOTE SESSION ........... ACCEPTED"
))
{
    if($intro -notmatch [regex]::Escape($required) -and $introHeader -notmatch [regex]::Escape($required))
    {
        throw "S4F-02 required intro beat/content is missing: $required"
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
    "Floppy144IntroDraw",
    "Floppy144IntroActionSkips",
    "Floppy144TimingStartSplash",
    "FLOPPY144_INTRO_DURATION_MS",
    "Floppy144OpenMainMenu"
))
{
    if($main -notmatch [regex]::Escape($required))
    {
        throw "Game coordinator intro wiring is missing: $required"
    }
}

if($main -notmatch 'global_screen\s*==\s*FLOPPY144_SCREEN_SPLASH[\s\S]*Floppy144IntroActionSkips')
{
    throw "Intro skip route is not wired through logical actions."
}

if($main -notmatch 'intro_elapsed_ms\s*>=\s*FLOPPY144_INTRO_DURATION_MS[\s\S]*Floppy144OpenMainMenu')
{
    throw "Normal intro completion does not transition to Session Control."
}

if($main -notmatch 'FLOPPY144_SCREEN_CREDITS:[\s\S]*F144_ACTION_CONFIRM[\s\S]*FLOPPY144_SCREEN_SPLASH[\s\S]*Floppy144TimingStartSplash')
{
    throw "Credits replay route is missing."
}

if($credits -notmatch [regex]::Escape("ENTER REPLAY INTRO"))
{
    throw "Credits does not advertise intro replay."
}

if($intro -notmatch [regex]::Escape("Floppy144SettingsTextElapsedMs"))
{
    throw "Intro does not respect text-speed settings."
}

if($main -notmatch [regex]::Escape("Floppy144SettingsApplyCrtFilter"))
{
    throw "Intro/main presentation no longer passes through the CRT filter."
}

if($audio -notmatch [regex]::Escape("intentionally silent"))
{
    throw "Expected Stage 4B silent-audio baseline changed; review S4F-02 audio policy."
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
