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
    "/DF144_SFX_REVIEW",
    ("/I" + (Join-Path $root "game\src")),
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
# The existing comparison executable remains the only interactive harness.
# A second, non-interactive diagnostic generates real waveform measurements
# from precisely the same original/A/B synthesis at a fixed gain and seed.
$metricExe=Join-Path $out "bf24_waveform_metrics.exe"
& cl.exe /nologo /TC /std:c11 /W4 /WX /O1 /MT /utf-8 `
    ("/I" + (Join-Path $root "src")) `
    (Join-Path $root "tools\bf24_review_metrics.c") `
    ("/Fe" + $metricExe) /link winmm.lib
if($LASTEXITCODE -ne 0) { throw "BF24 waveform metrics compiler failed." }
$metricsCsv=Join-Path $out "BF24_original_A_B_waveform_metrics.csv"
& $metricExe | Set-Content -LiteralPath $metricsCsv -Encoding utf8
if($LASTEXITCODE -ne 0) { throw "BF24 waveform metric export failed." }
Copy-Item -LiteralPath (Join-Path $root "docs\stage4\bf24_foley_review.md") `
    -Destination (Join-Path $out "BF24_REVIEW_GUIDE.md") -Force
Write-Host "Ready: $out\f144_audio_review.exe"
Write-Host "Registry: .\build\audio-review\f144_audio_review.exe --list"
Write-Host "Waveform evidence: $metricsCsv"
Write-Host "Review guide: $out\BF24_REVIEW_GUIDE.md"
Write-Host "Run the executable manually on Windows to hear ORIGINAL/A/B for all 16 active effects."
