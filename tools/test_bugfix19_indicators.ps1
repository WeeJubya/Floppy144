$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=Split-Path -Parent $PSScriptRoot
$renderer=Get-Content (Join-Path $root 'game\src\floppy144_site_2d.c') -Raw
$site=Get-Content (Join-Path $root 'game\src\floppy144_site_object.c') -Raw
$main=Get-Content (Join-Path $root 'game\src\floppy144_main.c') -Raw
$settings=Get-Content (Join-Path $root 'game\src\floppy144_settings_view.c') -Raw
$persistence=Get-Content (Join-Path $root 'game\src\floppy144_persistence.c') -Raw
foreach($term in @('Floppy144SiteFocusedParentId','Floppy144SiteRectForParentId','Floppy144Site2DDiagonalAxes','Floppy144Site2DOutlineQuad','Floppy144SiteFurnitureInspectionIndicator','show_indicators','Floppy144SiteAvailableActions(state)!=0U','Floppy144CabinetAccessAvailable')){
    if($renderer -notmatch [regex]::Escape($term)){throw "BF19 presentation contract missing: $term"}
}
if($renderer -notmatch 'Floppy144Site2DPixel' -or
   $renderer -notmatch 'Floppy144Site2DInspectionOverlay'){
    throw 'BF19 missing clipped selection/inspection overlay'
}
if($site -notmatch 'Floppy144SitePhysicalItemVisible' -or
   $site -notmatch '"SECURE_CABINET"'){
    throw 'BF19 inspectable/secure-cabinet semantics missing'
}
if($main -notmatch 'Floppy144Site2DDrawForPlayerStateWithIndicators' -or
   $main -notmatch 'global_settings.show_inspection_indicators'){
    throw 'BF19 runtime setting not connected'
}
if($settings -notmatch '"SHOW INSPECTION INDICATORS"' -or
   $persistence -notmatch 'payload\[5\]'){
    throw 'BF19 settings UI/persistence missing'
}
Write-Host 'BF19 SELECTION GLOW / INSPECTION SYMBOL WIRING: PASS'
