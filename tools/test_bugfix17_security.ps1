$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$data = Get-Content (Join-Path $root 'data/floppy144_game_data.json') -Raw | ConvertFrom-Json
$room = @($data.site_layout_source.rooms | Where-Object id -eq 'SECURITY')[0]
foreach ($id in @('SECURITY_DESK_01','SECURITY_DESK_02')) {
 $desk = @($data.furniture | Where-Object id -eq $id)[0]
 $source = @($room.geometry | Where-Object id -eq $id)[0]
 if ($desk.geometry.y -ne 57 -or $source.y -ne 57 -or $desk.geometry.width -ne 6 -or $desk.geometry.height -ne 4) { throw "Wrong desk geometry $id" }
}
$monitor = @($data.fixtures | Where-Object id -eq 'SECURITY_MONITOR_BANK')[0]
$source = @($room.geometry | Where-Object id -eq 'SECURITY_MONITOR_BANK')[0]
if ($monitor.geometry.x -ne 44 -or $monitor.geometry.y -ne 57 -or $monitor.geometry.width -ne 12 -or $monitor.geometry.height -ne 1 -or $source.y -ne 57) { throw 'Wrong monitor geometry' }
Write-Host 'BUG FIX 17: PASS'
