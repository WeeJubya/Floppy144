$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$main = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_main.c") -Raw
$recovery = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_recovery.c") -Raw
$view = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_settings_view.c") -Raw
$runtime = Get-Content -LiteralPath (Join-Path $root "game\src\floppy144_settings_runtime.c") -Raw

Write-Host "=== STAGE 4C SETTINGS BOUNDARY AUDIT ==="
foreach($required in @(
 'FLOPPY144_SCREEN_SETTINGS','FLOPPY144_MAIN_MENU_SETTINGS','Floppy144SettingsViewDraw',
 'Floppy144SettingsApplyCrtFilter','Floppy144SettingsTextElapsedMs',
 'f144PlatformSetMusicVolume','f144PlatformSetSfxVolume',
 'Floppy144TimingSetAutosaveInterval','Floppy144PersistenceSaveSettings'
)){
 if($main -notmatch [regex]::Escape($required)){throw "Settings wiring missing: $required"}
}
if($recovery -notmatch '"SETTINGS"'){throw "Settings menu label missing."}
foreach($required in @('"CRT MODE"','"TEXT SPEED"','"MUSIC VOLUME"','"SFX VOLUME"','"AUTOSAVE INTERVAL"')){
 if($view -notmatch [regex]::Escape($required)){throw "Settings field missing: $required"}
}
if($runtime -match 'windows\.h|midiOut|waveOut|PlaySound|mciSend'){throw "Portable settings runtime leaked native dependency."}
if($main -match 'midiOut|waveOut|PlaySound|mciSend'){throw "Settings bypassed platform audio abstraction."}
Write-Host "STAGE 4C SETTINGS BOUNDARY AUDIT: PASS"

$tempDir = if($env:RUNNER_TEMP){Join-Path $env:RUNNER_TEMP "floppy144-stage4-settings"}else{Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4-settings"}
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$exe = Join-Path $tempDir "stage4_settings_tests.exe"
$args = @(
 "/nologo","/TC","/std:c11","/W4","/WX","/utf-8",
 (Join-Path $PSScriptRoot "stage4_settings_tests.c"),
 (Join-Path $root "game\src\floppy144_settings.c"),
 (Join-Path $root "game\src\floppy144_settings_runtime.c"),
 (Join-Path $root "game\src\floppy144_settings_view.c"),
 (Join-Path $root "game\src\floppy144_timing.c"),
 (Join-Path $root "game\src\floppy144_draw.c"),
 ("/I"+(Join-Path $root "include")),
 ("/I"+(Join-Path $root "game\src")),
 ("/Fe"+$exe)
)
& cl.exe @args
if($LASTEXITCODE -ne 0){throw "Stage 4C settings regression build failed."}
& $exe
if($LASTEXITCODE -ne 0){throw "Stage 4C settings regression failed."}
