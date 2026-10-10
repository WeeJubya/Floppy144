$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$testSource = Join-Path $PSScriptRoot "stage4_audio_tests.c"

Write-Host "=== STAGE 4 AUDIO BOUNDARY AUDIT ==="

$gameSources = Get-ChildItem -Path (Join-Path $root "game\src") -File |
    Where-Object { $_.Extension -eq ".c" -or $_.Extension -eq ".h" }

$forbiddenPatterns = @(
    '(?i)#\s*include\s*[<"]mmsystem\.h[>"]',
    '\bmidiOut[A-Za-z0-9_]*\b',
    '\bwaveOut[A-Za-z0-9_]*\b',
    '\bPlaySound[A-Za-z0-9_]*\b',
    '\bsndPlaySound[A-Za-z0-9_]*\b',
    '\bmciSend[A-Za-z0-9_]*\b',
    '\bHMIDIOUT\b',
    '\bHWAVEOUT\b'
)

foreach($file in $gameSources)
{
    $sourceText = Get-Content -LiteralPath $file.FullName -Raw

    foreach($pattern in $forbiddenPatterns)
    {
        if($sourceText -match $pattern)
        {
            throw "Game-facing audio leaked a Windows multimedia dependency into $($file.Name): $pattern"
        }
    }
}

$platformHeader = Get-Content -LiteralPath (Join-Path $root "include\f144_platform.h") -Raw
$mainSource = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_main.c") -Raw
$win32Audio = Get-Content -LiteralPath (Join-Path $root "src\f144_win32_audio.c") -Raw

foreach($required in @(
    'F144MusicCueId',
    'F144SfxCueId',
    'f144PlatformAudioInit',
    'f144PlatformAudioShutdown',
    'f144PlatformPlayMusic',
    'f144PlatformStopMusic',
    'f144PlatformPlaySfx',
    'f144PlatformSetMusicVolume',
    'f144PlatformSetSfxVolume'
))
{
    if($platformHeader -notmatch [regex]::Escape($required))
    {
        throw "Platform audio contract is missing: $required"
    }
}

foreach($required in @(
    'f144PlatformAudioInit',
    'f144PlatformSetMusicVolume',
    'global_settings.music_volume',
    'f144PlatformSetSfxVolume',
    'global_settings.sfx_volume',
    'f144PlatformAudioShutdown'
))
{
    if($mainSource -notmatch [regex]::Escape($required))
    {
        throw "Application audio lifecycle/settings wiring is missing: $required"
    }
}

foreach($forbidden in @(
    'midiOut',
    'waveOut',
    'PlaySound',
    'mciSend'
))
{
    if($mainSource -match [regex]::Escape($forbidden))
    {
        throw "Win32 multimedia call leaked into the game coordinator: $forbidden"
    }
}

foreach($required in @(
    'f144Win32AudioInit',
    'f144Win32AudioShutdown',
    'f144Win32AudioPlayMusic',
    'f144Win32AudioStopMusic',
    'f144Win32AudioPlaySfx',
    'f144Win32AudioSetMusicVolume',
    'f144Win32AudioSetSfxVolume'
))
{
    if($win32Audio -notmatch [regex]::Escape($required))
    {
        throw "Win32 audio adapter is missing required callback: $required"
    }
}

# Native WinMM is now explicitly permitted here, but never in Core.
foreach($required in @('F144_AudioInit','F144_SFXInit',
                       'F144_MusicSetRestoration','F144_SFXPlay'))
{
    if($win32Audio -notmatch [regex]::Escape($required))
    {
        throw "Win32 adapter is missing original audio call: $required"
    }
}
$midiSource = Get-Content -LiteralPath (Join-Path $root "src\f144audio.c") -Raw
$sfxSource = Get-Content -LiteralPath (Join-Path $root "src\f144sfx.c") -Raw
$facade = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_audio.c") -Raw
if($midiSource -notmatch 'F144_TOTAL_STEPS\s+9600u' -or
   $midiSource -notmatch 'g_distortion\s*\*\s*6u' -or
   $midiSource -notmatch 'bar\s*>=\s*580u')
{
    throw "30-minute MIDI duration, restoration corruption or final minute is missing."
}
if($sfxSource -notmatch 'make_telephone' -or
   $sfxSource -notmatch '640u' -or
   $sfxSource -notmatch '790u')
{
    throw "Approved v0.9 lower telephone chirrup is missing."
}
foreach($pattern in $forbiddenPatterns)
{
    if($facade -match $pattern)
    {
        throw "Game audio façade leaked native WinMM dependency: $pattern"
    }
}
foreach($required in @('midiOutOpen','midiOutShortMsg'))
{
    if($midiSource -notmatch [regex]::Escape($required))
    {
        throw "Original F144MIDI implementation is missing: $required"
    }
}
foreach($required in @('waveOutOpen','waveOutWrite','F144_SFX_FOOTSTEP',
                       'F144_SFX_COFFEE_MACHINE'))
{
    $combined = $sfxSource + (Get-Content -LiteralPath (Join-Path $root "src\f144sfx.h") -Raw)
    if($combined -notmatch [regex]::Escape($required))
    {
        throw "Original/extended F144SFX is missing: $required"
    }
}
Write-Host "STAGE 4 AUDIO BOUNDARY AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4 AUDIO CONTRACT REGRESSION ==="

$tempDir = if($env:RUNNER_TEMP) {
    Join-Path $env:RUNNER_TEMP "floppy144-stage4-audio"
} else {
    Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-audio"
}

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exePath = Join-Path $tempDir "stage4_audio_tests.exe"
$includePlatform = "/I" + (Join-Path $root "include")
$includeGame = "/I" + (Join-Path $root "game\src")

$compileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/utf-8",
    $testSource,
    (Join-Path $root "src\f144_platform.c"),
    (Join-Path $root "src\f144_win32_audio.c"),
    (Join-Path $root "src\f144audio.c"),
    (Join-Path $root "src\f144sfx.c"),
    (Join-Path $root "game\src\floppy144_audio.c"),
    (Join-Path $root "game\src\floppy144_settings.c"),
    $includePlatform,
    $includeGame,
    "/I" + (Join-Path $root "src"),
    "/Fe$exePath",
    "/link",
    "winmm.lib"
)

& cl.exe @compileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 audio contract regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4 AUDIO CONTRACT REGRESSION ==="

& $exePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4 audio contract regression failed with exit code $LASTEXITCODE."
}
