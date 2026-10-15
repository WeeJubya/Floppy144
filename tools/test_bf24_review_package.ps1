# BF24 review-package verification of existing comparison executable.
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$root=Split-Path -Parent $PSScriptRoot
$dir=Join-Path $root "build\audio-review"
$exe=Join-Path $dir "f144_audio_review.exe"
$csv=Join-Path $dir "BF24_original_A_B_waveform_metrics.csv"
$guide=Join-Path $dir "BF24_REVIEW_GUIDE.md"
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Existing audio review executable missing"}
if(-not(Test-Path -LiteralPath $csv -PathType Leaf)){throw "BF24 audio metrics missing"}
if(-not(Test-Path -LiteralPath $guide -PathType Leaf)){throw "BF24 listening guide missing"}

# The --list pathway never initialises WinMM and can run on headless CI.
$catalogue=@(& $exe --list)
if($LASTEXITCODE -ne 0){throw "Audio-review executable failed no-device startup"}
if($catalogue.Count -ne 26){throw "Expected 25 registered cues plus header; got $($catalogue.Count)"}
if($catalogue[0] -notmatch '25 EFFECTS.*ORIGINAL.*REVISED A.*REVISED B'){
    throw "Review UI does not advertise all three candidates"
}
for($cue=0;$cue -lt 25;$cue++){
    if($catalogue[$cue+1] -notmatch ('^\s*' + $cue + ':\s+\S')){
        throw "Cue registry gap/mismatch at $cue"
    }
}
$rows=@(Import-Csv -LiteralPath $csv)
if($rows.Count -ne 78){throw "BF24 metrics expected 78 waveform records; found $($rows.Count)"}
for($cue=0;$cue -lt 25;$cue++){
    foreach($variant in @('ORIGINAL','REVISED_A','REVISED_B')){
        $found=@($rows | Where-Object {
            [int]$_.effect_id -eq $cue -and $_.variant -eq $variant
        })
        $expected=if($cue -eq 14){2}else{1}
        if($found.Count -ne $expected){
            throw "Missing Original/A/B waveforms for cue $cue $variant"
        }
        foreach($row in $found){
            if([int]$row.samples -lt 1 -or [double]$row.duration_ms -le 0){
                throw "Zero-length waveform for cue $cue $variant"
            }
            if([double]$row.peak_dbfs -gt 0.0){
                throw "Clipped waveform for cue $cue $variant"
            }
        }
    }
}
# Verify current movement coupling still uses actual displacement, the shared
# 160 ms animation contact, and clears the pending foot when blocked.
$main=Get-Content -LiteralPath (Join-Path $root 'game\src\floppy144_main.c') -Raw
foreach($required in @(
    'Floppy144RunStateMovePlayerSite(',
    'before_x!=global_run_state.player_site_x',
    'global_footstep_pending=true',
    'global_footstep_pending=false',
    'Floppy144PlayerVisualAdvance(',
    'FLOPPY144_PLAYER_WALK_FRAME_MS',
    'FLOPPY144_AUDIO_FOOTSTEP'
)){
    if(-not $main.Contains($required)){
        throw "BF24 footstep must remain coupled to actual movement: $required"
    }
}
$src=Get-Content -LiteralPath (Join-Path $root 'src\f144sfx.c') -Raw
if($src -notmatch '#ifdef F144_SFX_REVIEW' -or
   $src -notmatch 'samples = generate\(v, effect\)'){
    throw "Unapproved BF24 replacements may have entered gameplay"
}
$music=Get-Content -LiteralPath (Join-Path $root 'src\f144audio.c') -Raw
foreach($marker in @('F144_MENU_BPM           110.0',
    'F144_MENU_BARS          32u',
    'F144_COMPLETION_BPM         F144_MENU_BPM',
    'F144_COMPLETION_BARS        32u',
    'F144_BPM           80.0')){
    if($music -notmatch [regex]::Escape($marker)){
        throw "Approved BF22 scene or gameplay tempo changed: $marker"
    }
}
Write-Host "BF24 REVIEW PACKAGE: PASS (25 cues x 3 + alternate feet, shipping ORIGINAL unchanged)"
Write-Host "NOTE: Headless smoke checks never claim subjective listening approval."
