$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=Split-Path -Parent $PSScriptRoot
$midi=Get-Content (Join-Path $root 'src\f144audio.c') -Raw
$native=Get-Content (Join-Path $root 'src\f144_win32_audio.c') -Raw
$coordinator=Get-Content (Join-Path $root 'game\src\floppy144_main.c') -Raw
$review=Get-Content (Join-Path $root 'tools\f144_audio_review.c') -Raw
foreach($name in @('F144_MUSIC_MENU','F144_MUSIC_COMPLETION','menu_scene_step',
'completion_scene_step','F144_MENU_BPM','110.0','F144_MENU_BARS',
'F144_MENU_TOTAL_STEPS','F144_COMPLETION_BPM',
'F144_COMPLETION_BARS','F144_COMPLETION_TOTAL_STEPS',
'completion_motif','scene_note','F144_MusicSetScene')){
    if($midi -notmatch [regex]::Escape($name)){throw "BF22 MIDI engine contract missing: $name"}
}
foreach($name in @('FLOPPY144_AUDIO_MUSIC_MENU',
'FLOPPY144_AUDIO_MUSIC_COMPLETION','FLOPPY144_SCREEN_MAIN_MENU',
'FLOPPY144_SCREEN_COMPLETION')){
    if($coordinator -notmatch [regex]::Escape($name)){throw "BF22 routing missing: $name"}
}
if($native -notmatch 'cue==5U' -or $native -notmatch 'F144_MUSIC_MENU' -or
   $native -notmatch 'F144_MUSIC_COMPLETION'){
    throw 'BF22 WinMM cannot address standalone compositions'
}
foreach($name in @('THE WHIMSICAL ASSEMBLY LINE','A COMPLETED PRODUCT HAS BEEN DELIVERED',
'F144_MUSIC_GAMEPLAY','F144_MUSIC_MENU','F144_MUSIC_COMPLETION')){
    if($review -notmatch [regex]::Escape($name)){
        throw "BF22 original/menu/ending review harness missing: $name"
    }
}
if($midi -match '(?i)\.wav|\.mp3|\.ogg|\.mid"') {
    throw 'BF22 used forbidden stored music asset'
}
# Exercise generated MIDI events without an actual sound card.
$tempDir = if($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { [IO.Path]::GetTempPath() }
$include = '/I' + (Join-Path $root 'src')
foreach($test in @('bf22_menu_midi_tests','bf22_completion_midi_tests')) {
    $exe = Join-Path $tempDir ($test + '.exe')
    & cl.exe /nologo /TC /std:c11 /W4 /WX /utf-8 $include `
        (Join-Path $root ('tools\' + $test + '.c')) "/Fe$exe" /link winmm.lib
    if($LASTEXITCODE -ne 0) { throw "BF22 native MIDI event test $test failed to compile" }
    & $exe
    if($LASTEXITCODE -ne 0) { throw "BF22 native MIDI event test $test failed" }
}
Write-Host 'BF22 MENU + COMPLETION GM / 110 BPM / 32-BAR LOOPS / ROUTING: PASS'
