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

$win32ForbiddenPatterns = @(
    '(?i)#\s*include\s*[<"]mmsystem\.h[>"]',
    '\bmidiOut[A-Za-z0-9_]*\s*\(',
    '\bwaveOut[A-Za-z0-9_]*\s*\(',
    '\bPlaySound[A-Za-z0-9_]*\s*\(',
    '\bsndPlaySound[A-Za-z0-9_]*\s*\(',
    '\bmciSend[A-Za-z0-9_]*\s*\(',
    '\bHMIDIOUT\b\s+[A-Za-z_]',
    '\bHWAVEOUT\b\s+[A-Za-z_]'
)

foreach($pattern in $win32ForbiddenPatterns)
{
    if($win32Audio -match $pattern)
    {
        throw "S4B-04 unexpectedly introduced a Windows multimedia implementation before the generated-audio feature is approved: $pattern"
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
    (Join-Path $root "game\src\floppy144_settings.c"),
    $includePlatform,
    $includeGame,
    "/Fe$exePath"
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
