$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$main = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_main.c") -Raw
$settingsH = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_settings_view.h") -Raw
$credits = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_credits_view.c") -Raw
$provenance = Get-Content -LiteralPath (Join-Path $root "docs\stage4\runtime_provenance.md") -Raw
$license = Get-Content -LiteralPath (Join-Path $root "LICENSE") -Raw

Write-Host "=== STAGE 4C CREDITS / PROVENANCE AUDIT ==="
foreach($required in @(
 'FLOPPY144_SCREEN_CREDITS',
 'FLOPPY144_SETTINGS_OPTION_CREDITS',
 'Floppy144CreditsViewDraw'
)){
 if($main -notmatch [regex]::Escape($required) -and $settingsH -notmatch [regex]::Escape($required)){
   throw "Credits navigation wiring missing: $required"
 }
}
if($main -notmatch 'FLOPPY144_SCREEN_CREDITS:[\s\S]*?F144_ACTION_BACK[\s\S]*?FLOPPY144_SCREEN_SETTINGS'){
 throw "Credits Backspace route to Settings is missing."
}
foreach($required in @(
 'River2D-derived GPLv3 runtime material',
 'Copyright (C) 2026 BadAcronym',
 'preserve River2D attribution'
)){
 if($provenance -notmatch [regex]::Escape($required)){throw "Provenance evidence missing: $required"}
}
foreach($required in @('GNU GENERAL PUBLIC LICENSE','Version 3, 29 June 2007','Copyright (C) 2026 BadAcronym')){
 if($license -notmatch [regex]::Escape($required)){throw "Root LICENSE evidence missing: $required"}
}
foreach($required in @(
 'GREY DOOR REPUBLIC','WEEJUBYA','RIVER2D-DERIVED',
 'BADACRONYM','GNU GENERAL PUBLIC LICENSE VERSION 3',
 'LICENSE','NO WARRANTY'
)){
 if($credits -notmatch [regex]::Escape($required)){throw "Credits text missing: $required"}
}
if($credits -match 'COPYRIGHT \(C\) 2026 GREY DOOR REPUBLIC|COPYRIGHT \(C\) 2026 WEEJUBYA'){
 throw "Credits invented a whole-game copyright claim not established by provenance."
}
Write-Host "STAGE 4C CREDITS / PROVENANCE AUDIT: PASS"

$tempDir = if($env:RUNNER_TEMP){Join-Path $env:RUNNER_TEMP "floppy144-stage4-credits"}else{Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-credits"}
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exe=Join-Path $tempDir "stage4_credits_tests.exe"
$args=@(
 "/nologo","/TC","/std:c11","/W4","/WX","/utf-8",
 (Join-Path $PSScriptRoot "stage4_credits_tests.c"),
 (Join-Path $root "game\src\floppy144_credits_view.c"),
 (Join-Path $root "game\src\floppy144_draw.c"),
 ("/I"+(Join-Path $root "include")),
 ("/I"+(Join-Path $root "game\src")),
 ("/Fe"+$exe)
)
& cl.exe @args
if($LASTEXITCODE -ne 0){throw "Stage 4C credits regression build failed."}
& $exe
if($LASTEXITCODE -ne 0){throw "Stage 4C credits regression failed."}
