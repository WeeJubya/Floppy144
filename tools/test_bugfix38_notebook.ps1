param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$src = Join-Path $root 'game\src'
$main = Get-Content -LiteralPath (Join-Path $src 'floppy144_main.c') -Raw
foreach($required in @(
    'Floppy144NotebookViewOpen(&global_notebook)',
    'Floppy144NotebookViewSwitchPage(',
    'eAction==F144_ACTION_MOVE_RIGHT?1:-1'
)) {
    if(-not $main.Contains($required)) { throw "BF38 input routing missing: $required" }
}
$view = Get-Content -LiteralPath (Join-Path $src 'floppy144_notebook_view.c') -Raw
if($view -notmatch 'document_note_count' -or $view -notmatch 'notebook_order_count') {
    throw 'BF38 view must use the existing two persistent Notebook histories.'
}
if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'BF38 Notebook regression requires configured MSVC C compiler.'
}
$dir = if($env:RUNNER_TEMP) { Join-Path $env:RUNNER_TEMP 'floppy144-bf38' } else {
    Join-Path ([System.IO.Path]::GetTempPath()) 'floppy144-bf38'
}
New-Item -ItemType Directory -Force -Path $dir | Out-Null
$exe = Join-Path $dir 'bf38_notebook_tests.exe'
$arguments = @(
    '/nologo','/TC','/std:c11','/W4','/WX','/O1','/MT','/utf-8',
    ('/I' + $src),('/I' + (Join-Path $root 'include')),
    (Join-Path $PSScriptRoot 'bf38_notebook_tests.c'),
    (Join-Path $src 'floppy144_notebook_view.c'),('/Fe:' + $exe)
)
Push-Location $dir
try {
    & cl.exe @arguments
    if($LASTEXITCODE -ne 0) { throw "BF38 compilation failed: $LASTEXITCODE" }
}
finally { Pop-Location }
& $exe
if($LASTEXITCODE -ne 0) { throw "BF38 headless regression failed: $LASTEXITCODE" }
Write-Host 'BF38 NOTEBOOK PAGINATION REGRESSION: PASS'
