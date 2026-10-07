param()

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$SourceDir = Join-Path $Root "game\src"
$IncludeDir = Join-Path $Root "include"

Write-Host "=== STAGE 4D OFFICE CAMERA / ZOOM AUDIT ==="

$Site2DPath = Join-Path $SourceDir "floppy144_site_2d.c"
$CameraPath = Join-Path $SourceDir "floppy144_site_2d_camera.c"
$CameraHeaderPath = Join-Path $SourceDir "floppy144_site_2d_camera.h"
$SiteHeaderPath = Join-Path $SourceDir "floppy144_site.h"
$LayoutPath = Join-Path $SourceDir "floppy144_site_generated.def"

$Site2DSource = Get-Content -LiteralPath $Site2DPath -Raw
$CameraSource = Get-Content -LiteralPath $CameraPath -Raw
$CameraHeader = Get-Content -LiteralPath $CameraHeaderPath -Raw
$SiteHeader = Get-Content -LiteralPath $SiteHeaderPath -Raw
$LayoutSource = Get-Content -LiteralPath $LayoutPath -Raw

if($CameraHeader -notmatch '#define\s+FLOPPY144_SITE_2D_PIXELS_PER_UNIT\s+12\b') {
    throw "S4D-02 selected Site scale is not 12 px per Site unit."
}

foreach($Invariant in @(
    '#define FLOPPY144_SITE_FIXED_ONE                16',
    '#define FLOPPY144_SITE_PLAYER_VISUAL_WIDTH_X16       56',
    '#define FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16    32',
    '#define FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16    32',
    '#define FLOPPY144_SITE_MOVE_STEP_X16              8'
)) {
    if($SiteHeader -notmatch [regex]::Escape($Invariant)) {
        throw "Canonical Site gameplay invariant changed or disappeared: $Invariant"
    }
}

foreach($Required in @(
    'FLOPPY144_SITE_2D_VIEWPORT_WIDTH  576',
    'FLOPPY144_SITE_2D_VIEWPORT_HEIGHT 252',
    'FLOPPY144_SITE_2D_PIXELS_PER_UNIT 12'
)) {
    if($CameraHeader -notmatch [regex]::Escape($Required)) {
        throw "Camera configuration evidence missing: $Required"
    }
}

foreach($Required in @(
    'Floppy144Site2DBuildCameraAtScale',
    'camera->pixels_per_unit',
    'camera->visible_width16',
    'camera->visible_height16',
    'Floppy144Site2DProjectPoint'
)) {
    if($CameraSource -notmatch [regex]::Escape($Required)) {
        throw "Camera transform implementation evidence missing: $Required"
    }
}

if($Site2DSource -notmatch [regex]::Escape('#include "floppy144_site_2d_camera.h"')) {
    throw "Site renderer is not consuming the shared Stage 4D camera transform."
}

$ExpectedRooms = @(
    'RECEPTION',
    'CORRIDOR',
    'MAIN_OFFICE',
    'FACILITIES',
    'RECORDS_OFFICE',
    'IT_SUPPORT',
    'STAFF_ROOM',
    'SECRETARY_OFFICE',
    'DIRECTOR_OFFICE',
    'SECURITY',
    'SERVER_ROOM'
)

foreach($Room in $ExpectedRooms) {
    if($LayoutSource -notmatch ('SITE_ROOM_DEF\(FLOPPY144_ROOM_' + [regex]::Escape($Room) + ',')) {
        throw "Expected Site room is absent from generated camera data: $Room"
    }
}

Write-Host "WORLD/GEOMETRY INVARIANT AUDIT: PASS"
Write-Host ""
Write-Host "=== BUILD STAGE 4D OFFICE CAMERA REGRESSION ==="

if(-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw "MSVC compiler is not configured for the Stage 4D office-camera regression."
}

$TempDir =
    if($env:RUNNER_TEMP) {
        Join-Path $env:RUNNER_TEMP "floppy144-stage4d-office-camera"
    }
    else {
        Join-Path ([System.IO.Path]::GetTempPath()) "floppy144-stage4d-office-camera"
    }

New-Item -ItemType Directory -Force -Path $TempDir | Out-Null

$ExePath = Join-Path $TempDir "stage4_office_camera_tests.exe"

$Sources = @(
    (Join-Path $ScriptDir "stage4_office_camera_tests.c"),
    $CameraPath,
    (Join-Path $SourceDir "floppy144_site_rooms.c"),
    (Join-Path $SourceDir "floppy144_site_view.c"),
    (Join-Path $SourceDir "floppy144_site.c")
)

$Args = @(
    "/nologo",
    "/TC",
    "/std:c11",
    "/W4",
    "/WX",
    "/O1",
    "/utf-8",
    ("/I" + $SourceDir),
    ("/I" + $IncludeDir)
)

$Args += $Sources
$Args += ("/Fe:" + $ExePath)

Push-Location $TempDir
try {
    & cl.exe @Args

    if($LASTEXITCODE -ne 0) {
        throw "Stage 4D office-camera regression build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

Write-Host ""
Write-Host "=== RUN STAGE 4D OFFICE CAMERA REGRESSION ==="

$Output = @(
    & $ExePath 2>&1
)

$ExitCode = $LASTEXITCODE

foreach($Line in $Output) {
    Write-Host $Line
}

if($ExitCode -ne 0) {
    $Failures = @(
        $Output |
            Where-Object { $_ -match '^FAIL:' } |
            ForEach-Object { $_.ToString().Trim() }
    )

    if($Failures.Count -gt 0) {
        throw (
            "Stage 4D office-camera regression failed. Assertions: " +
            ($Failures -join " | ")
        )
    }

    throw "Stage 4D office-camera regression failed with exit code $ExitCode."
}

Write-Host "STAGE 4D OFFICE CAMERA REGRESSION: PASS"
