$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=Split-Path -Parent $PSScriptRoot
$midi=Get-Content (Join-Path $root 'src\f144audio.c') -Raw
$native=Get-Content (Join-Path $root 'src\f144_win32_audio.c') -Raw
$coordinator=Get-Content (Join-Path $root 'game\src\floppy144_main.c') -Raw
$review=Get-Content (Join-Path $root 'tools\f144_audio_review.c') -Raw
foreach($name in @('F144_MUSIC_MENU','F144_MUSIC_COMPLETION','menu_scene_step',
'completion_scene_step','70.0','65.0','scene_note','F144_MusicSetScene')){
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
foreach($name in @('PLEASE HOLD','YOUR CONTRIBUTION HAS BEEN NOTED',
'F144_MUSIC_GAMEPLAY','F144_MUSIC_MENU','F144_MUSIC_COMPLETION')){
    if($review -notmatch [regex]::Escape($name)){
        throw "BF22 original/menu/ending review harness missing: $name"
    }
}
if($midi -match '(?i)\.wav|\.mp3|\.ogg|\.mid"') {
    throw 'BF22 used forbidden stored music asset'
}
Write-Host 'BF22 TWO PROCEDURAL TRACKS / ROUTING / REVIEW HARNESS: PASS'
