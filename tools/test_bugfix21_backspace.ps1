$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=Split-Path -Parent $PSScriptRoot
$main=Get-Content (Join-Path $root 'game\src\floppy144_main.c') -Raw
$sfx=Get-Content (Join-Path $root 'src\f144sfx.c') -Raw
foreach($term in @('make_terminal_backspace','make_terminal_key',
    'F144_SFX_TERMINAL_BACKSPACE')){
    if($sfx -notmatch [regex]::Escape($term)){throw "BF21 procedural sound absent: $term"}
}
foreach($term in @('FLOPPY144_AUDIO_TERMINAL_BACKSPACE',
    'Floppy144ProfileNameEditBackspace',
    'Floppy144TerminalBackspace','global_intro_login_length')){
    if($main -notmatch [regex]::Escape($term)){throw "BF21 game input mismatch: $term"}
}
if($main -notmatch 'global_terminal\.input_length<before' -or
   $main -notmatch 'global_intro_login_length>0U'){
    throw 'BF21 may play Backspace when command/login buffer is empty'
}
if($sfx -match '(?i)\\.wav|\\.mp3|\\.ogg'){throw 'BF21 introduced asset dependency'}
$tmp=if($env:RUNNER_TEMP){Join-Path $env:RUNNER_TEMP 'bf21-test'}else{Join-Path ([IO.Path]::GetTempPath()) 'bf21-test'}
New-Item -ItemType Directory -Force -Path $tmp|Out-Null
$exe=Join-Path $tmp 'bf21_sfx_tests.exe'
& cl.exe /nologo /W4 /WX /TC /std:c11 /utf-8 (Join-Path $root 'tools\bf21_sfx_tests.c') "/I$(Join-Path $root 'src')" "/Fe$exe" winmm.lib
if($LASTEXITCODE -ne 0){throw 'BF21 native synthesis test build failed'}
& $exe
if($LASTEXITCODE -ne 0){throw 'BF21 native synthesis test failed'}
Write-Host 'BF21 BACKSPACE AUDIO / TEXT-ENTRY AUDIT: PASS'
