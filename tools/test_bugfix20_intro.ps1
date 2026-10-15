$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=Split-Path -Parent $PSScriptRoot
$main=Get-Content (Join-Path $root 'game\src\floppy144_main.c') -Raw
$intro=Get-Content (Join-Path $root 'game\src\floppy144_intro.c') -Raw
$header=Get-Content (Join-Path $root 'game\src\floppy144_intro.h') -Raw
$profile=Get-Content (Join-Path $root 'game\src\floppy144_profile.c') -Raw
$need=@('Floppy144CommitIntroLogin','Floppy144IntroDrawForProfile',
'Floppy144IntroLoginReady','Floppy144IntroAutoLoginReady','Floppy144DiscoveryProfileHasOperatorName','Floppy144DiscoveryProfileSetOperatorName',
'Floppy144DiscoveryProfileOperatorNameValid',
'Floppy144DiscoveryProfileOperatorNameCharacterSupported',
'Floppy144PersistenceSaveProfile','FLOPPY144_AUDIO_FLOPPY_INSERT')
foreach($term in $need){
 if($main -notmatch [regex]::Escape($term)){throw "BF20 missing login/audio integration: $term"}
}
foreach($term in @('SITE RECOVERY DISK','DISK 144/233',
'BOOTSTRAP','MEDIA HEADER','RECOVERY EXECUTABLE','INITIALISING',
'SYSTEM READY','NETWORK INTERFACE','SEARCHING',
'REMOTE SESSION ESTABLISHED','LOGIN:', 'GREY DOOR REPUBLIK', 'PRESENTS...', 'PRESS ENTER FOR GUEST ACCESS')){
 if($intro -notmatch [regex]::Escape($term)){throw "BF20 missing storyboard content: $term"}
}
foreach($term in @('FLOPPY144_INTRO_DISCOVERY_END_MS',
'FLOPPY144_INTRO_INSERTION_END_MS','FLOPPY144_INTRO_PROGRAM_END_MS',
'FLOPPY144_INTRO_DURATION_MS','FLOPPY144_INTRO_CRT_START_MS')){
 if($header -notmatch [regex]::Escape($term)){throw "BF20 hardcoded timing outside central contract: $term"}
}
if($main -notmatch "Floppy144TimingStopSplash" -or
   $main -notmatch 'global_intro_login_length' -or
   $profile -notmatch 'FLOPPY144_PROFILE_NAME_MAX_LENGTH') {
 throw "BF20 profile/terminal lifecycle missing"
}
if($main -match 'intro_elapsed_ms\s*>=\s*FLOPPY144_INTRO_DURATION_MS[\s\S]{0,200}Floppy144OpenMainMenu'){
 throw "BF20 can leave before LOGIN"
}
Write-Host 'BF20 FOUR-SCREEN STORYBOARD / LOGIN CONTRACT: PASS'
