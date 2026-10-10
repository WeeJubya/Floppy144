$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Build only the standalone listening harness. This is not linked into the game.
$root = Split-Path -Parent $PSScriptRoot
$out = Join-Path $root "build\audio-review"
New-Item -ItemType Directory -Force -Path $out | Out-Null

$argsList = @(
    "/nologo",
    "/TC",
    "/O1",
    "/MT",
    "/utf-8",
    ("/I" + (Join-Path $root "src")),
    (Join-Path $root "tools\f144_audio_review.c"),
    (Join-Path $root "src\f144audio.c"),
    (Join-Path $root "src\f144sfx.c"),
    ("/Fe" + (Join-Path $out "f144_audio_review.exe")),
    "/link",
    "winmm.lib",
    "user32.lib"
)
& cl.exe @argsList
if($LASTEXITCODE -ne 0) { throw "Audio review harness failed to build." }
Write-Host "Ready: $out\f144_audio_review.exe"
Write-Host "Run the executable manually to audition all 22 SFX and 30-minute sections."
