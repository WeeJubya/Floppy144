# Only newly authored host/display/timing plus portable platform dispatch.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$folder = if($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { [System.IO.Path]::GetTempPath() }
$exe = Join-Path $folder 'beta2_win32_host_tests.exe'
$sources = @(
    (Join-Path $root 'tools\beta2_win32_host_tests.c'),
    (Join-Path $root 'src\floppy144_win32_host.c'),
    (Join-Path $root 'src\floppy144_win32_display.c'),
    (Join-Path $root 'src\f144_win32_timing.c'),
    (Join-Path $root 'src\f144_platform.c'),
    (Join-Path $root 'src\f144_calendar.c')
)
$compilerArgs = @('/nologo','/TC','/std:c11','/W4','/WX','/utf-8',('/I' + (Join-Path $root 'include')))
$compilerArgs += $sources
$compilerArgs += "/Fe:$exe"
$compilerArgs += '/link'
$compilerArgs += 'user32.lib'
$compilerArgs += 'gdi32.lib'
& cl.exe @compilerArgs
if($LASTEXITCODE -ne 0) { throw "B2-04 host compile failed: $LASTEXITCODE" }
& $exe
if($LASTEXITCODE -ne 0) { throw "B2-04 host tests failed: $LASTEXITCODE" }
