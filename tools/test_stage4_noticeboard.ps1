$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$src = Join-Path $root "game\src"
$notice = Get-Content -LiteralPath (Join-Path $src "floppy144_noticeboard.c") -Raw
$cabinet = Get-Content -LiteralPath (Join-Path $src "floppy144_cabinet.c") -Raw
$main = Get-Content -LiteralPath (Join-Path $src "floppy144_main.c") -Raw
$canonical = Get-Content -LiteralPath (Join-Path $src "floppy144_game_data.generated.inc") -Raw
$regression = Get-Content -LiteralPath (Join-Path $PSScriptRoot "stage3b_cabinet_tests.c") -Raw

Write-Host "=== STAGE 4E DATE-AWARE NOTICEBOARD CONTRACT AUDIT ==="

foreach($required in @(
    'Floppy144GameDataAmbientForDate',
    'Floppy144VariationRange',
    'f144CalendarDateValid'
))
{
    if($notice -notmatch [regex]::Escape($required))
    {
        throw "Noticeboard does not use the existing canonical/variation/date service: $required"
    }
}
foreach($forbidden in @(
    '(?i)#\s*include\s*[<"]windows\.h[>"]',
    '\bGetLocalTime\s*\(',
    '\brand\s*\(',
    '\bsrand\s*\(',
    '\btime\s*\('
))
{
    if($notice -match $forbidden)
    {
        throw "Noticeboard introduced uncontrolled OS/random behaviour: $forbidden"
    }
}
foreach($required in @(
    'Floppy144CabinetSetNoticeboardDate',
    'f144PlatformCalendarDate',
    'Floppy144CabinetOpenParent'
))
{
    if($main -notmatch [regex]::Escape($required))
    {
        throw "Coordinator date/board integration missing: $required"
    }
}
if($cabinet -notmatch 'pItem == &pCabinet->sContextualFlyer' -or
   $cabinet -notmatch 'Floppy144CabinetVisibleContentCount' -or
   $cabinet -notmatch 'Floppy144DrawScrollbar')
{
    throw "Noticeboard contextual entry lost isolated inspect or scroll semantics."
}

$authoredIds = @('P-075','P-078','P-138','P-329','P-330','P-331','P-332','P-333','P-334')
foreach($id in $authoredIds)
{
    $escaped = [regex]::Escape($id)
    if($canonical -notmatch ('FLOPPY144_DATA_PHYSICAL_ITEM, "' + $escaped + '"'))
    {
        throw "Missing authored permanent noticeboard physical item: $id"
    }
}

for($index = 1; $index -le 8; ++$index)
{
    $ambientId = "AMB-NB-{0:D2}" -f $index
    if($canonical -notmatch [regex]::Escape('FLOPPY144_DATA_AMBIENT, "' + $ambientId + '"'))
    {
        throw "Missing canonical seasonal noticeboard flyer: $ambientId"
    }
}

foreach($expected in @(
    'Floppy144TestNoticeboardCalendar',
    '2026U, 1U, 15U',
    '2026U, 3U, 31U',
    '2026U, 7U, 15U',
    '2026U, 9U, 15U',
    '2026U, 10U, 31U',
    '2026U, 11U, 5U',
    '2026U, 12U, 20U',
    'memcmp(&run, &before, sizeof(run))'
))
{
    if($regression -notmatch [regex]::Escape($expected))
    {
        throw "Noticeboard date/gameplay regression case missing: $expected"
    }
}

Write-Host "STAGE 4E DATE-AWARE NOTICEBOARD CONTRACT AUDIT: PASS"
Write-Host "Full generated-data gameplay and actual Cabinet tests execute in Stage 3B.5."
