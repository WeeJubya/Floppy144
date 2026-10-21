# Only builds the isolated native display module and its test runner.
# Does not link Floppy144.exe or any River2D-derived runtime sources.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$compiler = Get-Command cl.exe -ErrorAction SilentlyContinue
if(-not $compiler) { throw 'MSVC x64 Developer environment is required.' }
$buildDir = Join-Path ([System.IO.Path]::GetTempPath()) 'floppy144-beta2-standalone-display'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$exe = Join-Path $buildDir 'beta2_win32_display_tests.exe'
$arguments = @(
    '/nologo', '/TC', '/std:c11', '/W4', '/WX', '/utf-8',
    ("/I" + (Join-Path $root 'include')),
    (Join-Path $root 'src\floppy144_win32_display.c'),
    (Join-Path $root 'tools\beta2_win32_display_tests.c'),
    ("/Fe:" + $exe),
    '/link', 'gdi32.lib', 'user32.lib'
)
& cl.exe @arguments
if($LASTEXITCODE -ne 0) { throw "Independent display compile failed: $LASTEXITCODE" }
& $exe
if($LASTEXITCODE -ne 0) { throw "Independent display tests failed: $LASTEXITCODE" }
Write-Host 'B2-03 INDEPENDENT DISPLAY HARNESS: PASS'
