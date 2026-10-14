$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=Split-Path -Parent $PSScriptRoot
$out=Join-Path $root 'build\audio-tests'
New-Item -ItemType Directory -Force $out | Out-Null
Push-Location $out
try {
    & cl.exe /nologo /TC /std:c11 /W4 /WX /utf-8 "/I$root\src" "/I$root\game\src" "$root\tools\bf23_progress_tests.c" "$root\src\f144audio.c" /Febf23.exe /link winmm.lib
    if($LASTEXITCODE -ne 0){throw 'BF23 native test build failed'}
    & .\bf23.exe
    if($LASTEXITCODE -ne 0){throw 'BF23 timing/tempo regression failed'}
    & cl.exe /nologo /TC /std:c11 /W4 /WX /utf-8 "/I$root\src" "$root\tools\bf24_sfx_tests.c" /Febf24.exe /link winmm.lib
    if($LASTEXITCODE -ne 0){throw 'BF24 native test build failed'}
    & .\bf24.exe
    if($LASTEXITCODE -ne 0){throw 'BF24 waveform regression failed'}
} finally {Pop-Location}
Write-Host 'BF23 / BF24 AUDIO REGRESSION: PASS (listening approval remains outstanding)'
