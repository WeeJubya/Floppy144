param()

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$SourceDir = Join-Path $Root "game\src"
$IncludeDir = Join-Path $Root "include"

Write-Host "=== STAGE 4D INSPECTION 2.5D AUDIT ==="

$Cabinet = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_cabinet.c") -Raw
$Renderer = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_cabinet_25d.c") -Raw
$SiteObject = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_site_object.c") -Raw
$RunState = Get-Content -LiteralPath (Join-Path $SourceDir "floppy144_run_state.c") -Raw

foreach($Required in @(
    'Floppy144CabinetEnhancedPresentationUnlocked',
    'FLOPPY144_ROOM_MAIN_OFFICE',
    'Floppy144Cabinet25DDraw',
    'Floppy144SiteRectForParentId'
))
{
    if(
        $Cabinet -notmatch [regex]::Escape($Required) -and
        $SiteObject -notmatch [regex]::Escape($Required)
    )
    {
        throw "Inspection presentation wiring is missing: $Required"
    }
}

foreach($Required in @(
    'Floppy144Cabinet25DFillQuad',
    'Floppy144Cabinet25DDrawDeskFamily',
    'Floppy144Cabinet25DDrawStorageFamily',
    'Floppy144Cabinet25DDrawTableFamily',
    'Floppy144Cabinet25DDrawChair',
    'Floppy144Cabinet25DDrawTrolley',
    'Floppy144Cabinet25DDrawWallFixture',
    'Floppy144Cabinet25DDrawDoor',
    'Floppy144Cabinet25DDrawFallback'
))
{
    if($Renderer -notmatch [regex]::Escape($Required))
    {
        throw "Reusable 2.5D renderer family is missing: $Required"
    }
}


# BUG FIX 10: 2.5D item markers must be projected onto a parent surface.
# The pre-reconstruction schematic may remain 2D in floppy144_cabinet.c,
# but the enhanced renderer must never reintroduce screen-space child grids.
foreach($Required in @(
    'Floppy144Cabinet25DChildSurface',
    'Floppy144Cabinet25DFaceCorners',
    'Floppy144Cabinet25DOnFace',
    'Floppy144Cabinet25DDrawSurfaceMarker',
    'Floppy144Cabinet25DDrawSurfaceChildren',
    'F144_25D_CHILD_SHELF'
))
{
    if($Renderer -notmatch [regex]::Escape($Required))
    {
        throw "Inspectable child surface projection is missing: $Required"
    }
}
if($Renderer.Contains('Floppy144Cabinet25DDrawMarkers('))
{
    throw "Legacy screen-space child marker renderer returned to 2.5D Inspection."
}

foreach($Forbidden in @(
    '.bmp',
    '.png',
    '.jpg',
    '.jpeg',
    '.gif'
))
{
    if($Renderer.ToLowerInvariant().Contains($Forbidden))
    {
        throw "Inspection renderer unexpectedly references external bitmap assets."
    }
}

foreach($GameplayToken in @(
    'FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16',
    'FLOPPY144_SITE_MOVE_STEP_X16',
    'FLOPPY144_SITE_DATA_INTERACTION_RANGE_X16'
))
{
    if($Renderer -match [regex]::Escape($GameplayToken))
    {
        throw "Inspection renderer leaked into world-space gameplay policy: $GameplayToken"
    }
}

if(
    $RunState -match 'cabinet_25d' -or
    $RunState -match 'EnhancedPresentation'
)
{
    throw "2.5D Inspection presentation leaked into persistent RunState."
}

Write-Host "STAGE 4D INSPECTION 2.5D AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4D INSPECTION 2.5D REGRESSION ==="

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue))
{
    throw "MSVC compiler is not configured for the Stage 4D Inspection regression."
}

$BuildDir =
    if($env:RUNNER_TEMP)
    {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4d-inspection25d"
    }
    else
    {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4d-inspection25d"
    }

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
$ExePath = Join-Path $BuildDir "stage4_inspection_25d_tests.exe"

$CompileArgs = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/O1",
    "/MT",
    "/utf-8",
    (Join-Path $ScriptDir "stage4_inspection_25d_tests.c"),
    (Join-Path $SourceDir "floppy144_cabinet_25d.c"),
    (Join-Path $SourceDir "floppy144_draw.c"),
    ("/I" + $IncludeDir),
    ("/I" + $SourceDir),
    ("/Fe:" + $ExePath)
)

& cl.exe @CompileArgs

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4D Inspection 2.5D regression build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "=== RUN STAGE 4D INSPECTION 2.5D REGRESSION ==="

& $ExePath

if($LASTEXITCODE -ne 0)
{
    throw "Stage 4D Inspection 2.5D regression failed with exit code $LASTEXITCODE."
}

Write-Host "STAGE 4D INSPECTION 2.5D REGRESSION: PASS"
