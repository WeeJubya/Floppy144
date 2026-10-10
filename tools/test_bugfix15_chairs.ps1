$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$data = Get-Content (Join-Path $root 'data/floppy144_game_data.json') -Raw | ConvertFrom-Json
$chairs = @($data.furniture | Where-Object { $_.id -match '_CHAIR_' })
$items = @($data.physical_items)
$occupied = @($items | Where-Object { $chairs.id -contains $_.parent_id })
$actual = @($occupied | Select-Object -ExpandProperty parent_id -Unique | Sort-Object)
$expected = @('DIRECTOR_OFFICE_CHAIR_02','DIRECTOR_OFFICE_CHAIR_06','DIRECTOR_OFFICE_CHAIR_11','FACILITIES_CHAIR_01','IT_SUPPORT_CHAIR_01','MAIN_OFFICE_CHAIR_01','MAIN_OFFICE_CHAIR_04','MAIN_OFFICE_CHAIR_06','RECEPTION_CHAIR_01','RECEPTION_CHAIR_04','RECEPTION_CHAIR_07','RECEPTION_CHAIR_13','RECEPTION_CHAIR_15','RECEPTION_CHAIR_18','RECORDS_OFFICE_CHAIR_01','RECORDS_OFFICE_CHAIR_03','SECRETARY_OFFICE_CHAIR_01','SECRETARY_OFFICE_CHAIR_03','SECURITY_CHAIR_02','SERVER_ROOM_CHAIR_01','STAFF_ROOM_CHAIR_01','STAFF_ROOM_CHAIR_04')
if ($chairs.Count -ne 56 -or $occupied.Count -ne 22 -or $actual.Count -ne 22) { throw 'Chair occupancy regression' }
if (Compare-Object $actual ($expected | Sort-Object)) { throw 'Chair occupancy not deterministic/authored' }
if ($items.Count -ne 600 -or $data.counts.physical_items -ne 600) { throw 'PI count regression' }
if (@($items.id | Select-Object -Unique).Count -ne $items.Count) { throw 'Duplicate PI ID' }
$parents = @($data.furniture.id) + @($data.fixtures.id) + @($data.connections.id)
foreach ($item in $items) { if ($item.parent_id -and $parents -notcontains $item.parent_id) { throw "Invalid PI parent $($item.id)" } }
$required = @($data.evidence | ForEach-Object { $_.physical_item_ids }) + @($data.interactions.physical_source)
foreach ($id in $required) { if ($id -and $items.id -notcontains $id) { throw "Missing required PI $id" } }
Write-Host 'BUG FIX 15: PASS; 22 of 56 chairs populated'
